import { defineConfig } from "vite";
import fs from "fs"
import vue from "@vitejs/plugin-vue";
import electron from "vite-plugin-electron";

fs.rmSync('dist', { recursive: true, force: true }) // v14.14.0

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    electron({
      main: {
        entry: 'electron/main.ts',
      },
      vite: {
        build: {
          sourcemap: false,
          outDir: 'dist/electron',
        },
      },
      renderer : {}
    }),
  ],
});