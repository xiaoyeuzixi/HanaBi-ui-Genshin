#pragma once

#include "imgui.h"

namespace hb {
namespace paint {

ImU32 MixI(ImU32 a, ImU32 b, float t);
ImU32 SetA(ImU32 c, int a);

void Fill(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 c);
void FillGrad(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 top, ImU32 bot);
void Stroke(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 c, float th);

void Stroke2(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 outer, ImU32 inner, float th);

void TopHighlight(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 c);

void Shadow(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 col, float dy, float sigma, float k);
void Glow(ImDrawList* dl, ImVec2 center, float radius, ImU32 col, int layers = 5);
void GlowEllipse(ImDrawList* dl, ImVec2 center, float rx, float ry, ImU32 col, int layers = 4);

void Knob(ImDrawList* dl, ImVec2 c, float radius, ImU32 c0, ImU32 c1, ImU32 shadowCol, float shadowDy,
          float shadowSigma, float k);
void Ring(ImDrawList* dl, ImVec2 c, float r, ImU32 col, float th, int seg = 0);

void Text(ImDrawList* dl, ImFont* f, float size, ImVec2 pos, ImU32 c, const char* s);
void TextC(ImDrawList* dl, ImFont* f, float size, ImVec2 center, ImU32 c, const char* s);
void TextR(ImDrawList* dl, ImFont* f, float size, ImVec2 rightMid, ImU32 c, const char* s);

void TextV(ImDrawList* dl, ImFont* f, float size, ImVec2 leftMid, ImU32 c, const char* s);

void TextTracked(ImDrawList* dl, ImFont* f, float size, ImVec2 pos, ImU32 c, const char* s, float track);
float TrackedWidth(ImFont* f, float size, const char* s, float track);
ImVec2 TextSize(ImFont* f, float size, const char* s);

void TextClip(ImDrawList* dl, ImFont* f, float size, ImVec2 leftMid, float maxW, ImU32 c, const char* s);

enum Icon {
    ICON_AIM = 0,
    ICON_EYE,
    ICON_TUNE,
    ICON_GRID,
    ICON_PALETTE,
    ICON_CHECK,
    ICON_CHEVRON,
    ICON_BOLT,
    ICON_CLOCK,
    ICON_SHIELD,
    ICON_DOT,
    ICON_KEY,
};
void DrawIcon(ImDrawList* dl, int icon, ImVec2 c, float size, ImU32 col, float th);

}
}
