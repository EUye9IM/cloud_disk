<template>
  <el-row>
    <el-col :span="8">
      <span>当前工作目录：{{ cwd }}/</span>
    </el-col>
    <el-col :span="16">
      <el-button @click="newFolder()">新建文件夹</el-button>
      <el-button @click="refresh()">刷新</el-button>
    </el-col>
  </el-row>
  <el-row>
    <el-col :span="24">
      <el-tree
        :data="dataSource"
        node-key="id"
        default-expand-all
        @current-change="changeCwd"
        :expand-on-click-node="false"
        :allow-drop="allowDrop"
        draggable
        @node-drag-start="handleDragStart"
        @node-drag-end="handleDragEnd"
      >
        <template #default="{ node, data }">
          <span class="custom-tree-node">
            <span> {{ data.name + (data.type === "folder" ? "/" : "") }} </span>
            <span> 大小：{{ data.size }} </span>
            <span> 最后修改时间：{{ data.time }} </span>
            <el-popconfirm
              confirm-button-text="确定"
              cancel-button-text="取消"
              title="确认删除？"
              @confirm="remove(node)"
            >
              <template #reference>
                <el-button link type="primary">删除</el-button>
              </template>
            </el-popconfirm>
            <span>
              <el-button link type="primary" @click="rename(node)"
                >重命名</el-button
              >
            </span>
            <span>
              <el-button link type="primary" @click="download(node)"
                >下载</el-button
              >
            </span>
          </span>
        </template>
      </el-tree>
    </el-col>
  </el-row>
  <el-row>
    <ul :span="24">
      <li v-for="file in uploadFiles" :key="file.id">
        <span>{{ file.name }}</span> - <span>{{ file.size }}</span> -
        <span v-if="file.error">{{ file.error }}</span>
        <span v-else>{{ file.path }}</span>
        <el-progress
          :percentage="file.progress"
          :color="file.progress === 100 ? '#67C23A' : '#409EFF'"
        ></el-progress>
      </li>
    </ul>
  </el-row>
  <el-row>
    <span :span="24">
      <el-button type="primary" class="file"
        >上传文件
        <input type="file" name="" id="" @change="uploadFile" />
      </el-button>
      <el-button type="primary" class="file"
        >上传文件夹
        <input
          type="file"
          webkitdirectory
          multiple
          name=""
          id=""
          @change="uploadFolder"
        />
      </el-button>

      <el-button v-if="!isPaused" @click="switchPause()">暂停</el-button>
      <el-button v-else @click="switchPause()">继续</el-button>
    </span>
  </el-row>
</template>

<script setup lang="ts">
import md5 from "md5";
import { onMounted, ref } from "vue";
import axios from "axios";
import { server_url } from "../config";
import { ElMessageBox } from "element-plus";
import type Node from "element-plus/es/components/tree/src/model/node";
import { store } from "../store";
import { ElMessage } from "element-plus";
import router from "../router";
import { DropType } from "element-plus/es/components/tree/src/tree.type";
import type { DragEvents } from "element-plus/es/components/tree/src/model/useDragNode";
import { UploadAjaxError } from "element-plus/lib/components/upload/src/ajax";
import streamSaver from "streamsaver";

import { pause, resume } from "pause-until";

interface Tree {
  id: number;
  name: string;
  time: string;
  size: number;
  children?: Tree[];
}

interface UploadFile {
  id: number;
  name: string;
  path: string;
  size: number;
  error: string | null;
  active: boolean;
  progress: number;
}

interface UploadFileOptions {
  onProgress: (percent: number) => void;
  onError: (error: string) => void;
  path: string;
  file: File;
}

const uploadFiles = ref<UploadFile[]>([]);

const dataSource = ref<Tree[]>();

const uploadFile = async (event: any) => {
  const files: FileList = event.target.files;

  for (let index = 0; index < files.length; index++) {
    const file = files.item(index)!;
    const uploadFile: UploadFile = {
      id: index,
      name: file.name,
      path: cwd.value + "/" + file.webkitRelativePath,
      size: file.size,
      error: null,
      active: true,
      progress: 0,
    };
    uploadFiles.value.push(uploadFile);
    console.log(file.name);
    upload({
      file: file,
      path: cwd.value + "/" + file.name,
      onError: function (error: string): void {
        uploadFile.error = error;
      },
      onProgress: function (percent: number): void {
        uploadFile.progress = percent;
        uploadFiles.value = [...uploadFiles.value];
      },
    });
  }
};

const uploadFolder = async (event: any) => {
  const files: FileList = event.target.files;

  const set = new Set<string>();

  for (let index = 0; index < files.length; index++) {
    const file = files.item(index)!;
    const folderPath = file.webkitRelativePath.substring(
      0,
      file.webkitRelativePath.lastIndexOf("/")
    );

    set.add(folderPath);
  }

  set.forEach(async (element) => {
    await axios.post(
      server_url + "/api/file/newfolder",
      {
        cwd: cwd.value,
        foldername: element,
      },
      {
        headers: {
          Authorization: localStorage.getItem("token")!,
        },
      }
    );
  });

  for (let index = 0; index < files.length; index++) {
    const file = files.item(index)!;
    const uploadFile: UploadFile = {
      id: index,
      name: file.name,
      path: cwd.value + "/" + file.webkitRelativePath,
      size: file.size,
      error: null,
      active: true,
      progress: 0,
    };
    uploadFiles.value.push(uploadFile);

    upload({
      file: file,
      path: uploadFile.path,
      onError: function (error: string): void {
        uploadFile.error = error;
      },
      onProgress: function (percent: number): void {
        uploadFile.progress = percent;
        uploadFiles.value = [...uploadFiles.value];
      },
    });
  }
};

const download = async (node: Node) => {
  console.log(node.data.type);
  if (node.data.type === "folder") {
    node.childNodes.forEach(async (element) => {
      await download(element);
    });
  } else {
    axios
      .post(
        server_url + "/api/predownload",
        {
          path: getNodePath(node),
        },
        {
          headers: {
            Authorization: localStorage.getItem("token")!,
          },
        }
      )
      .then(async (response) => {
        if (response.data.ret === 0) {
          const filestream = streamSaver.createWriteStream(node.data.name, {
            size: node.data.size,
          });
          /*
        fetch(server_url + response.data.url, {
          method: "GET",
          headers: {
            Authorization: localStorage.getItem("token")!,
          },
        }).then((response) => {
          console.log(node.data.name);
          filestream.getWriter().write();
          return response.body!.pipeTo(filestream);
        });*/
          const len = node.data.size;
          const slicesize = 1024 * 1024;
          const cnt = Math.ceil(len / slicesize);
          console.log(len, slicesize, cnt);
          if (cnt > 0) {
            const writer = filestream.getWriter();

            for (let i = 1; i <= cnt; i++) {
              const res = await fetch(
                server_url +
                  "/api/download?md5=" +
                  response.data.hash +
                  "&offset=" +
                  (i - 1) * slicesize +
                  "&length=" +
                  (i === cnt ? len - (i - 1) * slicesize : slicesize),
                {
                  method: "POST",
                }
              );
              const blob = await res.blob();
              const arrayBuffer = await blob.arrayBuffer();
              await writer.write(new Uint8Array(arrayBuffer));
            }
            await writer.close();
          }
          await filestream.close();
        } else {
          ElMessage({ message: response.data.msg, type: "error" });
        }
      })
      .catch(reportError);
  }
};

function getNodePath(node: Node): string {
  if (node === null || node.level === 0) {
    return "";
  }
  return getNodePath(node.parent) + "/" + node.data.name;
}

const changeCwd = (_: Tree, node: Node) => {
  if (node.data.type !== "folder" && node.level > 0) {
    node = node.parent;
  }
  const path = getNodePath(node);
  cwd.value = path;
};

const newFolder = () => {
  ElMessageBox.prompt("请输入文件夹名", "新建文件夹", {
    confirmButtonText: "确定",
    cancelButtonText: "取消",
    inputPattern: /^[^/\s][^/]*$/,
    inputErrorMessage: "文件夹名非法",
  }).then(({ value }) => {
    axios
      .post(
        server_url + "/api/file/newfolder",
        {
          cwd: cwd.value,
          foldername: value,
        },
        {
          headers: {
            Authorization: localStorage.getItem("token")!,
          },
        }
      )
      .then((response) => {
        if (response.data.ret === 0) {
          ElMessage({ message: "创建文件夹成功", type: "success" });
          refresh();
        } else if (response.data.ret === -2) {
          ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
          router.push("/login");
        } else {
          ElMessage({ message: response.data.msg, type: "error" });
        }
      })
      .catch(reportError);
  });
};

const remove = (node: Node) => {
  const parent = node.parent;
  changeCwd(parent.data as Tree, parent);

  axios
    .post(
      server_url + "/api/file/delete",
      {
        paths: [getNodePath(node)],
      },
      {
        headers: {
          Authorization: localStorage.getItem("token")!,
        },
      }
    )
    .then((response) => {
      if (response.data.ret === 0) {
        ElMessage({ message: "删除成功", type: "success" });
        refresh();
      } else if (response.data.ret === -2) {
        ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
        router.push("/login");
      } else {
        ElMessage({ message: response.data.msg, type: "error" });
      }
    })
    .catch(reportError);
};

const rename = (node: Node) => {
  ElMessageBox.prompt("请输入新文件名", "重命名", {
    confirmButtonText: "确定",
    cancelButtonText: "取消",
    inputPattern: /^[^/\s][^/]*$/,
    inputErrorMessage: "文件名非法",
  }).then(({ value }) => {
    const path = getNodePath(node);
    axios
      .post(
        server_url + "/api/file/rename",
        {
          cwd: path.substring(0, path.lastIndexOf("/")),
          oldname: node.data.name,
          newname: value,
        },
        {
          headers: {
            Authorization: localStorage.getItem("token")!,
          },
        }
      )
      .then((response) => {
        if (response.data.ret === 0) {
          ElMessage({ message: "重命名", type: "success" });
          refresh();
        } else if (response.data.ret === -2) {
          ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
          router.push("/login");
        } else {
          ElMessage({ message: response.data.msg, type: "error" });
        }
      })
      .catch(reportError);
  });
};

const isPaused = ref(false);

const switchPause = () => {
  if (isPaused.value === true) {
    isPaused.value = false;
    resume("resume");
  } else {
    isPaused.value = true;
  }
};

const upload = async (options: UploadFileOptions) => {
  const buffer = await options.file.arrayBuffer();
  const file_md5 = md5(new Uint8Array(buffer));

  options.onProgress(0);

  const upload_fileslice = async (num: number, slicesize: number) => {
    if (num === 0) {
      ElMessage({ message: "上传成功！", type: "success" });
      refresh();
      options.onProgress(100);
      return;
    }

    console.log(isPaused.value);
    if (isPaused.value === true) {
      await pause("resume");
    }

    const cnt = Math.ceil(buffer.byteLength / slicesize);
    const slice = buffer.slice((num - 1) * slicesize, num * slicesize);
    axios
      .post(server_url + "/api/file/upload", slice, {
        params: {
          md5: file_md5,
          num: num,
        },
        headers: {
          Authorization: localStorage.getItem("token")!,
          "Content-Type": "application/octet-stream",
        },
      })
      .then((response) => {
        console.log(response.data);
        if (response.data.ret === 0) {
          options.onProgress((response.data.count * 100.0) / cnt);
          upload_fileslice(response.data.next, slicesize);
        } else if (response.data.ret === -2) {
          ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
          router.push("/login");
        } else {
          ElMessage({ message: response.data.msg, type: "error" });
        }
      });
  };
  return new Promise(() => {
    axios
      .post(
        server_url + "/api/file/preupload",
        {
          path: options.path,
          size: buffer.byteLength,
          md5: file_md5,
        },
        {
          headers: {
            Authorization: localStorage.getItem("token")!,
          },
        }
      )
      .then((response) => {
        if (response.data.ret === 0) {
          ElMessage({ message: "秒传成功！", type: "success" });
          options.onProgress(100);
          refresh();
        } else if (response.data.ret > 0) {
          upload_fileslice(response.data.ret, response.data.slicesize);
        } else if (response.data.ret === -2) {
          ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
          router.push("/login");
        } else {
          ElMessage({ message: response.data.msg, type: "error" });
          options.onError(response.data.msg);
        }
      })
      .catch(reportError);
  });
};

const refresh = () => {
  axios
    .post(
      server_url + "/api/file/list",
      {
        path: "/",
      },
      {
        headers: {
          Authorization: localStorage.getItem("token")!,
        },
      }
    )
    .then((response) => {
      const ret = response.data.ret;
      if (ret === 0) {
        dataSource.value = response.data.files;
      } else if (ret === -2) {
        ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
        router.push("/login");
      } else {
        ElMessage({ message: "加载失败" + response.data.msg, type: "warning" });
      }
    })
    .catch(reportError);
};

const allowDrop = (
  draggingNode: Node,
  dropNode: Node,
  type: "prev" | "inner" | "next"
) => {
  if (type === "inner" && dropNode.data.type !== "folder") {
    return false;
  }
  return true;
};

const move = (from: string, to: string) => {
  console.log(from);
  axios
    .post(
      server_url + "/api/file/move",
      {
        oldcwd: from.substring(0, from.lastIndexOf("/")),
        newcwd: to,
        files: [from.substring(from.lastIndexOf("/") + 1)],
      },
      {
        headers: {
          Authorization: localStorage.getItem("token")!,
        },
      }
    )
    .then((response) => {
      const ret = response.data.ret;
      if (ret === 0) {
        //这里就不发消息了吧
      } else if (ret === -2) {
        ElMessage({ message: "登录已失效，请重新登录", type: "warning" });
        router.push("/login");
      } else {
        ElMessage({ message: "移动失败" + response.data.msg, type: "warning" });
        refresh();
      }
    })
    .catch(reportError);
};

let draggingNodePath: string;

const handleDragStart = (node: Node, ev: DragEvents) => {
  draggingNodePath = getNodePath(node);
};

const handleDragEnd = (
  draggingNode: Node,
  dropNode: Node,
  dropType: DropType,
  ev: DragEvents
) => {
  const target = dropType === "inner" ? dropNode : dropNode.parent;
  move(draggingNodePath, getNodePath(target));
};

const cwd = ref("");

onMounted(refresh);
</script>

<style scoped>
a {
  color: #42b983;
}

name {
  margin: 0 0.5em;
  font-weight: bold;
}

code {
  background-color: #eee;
  padding: 2px 4px;
  border-radius: 4px;
  color: #304455;
}

.file {
  position: relative;
  display: inline-block;
  overflow: hidden;
  text-decoration: none;
  text-indent: 0;
}
.file input {
  position: absolute;
  font-size: 100px;
  right: 0;
  top: 0;
  opacity: 0;
}

.el-row {
  margin-bottom: 20px;
}

.el-row:last-child {
  margin-bottom: 0;
}
</style>
