#pragma once

#include "ui/ui_ctx.h"

namespace hb {

Rc Card(UiCtx& c, int id, Rc r, bool hoverLift = true);

void SectionLabel(UiCtx& c, Rc r, const char* caps, const char* cn);

bool Toggle(UiCtx& c, int id, Rc r, bool& on, bool big = false);

bool Slider(UiCtx& c, int id, Rc labelRow, Rc track, float& v, float vmin, float vmax,
            const char* label, const char* unit, int decimals);

bool Segment(UiCtx& c, int id, Rc r, int& sel, const char* const* items, int n);

bool Chip(UiCtx& c, int id, Rc r, bool& sel, const char* label, int icon);

bool Keybind(UiCtx& c, int id, Rc r, int keySlot, const char* const* keyNames, int keyCount, bool& recording);

void StatBlock(UiCtx& c, Rc r, const char* label, const char* value, const char* unit, const float* hist,
               int histN, ImU32 lineCol);

bool ThemeCard(UiCtx& c, int id, Rc r, int themeIdx, bool selected);

void EspPreview(UiCtx& c, Rc r, float t);

bool Slots(UiCtx& c, int idBase, Rc r, int& sel, int n, int cols = 6);

void RingGauge(UiCtx& c, Rc r, float p, const char* cap, const char* val);
void Dot(UiCtx& c, ImVec2 p, float rad, ImU32 col, float glowK);

}
