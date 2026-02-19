#pragma once
#include "pch.h"

// ═══════════════════════════════════════════════════════════════════
//  Cheat Settings  — toggled / adjusted from the menu
// ═══════════════════════════════════════════════════════════════════
struct Settings {
    // ESP
    bool  espEnabled    = false;
    bool  espBox        = true;
    bool  espName       = true;
    bool  espHealth     = true;
    bool  espSnaplines  = true;
    bool  espDistance    = true;

    // Aimbot
    bool  aimbotEnabled = false;
    bool  aimbotFovCircle = true;
    float aimbotFov     = 90.f;   // degrees (screen‑space)
    float aimbotSmooth  = 5.f;    // 1 = instant, higher = smoother
    int   aimbotKey     = VK_RBUTTON; // right mouse button

    // Misc
    bool  godMode       = false;
    bool  infAmmo       = false;
    bool  noRecoil      = false;
};

// ═══════════════════════════════════════════════════════════════════
//  Runtime Globals
// ═══════════════════════════════════════════════════════════════════
inline Settings   g_Settings;
inline uintptr_t  g_ModuleBase  = 0;
inline int        g_ScreenW     = 0;
inline int        g_ScreenH     = 0;
inline bool       g_MenuOpen    = false;
inline bool       g_Running     = true;
inline HMODULE    g_DllHandle   = nullptr;
