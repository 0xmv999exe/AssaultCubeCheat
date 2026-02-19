#pragma once
#include "pch.h"

// ═══════════════════════════════════════════════════════════════════
//  Constants
// ═══════════════════════════════════════════════════════════════════
constexpr float PI      = 3.14159265358979323846f;
constexpr float RAD2DEG = 180.0f / PI;
constexpr float DEG2RAD = PI / 180.0f;

// ═══════════════════════════════════════════════════════════════════
//  Game Offsets  (relative to ac_client.exe module base)
// ═══════════════════════════════════════════════════════════════════
namespace Off {
    constexpr uintptr_t LocalPlayer = 0x18AC00;   // ptr -> Entity*
    constexpr uintptr_t EntityList  = 0x18AC04;   // ptr -> Entity*[]
    constexpr uintptr_t PlayerCount = 0x18AC0C;   // int
    constexpr uintptr_t ViewMatrix  = 0x17DFD0;   // float[16]
}

// ═══════════════════════════════════════════════════════════════════
//  Math Types
// ═══════════════════════════════════════════════════════════════════
struct Vec2 { float x, y; };

struct Vec3 {
    float x, y, z;
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    float Length()   const { return sqrtf(x*x + y*y + z*z); }
    float Length2D() const { return sqrtf(x*x + y*y); }
    float Dist(const Vec3& o) const { return (*this - o).Length(); }
};

// ═══════════════════════════════════════════════════════════════════
//  Entity Class  (AssaultCube 1.3.0.2)
// ═══════════════════════════════════════════════════════════════════
class Entity {
public:
    char  pad_0000[4];              // 0x0000
    Vec3  headPos;                  // 0x0004
    Vec3  requestedMovement;        // 0x0010
    char  pad_001C[12];             // 0x001C
    Vec3  feetPos;                  // 0x0028
    float viewHeading;              // 0x0034   0‒360
    float viewPitch;                // 0x0038  -90‒90
    char  pad_003C[4];              // 0x003C
    float recoilForcePitch;         // 0x0040
    char  pad_0044[12];             // 0x0044
    float playerCurrentHeight;      // 0x0050
    char  pad_0054[8];              // 0x0054
    int32_t onGround;               // 0x005C
    char  pad_0060[32];             // 0x0060
    int32_t moveCommand;            // 0x0080
    char  pad_0084[104];            // 0x0084
    int32_t health;                 // 0x00EC
    int32_t armor;                  // 0x00F0
    char  pad_00F4[12];             // 0x00F4
    int32_t akimboAvailable;        // 0x0100
    char  pad_0104[4];              // 0x0104
    int32_t ammoReservePistol;      // 0x0108
    int32_t ammoReserveCarbine;     // 0x010C
    int32_t ammoReserveShotgun;     // 0x0110
    int32_t ammoReserveSMG;         // 0x0114
    int32_t ammoReserveSniper;      // 0x0118
    int32_t ammoReserveAssault;     // 0x011C
    char  pad_0120[12];             // 0x0120
    int32_t ammoPistol;             // 0x012C
    int32_t ammoCarbine;            // 0x0130
    int32_t ammoShotgun;            // 0x0134
    int32_t ammoSMG;                // 0x0138
    int32_t ammoSniper;             // 0x013C
    int32_t ammoAssault;            // 0x0140
    int32_t grenades;               // 0x0144
    int32_t ammoAkimbo;             // 0x0148
    char  pad_014C[0xD9];           // 0x014C  → 0x0225
    char  name[16];                 // 0x0225
    char  pad_0235[0xD7];           // 0x0235  → 0x030C
    int32_t team;                   // 0x030C
    char  pad_0310[8];              // 0x0310
    int32_t dead;                   // 0x0318
};

// ═══════════════════════════════════════════════════════════════════
//  Helper: angle calculation  (Cube‑engine coords)
// ═══════════════════════════════════════════════════════════════════
inline void CalcAngle(const Vec3& src, const Vec3& dst, float& yaw, float& pitch) {
    Vec3 d = dst - src;
    float dist = d.Length2D();
    // +180° correction for AssaultCube's Cube‑engine coordinate convention
    yaw   = atan2f(-d.x, d.y) * RAD2DEG + 180.f;
    pitch = atan2f(d.z, dist)  * RAD2DEG;
    if (yaw >= 360.f) yaw -= 360.f;
    if (yaw <    0.f) yaw += 360.f;
}

inline float NormAngle(float a) {
    while (a >  180.f) a -= 360.f;
    while (a < -180.f) a += 360.f;
    return a;
}

// ═══════════════════════════════════════════════════════════════════
//  Helper: World → Screen
// ═══════════════════════════════════════════════════════════════════
inline bool W2S(const Vec3& world, Vec2& screen, const float* m, int w, int h) {
    float cX = world.x*m[0] + world.y*m[4] + world.z*m[8]  + m[12];
    float cY = world.x*m[1] + world.y*m[5] + world.z*m[9]  + m[13];
    float cW = world.x*m[3] + world.y*m[7] + world.z*m[11] + m[15];
    if (cW < 0.1f) return false;
    float ndcX = cX / cW;
    float ndcY = cY / cW;
    screen.x = (w * 0.5f) * (1.f + ndcX);
    screen.y = (h * 0.5f) * (1.f - ndcY);
    return true;
}
