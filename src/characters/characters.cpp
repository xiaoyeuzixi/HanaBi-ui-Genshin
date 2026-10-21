#include "characters/characters.h"
namespace hb {
namespace { Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);} }
Skin BaseCharacterSkin() {
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

const CharacterOpeningDef* FindCharacterOpening(Opening opening){
    static const CharacterOpeningDef defs[]={ganyu::OpeningInfo(),yae_miko::OpeningInfo(),yoimiya::OpeningInfo(),nahida::OpeningInfo(),furina::OpeningInfo()};
    for(const auto& def:defs)if(def.opening==opening)return &def;
    return nullptr;
}
}
