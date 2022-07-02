<template>
  <el-form
    :model="ruleForm"
    status-icon
    ref="ruleFormRef"
    label-width="120px"
    class="demo-ruleForm"
  >
    <el-form-item label="用户名" prop="username">
      <el-input v-model="ruleForm.username" maxlength="16" />
    </el-form-item>
    <el-form-item label="密码" prop="password">
      <el-input
        v-model="ruleForm.password"
        type="password"
        maxlength="20"
        autocomplete="off"
      />
    </el-form-item>
    <el-form-item>
      <el-button type="success" @click="submitForm()">登录</el-button>
      <el-button type="primary" @click="$router.push('register')"
        >注册</el-button
      >
    </el-form-item>
  </el-form>
</template>

<script lang="ts" setup>
import { reactive, ref } from "vue";
import { useStore } from "../store";
import { ElMessage, FormInstance } from "element-plus";
import axios from "axios";
import { server_url } from "../config";
import router from "../router";

const ruleFormRef = ref<FormInstance>();
const ruleForm = reactive({
  username: "",
  password: "",
});

const store = useStore();

const submitForm = () => {
  axios
    .post(
      server_url + "/api/login",
      {
        user: ruleForm.username,
        password: ruleForm.password,
      }
    )
    .then((response) => {
      if (response.data.ret === 0) {
        ElMessage({
          message: "登录成功！",
          type: "success",
        });

        console.log(response.headers)
        console.log(response.data)
        localStorage.setItem("token", response.data.token);
        localStorage.setItem("user", ruleForm.username);
        store.commit("checkLogin");
        router.push("/home");
      } else {
        ElMessage({
          message: response.data.msg,
          type: "error",
        });
      }
    })
    .catch((error) =>
      ElMessage({ message: error.status + " " + error.message, type: "error" })
    );
};
</script>

<style scoped></style>
