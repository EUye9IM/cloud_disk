import { app, BrowserWindow, dialog, ipcMain, session } from "electron";
import path from "path";
import { join } from "path";

export const ROOT_PATH = {
  // /dist
  dist: join(__dirname, "../.."),
  // /dist or /public
  public: join(__dirname, app.isPackaged ? "../.." : "../../../public"),
};

app.whenReady().then(() => {
  const win = new BrowserWindow({
    width: 1024,
    height: 768,
  });
  session.defaultSession.on("will-download", (event, item, webContents) => {
    console.log(item);
    item.pause();

    console.log(item.canResume());
    item.resume();
  });

  const openFileDialog = async (oldPath: string = app.getPath("downloads")) => {
    if (!win) return oldPath;

    const { canceled, filePaths } = await dialog.showOpenDialog(win, {
      title: "选择保存位置",
      properties: ["openDirectory", "createDirectory"],
      defaultPath: oldPath,
    });

    return !canceled ? filePaths[0] : oldPath;
  };

  ipcMain.handle("openFileDialog", (event, oldPath?) =>
    openFileDialog(oldPath)
  );

  const indexHtml = join(ROOT_PATH.dist, "index.html");

  if (app.isPackaged) {
    win.loadFile("dist/index.html");
  } else {
    win.loadURL("http://localhost:3000");
  }
});
