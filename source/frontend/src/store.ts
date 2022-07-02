import { InjectionKey } from "vue";
import { createStore, Store, useStore as baseUseStore } from "vuex";

// 为 store state 声明类型
export interface State {
  login: boolean
}

// 定义 injection key
export const key: InjectionKey<Store<State>> = Symbol();

export const store = createStore<State>({
  state: {
    login: false
  },
  mutations: {
    checkLogin(state) {
      state.login = localStorage.getItem("token") !== null;
    }
  },
});

export function useStore() {
  return baseUseStore(key);
}
