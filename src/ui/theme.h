#pragma once

#include "gfx/color.h"
#include "imgui.h"

namespace hb {

struct Skin {

    Rgba backdrop0, backdrop1;
    Rgba win0, win1;
    Rgba winStroke;
    Rgba winStroke2;
    Rgba sidebar0, sidebar1;
    Rgba hairline, divider;
    Rgba scrim;
    Rgba shadow;

    Rgba card0, card1;
    Rgba cardStroke, cardInnerHi, cardInnerLo;
    Rgba knob0, knob1;
    Rgba trackOff0, trackOff1, trackOn0, trackOn1;
    Rgba segTrack, segPill, segStroke;
    Rgba field0, field1, fieldStroke;

    Rgba text, text2, text3;
    Rgba accent, accent2, accentSoft, accentGlow;
    Rgba good, warn;

    float radius = 20.f;
    float cardRadius = 16.f;
    float shadowK = 1.f;
    float light = 0.f;
    float strokeW = 1.f;
};

enum class Element { None, Cryo, Electro, Pyro, Dendro, Hydro, Anemo, Geo };
enum class Scenery { Plain, Lanterns, Sakura, Fireworks, Sanctuary, Opera, Archive, Stone };
enum class Opening { Element, FaruzanMechanism, ZhongliMeteor, GanyuFrostLotus, YaeSesshouSakura, YoimiyaGoldfish, NahidaDreamSanctuary, FurinaWaterStage };
struct ThemeText {
    const char* constellation="H A N A B I   /   S T U D I O";
    const char* signature="你的工作空间";
    const char* sceneDetail="按自己的喜好，调整空间的细节";
    const char* particleLabel="微光粒子";
    const char* motto="A QUIET MOMENT";
};
struct ThemeDef {
    const char* nameCn;
    const char* nameEn;
    const char* tag;
    Skin skin;
    const char* key;
    bool character;
    Element element;
    Scenery scenery;
    const char* caption;
    Opening opening=Opening::Element;
    ThemeText text;
    ImU32 preview0=0,preview1=0;
};

constexpr int kThemeCount = 9;
constexpr int kGanyuTheme = 0, kLightTheme = 1, kDarkTheme = 2, kYaeTheme = 3, kYoimiyaTheme = 4, kNahidaTheme = 5, kFurinaTheme = 6, kFaruzanTheme = 7, kZhongliTheme = 8;

const ThemeDef& ThemeAt(int i);
inline bool IsCharacterTheme(int i) { return i >= 0 && i < kThemeCount && ThemeAt(i).character; }

Skin MixSkin(const Skin& a, const Skin& b, float t);

Skin BuildSkin(const Skin& from, const Skin& to, float chromeT, float cardT, float textT);

Skin WithAccent(Skin s, const Rgba& accent, const Rgba& accent2);

inline ImU32 Col(const Rgba& c) {
    const int r = int(c.r * 255.f + .5f), g = int(c.g * 255.f + .5f), b = int(c.b * 255.f + .5f);
    const int a = int(c.a * 255.f + .5f);
    return IM_COL32(r < 0 ? 0 : (r > 255 ? 255 : r), g < 0 ? 0 : (g > 255 ? 255 : g),
                    b < 0 ? 0 : (b > 255 ? 255 : b), a < 0 ? 0 : (a > 255 ? 255 : a));
}

inline ImU32 Fade(ImU32 c, float k) {
    const float a = float((c >> IM_COL32_A_SHIFT) & 0xFF) * (k < 0.f ? 0.f : (k > 1.f ? 1.f : k));
    return (c & ~IM_COL32_A_MASK) | (ImU32(a + .5f) << IM_COL32_A_SHIFT);
}

inline ImU32 Lighten(ImU32 c, float k) {
    const int r = (c >> IM_COL32_R_SHIFT) & 0xFF, g = (c >> IM_COL32_G_SHIFT) & 0xFF,
              b = (c >> IM_COL32_B_SHIFT) & 0xFF, a = (c >> IM_COL32_A_SHIFT) & 0xFF;
    auto up = [k](int v) { return int(v + (255 - v) * k + .5f); };
    return IM_COL32(up(r), up(g), up(b), a);
}

}
