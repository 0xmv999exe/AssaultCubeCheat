#include "hooks.h"
#include "globals.h"
#include "render.h"
#include "features.h"
#include "menu.h"
#include "game.h"

// ═══════════════════════════════════════════════════════════════════
//  TrampolineHook implementation
// ═══════════════════════════════════════════════════════════════════
bool TrampolineHook::Install(BYTE* target, BYTE* detour) {
    m_target = target;

    // allocate executable memory for the trampoline
    m_trampoline = (BYTE*)VirtualAlloc(nullptr, 64,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!m_trampoline) return false;

    // copy original 5 bytes into trampoline
    memcpy(m_stolen, m_target, 5);
    memcpy(m_trampoline, m_target, 5);

    // append JMP back to target+5
    m_trampoline[5] = 0xE9;
    *(DWORD*)(m_trampoline + 6) =
        (DWORD)(m_target + 5) - (DWORD)(m_trampoline + 10);

    // overwrite target with JMP to detour
    DWORD oldProt;
    VirtualProtect(m_target, 5, PAGE_EXECUTE_READWRITE, &oldProt);
    m_target[0] = 0xE9;
    *(DWORD*)(m_target + 1) = (DWORD)detour - (DWORD)(m_target + 5);
    VirtualProtect(m_target, 5, oldProt, &oldProt);

    m_installed = true;
    return true;
}

void TrampolineHook::Remove() {
    if (!m_installed) return;
    DWORD oldProt;
    VirtualProtect(m_target, 5, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(m_target, m_stolen, 5);
    VirtualProtect(m_target, 5, oldProt, &oldProt);
    VirtualFree(m_trampoline, 0, MEM_RELEASE);
    m_installed = false;
}

// ═══════════════════════════════════════════════════════════════════
//  wglSwapBuffers hook
// ═══════════════════════════════════════════════════════════════════
static TrampolineHook g_SwapHook;

typedef BOOL(WINAPI* fnSwapBuffers)(HDC);
static fnSwapBuffers oSwapBuffers = nullptr;

// one‑time OpenGL init
static bool s_glInit = false;
static void InitGL(HDC hdc) {
    Render::InitFont(hdc);
    s_glInit = true;
}

// ── INSERT key toggle (with debounce) ──────────────────────────────
static bool s_insWasDown = false;

static void HandleInput() {
    bool insDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
    if (insDown && !s_insWasDown)
        g_MenuOpen = !g_MenuOpen;
    s_insWasDown = insDown;
}

// ── Hook callback ──────────────────────────────────────────────────
static BOOL WINAPI hkSwapBuffers(HDC hdc) {
    // IMPORTANT: draw BEFORE calling original SwapBuffers
    // so our overlay is rendered onto the current back-buffer
    // that is about to be presented. Drawing after swap would
    // put our pixels into the NEXT frame's buffer → invisible
    // in fullscreen or tearing in windowed mode.

    if (!s_glInit) InitGL(hdc);

    // viewport dimensions
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    g_ScreenW = vp[2];
    g_ScreenH = vp[3];

    HandleInput();

    // ── save full GL state (attribs + both matrix stacks) ──
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // set up 2‑D orthographic projection (pushes its own matrices)
    Render::Begin2D(g_ScreenW, g_ScreenH);

    // read game pointers
    Entity* local   = *(Entity**)(g_ModuleBase + Off::LocalPlayer);
    Entity** eList  = *(Entity***)(g_ModuleBase + Off::EntityList);
    int count       = *(int*)(g_ModuleBase + Off::PlayerCount);
    float* matrix   = (float*)(g_ModuleBase + Off::ViewMatrix);

    if (local) {
        Features::RunESP(local, eList, count, matrix);
        Features::RunAimbot(local, eList, count, matrix);
        Features::RunMisc(local);
    }

    Menu::Draw();
    if (g_MenuOpen) Menu::HandleInput();

    // restore projection + modelview (popped inside End2D)
    Render::End2D();

    // restore all GL attributes the game had before
    glPopAttrib();

    // NOW swap the buffer with our overlay drawn on it
    return oSwapBuffers(hdc);
}

// ═══════════════════════════════════════════════════════════════════
//  Public API
// ═══════════════════════════════════════════════════════════════════
bool Hooks::Init() {
    HMODULE gl = GetModuleHandleA("opengl32.dll");
    if (!gl) return false;

    BYTE* target = (BYTE*)GetProcAddress(gl, "wglSwapBuffers");
    if (!target) return false;

    if (!g_SwapHook.Install(target, (BYTE*)hkSwapBuffers))
        return false;

    oSwapBuffers = (fnSwapBuffers)g_SwapHook.GetTrampoline();
    return true;
}

void Hooks::Shutdown() {
    g_SwapHook.Remove();
}
