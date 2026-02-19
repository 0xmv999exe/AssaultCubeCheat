#include "menu.h"
#include "globals.h"
#include "render.h"
#include <cstdio>

// ═══════════════════════════════════════════════════════════════════
//  Menu item types
// ═══════════════════════════════════════════════════════════════════
enum class ItemType { Toggle, Slider, Label };

struct MenuItem {
    const char* label;
    ItemType    type;
    bool*       boolVal;        // for Toggle
    float*      floatVal;       // for Slider
    float       fMin, fMax, fStep;
};

// ═══════════════════════════════════════════════════════════════════
//  Items list
// ═══════════════════════════════════════════════════════════════════
static MenuItem s_items[] = {
    // ── ESP ──
    { "--- ESP ---",       ItemType::Label,  nullptr, nullptr, 0,0,0 },
    { "ESP Enabled",       ItemType::Toggle, &g_Settings.espEnabled,   nullptr, 0,0,0 },
    { "Box",               ItemType::Toggle, &g_Settings.espBox,       nullptr, 0,0,0 },
    { "Name",              ItemType::Toggle, &g_Settings.espName,      nullptr, 0,0,0 },
    { "Health Bar",        ItemType::Toggle, &g_Settings.espHealth,    nullptr, 0,0,0 },
    { "Snaplines",         ItemType::Toggle, &g_Settings.espSnaplines, nullptr, 0,0,0 },
    { "Distance",          ItemType::Toggle, &g_Settings.espDistance,  nullptr, 0,0,0 },
    // ── Aimbot ──
    { "--- AIMBOT ---",    ItemType::Label,  nullptr, nullptr, 0,0,0 },
    { "Aimbot Enabled",    ItemType::Toggle, &g_Settings.aimbotEnabled,   nullptr, 0,0,0 },
    { "FOV Circle",        ItemType::Toggle, &g_Settings.aimbotFovCircle, nullptr, 0,0,0 },
    { "Aim FOV",           ItemType::Slider, nullptr, &g_Settings.aimbotFov,    5.f, 180.f, 5.f },
    // ── Misc ──
    { "--- MISC ---",      ItemType::Label,  nullptr, nullptr, 0,0,0 },
    { "God Mode",          ItemType::Toggle, &g_Settings.godMode,   nullptr,  0,0,0 },
    { "Infinite Ammo",     ItemType::Toggle, &g_Settings.infAmmo,   nullptr,  0,0,0 },
    { "No Recoil",         ItemType::Toggle, &g_Settings.noRecoil,  nullptr,  0,0,0 },
};
static constexpr int ITEM_COUNT = sizeof(s_items) / sizeof(s_items[0]);
static int s_sel = 1;   // selected index (skip first label)

// input debounce
static bool s_keyDown[256] = {};
static bool KeyPressed(int vk) {
    bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = down && !s_keyDown[vk];
    s_keyDown[vk] = down;
    return pressed;
}

// ═══════════════════════════════════════════════════════════════════
//  Draw
// ═══════════════════════════════════════════════════════════════════
void Menu::Draw() {
    if (!g_MenuOpen) return;

    const float mx = 30.f;     // menu x
    const float my = 60.f;     // menu y
    const float mw = 260.f;    // width
    const float rowH = 20.f;
    const float titleH = 30.f;
    float mh = titleH + rowH * ITEM_COUNT + 10.f;

    // ── background panel ──
    Render::DrawFilledBox(mx, my, mw, mh, 0.08f, 0.08f, 0.12f, 0.92f);
    // border
    Render::DrawOutlineBox(mx, my, mw, mh, 0.35f, 0.55f, 1.f, 0.9f, 2.f);

    // ── title bar ──
    Render::DrawFilledBox(mx, my, mw, titleH, 0.15f, 0.25f, 0.55f, 0.95f);
    Render::DrawStringCentered(mx + mw * 0.5f, my + 8.f,
                               "ACCheat  -  INSERT to toggle",
                               1.f, 1.f, 1.f, 1.f);

    // ── items ──
    float iy = my + titleH + 4.f;
    for (int i = 0; i < ITEM_COUNT; ++i) {
        MenuItem& it = s_items[i];
        bool selected = (i == s_sel);
        float ix = mx + 10.f;

        // selection highlight
        if (selected)
            Render::DrawFilledBox(mx + 2.f, iy, mw - 4.f, rowH,
                                  0.25f, 0.4f, 0.85f, 0.35f);

        if (it.type == ItemType::Label) {
            // section header
            Render::DrawString(ix, iy + 3.f, it.label,
                               0.5f, 0.7f, 1.f, 1.f);
        }
        else if (it.type == ItemType::Toggle) {
            bool on = it.boolVal ? *it.boolVal : false;
            Render::DrawString(ix, iy + 3.f, it.label,
                               0.9f, 0.9f, 0.9f, 1.f);
            // ON / OFF badge
            const char* badge = on ? "[ON]" : "[OFF]";
            float br = on ? 0.2f : 0.8f;
            float bg = on ? 1.0f : 0.3f;
            float bb = on ? 0.4f : 0.3f;
            Render::DrawString(mx + mw - 50.f, iy + 3.f,
                               badge, br, bg, bb, 1.f);
        }
        else if (it.type == ItemType::Slider) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s: %.1f", it.label,
                     it.floatVal ? *it.floatVal : 0.f);
            Render::DrawString(ix, iy + 3.f, buf,
                               0.9f, 0.9f, 0.9f, 1.f);
            // mini bar
            if (it.floatVal) {
                float pct = (*it.floatVal - it.fMin) / (it.fMax - it.fMin);
                float barX  = mx + mw - 80.f;
                float barW  = 65.f;
                float barY2 = iy + rowH - 5.f;
                Render::DrawFilledBox(barX, barY2, barW, 4.f,
                                      0.2f, 0.2f, 0.3f, 0.8f);
                Render::DrawFilledBox(barX, barY2, barW * pct, 4.f,
                                      0.3f, 0.6f, 1.f, 0.9f);
            }
        }

        iy += rowH;
    }

    // credits
    iy += 10.f;
    Render::DrawStringCentered(mx + mw * 0.5f, iy, "Made by mv999exe", 0.4f, 0.7f, 1.f, 1.f);
    iy += 12.f;
    Render::DrawStringCentered(mx + mw * 0.5f, iy, "discord.gg/Kf9YrsKcyN", 0.4f, 0.4f, 0.8f, 1.f);

    // footer hint
    Render::DrawString(mx + 6.f, iy + 18.f,
                       "UP/DOWN: Navigate   LEFT/RIGHT: Change",
                       0.5f, 0.5f, 0.5f, 0.7f);
}

// ═══════════════════════════════════════════════════════════════════
//  Input
// ═══════════════════════════════════════════════════════════════════
void Menu::HandleInput() {
    // navigate
    if (KeyPressed(VK_UP)) {
        do { s_sel--; if (s_sel < 0) s_sel = ITEM_COUNT - 1; }
        while (s_items[s_sel].type == ItemType::Label);
    }
    if (KeyPressed(VK_DOWN)) {
        do { s_sel++; if (s_sel >= ITEM_COUNT) s_sel = 0; }
        while (s_items[s_sel].type == ItemType::Label);
    }

    MenuItem& cur = s_items[s_sel];

    // toggle
    if (cur.type == ItemType::Toggle &&
        (KeyPressed(VK_RIGHT) || KeyPressed(VK_LEFT) || KeyPressed(VK_RETURN))) {
        if (cur.boolVal) *cur.boolVal = !*cur.boolVal;
    }

    // slider
    if (cur.type == ItemType::Slider && cur.floatVal) {
        if (KeyPressed(VK_RIGHT)) {
            *cur.floatVal += cur.fStep;
            if (*cur.floatVal > cur.fMax) *cur.floatVal = cur.fMax;
        }
        if (KeyPressed(VK_LEFT)) {
            *cur.floatVal -= cur.fStep;
            if (*cur.floatVal < cur.fMin) *cur.floatVal = cur.fMin;
        }
    }
}
