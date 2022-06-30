import { createRouter, createWebHashHistory } from "vue-router";
import { RouteRecordRaw } from "vue-router";
import LoginVue from "./components/Login.vue";
import RegisterVue from "./components/Register.vue";
import HomeVue from "./components/Home.vue";
import { store } from "./store";
import { ElMessage } from "element-plus";

const routes: Array<RouteRecordRaw> = [
  { path: "/login", name: "Login", component: LoginVue },
  { path: "/register", name: "Register", component: RegisterVue },
  { path: "/home", name: "Home", component: HomeVue },
];

const router = createRouter({
  // 4. 内部提供了 history 模式的实现。为了简单起见，我们在这里使用 hash 模式。
  history: createWebHashHistory(),
  routes: routes, // `routes: routes` 的缩写
});

router.beforeEach(async (to, from) => {
  if (
    to.name !== "Login" &&
    to.name !== "Register" &&
    !store.state.login
  ) {
    ElMessage({ message: "请登录", type: "warning", showClose: true });
    return { name: "Login" };
  }
  return true;
});

export default router;
