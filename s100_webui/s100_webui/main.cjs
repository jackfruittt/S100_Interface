const { app, BrowserWindow } = require("electron");
const path = require("path");

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: true,
     // preload: path.join(__dirname, "preload.js"), //  preload.js if neeed
    },
  });

  // Load React app's index.html file
  mainWindow.loadFile(path.join(__dirname, 'dist', 'index.html'));

  // Open DevTools 
  if (process.env.NODE_ENV === 'development') {
    mainWindow.webContents.openDevTools();
  }

  mainWindow.on("closed", () => {
    mainWindow = null;
  });
}

// Wait for the Electron app to be ready
app.whenReady().then(() => {
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

// Quit the app when all windows are closed (except on macOS)
app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});


