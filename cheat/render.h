#pragma once
#include "pch.h"

namespace Render {
    void InitFont(HDC hdc);

    void Begin2D(int w, int h);
    void End2D();

    void DrawLine(float x1, float y1, float x2, float y2,
                  float r, float g, float b, float a = 1.f, float width = 1.f);
    void DrawOutlineBox(float x, float y, float w, float h,
                        float r, float g, float b, float a = 1.f, float width = 1.f);
    void DrawFilledBox(float x, float y, float w, float h,
                       float r, float g, float b, float a = 1.f);
    void DrawCircle(float cx, float cy, float radius, int segments,
                    float r, float g, float b, float a = 1.f, float width = 1.f);
    void DrawHealthBar(float x, float y, float h, int health, int maxHealth = 100);
    void DrawString(float x, float y, const char* text,
                    float r, float g, float b, float a = 1.f);
    void DrawStringCentered(float x, float y, const char* text,
                            float r, float g, float b, float a = 1.f);
}
