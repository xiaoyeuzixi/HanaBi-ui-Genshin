#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::yoimiya {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() {
    Skin s=BaseCharacterSkin();
    s.backdrop0=C(0x32170F,30);s.backdrop1=C(0x542716,42);
    s.win0=C(0x351A13,22);s.win1=C(0x271410,46);
    s.sidebar0=C(0x3E2018,164);s.sidebar1=C(0x2B1714,198);
    s.winStroke=C(0xF6C899,70);s.hairline=C(0xF6D6A2,40);
    s.card0=C(0x4F2A20,124);s.card1=C(0x382019,158);
    s.cardStroke=C(0xF6C695,55);s.cardInnerHi=C(0xFFF0D5,30);
    s.text=C(0xFFF5E5,255);s.text2=C(0xEAD1B7,255);s.text3=C(0xC5A58B,255);
    s.accent=C(0xFFD19B,255);s.accent2=C(0xF09173,255);
    s.accentSoft=C(0xFFD19B,36);s.accentGlow=C(0xFF8A5D,120);
    s.trackOn0=s.accent;s.trackOn1=s.accent2;s.segTrack=C(0x281611,82);
    s.segPill=C(0xFFD19B,32);s.field0=C(0xFFF0D5,14);s.fieldStroke=C(0xF6C695,30);
    s.good=C(0xD5DC9E,255);s.warn=C(0xFFB776,255);s.radius=18;s.cardRadius=12;
    return s;
}
}
ThemeDef MakeTheme() { return {"宵宫", "YOIMIYA", "PYRO", MakeSkin(), "yoimiya", true, Element::Pyro, Scenery::Fireworks, "烟火", Opening::YoimiyaGoldfish, {"C A R A S S I U S   A U R A T U S", "烟火映星河", "暖金烟火 · 金鱼绳饰", "火星与余烬", "A SUMMER SPARK"}, IM_COL32(76,36,23,240), IM_COL32(120,62,34,245)}; }
}

namespace hb {
void Shell::DrawYoimiyaUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        Goldfish(dl,L(W-83,H-82,0,0).a,Px(47),IM_COL32(255,204,140,int(36*g)),t);
        Firework(dl,L(W-202,H-48,0,0).a,Px(25),IM_COL32(255,218,162,int(28*g)),.2f);
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        const auto inset=r.In(Px(7),Px(5));
        paint::Fill(dl,inset.a,inset.b,Px(6),IM_COL32(76,35,23,int((100+hover*64)*g)));
        paint::Stroke(dl,inset.a,inset.b,Px(6),IM_COL32(255,202,132,int((65+hover*120)*g)),Px(.85f));
        const ImU32 gold=IM_COL32(255,220,164,int((160+hover*80)*g));
        dl->AddLine({inset.a.x+Px(5),inset.a.y+Px(2)},{inset.b.x-Px(5),inset.a.y+Px(2)},gold,Px(.7f));
        col=paint::MixI(cx.C(cx.sk.accent),i==2?IM_COL32(255,154,117,255):IM_COL32(255,243,220,255),hover);
        if(i==0){dl->AddBezierCubic({p.x-z,p.y},{p.x-z*.3f,p.y+Px(2)},{p.x+z*.3f,p.y+Px(2)},{p.x+z,p.y},col,Px(1.3f));dl->AddCircleFilled({p.x,p.y+Px(5)},Px(.8f),gold);}
        if(i==1){dl->AddRect({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1),0,Px(1.1f));dl->AddLine({p.x-z,p.y-z-Px(2)},{p.x+z,p.y-z-Px(2)},gold,Px(.8f));}
        if(i==2){dl->AddLine({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1.2f));dl->AddLine({p.x-z,p.y+z},{p.x+z,p.y-z},col,Px(1.2f));dl->AddCircleFilled(p,Px(1),gold);}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Firework(dl,L(140,mark.c().y/scale,0,0).a,Px(8),Fade(cx.C(cx.sk.accent),.5f*g),t*.04f);
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        Goldfish(dl,L(81,H-125,0,0).a,Px(31),Fade(cx.C(cx.sk.accent),.58f*g),t);
        Firework(dl,L(120,H-151,0,0).a,Px(13),Fade(cx.C(cx.sk.accent2),.44f*g),.2f);
        Label(43,H-79,"稻妻 · 长野原",12,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        Firework(cx.dl,L(xx+cw-53,y+43,0,0).a,Px(27),IM_COL32(255,214,144,90),.15f+sinf(t*.25f)*.02f);
        Goldfish(cx.dl,L(xx+cw-61,y+84,0,0).a,Px(29),IM_COL32(255,189,118,95),t);
        if(drawPyro&&m.navSel==4){const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;drawPyro(cx.dl,L(xx+34,y+30,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);}
        for(int j=0;j<2;++j)cx.dl->AddBezierCubic(L(xx+14,y+88+j*6,0,0).a,L(xx+cw*.35f,y+76+j*6,0,0).a,L(xx+cw*.6f,y+108+j*6,0,0).a,L(xx+cw-14,y+90+j*6,0,0).a,IM_COL32(255,198,130,45),Px(.8f));
        break;
    }
    }
}
}
