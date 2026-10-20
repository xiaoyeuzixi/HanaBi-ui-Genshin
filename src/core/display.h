#pragma once

#include <algorithm>

namespace hb {

struct DisplayFit {
    int   screenW = 0, screenH = 0;
    int   workW = 0, workH = 0;
    float dpiScale = 1.0f;
    float uiScale = 1.0f;
    int   winW = 1050, winH = 680;
    int   pxW = 1050, pxH = 680;
    int   posX = 0, posY = 0;
};

constexpr float kBaseW = 1050.f;
constexpr float kBaseH = 680.f;

constexpr float kMaxFillW = 0.86f;
constexpr float kMaxFillH = 0.88f;

constexpr float kMinScale = 0.80f;
constexpr float kMaxScale = 1.60f;

inline DisplayFit FitToDisplay(int screenW, int screenH, int workW, int workH, float dpiScale) {
    DisplayFit f;
    f.screenW = screenW;
    f.screenH = screenH;
    f.workW = workW > 0 ? workW : screenW;
    f.workH = workH > 0 ? workH : screenH;
    f.dpiScale = dpiScale > 0.f ? dpiScale : 1.f;

    const float fitW = (float(f.workW) * kMaxFillW) / kBaseW;
    const float fitH = (float(f.workH) * kMaxFillH) / kBaseH;
    float s = std::min({f.dpiScale, fitW, fitH, kMaxScale});
    s = std::max(s, kMinScale);
    f.uiScale = s;

    f.pxW = (int)(kBaseW * s + 0.5f);
    f.pxH = (int)(kBaseH * s + 0.5f);

    f.winW = (int)(kBaseW + 0.5f);
    f.winH = (int)(kBaseH + 0.5f);

    f.posX = (f.workW - f.pxW) / 2;
    f.posY = (int)((f.workH - f.pxH) * 0.46f);
    return f;
}

}
