#include "render.h"

// ═══════════════════════════════════════════════════════════════════
//  Bitmap font via wglUseFontBitmaps
// ═══════════════════════════════════════════════════════════════════
static GLuint s_fontBase = 0;
static int    s_charW    = 8;   // approximate glyph width (monospace)
static int    s_charH    = 14;  // approximate glyph height

void Render::InitFont(HDC hdc) {
    s_fontBase = glGenLists(256);
    HFONT font = CreateFontA(
        14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT old = (HFONT)SelectObject(hdc, font);
    wglUseFontBitmaps(hdc, 0, 256, s_fontBase);
    // query actual metrics
    TEXTMETRICA tm;
    if (GetTextMetricsA(hdc, &tm)) {
        s_charW = tm.tmAveCharWidth;
        s_charH = tm.tmHeight;
    }
    SelectObject(hdc, old);
    DeleteObject(font);
}

// ═══════════════════════════════════════════════════════════════════
//  2‑D overlay helpers
// ═══════════════════════════════════════════════════════════════════
void Render::Begin2D(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);   // top‑left origin
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Render::End2D() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}

// ═══════════════════════════════════════════════════════════════════
//  Primitives
// ═══════════════════════════════════════════════════════════════════
void Render::DrawLine(float x1, float y1, float x2, float y2,
                      float r, float g, float b, float a, float width) {
    glLineWidth(width);
    glColor4f(r, g, b, a);
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
}

void Render::DrawOutlineBox(float x, float y, float w, float h,
                            float r, float g, float b, float a, float width) {
    glLineWidth(width);
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
}

void Render::DrawFilledBox(float x, float y, float w, float h,
                           float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
}

void Render::DrawCircle(float cx, float cy, float radius, int seg,
                        float r, float g, float b, float a, float width) {
    glLineWidth(width);
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; ++i) {
        float angle = 2.f * 3.14159265f * (float)i / (float)seg;
        glVertex2f(cx + cosf(angle) * radius,
                   cy + sinf(angle) * radius);
    }
    glEnd();
}

void Render::DrawHealthBar(float x, float y, float h, int health, int maxHP) {
    float pct = (float)health / (float)maxHP;
    if (pct > 1.f) pct = 1.f;
    if (pct < 0.f) pct = 0.f;

    float barW  = 4.f;
    float barH  = h;
    float fillH = barH * pct;

    // background
    DrawFilledBox(x - barW - 3.f, y, barW, barH, 0.f, 0.f, 0.f, 0.6f);
    // fill (green→red gradient based on health)
    float rr = 1.f - pct, gg = pct;
    DrawFilledBox(x - barW - 3.f, y + (barH - fillH), barW, fillH, rr, gg, 0.f, 0.9f);
    // outline
    DrawOutlineBox(x - barW - 3.f, y, barW, barH, 0.f, 0.f, 0.f, 0.8f);
}

// ═══════════════════════════════════════════════════════════════════
//  Text
// ═══════════════════════════════════════════════════════════════════
void Render::DrawString(float x, float y, const char* text,
                        float r, float g, float b, float a) {
    if (!s_fontBase || !text) return;
    glColor4f(r, g, b, a);
    glRasterPos2f(x, y + s_charH);          // baseline offset
    glListBase(s_fontBase);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void Render::DrawStringCentered(float x, float y, const char* text,
                                float r, float g, float b, float a) {
    float tw = (float)(strlen(text) * s_charW);
    DrawString(x - tw * 0.5f, y, text, r, g, b, a);
}
