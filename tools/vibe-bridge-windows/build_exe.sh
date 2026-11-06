#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"

echo "=========================================="
echo " Building VibeBridge.exe & Windows Zip"
echo "=========================================="

# Check for mingw-w64
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "x86_64-w64-mingw32-gcc not found. Please install mingw-w64 via brew: brew install mingw-w64"
    exit 1
fi

# 1. Compile resource with icon
x86_64-w64-mingw32-windres resource.rc -O coff -o resource.res

# 2. Compile native launcher with Windows subsystem (no console window)
x86_64-w64-mingw32-gcc -O2 -mwindows launcher.c resource.res -o VibeBridge.exe -lshlwapi

echo "✅ VibeBridge.exe compiled successfully!"

# 3. Create release zip
mkdir -p dist
rm -f dist/VibeBridge-Windows-v1.0.zip
zip -j dist/VibeBridge-Windows-v1.0.zip VibeBridge.exe VibeBridge.bat app.py icon.ico README.txt requirements.txt

echo "=========================================="
echo "🎉 Successfully built Windows release!"
echo "Location: $DIR/dist/VibeBridge-Windows-v1.0.zip"
echo "EXE: $DIR/VibeBridge.exe"
echo "=========================================="

