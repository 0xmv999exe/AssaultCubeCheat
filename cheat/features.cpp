#include "features.h"
#include "globals.h"
#include "render.h"
#include <cstdio>

// ═══════════════════════════════════════════════════════════════════
//  ESP  — boxes, names, health bars, snaplines, distance
// ═══════════════════════════════════════════════════════════════════
void Features::RunESP(Entity* local, Entity** eList, int count,
                      float* matrix)
{
    if (!g_Settings.espEnabled) return;

    for (int i = 0; i < count; ++i) {
        Entity* ent = eList[i];
        if (!ent || ent == local)    continue;
        if (ent->health <= 0)        continue;
        if (ent->dead)               continue;

        // project head & feet to screen
        Vec2 sHead, sFeet;
        if (!W2S(ent->headPos, sHead, matrix, g_ScreenW, g_ScreenH)) continue;
        if (!W2S(ent->feetPos, sFeet, matrix, g_ScreenW, g_ScreenH)) continue;

        float boxH = sFeet.y - sHead.y;
        if (boxH < 4.f) continue;                 // too far / off screen
        float boxW = boxH * 0.5f;
        float boxX = sHead.x - boxW * 0.5f;
        float boxY = sHead.y;

        // colour by team
        bool sameTeam = (ent->team == local->team);
        float cr = sameTeam ? 0.2f : 1.0f;
        float cg = sameTeam ? 0.8f : 0.2f;
        float cb = sameTeam ? 0.2f : 0.2f;

        // ── box ──
        if (g_Settings.espBox) {
            // outer black outline for contrast
            Render::DrawOutlineBox(boxX - 1, boxY - 1, boxW + 2, boxH + 2,
                                  0.f, 0.f, 0.f, 0.7f, 1.f);
            Render::DrawOutlineBox(boxX, boxY, boxW, boxH,
                                  cr, cg, cb, 1.f, 1.f);
        }

        // ── health bar ──
        if (g_Settings.espHealth) {
            Render::DrawHealthBar(boxX, boxY, boxH, ent->health, 100);
        }

        // ── name ──
        if (g_Settings.espName) {
            Render::DrawStringCentered(sHead.x, boxY - 14.f,
                                       ent->name, 1.f, 1.f, 1.f, 1.f);
        }

        // ── distance ──
        if (g_Settings.espDistance) {
            float dist = local->headPos.Dist(ent->headPos);
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0fm", dist);
            Render::DrawStringCentered(sHead.x, sFeet.y + 2.f,
                                       buf, 0.8f, 0.8f, 0.8f, 1.f);
        }

        // ── snaplines ──
        if (g_Settings.espSnaplines) {
            Render::DrawLine((float)(g_ScreenW / 2), (float)g_ScreenH,
                             sHead.x, sFeet.y,
                             cr, cg, cb, 0.5f, 1.f);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  AIMBOT  — aim‑lock on enemy head while holding RMB
//
//  Behaviour (standard aim‑lock):
//    • Press RMB   → select closest enemy in FOV, LOCK onto them
//    • Hold  RMB   → view angles snap to locked target's head every frame
//    • Release RMB → lock clears, normal mouse control returns
//    • If locked target dies → auto‑select next closest enemy
// ═══════════════════════════════════════════════════════════════════
static Entity* s_lockedTarget = nullptr;   // persists across frames

void Features::RunAimbot(Entity* local, Entity** eList, int count,
                         float* matrix)
{
    // draw FOV circle whenever aimbot is enabled
    if (g_Settings.aimbotEnabled && g_Settings.aimbotFovCircle) {
        float radius = g_Settings.aimbotFov * ((float)g_ScreenW / 360.f);
        Render::DrawCircle((float)(g_ScreenW / 2), (float)(g_ScreenH / 2),
                           radius, 64, 1.f, 1.f, 1.f, 0.3f, 1.f);
    }

    if (!g_Settings.aimbotEnabled) { s_lockedTarget = nullptr; return; }

    bool keyHeld = (GetAsyncKeyState(g_Settings.aimbotKey) & 0x8000) != 0;

    // ── key released → clear lock ──
    if (!keyHeld) {
        s_lockedTarget = nullptr;
        return;
    }

    // ── validate current lock (target still alive & valid?) ──
    if (s_lockedTarget) {
        if (s_lockedTarget->health <= 0 || s_lockedTarget->dead)
            s_lockedTarget = nullptr;   // target died → will re‑select below
    }

    // ── if no locked target, pick the closest enemy in FOV ──
    if (!s_lockedTarget) {
        float bestDist = 1e9f;

        for (int i = 0; i < count; ++i) {
            Entity* ent = eList[i];
            if (!ent || ent == local)    continue;
            if (ent->health <= 0)        continue;
            if (ent->dead)               continue;
            if (ent->team == local->team) continue;   // skip teammates

            Vec2 s;
            if (!W2S(ent->headPos, s, matrix, g_ScreenW, g_ScreenH)) continue;

            // distance from crosshair (screen centre)
            float dx = s.x - g_ScreenW * 0.5f;
            float dy = s.y - g_ScreenH * 0.5f;
            float d  = sqrtf(dx * dx + dy * dy);

            // FOV check (in pixels)
            float fovPx = g_Settings.aimbotFov * ((float)g_ScreenW / 360.f);
            if (d > fovPx) continue;

            if (d < bestDist) {
                bestDist      = d;
                s_lockedTarget = ent;
            }
        }
    }

    if (!s_lockedTarget) return;

    // ── LOCK: set view angles directly to target head ──
    float tYaw, tPitch;
    CalcAngle(local->headPos, s_lockedTarget->headPos, tYaw, tPitch);

    local->viewHeading = tYaw;
    local->viewPitch   = tPitch;

    // clamp
    if (local->viewHeading < 0.f)   local->viewHeading += 360.f;
    if (local->viewHeading > 360.f) local->viewHeading -= 360.f;
    if (local->viewPitch >  90.f)   local->viewPitch =  90.f;
    if (local->viewPitch < -90.f)   local->viewPitch = -90.f;
}

// ═══════════════════════════════════════════════════════════════════
//  MISC  — god mode, infinite ammo, no recoil
// ═══════════════════════════════════════════════════════════════════
void Features::RunMisc(Entity* local) {
    if (g_Settings.godMode) {
        local->health = 999;
        local->armor  = 999;
    }

    if (g_Settings.infAmmo) {
        local->ammoPistol   = 999;
        local->ammoCarbine  = 999;
        local->ammoShotgun  = 999;
        local->ammoSMG      = 999;
        local->ammoSniper   = 999;
        local->ammoAssault  = 999;
        local->grenades     = 999;
        local->ammoAkimbo   = 999;
        local->ammoReservePistol  = 999;
        local->ammoReserveCarbine = 999;
        local->ammoReserveShotgun = 999;
        local->ammoReserveSMG     = 999;
        local->ammoReserveSniper  = 999;
        local->ammoReserveAssault = 999;
    }

    if (g_Settings.noRecoil) {
        local->recoilForcePitch = 0.f;
    }
}
