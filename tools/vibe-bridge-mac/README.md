# ⚡ CYD Vibe Bridge Hub (macOS App & .dmg)

A native macOS companion application to run and monitor local bridge daemons for your **Cheap Yellow Display (CYD)** AI monitors:
- 🔵 **Project 47**: Antigravity Monitor (Port `5047`)
- 🟠 **Project 48**: Claude Code Monitor (Port `5048`)
- 🟢 **Project 49**: OpenAI Codex Monitor (Port `5049`)

---

## 🚀 Features

- **No Terminal Required**: Launch all 3 bridges with a single click.
- **Auto IP Detection**: Automatically finds your Mac's LAN IP address (`192.168.x.x`) and provides a **"📋 Copy IP"** button to easily paste into `BRIDGE_HOST` in `main.cpp`.
- **Individual or Batch Controls**: Start and stop Antigravity, Claude, or Codex bridges individually or all together.
- **Live Activity Console**: See incoming HTTP requests and turn tokens from your CYD displays in real time.
- **100% Offline & Private**: Reads local turn logs directly from `~/.gemini/antigravity/`, `~/.claude/projects/`, and `~/.codex/sessions/`. No cloud credentials on the device.

---

## 📦 Installation via DMG

1. Download or open `VibeBridge-v1.0.dmg`.
2. Drag **VibeBridge.app** into your **Applications** folder.
3. Open **VibeBridge** from Applications or Spotlight.
4. Click **"🚀 Start All Bridges"**.

---

## 🛠️ Building from Source

To rebuild the `.app` bundle and `.dmg` installer:

```bash
cd tools/vibe-bridge-mac
./build_dmg.sh
```

The output will be placed in `dist/VibeBridge-v1.0.dmg`.

---

## 📺 Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

MIT © Cheap Yellow Display Community

