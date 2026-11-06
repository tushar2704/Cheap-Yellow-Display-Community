@echo off
title CYD Vibe Bridge Hub Launcher
color 0b

echo ======================================================
echo    CYD VIBE BRIDGE HUB — WINDOWS LAUNCHER
echo ======================================================
echo.

:: 1. Check for Python
where py >nul 2>&1
if %errorlevel%==0 (
    set PY_CMD=py -3
    goto FOUND_PY
)

where python >nul 2>&1
if %errorlevel%==0 (
    set PY_CMD=python
    goto FOUND_PY
)

echo [!] Python 3 was not detected on your system.
echo [!] Please install Python 3 from https://www.python.org/downloads/
echo [!] Make sure to check "Add python.exe to PATH" during installation.
echo.
pause
exit /b 1

:FOUND_PY
echo [*] Python detected: %PY_CMD%

:: 2. Check Flask
echo [*] Checking required dependencies...
%PY_CMD% -c "import flask" >nul 2>&1
if %errorlevel% neq 0 (
    echo [*] Installing Flask...
    %PY_CMD% -m pip install flask
)

:: 3. Launch App
echo [*] Starting CYD Vibe Bridge Hub GUI...
start "" %PY_CMD% app.py
exit /b 0
