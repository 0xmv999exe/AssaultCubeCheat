#pragma once
#include "pch.h"

// ═══════════════════════════════════════════════════════════════════
//  5‑byte trampoline JMP hook for x86 (32‑bit)
// ═══════════════════════════════════════════════════════════════════
class TrampolineHook {
    BYTE* m_target      = nullptr;
    BYTE* m_trampoline  = nullptr;
    BYTE  m_stolen[5]   = {};
    bool  m_installed    = false;

public:
    bool Install(BYTE* target, BYTE* detour);
    void Remove();
    BYTE* GetTrampoline() const { return m_trampoline; }
    bool  IsInstalled()   const { return m_installed; }
};

// ═══════════════════════════════════════════════════════════════════
//  Hook manager  — sets up and tears down the wglSwapBuffers hook
// ═══════════════════════════════════════════════════════════════════
namespace Hooks {
    bool  Init();
    void  Shutdown();
}
