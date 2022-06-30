import { app, BrowserWindow } from "electron";

app.whenReady().then(() => {
  const window = new BrowserWindow();
  window.setMenu(null);
  window.loadURL("http://localhost:3000");
});
