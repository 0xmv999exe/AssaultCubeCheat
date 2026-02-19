#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

// ═══════════════════════════════════════════════════════════════════
//  Find process ID by name
// ═══════════════════════════════════════════════════════════════════
static DWORD FindProcess(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) {
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return 0;
}

// ═══════════════════════════════════════════════════════════════════
int main() {
    SetConsoleTitleA("ACCheat Injector");

    std::cout << "============================================\n";
    std::cout << "   ACCheat DLL Injector\n";
    std::cout << "   AssaultCube 1.3.0.2\n";
    std::cout << "============================================\n\n";

    // --- locate process ---
    std::cout << "[*] Searching for ac_client.exe ...\n";
    DWORD pid = 0;
    for (int i = 0; i < 60 && !pid; ++i) {
        pid = FindProcess(L"ac_client.exe");
        if (!pid) Sleep(1000);
    }
    if (!pid) {
        std::cerr << "[!] ac_client.exe not found. Start the game first!\n";
        system("pause");
        return 1;
    }
    std::cout << "[+] Found ac_client.exe  (PID " << pid << ")\n";

    // --- resolve DLL path ---
    char dllPath[MAX_PATH]{};
    GetFullPathNameA("ACCheat.dll", MAX_PATH, dllPath, nullptr);
    DWORD attr = GetFileAttributesA(dllPath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[!] ACCheat.dll not found beside the injector!\n";
        system("pause");
        return 1;
    }
    std::cout << "[+] DLL: " << dllPath << "\n";

    // --- open process ---
    HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!proc) {
        std::cerr << "[!] OpenProcess failed (try running as admin).\n";
        system("pause");
        return 1;
    }

    // --- allocate + write path ---
    size_t pathLen = strlen(dllPath) + 1;
    void* mem = VirtualAllocEx(proc, nullptr, pathLen,
                               MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem) {
        std::cerr << "[!] VirtualAllocEx failed.\n";
        CloseHandle(proc);
        system("pause");
        return 1;
    }
    WriteProcessMemory(proc, mem, dllPath, pathLen, nullptr);

    // --- create remote thread (LoadLibraryA) ---
    FARPROC loadLib = GetProcAddress(GetModuleHandleA("kernel32.dll"),
                                     "LoadLibraryA");
    HANDLE thread = CreateRemoteThread(proc, nullptr, 0,
                        (LPTHREAD_START_ROUTINE)loadLib,
                        mem, 0, nullptr);
    if (!thread) {
        std::cerr << "[!] CreateRemoteThread failed.\n";
        VirtualFreeEx(proc, mem, 0, MEM_RELEASE);
        CloseHandle(proc);
        system("pause");
        return 1;
    }

    WaitForSingleObject(thread, 5000);

    // cleanup
    VirtualFreeEx(proc, mem, 0, MEM_RELEASE);
    CloseHandle(thread);
    CloseHandle(proc);

    std::cout << "\n[+] DLL injected successfully!\n";
    std::cout << "[*] Press INSERT in-game to open the cheat menu.\n";
    std::cout << "[*] Press END to eject the cheat.\n\n";
    system("pause");
    return 0;
}
