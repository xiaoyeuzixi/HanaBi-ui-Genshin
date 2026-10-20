#include "ui/theme.h"

#include <cmath>

namespace hb {
namespace {

inline Rgba C(uint32_t hex, int a) { return Hex(hex, float(a) / 255.f); }

inline Rgba A(const Rgba& c, int a) { return Rgba{c.r, c.g, c.b, float(a) / 255.f}; }

Skin MakeGanyu() {
    Skin s;

    s.backdrop0 = C(0x060B14, 24);
    s.backdrop1 = C(0x0A101C, 40);
    s.win0 = C(0x0C1A30, 12);
    s.win1 = C(0x091525, 34);
    s.winStroke = C(0x8FE6F6, 54);
    s.winStroke2 = C(0x000000, 60);
    s.sidebar0 = C(0x0C192B, 115);
    s.sidebar1 = C(0x091321, 160);
    s.hairline = C(0x8FE6F6, 22);
    s.divider = C(0xEAF4FF, 16);
    s.scrim = C(0x000000, 70);
    s.shadow = C(0x02060E, 90);

    s.card0 = C(0x152A41, 84);
    s.card1 = C(0x102137, 114);
    s.cardStroke = C(0xA0CDE7, 40);
    s.cardInnerHi = C(0xFFFFFF, 34);
    s.cardInnerLo = C(0x000000, 44);
    s.knob0 = C(0xFFFFFF, 255);
    s.knob1 = C(0xE3F3FA, 255);
    s.trackOff0 = C(0xFFFFFF, 34);
    s.trackOff1 = C(0xFFFFFF, 22);
    s.trackOn0 = C(0x8FE6F6, 255);
    s.trackOn1 = C(0x5FC9E8, 255);
    s.segTrack = C(0x000000, 56);
    s.segPill = C(0xFFFFFF, 30);
    s.segStroke = C(0xFFFFFF, 34);
    s.field0 = C(0xFFFFFF, 10);
    s.field1 = C(0xFFFFFF, 5);
    s.fieldStroke = C(0xFFFFFF, 22);

    s.text = C(0xEAF4FF, 255);
    s.text2 = C(0xBED2E5, 255);
    s.text3 = C(0x94ABC4, 255);
    s.accent = C(0x8FE6F6, 255);
    s.accent2 = C(0xB6A8FF, 255);
    s.accentSoft = C(0x8FE6F6, 34);
    s.accentGlow = C(0x8FE6F6, 120);
    s.good = C(0x6FF0C0, 255);
    s.warn = C(0xFFC978, 255);

    s.radius = 20.f;
    s.cardRadius = 14.f;
    s.shadowK = 1.f;
    s.light = 0.f;
    s.strokeW = 1.f;
    return s;
}

Skin MakeMica() {
    Skin s;

    s.backdrop0 = C(0xF0F2F6, 255);
    s.backdrop1 = C(0xE8ECF1, 255);
    s.win0 = C(0xFBFCFE, 255);
    s.win1 = C(0xF1F3F8, 255);
    s.winStroke = C(0xFFFFFF, 235);
    s.winStroke2 = C(0x1E283C, 40);
    s.sidebar0 = C(0xF7F9FC, 236);
    s.sidebar1 = C(0xEDF0F6, 242);
    s.hairline = C(0x1E283C, 26);
    s.divider = C(0x1E283C, 24);
    s.scrim = C(0x1E283C, 16);
    s.shadow = C(0x1E283C, 84);

    s.card0 = C(0xFFFFFF, 255);
    s.card1 = C(0xFBFCFE, 250);
    s.cardStroke = C(0x1E283C, 24);
    s.cardInnerHi = C(0xFFFFFF, 245);
    s.cardInnerLo = C(0x1E283C, 18);
    s.knob0 = C(0xFFFFFF, 255);
    s.knob1 = C(0xF2F5FA, 255);
    s.trackOff0 = C(0x1E283C, 40);
    s.trackOff1 = C(0x1E283C, 26);
    s.trackOn0 = C(0x0A84FF, 255);
    s.trackOn1 = C(0x3FA0FF, 255);
    s.segTrack = C(0x1E283C, 26);
    s.segPill = C(0xFFFFFF, 255);
    s.segStroke = C(0x1E283C, 30);
    s.field0 = C(0x1E283C, 14);
    s.field1 = C(0x1E283C, 7);
    s.fieldStroke = C(0x1E283C, 24);

    s.text = C(0x111A2B, 255);
    s.text2 = C(0x5B6678, 255);
    s.text3 = C(0x98A2B3, 255);
    s.accent = C(0x0A84FF, 255);
    s.accent2 = C(0x5AC8FA, 255);
    s.accentSoft = C(0x0A84FF, 30);
    s.accentGlow = C(0x0A84FF, 90);
    s.good = C(0x12B981, 255);
    s.warn = C(0xE8A33D, 255);

    s.radius = 20.f;
    s.cardRadius = 16.f;
    s.shadowK = 1.25f;
    s.light = 1.f;
    s.strokeW = 1.f;
    return s;
}

Skin MakeObsidian() {
    Skin s;
    s.backdrop0 = C(0x101114, 255);
    s.backdrop1 = C(0x08090B, 255);
    s.win0 = C(0x171B21, 255);
    s.win1 = C(0x101317, 255);
    s.winStroke = C(0xFFFFFF, 30);
    s.winStroke2 = C(0x000000, 90);
    s.sidebar0 = C(0x13161B, 228);
    s.sidebar1 = C(0x0C0E11, 238);
    s.hairline = C(0x4DA3FF, 26);
    s.divider = C(0xFFFFFF, 16);
    s.scrim = C(0x000000, 74);
    s.shadow = C(0x000000, 205);

    s.card0 = C(0xFFFFFF, 16);
    s.card1 = C(0xFFFFFF, 8);
    s.cardStroke = C(0xFFFFFF, 16);
    s.cardInnerHi = C(0xFFFFFF, 28);
    s.cardInnerLo = C(0x000000, 60);
    s.knob0 = C(0xFFFFFF, 255);
    s.knob1 = C(0xE9EDF3, 255);
    s.trackOff0 = C(0xFFFFFF, 30);
    s.trackOff1 = C(0xFFFFFF, 18);
    s.trackOn0 = C(0x4DA3FF, 255);
    s.trackOn1 = C(0x2E7BE0, 255);
    s.segTrack = C(0x000000, 70);
    s.segPill = C(0xFFFFFF, 26);
    s.segStroke = C(0xFFFFFF, 28);
    s.field0 = C(0xFFFFFF, 12);
    s.field1 = C(0xFFFFFF, 6);
    s.fieldStroke = C(0xFFFFFF, 20);

    s.text = C(0xF5F6F8, 255);
    s.text2 = C(0x9BA3AF, 255);
    s.text3 = C(0x5E6672, 255);
    s.accent = C(0x4DA3FF, 255);
    s.accent2 = C(0x9BE0FF, 255);
    s.accentSoft = C(0x4DA3FF, 32);
    s.accentGlow = C(0x4DA3FF, 110);
    s.good = C(0x3DDC97, 255);
    s.warn = C(0xFFB74D, 255);

    s.radius = 20.f;
    s.cardRadius = 16.f;
    s.shadowK = 1.15f;
    s.light = 0.f;
    s.strokeW = 1.f;
    return s;
}

ThemeDef g_defs[kThemeCount] = {
    {"甘雨", "GANYU", "CRYO", MakeGanyu()},
    {"云母", "MICA", "LIGHT", MakeMica()},
    {"曜石", "OBSIDIAN", "DARK", MakeObsidian()},
};

inline Rgba MixC(const Rgba& a, const Rgba& b, float t) { return MixLinear(a, b, t); }
inline float MixF(float a, float b, float t) { return a + (b - a) * t; }

}

const ThemeDef& ThemeAt(int i) { return g_defs[(i < 0 || i >= kThemeCount) ? 0 : i]; }

Skin MixSkin(const Skin& a, const Skin& b, float t) {
    if (t <= 0.f) return a;
    if (t >= 1.f) return b;
    Skin r;
#define MIX(f) r.f = MixC(a.f, b.f, t)
    MIX(backdrop0); MIX(backdrop1); MIX(win0); MIX(win1); MIX(winStroke); MIX(winStroke2);
    MIX(sidebar0); MIX(sidebar1); MIX(hairline); MIX(divider); MIX(scrim); MIX(shadow);
    MIX(card0); MIX(card1); MIX(cardStroke); MIX(cardInnerHi); MIX(cardInnerLo);
    MIX(knob0); MIX(knob1); MIX(trackOff0); MIX(trackOff1); MIX(trackOn0); MIX(trackOn1);
    MIX(segTrack); MIX(segPill); MIX(segStroke); MIX(field0); MIX(field1); MIX(fieldStroke);
    MIX(text); MIX(text2); MIX(text3);
    MIX(accent); MIX(accent2); MIX(accentSoft); MIX(accentGlow); MIX(good); MIX(warn);
#undef MIX
    r.radius = MixF(a.radius, b.radius, t);
    r.cardRadius = MixF(a.cardRadius, b.cardRadius, t);
    r.shadowK = MixF(a.shadowK, b.shadowK, t);
    r.light = MixF(a.light, b.light, t);
    r.strokeW = MixF(a.strokeW, b.strokeW, t);
    return r;
}

Skin BuildSkin(const Skin& from, const Skin& to, float chromeT, float cardT, float textT) {
    Skin r = from;

#define MIXC(f) r.f = MixC(from.f, to.f, chromeT)
    MIXC(backdrop0); MIXC(backdrop1); MIXC(win0); MIXC(win1); MIXC(winStroke); MIXC(winStroke2);
    MIXC(sidebar0); MIXC(sidebar1); MIXC(hairline); MIXC(divider); MIXC(scrim); MIXC(shadow);
    r.radius = MixF(from.radius, to.radius, chromeT);
    r.shadowK = MixF(from.shadowK, to.shadowK, chromeT);
    r.light = MixF(from.light, to.light, chromeT);
#undef MIXC

#define MIXK(f) r.f = MixC(from.f, to.f, cardT)
    MIXK(card0); MIXK(card1); MIXK(cardStroke); MIXK(cardInnerHi); MIXK(cardInnerLo);
    MIXK(knob0); MIXK(knob1); MIXK(trackOff0); MIXK(trackOff1); MIXK(trackOn0); MIXK(trackOn1);
    MIXK(segTrack); MIXK(segPill); MIXK(segStroke); MIXK(field0); MIXK(field1); MIXK(fieldStroke);
    r.cardRadius = MixF(from.cardRadius, to.cardRadius, cardT);
#undef MIXK

#define MIXT(f) r.f = MixC(from.f, to.f, textT)
    MIXT(text); MIXT(text2); MIXT(text3);
    MIXT(accent); MIXT(accent2); MIXT(accentSoft); MIXT(accentGlow); MIXT(good); MIXT(warn);
#undef MIXT
    r.strokeW = MixF(from.strokeW, to.strokeW, textT);
    return r;
}

Skin WithAccent(Skin s, const Rgba& accent, const Rgba& accent2) {
    s.accent = accent;
    s.accent2 = accent2;
    s.accentSoft = WithA(accent, 0.13f * (1.f - s.light * 0.55f) + 0.045f);
    s.accentGlow = WithA(accent, 0.46f);
    s.trackOn0 = accent;
    s.trackOn1 = Mix(accent, accent2, 0.55f);
    s.trackOn1.a = accent.a;
    return s;
}

}
