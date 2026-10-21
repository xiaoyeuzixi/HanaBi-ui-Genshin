#pragma once
#include "ui/theme.h"
#include "intro/crystal_material.h"
namespace hb {
struct Intro;
struct CharacterOpeningDef {
    Opening opening;
    Element element;
    float radius;
    unsigned dark,base,glow;
    void (*scene)(Intro&,ImDrawList*,ImVec2,float,float,float);
    std::vector<CrystalVertex> (*glyph)(ImVec2,float,float);
    void (*backdrop)(Intro&,ImDrawList*,ImVec2);
};
Skin BaseCharacterSkin();
const CharacterOpeningDef* FindCharacterOpening(Opening);
namespace ganyu { ThemeDef MakeTheme(); CharacterOpeningDef OpeningInfo(); }
namespace yae_miko { ThemeDef MakeTheme(); CharacterOpeningDef OpeningInfo(); }
namespace yoimiya { ThemeDef MakeTheme(); CharacterOpeningDef OpeningInfo(); }
namespace nahida { ThemeDef MakeTheme(); CharacterOpeningDef OpeningInfo(); }
namespace furina { ThemeDef MakeTheme(); CharacterOpeningDef OpeningInfo(); }
namespace faruzan { ThemeDef MakeTheme(); }
namespace zhongli { ThemeDef MakeTheme(); }
}
