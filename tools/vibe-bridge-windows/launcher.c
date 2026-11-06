#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * VibeBridge Windows Native Launcher
 * - Detects Python on Windows (py.exe, pythonw.exe, python.exe, or local python/ folder)
 * - Checks & installs Flask automatically if needed
 * - Launches app.py with native GUI (no black console popup)
 * - Prompts to download Python if not found
 */

int file_exists(const char *path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int dir_exists(const char *path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int run_command_silent(const char *cmd) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    char cmd_copy[1024];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    if (CreateProcessA(NULL, cmd_copy, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 30000); // 30s timeout
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return (exitCode == 0);
    }
    return 0;
}

int find_python(char *out_path, size_t max_len) {
    // 1. Check local portable python directory
    if (file_exists("python\\pythonw.exe")) {
        strncpy(out_path, "python\\pythonw.exe", max_len);
        return 1;
    }
    if (file_exists("python\\python.exe")) {
        strncpy(out_path, "python\\python.exe", max_len);
        return 1;
    }

    // 2. Check py launcher (standard on Windows)
    if (run_command_silent("py -3 -c \"import sys; sys.exit(0)\"")) {
        strncpy(out_path, "py -3", max_len);
        return 1;
    }

    // 3. Check pythonw on PATH
    if (run_command_silent("pythonw -c \"import sys; sys.exit(0)\"")) {
        strncpy(out_path, "pythonw", max_len);
        return 1;
    }

    // 4. Check python on PATH
    if (run_command_silent("python -c \"import sys; sys.exit(0)\"")) {
        strncpy(out_path, "python", max_len);
        return 1;
    }

    // 5. Check common %LOCALAPPDATA% installation paths
    char local_appdata[MAX_PATH];
    if (GetEnvironmentVariableA("LOCALAPPDATA", local_appdata, MAX_PATH)) {
        const char *py_versions[] = {"313", "312", "311", "310", "39"};
        for (int i = 0; i < 5; i++) {
            char candidate[MAX_PATH];
            snprintf(candidate, sizeof(candidate), "%s\\Programs\\Python\\Python%s\\pythonw.exe", local_appdata, py_versions[i]);
            if (file_exists(candidate)) {
                snprintf(out_path, max_len, "\"%s\"", candidate);
                return 1;
            }
            snprintf(candidate, sizeof(candidate), "%s\\Programs\\Python\\Python%s\\python.exe", local_appdata, py_versions[i]);
            if (file_exists(candidate)) {
                snprintf(out_path, max_len, "\"%s\"", candidate);
                return 1;
            }
        }
    }

    // 6. Check C:\Python3*
    const char *sys_versions[] = {"313", "312", "311", "310", "39"};
    for (int i = 0; i < 5; i++) {
        char candidate[MAX_PATH];
        snprintf(candidate, sizeof(candidate), "C:\\Python%s\\pythonw.exe", sys_versions[i]);
        if (file_exists(candidate)) {
            snprintf(out_path, max_len, "\"%s\"", candidate);
            return 1;
        }
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    char python_cmd[MAX_PATH];

    if (!find_python(python_cmd, sizeof(python_cmd))) {
        int choice = MessageBoxA(
            NULL,
            "Python 3 was not detected on your Windows system.\n\n"
            "Vibe Bridge Hub requires Python 3 to run.\n\n"
            "Would you like to open the official Python download page now?\n"
            "(Important: Be sure to check 'Add python.exe to PATH' during installation!)",
            "Python Required — Vibe Bridge Hub",
            MB_ICONEXCLAMATION | MB_YESNO
        );

        if (choice == IDYES) {
            ShellExecuteA(NULL, "open", "https://www.python.org/downloads/", NULL, NULL, SW_SHOWNORMAL);
        }
        return 1;
    }

    // Check if flask is installed
    char test_flask_cmd[512];
    snprintf(test_flask_cmd, sizeof(test_flask_cmd), "%s -c \"import flask; sys.exit(0)\"", python_cmd);
    if (!run_command_silent(test_flask_cmd)) {
        // Automatically install flask silently
        char install_cmd[512];
        snprintf(install_cmd, sizeof(install_cmd), "%s -m pip install flask", python_cmd);
        run_command_silent(install_cmd);
    }

    // Launch app.py
    char launch_cmd[512];
    snprintf(launch_cmd, sizeof(launch_cmd), "%s app.py", python_cmd);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, launch_cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        MessageBoxA(NULL, "Failed to launch app.py. Please verify app.py exists in the same folder.", "Launch Error", MB_ICONERROR);
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

