#pragma once

#include <cfloat>

#include "core/spring.h"
#include "imgui.h"
#include "ui/theme.h"

namespace hb {

struct Rc {
    ImVec2 a{0, 0}, b{0, 0};
    Rc() = default;
    Rc(float x0, float y0, float x1, float y1) : a(x0, y0), b(x1, y1) {}
    Rc(ImVec2 p0, ImVec2 p1) : a(p0), b(p1) {}
    float w() const { return b.x - a.x; }
    float h() const { return b.y - a.y; }
    ImVec2 c() const { return ImVec2((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f); }
    ImVec2 size() const { return ImVec2(b.x - a.x, b.y - a.y); }
    Rc In(float d) const { return Rc(a.x + d, a.y + d, b.x - d, b.y - d); }
    Rc In(float dx, float dy) const { return Rc(a.x + dx, a.y + dy, b.x - dx, b.y - dy); }
    Rc Off(float dx, float dy) const { return Rc(a.x + dx, a.y + dy, b.x + dx, b.y + dy); }
    bool Has(ImVec2 p) const { return p.x >= a.x && p.x <= b.x && p.y >= a.y && p.y <= b.y; }
    Rc Clip(const Rc& o) const {
        return Rc(a.x > o.a.x ? a.x : o.a.x, a.y > o.a.y ? a.y : o.a.y, b.x < o.b.x ? b.x : o.b.x,
                  b.y < o.b.y ? b.y : o.b.y);
    }
};

struct Smooth {
    Chan ch;
    bool init = false;

    void Target(float t) { Retarget(ch, t); }
    void Snap(float v) { SnapTo(ch, v); init = true; }
    float V() const { return ch.x; }
    float V(float from) const { return init ? ch.x : from; }

    void Update(float dt, const SpringSpec& s = springs::S_UI) {
        if (!init) { SnapTo(ch, ch.target); init = true; return; }
        if (dt <= 0.f) return;
        constexpr float kH = 1.f / 240.f;
        float remain = dt > 0.1f ? 0.1f : dt;
        while (remain > 1e-6f) {
            const float step = remain > kH ? kH : remain;
            Step(ch, s, step);
            remain -= step;
        }
        ch.prev = ch.x;

        if (ch.v * ch.v < 1e-6f && (ch.x - ch.target) * (ch.x - ch.target) < 1e-7f) {
            ch.x = ch.target;
            ch.v = 0.f;
        }
    }
};

struct WState {
    Smooth hov, prs, act, val, aux;
    Smooth itemHover[8];
    float  raw = 0.f;
};

struct EnterUnit {
    bool   init = false;
    Chan   shift;
    Smooth scale;
    Smooth alpha;
};

constexpr int kMaxWid = 320;

struct UiCtx {
    ImDrawList* dl = nullptr;

    ImFont* fUi = nullptr;
    ImFont* fSm = nullptr;
    ImFont* fXs = nullptr;
    ImFont* fLg = nullptr;
    ImFont* fXl = nullptr;
    ImFont* fCaps = nullptr;
    ImFont* fDisp = nullptr;

    float sc = 1.f;
    float textScale = 1.12f;
    float t = 0.f;
    float dt = 0.f;
    Skin sk;

    ImVec2 mouse{0, 0};
    bool down = false, pressed = false, released = false;
    bool interactive = true;
    int  hotId = -1;
    int  activeId = -1;

    WState* pool = nullptr;
    int     poolN = 0;

    float Px(float v) const { return v * sc; }
    float Fs(float v) const { return v * sc * textScale; }
    ImU32 C(const Rgba& c) const { return Col(c); }

    WState& St(int id) {
        static WState dummy;
        if (!pool || id < 0 || id >= poolN) return dummy;
        return pool[id];
    }

    bool Hover(const Rc& r) const {
        return interactive && mouse.x >= r.a.x && mouse.x <= r.b.x && mouse.y >= r.a.y && mouse.y <= r.b.y;
    }

    bool Claim(int id, const Rc& r) {
        if (!interactive) return false;
        const bool over = r.Has(mouse);
        if (over) hotId = id;
        if (pressed && over && activeId < 0) {
            activeId = id;
            return true;
        }
        return false;
    }
    bool IsActive(int id) const { return activeId == id; }
};

int  VtxBegin(ImDrawList* dl);
void VtxApply(ImDrawList* dl, int start, ImVec2 pivot, float scale, ImVec2 shift, float alpha);

}
