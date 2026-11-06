# ⚡ CYD Vibe Bridge Hub (Windows .exe & Zip)

A native Windows companion application to run and monitor local bridge daemons for your **Cheap Yellow Display (CYD)** AI monitors:
- 🔵 **Project 47**: Antigravity Monitor (Port `5047`)
- 🟠 **Project 48**: Claude Code Monitor (Port `5048`)
- 🟢 **Project 49**: OpenAI Codex Monitor (Port `5049`)

---

## 🚀 Features

- **Double-Click EXE (`VibeBridge.exe`)**: No command prompt required — native Windows GUI launcher.
- **Smart Python Detection**: Searches for `py.exe`, `pythonw.exe`, `python.exe`, or standard `%LOCALAPPDATA%` paths.
- **Auto Dependency Handling**: Automatically detects and installs `flask` if missing.
- **Fallback Batch Launcher (`VibeBridge.bat`)**: Included for systems without executable permissions.
- **Auto LAN IP Detection**: Finds your Windows machine's IP (`192.168.x.x`) with one-click **"📋 Copy IP"**.
- **100% Offline & Private**: Reads local turn logs directly from `%USERPROFILE%\.gemini\antigravity\`, `%USERPROFILE%\.claude\projects\`, and `%USERPROFILE%\.codex\sessions\`.

---

## 📦 How to Use on Windows

1. Extract `VibeBridge-Windows-v1.0.zip` (or place `VibeBridge.exe` and `app.py` in a folder).
2. Double-click **`VibeBridge.exe`** (or `VibeBridge.bat`).
3. Click **"📋 Copy IP"** in the window.
4. Paste that IP into `BRIDGE_HOST` in your CYD's `src/main.cpp`.
5. Click **"🚀 Start All Bridges"** — your CYD display connects and renders live turn stats!

---

## 🛠️ Building from Source

To cross-compile `VibeBridge.exe` from macOS or Linux:

```bash
brew install mingw-w64   # On macOS
cd tools/vibe-bridge-windows
./build_exe.sh
```

---

## 📺 Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

MIT © Cheap Yellow Display Community
