#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::nahida {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() {
    Skin s=BaseCharacterSkin();
    s.backdrop0=C(0x0D241A,30);s.backdrop1=C(0x192D1D,42);
    s.win0=C(0x132A20,25);s.win1=C(0x10251C,45);
    s.sidebar0=C(0x142D23,189);s.sidebar1=C(0x10251D,211);
    s.winStroke=C(0xD0E8B3,65);s.hairline=C(0xE0D9A4,43);
    s.card0=C(0x18382A,176);s.card1=C(0x152E22,194);
    s.cardStroke=C(0xC6E2AD,57);s.cardInnerHi=C(0xF5FFE7,25);
    s.text=C(0xF5FBEA,255);s.text2=C(0xD5E3C9,255);s.text3=C(0xA8BCA3,255);
    s.accent=C(0xC3EDA6,255);s.accent2=C(0xDFD3A1,255);
    s.accentSoft=C(0xBAEBA3,35);s.accentGlow=C(0x9FE68C,112);
    s.trackOn0=s.accent;s.trackOn1=C(0x8BCDA2,255);s.segTrack=C(0x07180F,94);
    s.segPill=C(0xD7EDBE,31);s.field0=C(0xF0F9DC,14);s.fieldStroke=C(0xD3E7B8,32);
    s.good=C(0xBFE6AB,255);s.warn=C(0xE6CF93,255);s.radius=20;s.cardRadius=15;
    return s;
}
}
ThemeDef MakeTheme() { return {"纳西妲", "NAHIDA", "DENDRO", MakeSkin(), "nahida", true, Element::Dendro, Scenery::Sanctuary, "新芽", Opening::NahidaDreamSanctuary, {"S A P I E N T I A   O R O M A S D I S", "聆听万物生长", "新芽透玉 · 月莲金纹", "梦境与萤光", "A DREAM TAKES ROOT"}, IM_COL32(23,53,37,245), IM_COL32(53,87,47,245)}; }
}

namespace hb {
void Shell::DrawNahidaUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        Sprig(dl,L(W-65,H-75,0,0).a,Px(47),IM_COL32(215,232,159,int(44*g)),t);
        for(int side:{-1,1})dl->AddBezierCubic(L(W-230,H-28,0,0).a,L(W-140,H-25,0,0).a,L(W-130,H-98,0,0).a,L(W-70+side*30,H-102,0,0).a,IM_COL32(197,222,164,int(25*g)),Px(.7f));
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        const auto inset=r.In(Px(7),Px(5));
        paint::Fill(dl,inset.a,inset.b,Px(10),IM_COL32(17,52,35,int((155+hover*65)*g)));
        paint::Stroke(dl,inset.a,inset.b,Px(10),IM_COL32(210,229,165,int((95+hover*100)*g)),Px(.8f));
        col=Fade(paint::MixI(cx.C(cx.sk.accent),i==2?IM_COL32(255,203,180,255):IM_COL32(248,255,228,255),hover),g);
        dl->AddBezierCubic({p.x-Px(10),p.y-Px(9)},{p.x-Px(4),p.y-Px(5)},{p.x+Px(4),p.y-Px(5)},{p.x+Px(10),p.y-Px(9)},IM_COL32(225,221,158,int(125*g)),Px(.7f));
        if(i==0){dl->AddBezierCubic({p.x-z,p.y},{p.x-z*.2f,p.y+Px(2)},{p.x+z*.4f,p.y+Px(1)},{p.x+z,p.y-Px(1)},col,Px(1.2f));}
        if(i==1){dl->AddRect({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(2),0,Px(1.1f));dl->AddCircleFilled(p,Px(.8f),col);}
        if(i==2){dl->AddLine({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1.2f));dl->AddLine({p.x-z,p.y+z},{p.x+z,p.y-z},col,Px(1.2f));}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Sprig(dl,L(141,mark.c().y/scale+2,0,0).a,Px(7),Fade(cx.C(cx.sk.accent),.5f*g),t);
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        Sprig(dl,L(86,H-124,0,0).a,Px(26),Fade(cx.C(cx.sk.accent),.62f*g),t);
        const ImU32 gold=IM_COL32(218,216,161,int(130*g));
        for(float side:{-1.f,1.f})dl->AddBezierCubic(L(86+side*38,H-137,0,0).a,L(86+side*33,H-91,0,0).a,L(86+side*15,H-91,0,0).a,L(86,H-101,0,0).a,gold,Px(.75f));
        Label(43,H-79,"须弥 · 净善宫",12,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        Sprig(cx.dl,L(xx+cw-35,y+62,0,0).a,Px(31),IM_COL32(211,231,161,110),t);
        if(drawDendro&&m.navSel==4){const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;drawDendro(cx.dl,L(xx+34,y+30,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);}
        cx.dl->AddBezierCubic(L(xx+14,y+89,0,0).a,L(xx+cw*.4f,y+58,0,0).a,L(xx+cw*.7f,y+103,0,0).a,L(xx+cw-14,y+82,0,0).a,IM_COL32(219,230,163,50),Px(.8f));
        break;
    }
    }
}
}
