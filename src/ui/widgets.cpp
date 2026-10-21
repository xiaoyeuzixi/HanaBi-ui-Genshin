#include "ui/widgets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "ui/paint.h"

namespace hb {
namespace {

constexpr float kPi = 3.14159265358979323846f;

inline float LerpF(float a, float b, float t) { return a + (b - a) * t; }

inline ImU32 A(ImU32 c, float k) { return paint::SetA(c, int(((c >> IM_COL32_A_SHIFT) & 0xFF) * k + .5f)); }

inline ImU32 Blend(ImU32 a, ImU32 b, float t) { return paint::MixI(a, b, t); }

void ShadowOf(UiCtx& c, const Rc& r, float rad, float sigma, float dy, float k) {
    paint::Shadow(c.dl, r.a, r.b, rad, c.C(c.sk.shadow), c.Px(dy), c.Px(sigma), k * c.sk.shadowK);
}

void InnerEdge(UiCtx& c, const Rc& r, float rad, ImU32 col, float inset) {
    if (((col >> IM_COL32_A_SHIFT) & 0xFF) == 0) return;
    paint::Stroke(c.dl, ImVec2(r.a.x + inset, r.a.y + inset), ImVec2(r.b.x - inset, r.b.y - inset),
                  rad > inset ? rad - inset : 0.f, col, 1.f);
}

void StatusDot(UiCtx& c, ImVec2 p, float rad, ImU32 col, float glowK) {
    if (glowK > 0.01f) paint::Glow(c.dl, p, rad * 3.2f, A(col, 0.36f * glowK), 3);
    c.dl->AddCircleFilled(p, rad, col, 0);
    c.dl->AddCircleFilled(ImVec2(p.x, p.y - rad * 0.28f), rad * 0.55f, IM_COL32(255, 255, 255, 70), 0);
}

}

Rc Card(UiCtx& c, int id, Rc r, bool lift) {
    float rad=c.Px(c.sk.cardRadius);
    paint::Shadow(c.dl,r.a,r.b,rad,Fade(c.C(c.sk.shadow),.09f),c.Px(1.5f),c.Px(12),.65f);
    paint::FillGrad(c.dl,r.a,r.b,rad,c.C(c.sk.card0),c.C(c.sk.card1));
    paint::Stroke(c.dl,r.a,r.b,rad,c.C(c.sk.cardStroke),1);
    paint::TopHighlight(c.dl,r.a,r.b,rad,Fade(c.C(c.sk.cardInnerHi),.45f));
    return r.In(c.Px(20));
}

void SectionLabel(UiCtx& c, Rc r, const char* caps, const char* cn) {
    ImDrawList* dl = c.dl;
    const float fs = c.Fs(9.5f);
    const float tr = c.Px(1.5f);
    float x = r.a.x;
    if (caps && caps[0]) {
        const ImU32 cc = A(c.C(c.sk.text3), 1.f);
        paint::TextTracked(dl, c.fCaps, fs, ImVec2(x, r.c().y - fs * 0.62f), cc, caps, tr);
        x += paint::TrackedWidth(c.fCaps, fs, caps, tr) + c.Px(9);
    }

    if (r.b.x - x > c.Px(12)) {
        const ImU32 lc = c.C(c.sk.divider);
        dl->AddRectFilledMultiColor(ImVec2(x, r.c().y), ImVec2(r.b.x, r.c().y + 1.f), lc, paint::SetA(lc, 0),
                                    paint::SetA(lc, 0), lc);
    }
    if (cn && cn[0]) {
        const float cnfs = c.Fs(11.5f);
        const ImVec2 ts = paint::TextSize(c.fSm, cnfs, cn);
        paint::Text(dl, c.fSm, cnfs, ImVec2(r.b.x - ts.x, r.c().y - ts.y * 0.5f), A(c.C(c.sk.text3), 0.9f), cn);
    }
}

bool Toggle(UiCtx& c, int id, Rc r, bool& on, bool big) {
    auto& w=c.St(id);Rc hit=r.In(-c.Px(4),-c.Px(5));
    if(c.Claim(id,hit))on=!on;
    bool active=c.IsActive(id)&&c.down;
    w.val.Target(on?1.f:0.f);w.val.Update(c.dt,springs::S_SNAP);
    w.prs.Target(active?1.f:0.f);w.prs.Update(c.dt,springs::S_SNAP);
    w.hov.Target(c.Hover(hit)?1.f:0.f);w.hov.Update(c.dt);
    float k=std::clamp(w.val.V(on?1.f:0.f),0.f,1.f),rad=r.h()*.5f;
    paint::FillGrad(c.dl,r.a,r.b,rad,Blend(c.C(c.sk.trackOff0),c.C(c.sk.trackOn0),k),Blend(c.C(c.sk.trackOff1),c.C(c.sk.trackOn1),k));
    paint::Stroke(c.dl,r.a,r.b,rad,Fade(c.C(c.sk.cardStroke),.6f),1);
    float kr=rad-c.Px(2.5f),kx=LerpF(r.a.x+rad,r.b.x-rad,k);
    paint::Knob(c.dl,ImVec2(kx,r.c().y),kr*(1+w.prs.V()*.035f),c.C(c.sk.knob0),c.C(c.sk.knob1),c.C(c.sk.shadow),c.Px(1),c.Px(2),.6f);
    if(c.Hover(hit))ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    return active;
}

bool Slider(UiCtx& c, int id, Rc labelRow, Rc track, float& v, float vmin, float vmax, const char* label,
            const char* unit, int decimals) {
    ImDrawList* dl = c.dl;
    WState& w = c.St(id);
    const float span = (vmax - vmin) != 0.f ? (vmax - vmin) : 1.f;
    const float railH = c.Px(4);
    const float knobR = c.Px(7);
    const float y = track.c().y;

    Rc hit(track.a.x - knobR, y - c.Px(12), track.b.x + knobR, y + c.Px(12));
    if (c.pressed && c.Hover(hit) && c.activeId < 0) c.activeId = id;
    const bool active = c.IsActive(id);
    if (active && c.down) {
        float k = (c.mouse.x - track.a.x) / (track.w() > 0.f ? track.w() : 1.f);
        k = k < 0.f ? 0.f : (k > 1.f ? 1.f : k);
        v = vmin + k * span;
    }
    if(active && !c.down)c.activeId=-1;
    const float ratio = (v - vmin) / span;
    w.val.Target(ratio < 0.f ? 0.f : (ratio > 1.f ? 1.f : ratio));
    w.hov.Target((c.Hover(hit) && c.activeId < 0) ? 1.f : 0.f);
    w.prs.Target(active ? 1.f : 0.f);

    if (active) w.val.Snap(ratio); else w.val.Update(c.dt, springs::S_UI);
    w.hov.Update(c.dt, springs::S_UI);
    w.prs.Update(c.dt, springs::S_SNAP);
    const float k = w.val.V(ratio);
    const float hv = w.hov.V(0.f);
    const float pv = w.prs.V(0.f);

    const float lfs = c.Fs(12.5f);
    paint::TextV(dl, c.fSm, lfs, ImVec2(labelRow.a.x, labelRow.c().y), A(c.C(c.sk.text2), 1.f), label);

    char buf[48];
    if (decimals <= 0)
        snprintf(buf, sizeof(buf), "%d", int(v + (v < 0 ? -0.5f : 0.5f)));
    else
        snprintf(buf, sizeof(buf), "%.*f", decimals, v);
    char num[64];
    if (unit && unit[0])
        snprintf(num, sizeof(num), "%s %s", buf, unit);
    else
        snprintf(num, sizeof(num), "%s", buf);
    const float bigFs = c.Fs(12.0f) * (1.f + 0.04f * pv);
    const ImVec2 ts = paint::TextSize(c.fSm, bigFs, num);
    paint::Text(dl, c.fSm, bigFs, ImVec2(labelRow.b.x - ts.x, labelRow.c().y - ts.y * 0.5f),
                Blend(c.C(c.sk.text2), c.C(c.sk.accent), pv * 0.75f), num);

    const float filled = track.w() * k;
    paint::FillGrad(dl, ImVec2(track.a.x, y - railH * 0.5f), ImVec2(track.b.x, y + railH * 0.5f), railH * 0.5f,
                    c.C(c.sk.trackOff0), c.C(c.sk.trackOff1));
    if (filled > 0.5f) {
        const ImU32 a0 = c.C(c.sk.accent), a1 = c.C(c.sk.accent2);
        const ImU32 a2 = paint::MixI(a0, a1, 0.5f);
        dl->AddRectFilledMultiColor(ImVec2(track.a.x, y - railH * 0.5f), ImVec2(track.a.x + filled, y + railH * 0.5f),
                                    a0, a1, a1, a2);

        dl->AddRectFilledMultiColor(ImVec2(track.a.x, y - railH * 0.5f),
                                    ImVec2(track.a.x + filled, y - railH * 0.5f + 1.f),
                                    IM_COL32(255, 255, 255, 70), IM_COL32(255, 255, 255, 40),
                                    IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 0));

        paint::Glow(dl, ImVec2(track.a.x + filled, y), knobR * 2.6f, A(c.C(c.sk.accent), (0.06f + 0.10f * pv)*(1.f-c.sk.light*.8f)), 3);
    }

    const float kr = knobR * (1.f + 0.10f * pv + 0.05f * hv);
    paint::Knob(dl, ImVec2(track.a.x + filled, y), kr, c.C(c.sk.knob0), c.C(c.sk.knob1), Fade(c.C(c.sk.shadow),.25f),
                c.Px(1.f), c.Px(3.f), .7f);
    if (pv > 0.01f) paint::Ring(dl, ImVec2(track.a.x + filled, y), kr + c.Px(1.5f), A(c.C(c.sk.accent), 0.5f * pv), 1.f);
    return active;
}

bool Segment(UiCtx& c, int id, Rc r, int& sel, const char* const* items, int n) {
    ImDrawList* dl = c.dl;
    if (n <= 0) return false;
    WState& w = c.St(id);
    const float rad = r.h() * 0.42f;
    const float cellW = r.w() / float(n);

    int hoverIdx = -1;
    if (c.Hover(r)) hoverIdx = std::clamp(int((c.mouse.x - r.a.x) / cellW),0,n-1);
    if (c.pressed && hoverIdx >= 0 && c.Hover(r.In(-c.Px(2), -c.Px(4))) && c.activeId < 0) {
        if (sel != hoverIdx) {

            sel = hoverIdx;
        }
        c.activeId = id;
    }
    const bool held = c.IsActive(id);
    if (held && !c.down) c.activeId = -1;

    w.val.Target(float(sel));
    w.val.Update(c.dt, springs::S_SNAP);

    const float selF = w.val.V(float(sel));

    paint::Fill(dl, r.a, r.b, rad, c.C(c.sk.segTrack));

    for(int i=0;i<n&&i<8;i++) {
        auto& hover=w.itemHover[i];
        hover.Target(hoverIdx==i&&sel!=i?1.f:0.f);hover.Update(c.dt,springs::S_CRIT);
        const Rc hr(r.a.x + cellW * i + c.Px(3), r.a.y + c.Px(3), r.a.x + cellW * (i + 1) - c.Px(3),
                    r.b.y - c.Px(3));
        paint::Fill(dl, hr.a, hr.b, rad * 0.8f, A(c.C(c.sk.accent), 0.055f*hover.V()));
    }

    const float pillW = cellW - c.Px(6);
    const float cx = r.a.x + cellW * (selF + 0.5f);
    const Rc pill(cx - pillW * 0.5f, r.a.y + c.Px(3), cx + pillW * 0.5f, r.b.y - c.Px(3));
    paint::Shadow(dl, pill.a, pill.b, rad * 0.8f, Fade(c.C(c.sk.shadow),.18f), c.Px(1.f), c.Px(3.f), .5f);
    paint::FillGrad(dl, pill.a, pill.b, rad * 0.8f,
                    c.C(c.sk.segPill),c.C(c.sk.segPill));
    paint::Stroke(dl, pill.a, pill.b, rad * 0.8f, c.C(c.sk.segStroke), 1.f);
    if (c.sk.light > 0.5f) {
        InnerEdge(c, pill, rad * 0.8f, IM_COL32(255, 255, 255, 220), 1.f);
    } else {

        const float hiW = pill.w() - rad * 1.4f;
        if (hiW > 4.f)
            dl->AddRectFilledMultiColor(ImVec2(pill.a.x + rad * 0.7f, pill.a.y + 1.f),
                                        ImVec2(pill.a.x + rad * 0.7f + hiW, pill.a.y + 2.f),
                                        IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 110),
                                        IM_COL32(255, 255, 255, 110), IM_COL32(255, 255, 255, 0));
    }

    float fs = c.Fs(12.5f);
    for(int i=0;i<n;++i){const float width=paint::TextSize(c.fSm,fs,items[i]).x;if(width>cellW-c.Px(10))fs*=std::max(.1f,cellW-c.Px(10))/width;}
    for (int i = 0; i < n; ++i) {
        const float ccx = r.a.x + cellW * (i + 0.5f);
        const ImU32 col = c.C(i==sel?c.sk.text:c.sk.text2);
        paint::TextC(dl, c.fSm, fs, ImVec2(ccx, r.c().y + c.Px(0.5f)), col, items[i]);
    }
    return held;
}

bool Chip(UiCtx& c, int id, Rc r, bool& sel, const char* label, int icon) {
    ImDrawList* dl = c.dl;
    WState& w = c.St(id);
    const float rad = r.h() * 0.5f;
    const bool over = c.Hover(r);
    if (c.pressed && over && c.activeId < 0) {
        sel = !sel;
        c.activeId = id;
    }
    const bool held = c.IsActive(id);
    if (held && !c.down) c.activeId = -1;
    w.val.Target(sel ? 1.f : 0.f);
    w.hov.Target((over && c.activeId < 0) ? 1.f : 0.f);
    w.prs.Target(held ? 1.f : 0.f);
    w.val.Update(c.dt, springs::S_SNAP);
    w.hov.Update(c.dt, springs::S_UI);
    w.prs.Update(c.dt, springs::S_SNAP);
    const float k = w.val.V(0.f), hv = w.hov.V(0.f), pv = w.prs.V(0.f);

    const float s = 1.f - 0.02f * pv;
    const Rc rr(r.c().x - r.w() * 0.5f * s, r.c().y - r.h() * 0.5f * s, r.c().x + r.w() * 0.5f * s,
                r.c().y + r.h() * 0.5f * s);

    paint::Shadow(dl, rr.a, rr.b, rad, c.C(c.sk.shadow), c.Px(2.f), c.Px(5.f), 0.55f + 0.45f * hv);
    const ImU32 fillU = Blend(Blend(c.C(c.sk.field0), c.C(c.sk.field1), 0.4f), c.C(c.sk.accentSoft), k);
    paint::FillGrad(dl, rr.a, rr.b, rad, fillU, A(fillU, 0.78f));
    paint::Stroke(dl, rr.a, rr.b, rad, Blend(c.C(c.sk.fieldStroke), c.C(c.sk.accent), k * 0.85f), 1.f);
    if (k > 0.02f) {
        paint::GlowEllipse(dl, rr.c(), rr.w() * 0.55f, rr.h() * 0.75f, A(c.C(c.sk.accent), 0.16f * k), 3);
    }

    const ImU32 fg = Blend(c.C(c.sk.text2), c.C(c.sk.accent), k);
    const float fs = c.Fs(12.5f);
    const float iconSz = c.Px(11.f);
    const ImVec2 tsz = paint::TextSize(c.fSm, fs, label);
    const float gap = c.Px(5.f);
    const float totalW = (icon >= 0 ? iconSz + gap : 0.f) + tsz.x;
    float x = rr.c().x - totalW * 0.5f;
    if (icon >= 0) {
        paint::DrawIcon(dl, icon, ImVec2(x + iconSz * 0.5f, rr.c().y), iconSz, fg, c.Px(1.2f));
        x += iconSz + gap;
    }
    paint::TextV(dl, c.fSm, fs, ImVec2(x, rr.c().y + c.Px(0.5f)), fg, label);
    return held;
}

bool Keybind(UiCtx& c, int id, Rc r, int keySlot, const char* const* keyNames, int keyCount, bool& recording) {
    ImDrawList* dl = c.dl;
    WState& w = c.St(id);
    const float rad = c.Px(8.f);
    const bool over = c.Hover(r);
    if (c.pressed && over && c.activeId < 0) c.activeId = id;
    const bool held = c.IsActive(id);
    if (held && !c.down) c.activeId = -1;

    w.hov.Target((over && c.activeId < 0) ? 1.f : 0.f);
    w.prs.Target(held ? 1.f : 0.f);
    w.val.Target(recording ? 1.f : 0.f);
    w.hov.Update(c.dt, springs::S_UI);
    w.prs.Update(c.dt, springs::S_SNAP);
    w.val.Update(c.dt, springs::S_SNAP);
    const float hv = w.hov.V(0.f), pv = w.prs.V(0.f), rk = w.val.V(0.f);

    const float s = 1.f - 0.02f * pv;
    const Rc rr(r.c().x - r.w() * 0.5f * s, r.c().y - r.h() * 0.5f * s, r.c().x + r.w() * 0.5f * s,
                r.c().y + r.h() * 0.5f * s);
    paint::FillGrad(dl, rr.a, rr.b, rad, Blend(c.C(c.sk.field0), c.C(c.sk.accentSoft), rk * 0.65f),
                    Blend(c.C(c.sk.field1), c.C(c.sk.accentSoft), rk * 0.35f));

    const float breath = 0.5f + 0.5f * std::sin(c.t * (2.f * kPi / 1.2f));
    ImU32 strokeCol;
    if (rk > 0.01f) {
        strokeCol = paint::SetA(c.C(c.sk.accent), int(LerpF(60.f, 140.f, breath) * rk + 60.f * (1.f - rk)));
    } else {
        strokeCol = Blend(c.C(c.sk.fieldStroke), c.C(c.sk.accent), hv * 0.45f);
    }
    paint::Stroke(dl, rr.a, rr.b, rad, strokeCol, rk > 0.5f ? c.Px(1.4f) : 1.f);
    if (rk > 0.01f) paint::GlowEllipse(dl, rr.c(), rr.w() * 0.6f, rr.h() * 0.9f, A(c.C(c.sk.accent), 0.13f * rk * breath), 3);

    const char* nm = "—";
    if (recording) {
        nm = "...";
    } else if (keyNames && keySlot >= 0 && keySlot < keyCount) {
        nm = keyNames[keySlot];
    }
    const float fs = c.Fs(11.5f);
    const ImU32 fg = Blend(c.C(c.sk.text2), c.C(c.sk.text), LerpF(0.55f, 1.f, rk));
    paint::TextC(dl, c.fSm, fs, ImVec2(rr.c().x, rr.c().y + c.Px(0.5f)), fg, nm);
    return held;
}

void StatBlock(UiCtx& c, Rc r, const char* label, const char* value, const char* unit, const float* hist,
               int histN, ImU32 lineCol) {
    ImDrawList* dl = c.dl;
    paint::Text(dl, c.fSm, c.Fs(11.5f), ImVec2(r.a.x, r.a.y), A(c.C(c.sk.text3), 1.f), label);
    const float vfs = c.Fs(22.f);
    const ImVec2 vs = paint::TextSize(c.fLg, vfs, value);
    paint::Text(dl, c.fLg, vfs, ImVec2(r.a.x - c.Px(0.5f), r.a.y + c.Px(12.f)), c.C(c.sk.text), value);
    if (unit && unit[0]) {
        paint::Text(dl, c.fSm, c.Fs(10.5f), ImVec2(r.a.x + vs.x + c.Px(3.f), r.a.y + c.Px(12.f) + vfs * 0.62f),
                    A(c.C(c.sk.text3), 1.f), unit);
    }

    if (hist && histN > 1) {
        const Rc sp(r.a.x, r.b.y - c.Px(20.f), r.b.x, r.b.y);
        const int n = histN;
        const float stepX = sp.w() / float(n - 1);
        static ImVec2 pts[64];
        if (n <= 64) {
            for (int i = 0; i < n; ++i) {
                const float hv = hist[i] < 0.f ? 0.f : (hist[i] > 1.f ? 1.f : hist[i]);
                pts[i] = ImVec2(sp.a.x + stepX * float(i), sp.b.y - c.Px(1.f) - hv * sp.h());
            }

            static ImVec2 poly[68];
            for (int i = 0; i < n; ++i) poly[i] = pts[i];
            poly[n] = ImVec2(sp.b.x, sp.b.y + c.Px(1.f));
            poly[n + 1] = ImVec2(sp.a.x, sp.b.y + c.Px(1.f));
            ImU32 top = paint::SetA(lineCol, 60);
            dl->AddConcavePolyFilled(poly, n + 2, top);
            dl->AddPolyline(pts, n, lineCol, 0, c.Px(1.5f));
            dl->AddCircleFilled(pts[n - 1], c.Px(2.2f), lineCol, 0);
            paint::Glow(dl, pts[n - 1], c.Px(7.f), paint::SetA(lineCol, 90), 3);
        }
    }
}

bool ThemeCard(UiCtx& c, int id, Rc r, int themeIdx, bool selected) {
    ImDrawList* dl = c.dl;
    WState& w = c.St(id);
    const ThemeDef& td = ThemeAt(themeIdx);
    const float rad = c.Px(12.f);
    const bool over = c.Hover(r);
    w.hov.Target((over && c.activeId < 0) ? 1.f : 0.f);
    w.val.Target(selected ? 1.f : 0.f);
    w.hov.Update(c.dt, springs::S_UI);
    w.val.Update(c.dt, springs::S_SNAP);
    const float hv = w.hov.V(0.f), k = w.val.V(0.f);

    const float blockH = c.Px(52.f);
    const Rc blk(r.a.x, r.a.y, r.b.x, r.a.y + blockH);
    const Rc rr(blk.a.x, blk.a.y - c.Px(1.f) * hv, blk.b.x, blk.b.y - c.Px(1.f) * hv);

    paint::Shadow(dl, rr.a, rr.b, rad, c.C(td.skin.shadow), c.Px(3.f + 2.f * hv), c.Px(7.f + 4.f * hv),
                  0.85f + 0.45f * hv);

    const Rgba w0 = td.skin.win0, w1 = td.skin.win1;
    paint::FillGrad(dl, rr.a, rr.b, rad, Col(WithA(w0, 1.f)), Col(WithA(w1, 1.f)));

    paint::GlowEllipse(dl, ImVec2(rr.c().x, rr.b.y - rr.h() * 0.25f), rr.w() * 0.55f, rr.h() * 0.5f,
                       Col(WithA(td.skin.accent, 0.16f)), 3);

    const float dotR = c.Px(9.f);
    paint::Glow(dl, rr.c(), dotR * 2.2f, Col(WithA(td.skin.accent, 0.30f)), 3);
    dl->AddCircleFilled(rr.c(), dotR, Col(td.skin.accent), 0);
    dl->AddCircleFilled(ImVec2(rr.c().x, rr.c().y - dotR * 0.25f), dotR * 0.62f,
                        IM_COL32(255, 255, 255, 46), 0);

    if (k > 0.02f) {
        paint::Stroke(dl, rr.a, rr.b, rad, paint::SetA(c.C(c.sk.accent), int(200 * k + .5f)), c.Px(1.6f) * (0.4f + 0.6f * k));
        paint::Glow(dl, ImVec2(rr.b.x - c.Px(9.f), rr.a.y + c.Px(9.f)), c.Px(9.f),
                    paint::SetA(c.C(c.sk.accent), int(120 * k + .5f)), 3);
        dl->AddCircleFilled(ImVec2(rr.b.x - c.Px(9.f), rr.a.y + c.Px(9.f)), c.Px(3.2f) * k, c.C(c.sk.accent), 0);
    } else {
        paint::Stroke(dl, rr.a, rr.b, rad, Blend(c.C(c.sk.cardStroke), c.C(c.sk.accent), hv * 0.4f), 1.f);
    }

    const float fs = c.Fs(11.f);
    paint::TextC(dl, c.fSm, fs, ImVec2(r.c().x, r.b.y - c.Px(8.f)), Blend(c.C(c.sk.text3), c.C(c.sk.text), k),
                 td.nameCn);
    return over;
}

void EspPreview(UiCtx& c, Rc r, float t) {
    ImDrawList* dl = c.dl;
    const float rad = c.Px(10.f);
    dl->PushClipRect(r.a, r.b, true);

    const float hor = r.a.y + r.h() * 0.56f;
    const ImU32 skyTop = IM_COL32(8, 14, 26, 255);
    const ImU32 skyMid = IM_COL32(22, 40, 64, 255);
    const ImU32 skyHor = IM_COL32(40, 66, 96, 255);
    dl->AddRectFilledMultiColor(ImVec2(r.a.x, r.a.y), ImVec2(r.b.x, hor), skyTop, skyTop, skyMid, skyMid);
    dl->AddRectFilledMultiColor(ImVec2(r.a.x, hor - r.h() * 0.30f), ImVec2(r.b.x, hor), paint::SetA(skyMid, 0),
                                paint::SetA(skyMid, 0), skyHor, skyHor);

    const ImU32 ac = c.C(c.sk.accent);
    paint::GlowEllipse(dl, ImVec2(r.a.x + r.w() * 0.66f, hor - r.h() * 0.04f), r.w() * 0.34f, r.h() * 0.10f,
                       paint::SetA(ac, 46), 5);

    dl->AddRectFilledMultiColor(ImVec2(r.a.x, hor), ImVec2(r.b.x, r.b.y), IM_COL32(10, 17, 28, 255),
                                IM_COL32(10, 17, 28, 255), IM_COL32(6, 10, 18, 255), IM_COL32(6, 10, 18, 255));

    for (int i = 0; i < 5; ++i) {
        const float tt = float(i) / 5.f;
        const float yy = hor + (r.b.y - hor) * tt * tt;
        dl->AddLine(ImVec2(r.a.x, yy), ImVec2(r.b.x, yy), paint::SetA(ac, int(9 + 9 * tt)), 1.f);
    }
    for (int i = -2; i <= 2; ++i) {
        const float x = r.c().x + float(i) * r.w() * 0.16f;
        const float x2 = r.c().x + float(i) * r.w() * 0.44f;
        dl->AddLine(ImVec2(x, hor), ImVec2(x2, r.b.y), paint::SetA(ac, 14), 1.f);
    }

    {

        const ImVec2 l[7] = {ImVec2(r.a.x - 4, hor + c.Px(6.f)),
                             ImVec2(r.a.x + r.w() * 0.09f, hor - r.h() * 0.20f),
                             ImVec2(r.a.x + r.w() * 0.17f, hor - r.h() * 0.04f),
                             ImVec2(r.a.x + r.w() * 0.27f, hor - r.h() * 0.23f),
                             ImVec2(r.a.x + r.w() * 0.38f, hor + r.h() * 0.02f),
                             ImVec2(r.a.x + r.w() * 0.38f, r.b.y), ImVec2(r.a.x - 4, r.b.y)};
        for (int i = 0; i < 7; ++i) dl->PathLineTo(l[i]);
        dl->PathFillConvex(IM_COL32(9, 15, 26, 246));

        dl->PathLineTo(ImVec2(r.b.x + 4, hor + c.Px(4.f)));
        dl->PathLineTo(ImVec2(r.b.x - r.w() * 0.11f, hor - r.h() * 0.26f));
        dl->PathLineTo(ImVec2(r.b.x - r.w() * 0.20f, hor - r.h() * 0.03f));
        dl->PathLineTo(ImVec2(r.b.x - r.w() * 0.31f, hor - r.h() * 0.16f));
        dl->PathLineTo(ImVec2(r.b.x - r.w() * 0.42f, hor + r.h() * 0.03f));
        dl->PathLineTo(ImVec2(r.b.x - r.w() * 0.42f, r.b.y));
        dl->PathLineTo(ImVec2(r.b.x + 4, r.b.y));
        dl->PathFillConvex(IM_COL32(8, 13, 23, 220));

        dl->AddLine(ImVec2(r.a.x + r.w() * 0.09f, hor - r.h() * 0.20f),
                    ImVec2(r.a.x + r.w() * 0.13f, hor - r.h() * 0.11f), paint::SetA(ac, 34), 1.f);
        dl->AddLine(ImVec2(r.b.x - r.w() * 0.11f, hor - r.h() * 0.26f),
                    ImVec2(r.b.x - r.w() * 0.155f, hor - r.h() * 0.15f), paint::SetA(ac, 40), 1.f);
    }

    {
        const float bob = std::sin(t * 1.7f) * c.Px(1.6f);
        const float bw = r.w() * 0.19f, bh = r.h() * 0.30f;

        const ImVec2 ctr(r.c().x + r.w() * 0.14f, hor - bh * 0.50f + bob);
        const Rc box(ctr.x - bw * 0.5f, ctr.y - bh * 0.5f, ctr.x + bw * 0.5f, ctr.y + bh * 0.5f);
        const float tick = std::min(bw, bh) * 0.28f;
        const ImU32 boxCol = IM_COL32(255, 255, 255, 226);
        const float th = c.Px(1.4f);

        dl->AddLine(box.a, ImVec2(box.a.x + tick, box.a.y), boxCol, th);
        dl->AddLine(box.a, ImVec2(box.a.x, box.a.y + tick), boxCol, th);
        dl->AddLine(ImVec2(box.b.x - tick, box.a.y), ImVec2(box.b.x, box.a.y), boxCol, th);
        dl->AddLine(ImVec2(box.b.x, box.a.y), ImVec2(box.b.x, box.a.y + tick), boxCol, th);
        dl->AddLine(ImVec2(box.a.x, box.b.y - tick), ImVec2(box.a.x, box.b.y), boxCol, th);
        dl->AddLine(ImVec2(box.a.x, box.b.y), ImVec2(box.a.x + tick, box.b.y), boxCol, th);
        dl->AddLine(ImVec2(box.b.x - tick, box.b.y), ImVec2(box.b.x, box.b.y), boxCol, th);
        dl->AddLine(ImVec2(box.b.x, box.b.y - tick), ImVec2(box.b.x, box.b.y), boxCol, th);

        const Rc bar(box.a.x, box.a.y - c.Px(7.f), box.b.x, box.a.y - c.Px(4.f));
        dl->AddRectFilled(bar.a, bar.b, IM_COL32(0, 0, 0, 140), c.Px(1.5f));
        const float hp = 0.72f + 0.05f * std::sin(t * 0.9f);
        const Rc barF(bar.a.x, bar.a.y, bar.a.x + bar.w() * hp, bar.b.y);
        dl->AddRectFilledMultiColor(barF.a, barF.b, IM_COL32(120, 240, 200, 255), IM_COL32(150, 255, 214, 255),
                                    IM_COL32(90, 220, 180, 255), IM_COL32(110, 235, 195, 255));
        paint::Glow(dl, barF.c(), c.Px(7.f), IM_COL32(140, 255, 210, 55), 3);

        paint::TextC(dl, c.fCaps, c.Fs(8.5f), ImVec2(ctr.x, bar.a.y - c.Px(8.f)), IM_COL32(230, 244, 255, 235),
                     "KANO");
        char dist[24];
        snprintf(dist, sizeof(dist), "%.1f m", 38.4f + std::sin(t * 1.1f) * 0.6f);
        paint::TextC(dl, c.fXs, c.Fs(8.f), ImVec2(ctr.x, box.b.y + c.Px(8.f)), paint::SetA(ac, 200), dist);

        const ImVec2 head(ctr.x, box.a.y + bh * 0.14f);
        const ImVec2 neck(ctr.x, box.a.y + bh * 0.27f);
        const ImVec2 pelvis(ctr.x, box.a.y + bh * 0.62f);
        const ImVec2 lh(ctr.x - bw * 0.34f, box.a.y + bh * 0.52f);
        const ImVec2 rh(ctr.x + bw * 0.34f, box.a.y + bh * 0.52f);
        const ImVec2 lf(ctr.x - bw * 0.20f, box.b.y - c.Px(1.f));
        const ImVec2 rf(ctr.x + bw * 0.20f, box.b.y - c.Px(1.f));
        const ImU32 bone = paint::SetA(ac, 150);
        const float bth = c.Px(1.1f);
        dl->AddLine(neck, pelvis, bone, bth);
        dl->AddLine(neck, lh, bone, bth);
        dl->AddLine(neck, rh, bone, bth);
        dl->AddLine(pelvis, lf, bone, bth);
        dl->AddLine(pelvis, rf, bone, bth);
        const ImVec2 joints[6] = {head, neck, pelvis, lh, rh, lf};
        for (int i = 0; i < 6; ++i) {
            dl->AddCircleFilled(joints[i], c.Px(1.6f), IM_COL32(255, 255, 255, 220), 0);
            dl->AddCircle(joints[i], c.Px(3.4f), paint::SetA(ac, 70), 0, 1.f);
        }
        dl->AddCircle(head, c.Px(4.6f), paint::SetA(ac, 120), 0, c.Px(1.2f));
    }

    {
        const ImVec2 cc(r.c().x - r.w() * 0.16f, hor - r.h() * 0.10f);
        const float R = c.Px(11.f);
        const ImU32 cc2 = IM_COL32(255, 255, 255, 210);
        const float gap = c.Px(2.5f);
        dl->AddLine(ImVec2(cc.x - R, cc.y), ImVec2(cc.x - gap, cc.y), cc2, c.Px(1.3f));
        dl->AddLine(ImVec2(cc.x + gap, cc.y), ImVec2(cc.x + R, cc.y), cc2, c.Px(1.3f));
        dl->AddLine(ImVec2(cc.x, cc.y - R), ImVec2(cc.x, cc.y - gap), cc2, c.Px(1.3f));
        dl->AddLine(ImVec2(cc.x, cc.y + gap), ImVec2(cc.x, cc.y + R), cc2, c.Px(1.3f));
        dl->AddCircle(cc, R * 0.66f, paint::SetA(ac, 170), 0, c.Px(1.1f));
        dl->AddCircleFilled(cc, c.Px(1.5f), cc2, 0);
        paint::Glow(dl, cc, R * 1.3f, paint::SetA(ac, 40), 2);
    }

    dl->AddRectFilledMultiColor(ImVec2(r.a.x, r.a.y), ImVec2(r.b.x, r.a.y + r.h() * 0.22f), IM_COL32(0, 0, 0, 90),
                                IM_COL32(0, 0, 0, 90), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0));
    dl->AddRectFilledMultiColor(ImVec2(r.a.x, r.b.y - r.h() * 0.20f), ImVec2(r.b.x, r.b.y), IM_COL32(0, 0, 0, 0),
                                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 110), IM_COL32(0, 0, 0, 110));
    dl->PopClipRect();

    paint::Stroke(dl, r.a, r.b, rad, c.C(c.sk.cardStroke), 1.f);
}

bool Slots(UiCtx& c, int idBase, Rc r, int& sel, int n, int cols) {
    ImDrawList* dl = c.dl;
    const int nCols = cols < 1 ? 1 : cols;
    const float sz = c.Px(18.f);
    const float gap = c.Px(9.f);
    const int rows = (n + nCols - 1) / nCols;
    const float totalW = float(nCols) * sz + float(nCols - 1) * gap;
    const float totalH = float(rows) * sz + float(rows - 1) * gap;
    const float x0 = r.c().x - totalW * 0.5f;
    const float y0 = r.c().y - totalH * 0.5f;
    bool changed = false;
    for (int i = 0; i < n; ++i) {
        const int col = i % nCols, row = i / nCols;
        const Rc s(x0 + float(col) * (sz + gap), y0 + float(row) * (sz + gap),
                   x0 + float(col) * (sz + gap) + sz, y0 + float(row) * (sz + gap) + sz);
        WState& w = c.St(idBase + i);
        const bool over = c.Hover(s);
        if (c.pressed && over && c.activeId < 0) {
            sel = i;
            changed = true;
            c.activeId = idBase + i;
        }
        if (c.IsActive(idBase + i) && !c.down) c.activeId = -1;
        w.val.Target(i == sel ? 1.f : 0.f);
        w.hov.Target((over && c.activeId < 0) ? 1.f : 0.f);
        w.val.Update(c.dt, springs::S_SNAP);
        w.hov.Update(c.dt, springs::S_UI);
        const float k = w.val.V(0.f), hv = w.hov.V(0.f);
        const float rad2 = c.Px(5.f);
        if (k > 0.01f) paint::Glow(dl, s.c(), sz * 0.9f, paint::SetA(c.C(c.sk.accent), int(60 * k + .5f)), 2);
        paint::FillGrad(dl, s.a, s.b, rad2, Blend(c.C(c.sk.field0), c.C(c.sk.accentSoft), k),
                        Blend(c.C(c.sk.field1), A(c.C(c.sk.accentSoft), 0.7f), k));
        paint::Stroke(dl, s.a, s.b, rad2, Blend(Blend(c.C(c.sk.fieldStroke), c.C(c.sk.accent), hv * 0.5f),
                                                c.C(c.sk.accent), k * 0.9f), 1.f);
        if (k > 0.35f) {
            paint::DrawIcon(dl, paint::ICON_CHECK, s.c(), sz * 0.60f, paint::SetA(c.C(c.sk.accent), int(255 * (k - 0.35f) / 0.65f)), c.Px(1.2f));
        } else {
            char num[4];
            snprintf(num, sizeof(num), "%d", i + 1);
            paint::TextC(dl, c.fXs, c.Fs(8.5f), s.c(), A(c.C(c.sk.text3), 1.f - k), num);
        }
    }
    return changed;
}

void RingGauge(UiCtx& c, Rc r, float p, const char* cap, const char* val) {
    ImDrawList* dl = c.dl;
    const ImVec2 cc = r.c();
    const float rad = std::min(r.w(), r.h()) * 0.5f - c.Px(2.f);
    dl->AddCircle(cc, rad, paint::SetA(c.C(c.sk.text3), 60), 0, c.Px(2.5f));
    if (p > 0.001f) {
        dl->PathArcTo(cc, rad, -kPi * 0.5f, -kPi * 0.5f + kPi * 2.f * (p > 1.f ? 1.f : p), 32);
        dl->PathStroke(c.C(c.sk.accent), 0, c.Px(2.5f));
    }
    if (val) paint::TextC(dl, c.fSm, c.Fs(8.5f), ImVec2(cc.x, cc.y - c.Px(3.f)), c.C(c.sk.text), val);
    if (cap) paint::TextC(dl, c.fXs, c.Fs(7.5f), ImVec2(cc.x, cc.y + c.Px(7.f)), A(c.C(c.sk.text3), 1.f), cap);
}

void Dot(UiCtx& c, ImVec2 p, float rad, ImU32 col, float glowK) { StatusDot(c, p, rad, col, glowK); }

}
