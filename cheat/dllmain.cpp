#include "pch.h"
#include "globals.h"
#include "hooks.h"
#include <TlHelp32.h>

// ═══════════════════════════════════════════════════════════════════
//  Main cheat thread  — runs after injection
// ═══════════════════════════════════════════════════════════════════
static DWORD WINAPI MainThread(LPVOID param) {
    // resolve module base of the game executable
    g_ModuleBase = (uintptr_t)GetModuleHandleA("ac_client.exe");
    if (!g_ModuleBase) {
        MessageBoxA(nullptr, "Failed to find ac_client.exe module!",
                    "ACCheat", MB_ICONERROR);
        FreeLibraryAndExitThread(g_DllHandle, 1);
        return 1;
    }

    // install wglSwapBuffers hook
    if (!Hooks::Init()) {
        MessageBoxA(nullptr, "Failed to hook wglSwapBuffers!",
                    "ACCheat", MB_ICONERROR);
        FreeLibraryAndExitThread(g_DllHandle, 1);
        return 1;
    }

    // wait until user presses END to eject
    while (g_Running) {
        if (GetAsyncKeyState(VK_END) & 1) {
            g_Running = false;
        }
        Sleep(50);
    }

    // cleanup
    Hooks::Shutdown();
    Sleep(100);
    FreeLibraryAndExitThread(g_DllHandle, 0);
    return 0;
}

// ═══════════════════════════════════════════════════════════════════
//  DLL Entry Point
// ═══════════════════════════════════════════════════════════════════
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        g_DllHandle = hModule;
        HANDLE h = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        if (h) CloseHandle(h);
    }
    return TRUE;
}
