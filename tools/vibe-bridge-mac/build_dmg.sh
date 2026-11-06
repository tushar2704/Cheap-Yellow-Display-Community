#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DIST_DIR="$DIR/dist"
APP_NAME="VibeBridge"
APP_BUNDLE="$DIST_DIR/$APP_NAME.app"
DMG_PATH="$DIST_DIR/VibeBridge-v1.0.dmg"

echo "=========================================="
echo " Building $APP_NAME.app & .dmg"
echo "=========================================="

rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"

# 1. Info.plist
cat << 'EOF' > "$APP_BUNDLE/Contents/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>VibeBridge</string>
    <key>CFBundleDisplayName</key>
    <string>Vibe Bridge Hub</string>
    <key>CFBundleIdentifier</key>
    <string>com.cyd.vibebridge</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleExecutable</key>
    <string>VibeBridge</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.13</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

# 2. Executable launcher
cat << 'EOF' > "$APP_BUNDLE/Contents/MacOS/VibeBridge"
#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
RESOURCES="$DIR/../Resources"

# Include user site-packages across common Python 3 versions
for v in 3.9 3.10 3.11 3.12 3.13; do
    if [ -d "$HOME/Library/Python/$v/lib/python/site-packages" ]; then
        export PYTHONPATH="$HOME/Library/Python/$v/lib/python/site-packages:$PYTHONPATH"
    fi
done

PYTHON_BIN="$(which python3 || echo '/usr/bin/python3')"

exec "$PYTHON_BIN" "$RESOURCES/app.py"
EOF

chmod +x "$APP_BUNDLE/Contents/MacOS/VibeBridge"

# 3. Copy app resources and icons
cp "$DIR/assets/AppIcon.icns" "$APP_BUNDLE/Contents/Resources/AppIcon.icns"
cp "$DIR/app.py" "$APP_BUNDLE/Contents/Resources/app.py"

echo "✅ App bundle created at: $APP_BUNDLE"

# 4. Prepare DMG Staging Folder
STAGING="$DIST_DIR/dmg_staging"
mkdir -p "$STAGING"
cp -R "$APP_BUNDLE" "$STAGING/"
ln -s /Applications "$STAGING/Applications"

cat << 'EOF' > "$STAGING/QUICK_START.txt"
======================================================
  CYD VIBE BRIDGE HUB — QUICK START
======================================================

1. INSTALLATION:
   Drag 'VibeBridge.app' into your 'Applications' folder.

2. RUN:
   Double-click VibeBridge in Applications.

3. CONFIGURE YOUR CYD (Cheap Yellow Display):
   - In the VibeBridge window, note your Mac's LOCAL IP (or click "Copy IP").
   - Open 'src/main.cpp' in your project (47, 48, or 49).
   - Set:
       const char* BRIDGE_HOST = "YOUR_MAC_IP";
   - Flash your ESP32 display using PlatformIO or Arduino IDE.

4. START BRIDGES:
   - Click "Start All Bridges" in VibeBridge.
   - Your CYD display will connect and update live!

Tutorial: https://youtube.com/@IamNotIntoTech
3D Enclosures: https://youtube.com/@Makerrdesigns
======================================================
EOF

# 5. Build .dmg with hdiutil
echo "Creating DMG..."
hdiutil create \
    -volname "VibeBridge-Installer" \
    -srcfolder "$STAGING" \
    -ov \
    -format UDZO \
    "$DMG_PATH"

rm -rf "$STAGING"

echo "=========================================="
echo "🎉 Successfully built .dmg file!"
echo "Location: $DMG_PATH"
echo "Size: $(du -sh "$DMG_PATH" | cut -f1)"
echo "=========================================="
