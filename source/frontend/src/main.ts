import { createApp } from "vue";
import App from "./App.vue";
import ElementPlus from "element-plus";
import "element-plus/dist/index.css";
import router from "./router";
import { key, store } from "./store";

const app = createApp(App);

app.use(router);
app.use(ElementPlus);
app.use(store, key);

app.mount("#app");
