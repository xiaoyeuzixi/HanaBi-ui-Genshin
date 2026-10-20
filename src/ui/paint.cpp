#include "ui/paint.h"

#include <cmath>

namespace hb {
namespace paint {
namespace {

constexpr float kPi = 3.14159265358979323846f;

template <typename F>
void RoundRect(ImVec2 a, ImVec2 b, float r, F&& emit) {
    const float w = b.x - a.x, h = b.y - a.y;
    if (r < 0.f) r = 0.f;
    const float maxR = (w < h ? w : h) * 0.5f;
    if (r > maxR) r = maxR;
    if (r <= 0.01f) {
        emit(ImVec2(a.x, a.y));
        emit(ImVec2(b.x, a.y));
        emit(ImVec2(b.x, b.y));
        emit(ImVec2(a.x, b.y));
        return;
    }
    int seg = int(r * 0.75f) + 3;
    if (seg < 4) seg = 4;
    if (seg > 10) seg = 10;
    const ImVec2 cs[4] = {ImVec2(b.x - r, a.y + r), ImVec2(b.x - r, b.y - r), ImVec2(a.x + r, b.y - r),
                           ImVec2(a.x + r, a.y + r)};
    const float base[4] = {-kPi * 0.5f, 0.f, kPi * 0.5f, kPi};
    for (int c = 0; c < 4; ++c) {
        for (int i = 0; i <= seg; ++i) {
            const float ang = base[c] + (kPi * 0.5f) * (float(i) / float(seg));
            emit(ImVec2(cs[c].x + std::cos(ang) * r, cs[c].y + std::sin(ang) * r));
        }
    }
}

template <typename F>
void RoundRectInset(ImVec2 a, ImVec2 b, float r, float in, F&& emit) {
    RoundRect(ImVec2(a.x + in, a.y + in), ImVec2(b.x - in, b.y - in), r > in ? r - in : 0.f, emit);
}

constexpr int kMaxPts = 64;

}

ImU32 MixI(ImU32 a, ImU32 b, float t) {
    if (t <= 0.f) return a;
    if (t >= 1.f) return b;
    const int ar = (a >> IM_COL32_R_SHIFT) & 0xFF, ag = (a >> IM_COL32_G_SHIFT) & 0xFF,
              ab = (a >> IM_COL32_B_SHIFT) & 0xFF, aa = (a >> IM_COL32_A_SHIFT) & 0xFF;
    const int br = (b >> IM_COL32_R_SHIFT) & 0xFF, bg = (b >> IM_COL32_G_SHIFT) & 0xFF,
              bb = (b >> IM_COL32_B_SHIFT) & 0xFF, ba = (b >> IM_COL32_A_SHIFT) & 0xFF;
    return IM_COL32(int(ar + (br - ar) * t + .5f), int(ag + (bg - ag) * t + .5f),
                    int(ab + (bb - ab) * t + .5f), int(aa + (ba - aa) * t + .5f));
}

ImU32 SetA(ImU32 c, int a) {
    if (a < 0) a = 0;
    if (a > 255) a = 255;
    return (c & ~IM_COL32_A_MASK) | (ImU32(a) << IM_COL32_A_SHIFT);
}

void Fill(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 c) {
    if (((c >> IM_COL32_A_SHIFT) & 0xFF) == 0) return;
    dl->AddRectFilled(a, b, c, r);
}

void FillGrad(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 top, ImU32 bot) {
    const bool ta = ((top >> IM_COL32_A_SHIFT) & 0xFF) != 0;
    const bool ba = ((bot >> IM_COL32_A_SHIFT) & 0xFF) != 0;
    if (!ta && !ba) return;

    if (top == bot) {
        dl->AddRectFilled(a, b, top, r);
        return;
    }
    ImVec2 points[kMaxPts];
    int count = 0;
    RoundRect(a, b, r, [&](ImVec2 p) { points[count++] = p; });
    const int first = dl->VtxBuffer.Size;
    dl->AddConvexPolyFilled(points, count, IM_COL32_WHITE);
    const float height = b.y - a.y;
    for (int i = first; i < dl->VtxBuffer.Size; ++i) {
        ImDrawVert& v = dl->VtxBuffer[i];
        const float k = height > 0.f ? (v.pos.y - a.y) / height : 0.f;
        const ImU32 color = MixI(top, bot, k);
        const int aa = int((v.col >> IM_COL32_A_SHIFT) & 255);
        v.col = SetA(color, int(((color >> IM_COL32_A_SHIFT) & 255) * aa / 255));
    }
}

void Stroke(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 c, float th) {
    if (((c >> IM_COL32_A_SHIFT) & 0xFF) == 0 || th <= 0.f) return;
    dl->AddRect(a, b, c, r, ImDrawFlags_RoundCornersAll, th);
}

void Stroke2(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 outer, ImU32 inner, float th) {
    if (th <= 0.f) return;
    Stroke(dl, a, b, r, outer, th);
    if (((inner >> IM_COL32_A_SHIFT) & 0xFF) == 0) return;
    Stroke(dl, ImVec2(a.x + th, a.y + th), ImVec2(b.x - th, b.y - th), r > th ? r - th : 0.f, inner, th);
}

void TopHighlight(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 c) {
    const int a0 = (c >> IM_COL32_A_SHIFT) & 0xFF;
    if (a0 == 0) return;

    const float x0 = a.x + r, x1 = b.x - r;
    if (x1 - x0 < 2.f) return;
    const float y = a.y + 0.5f;
    dl->AddRectFilledMultiColor(ImVec2(x0, y), ImVec2(x1, y + 1.f), SetA(c, 0), SetA(c, 0), SetA(c, 0), SetA(c, 0));

    dl->AddRectFilled(ImVec2(x0 + 8.f, y), ImVec2(x1 - 8.f, y + 1.f), c);
    dl->AddRectFilledMultiColor(ImVec2(x0, y), ImVec2(x0 + 8.f, y + 1.f), SetA(c, 0), c, c, SetA(c, 0));
    dl->AddRectFilledMultiColor(ImVec2(x1 - 8.f, y), ImVec2(x1, y + 1.f), c, SetA(c, 0), SetA(c, 0), c);
}

void Shadow(ImDrawList* dl, ImVec2 a, ImVec2 b, float r, ImU32 col, float dy, float sigma, float k) {
    const int a0 = (col >> IM_COL32_A_SHIFT) & 0xFF;
    if (a0 == 0 || k <= 0.f) return;

    static const float kOff[4] = {0.30f, 0.62f, 1.05f, 1.70f};
    static const float kAlpha[4] = {0.52f, 0.30f, 0.17f, 0.09f};
    for (int i = 0; i < 4; ++i) {
        const float g = sigma * kOff[i] * k;
        const ImVec2 mn(a.x - g, a.y - g + dy * (0.45f + 0.35f * float(i)));
        const ImVec2 mx(b.x + g, b.y + g + dy * (0.45f + 0.35f * float(i)));
        const ImU32 c = SetA(col, int(a0 * kAlpha[i] + .5f));
        dl->AddRectFilled(mn, mx, c, r + g * 0.92f);
    }
}

void Glow(ImDrawList* dl, ImVec2 center, float radius, ImU32 col, int layers) {
    const int a0 = (col >> IM_COL32_A_SHIFT) & 0xFF;
    if (a0 == 0 || layers <= 0) return;
    for (int i = layers; i >= 1; --i) {
        const float t = float(i) / float(layers);
        const float rad = radius * (0.55f + 0.45f * t) + 1.f;

        const ImU32 c = SetA(col, int(a0 * (0.16f + 0.14f * (1.f - t)) + .5f));
        dl->AddCircleFilled(center, rad, c, 0);
    }
}

void GlowEllipse(ImDrawList* dl, ImVec2 center, float rx, float ry, ImU32 col, int layers) {
    const int a0 = (col >> IM_COL32_A_SHIFT) & 0xFF;
    if (a0 == 0 || layers <= 0) return;
    for (int i = layers; i >= 1; --i) {
        const float t = float(i) / float(layers);
        const float s = 0.72f + 0.42f * t;
        const ImU32 c = SetA(col, int(a0 * (0.13f + 0.16f * (1.f - t)) + .5f));
        dl->AddEllipseFilled(center, ImVec2(rx * s, ry * s), c, 0.f, 0);
    }
}

void Knob(ImDrawList* dl, ImVec2 c, float radius, ImU32 c0, ImU32 c1, ImU32 shadowCol, float shadowDy,
          float shadowSigma, float k) {

    Shadow(dl, ImVec2(c.x - radius, c.y - radius), ImVec2(c.x + radius, c.y + radius), radius, shadowCol,
           shadowDy, shadowSigma, k);

    dl->AddCircleFilled(ImVec2(c.x, c.y - 0.35f), radius, c0, 0);
    dl->AddCircleFilled(ImVec2(c.x, c.y + radius * 0.42f), radius * 0.96f, SetA(c1, 40), 0);

    dl->PathArcTo(ImVec2(c.x, c.y - radius * 0.04f), radius * 0.78f, -kPi * 0.88f, -kPi * 0.30f, 10);
    dl->PathStroke(IM_COL32(255, 255, 255, 170), 0, radius * 0.16f);
}

void Ring(ImDrawList* dl, ImVec2 c, float r, ImU32 col, float th, int seg) { dl->AddCircle(c, r, col, seg, th); }

ImVec2 TextSize(ImFont* f, float size, const char* s) {
    if (!f || !s) return ImVec2(0, 0);
    return f->CalcTextSizeA(size, FLT_MAX, 0.f, s);
}

void Text(ImDrawList* dl, ImFont* f, float size, ImVec2 pos, ImU32 c, const char* s) {
    if (!f || !s || !s[0]) return;
    dl->AddText(f, size, pos, c, s);
}

void TextC(ImDrawList* dl, ImFont* f, float size, ImVec2 center, ImU32 c, const char* s) {
    if (!f || !s) return;
    const ImVec2 ts = TextSize(f, size, s);
    dl->AddText(f, size, ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f), c, s);
}

void TextR(ImDrawList* dl, ImFont* f, float size, ImVec2 rightMid, ImU32 c, const char* s) {
    if (!f || !s) return;
    const ImVec2 ts = TextSize(f, size, s);
    dl->AddText(f, size, ImVec2(rightMid.x - ts.x, rightMid.y - ts.y * 0.5f), c, s);
}

void TextV(ImDrawList* dl, ImFont* f, float size, ImVec2 leftMid, ImU32 c, const char* s) {
    if (!f || !s) return;
    const ImVec2 ts = TextSize(f, size, s);
    dl->AddText(f, size, ImVec2(leftMid.x, leftMid.y - ts.y * 0.5f), c, s);
}

void TextTracked(ImDrawList* dl, ImFont* f, float size, ImVec2 pos, ImU32 c, const char* s, float track) {
    if (!f || !s) return;
    float x = pos.x;
    const char* p = s;
    while (*p) {

        unsigned char ch = (unsigned char)*p;
        int len = ch < 0x80 ? 1 : (ch >> 5) == 0x6 ? 2 : (ch >> 4) == 0xE ? 3 : 4;
        const char* next = p + len;
        const float gw = f->CalcTextSizeA(size, FLT_MAX, 0.f, p, next).x;
        dl->AddText(f, size, ImVec2(x, pos.y), c, p, next);
        x += gw + track;
        p = next;
    }
}

float TrackedWidth(ImFont* f, float size, const char* s, float track) {
    if (!f || !s) return 0.f;
    float x = 0.f;
    int n = 0;
    const char* p = s;
    while (*p) {
        unsigned char ch = (unsigned char)*p;
        int len = ch < 0x80 ? 1 : (ch >> 5) == 0x6 ? 2 : (ch >> 4) == 0xE ? 3 : 4;
        x += f->CalcTextSizeA(size, FLT_MAX, 0.f, p, p + len).x;
        p += len;
        ++n;
    }
    if (n > 1) x += track * float(n - 1);
    return x;
}

void TextClip(ImDrawList* dl, ImFont* f, float size, ImVec2 leftMid, float maxW, ImU32 c, const char* s) {
    if (!f || !s) return;
    const ImVec2 ts = TextSize(f, size, s);
    if (ts.x <= maxW) {
        dl->AddText(f, size, ImVec2(leftMid.x, leftMid.y - ts.y * 0.5f), c, s);
        return;
    }

    const char* p = s;
    float x = 0.f;
    const char* cut = s;
    while (*p) {
        unsigned char ch = (unsigned char)*p;
        int len = ch < 0x80 ? 1 : (ch >> 5) == 0x6 ? 2 : (ch >> 4) == 0xE ? 3 : 4;
        const float gw = f->CalcTextSizeA(size, FLT_MAX, 0.f, p, p + len).x;
        if (x + gw > maxW - 8.f) break;
        x += gw;
        p += len;
        cut = p;
    }
    dl->AddText(f, size, ImVec2(leftMid.x, leftMid.y - ts.y * 0.5f), c, s, cut);
    dl->AddText(f, size, ImVec2(leftMid.x + x, leftMid.y - ts.y * 0.5f), c, "...");
}

void DrawIcon(ImDrawList* dl, int icon, ImVec2 c, float size, ImU32 col, float th) {
    const float h = size * 0.5f;
    const int a = (col >> IM_COL32_A_SHIFT) & 0xFF;
    if (a == 0) return;
    const ImU32 dim = SetA(col, a * 55 / 100);
    switch (icon) {
        case ICON_AIM: {
            dl->AddCircle(c, h * 0.70f, col, 0, th);
            dl->AddCircleFilled(c, h * 0.16f, col, 0);
            const float r0 = h * 0.98f, r1 = h * 1.34f;
            dl->AddLine(ImVec2(c.x, c.y - r1), ImVec2(c.x, c.y - r0), col, th);
            dl->AddLine(ImVec2(c.x, c.y + r0), ImVec2(c.x, c.y + r1), col, th);
            dl->AddLine(ImVec2(c.x - r1, c.y), ImVec2(c.x - r0, c.y), col, th);
            dl->AddLine(ImVec2(c.x + r0, c.y), ImVec2(c.x + r1, c.y), col, th);
            break;
        }
        case ICON_EYE: {

            const int seg = 14;
            dl->PathClear();
            for (int i = 0; i <= seg; ++i) {
                const float t = float(i) / float(seg);
                const float ang = kPi * t;
                dl->PathLineTo(ImVec2(c.x - h + 2.f * h * t, c.y - std::sin(ang) * h * 0.62f));
            }
            dl->PathStroke(col, 0, th);
            dl->PathClear();
            for (int i = 0; i <= seg; ++i) {
                const float t = float(i) / float(seg);
                const float ang = kPi * t;
                dl->PathLineTo(ImVec2(c.x - h + 2.f * h * t, c.y + std::sin(ang) * h * 0.62f));
            }
            dl->PathStroke(col, 0, th);
            dl->AddCircleFilled(c, h * 0.30f, col, 0);
            break;
        }
        case ICON_TUNE: {

            const float xs[3] = {c.x - h * 0.62f, c.x, c.x + h * 0.62f};
            const float ps[3] = {0.34f, 0.68f, 0.46f};
            for (int i = 0; i < 3; ++i) {
                dl->AddLine(ImVec2(xs[i], c.y - h), ImVec2(xs[i], c.y + h), dim, th);
                dl->AddCircleFilled(ImVec2(xs[i], c.y - h + 2.f * h * ps[i]), h * 0.26f, col, 0);
            }
            break;
        }
        case ICON_GRID: {
            const float g = h * 0.38f, q = h * 0.88f;
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 2; ++j) {
                    const ImVec2 p0(c.x - q + i * (q + g * 0.4f), c.y - q + j * (q + g * 0.4f));
                    dl->AddRect(p0, ImVec2(p0.x + q * 0.72f, p0.y + q * 0.72f), col, h * 0.18f, 0, th);
                }
            break;
        }
        case ICON_PALETTE: {
            dl->AddCircle(c, h * 0.92f, col, 0, th);
            dl->AddCircleFilled(ImVec2(c.x - h * 0.34f, c.y - h * 0.34f), h * 0.20f, col, 0);
            dl->AddCircleFilled(ImVec2(c.x + h * 0.34f, c.y - h * 0.34f), h * 0.20f, col, 0);
            dl->AddCircleFilled(ImVec2(c.x - h * 0.34f, c.y + h * 0.30f), h * 0.20f, col, 0);
            break;
        }
        case ICON_CHECK: {
            dl->PathLineTo(ImVec2(c.x - h * 0.72f, c.y + h * 0.05f));
            dl->PathLineTo(ImVec2(c.x - h * 0.18f, c.y + h * 0.58f));
            dl->PathLineTo(ImVec2(c.x + h * 0.74f, c.y - h * 0.52f));
            dl->PathStroke(col, 0, th * 1.15f);
            break;
        }
        case ICON_CHEVRON: {
            dl->PathLineTo(ImVec2(c.x - h * 0.45f, c.y - h * 0.28f));
            dl->PathLineTo(ImVec2(c.x, c.y + h * 0.24f));
            dl->PathLineTo(ImVec2(c.x + h * 0.45f, c.y - h * 0.28f));
            dl->PathStroke(col, 0, th);
            break;
        }
        case ICON_BOLT: {
            dl->PathLineTo(ImVec2(c.x + h * 0.18f, c.y - h));
            dl->PathLineTo(ImVec2(c.x - h * 0.62f, c.y + h * 0.12f));
            dl->PathLineTo(ImVec2(c.x - h * 0.06f, c.y + h * 0.12f));
            dl->PathLineTo(ImVec2(c.x - h * 0.20f, c.y + h));
            dl->PathLineTo(ImVec2(c.x + h * 0.62f, c.y - h * 0.14f));
            dl->PathLineTo(ImVec2(c.x + h * 0.06f, c.y - h * 0.14f));
            dl->PathStroke(col, ImDrawFlags_Closed, th);
            break;
        }
        case ICON_CLOCK: {
            dl->AddCircle(c, h * 0.92f, col, 0, th);
            dl->AddLine(ImVec2(c.x, c.y - h * 0.52f), ImVec2(c.x, c.y), col, th);
            dl->AddLine(ImVec2(c.x, c.y), ImVec2(c.x + h * 0.40f, c.y + h * 0.24f), col, th);
            break;
        }
        case ICON_SHIELD: {
            dl->PathLineTo(ImVec2(c.x, c.y - h * 0.95f));
            dl->PathLineTo(ImVec2(c.x + h * 0.85f, c.y - h * 0.55f));
            dl->PathLineTo(ImVec2(c.x + h * 0.62f, c.y + h * 0.55f));
            dl->PathLineTo(ImVec2(c.x, c.y + h));
            dl->PathLineTo(ImVec2(c.x - h * 0.62f, c.y + h * 0.55f));
            dl->PathLineTo(ImVec2(c.x - h * 0.85f, c.y - h * 0.55f));
            dl->PathStroke(col, ImDrawFlags_Closed, th);
            break;
        }
        case ICON_KEY: {
            dl->AddCircle(ImVec2(c.x - h * 0.42f, c.y - h * 0.30f), h * 0.46f, col, 0, th);
            dl->AddLine(ImVec2(c.x - h * 0.10f, c.y + h * 0.02f), ImVec2(c.x + h * 0.82f, c.y + h * 0.86f), col, th);
            dl->AddLine(ImVec2(c.x + h * 0.34f, c.y + h * 0.48f), ImVec2(c.x + h * 0.62f, c.y + h * 0.20f), col, th);
            break;
        }
        default: dl->AddCircleFilled(c, h * 0.34f, col, 0); break;
    }
}

}
}
