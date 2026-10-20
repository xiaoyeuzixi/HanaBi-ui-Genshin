#pragma once

#include "imgui.h"

namespace hb {

struct Fonts {
    ImFont* opening = nullptr;
    ImFont* display = nullptr;
    ImFont* displaySmall = nullptr;
    ImFont* caps = nullptr;
    ImFont* ui = nullptr;
    ImFont* uiSm = nullptr;
    ImFont* uiXs = nullptr;
    ImFont* uiLg = nullptr;
    ImFont* uiXl = nullptr;
};

Fonts& LoadFonts(float scale);

void ReloadFonts(float scale);

}
