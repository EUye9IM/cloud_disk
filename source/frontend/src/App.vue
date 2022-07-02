<script setup lang="ts">
// This starter template is using Vue 3 <script setup> SFCs
// Check out https://vuejs.org/api/sfc-script-setup.html#script-setup
import axios from "axios";
import { onMounted } from "vue";
import { server_url } from "./config";
import router from "./router";
import { store } from "./store";

const logout = () => {
  const token = localStorage.getItem("token");
  if (token) {
    axios
      .post(
        server_url + "/api/logout",
        {
          user: localStorage.getItem("user"),
        },
        {
          headers: {
            Authorization: token,
          },
        }
      )
      .then((response) => {
        if (response.data.ret === 0) {
          localStorage.removeItem("token");
          localStorage.removeItem("user");
          store.commit("checkLogin");
        }
        router.push("/login");
      });
  } else {
    console.log("未登陆！");
  }
};

onMounted(() => store.commit("checkLogin"));
</script>

<template>
  <div id="app">
    <p>
      <!--使用 router-link 组件进行导航 -->
      <!--通过传递 `to` 来指定链接 -->
      <!--`<router-link>` 将呈现一个带有正确 `href` 属性的 `<a>` 标签-->
      <el-button @click="$router.push('/home')">主页</el-button>
      <el-button v-if="store.state.login" @click="logout()">注销</el-button>
      <el-button v-else @click="$router.push('/register')"> 注册 </el-button>
      <el-button v-else @click="$router.push('/login')">登录</el-button>
    </p>
    <!-- 路由出口 -->
    <!-- 路由匹配到的组件将渲染在这里 -->
    <router-view />
  </div>
</template>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}

#nav {
  margin-top: 60px;
  padding: 20px;
}
#nav a {
  font-weight: bold;
  color: #2c3e50;
}
#nav a.router-link-exact-active {
  color: #42b983;
}
</style>
