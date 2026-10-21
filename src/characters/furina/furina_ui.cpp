#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::furina {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() {
    Skin s=BaseCharacterSkin();
    s.backdrop0=C(0x08172C,30);s.backdrop1=C(0x132D4D,44);
    s.win0=C(0x0D2140,22);s.win1=C(0x0B1B35,42);
    s.sidebar0=C(0x102543,169);s.sidebar1=C(0x0C1934,202);
    s.winStroke=C(0xBDDFF7,70);s.hairline=C(0xD9D3B9,42);
    s.card0=C(0x173554,134);s.card1=C(0x10253F,171);
    s.cardStroke=C(0xB8D9F2,55);s.cardInnerHi=C(0xEDF8FF,30);
    s.text=C(0xF3F9FF,255);s.text2=C(0xC9DEF3,255);s.text3=C(0x9BB7D4,255);
    s.accent=C(0xA6E2FA,255);s.accent2=C(0xAABCF4,255);
    s.accentSoft=C(0x9DDDF7,36);s.accentGlow=C(0x72CFF5,118);
    s.trackOn0=s.accent;s.trackOn1=C(0x80A9EB,255);s.segTrack=C(0x08162D,82);
    s.segPill=C(0xC8E9FB,33);s.field0=C(0xE3F5FF,14);s.fieldStroke=C(0xBFD9F2,32);
    s.good=C(0xADDDD9,255);s.warn=C(0xE9D4A4,255);s.radius=18;s.cardRadius=13;
    return s;
}
}
ThemeDef MakeTheme() { return {"芙宁娜", "FURINA", "HYDRO", MakeSkin(), "furina", true, Element::Hydro, Scenery::Opera, "水幕", Opening::FurinaWaterStage, {"A N I M U L A   C H O R A G I", "倾听水的回响", "水色帷幕 · 银边缎带", "水珠与浮光", "A CURTAIN OF WATER"}, IM_COL32(13,31,65,245), IM_COL32(30,68,100,245)}; }
}

namespace hb {
void Shell::DrawFurinaUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        TopHat(dl,L(W-72,H-80,0,0).a,Px(40),IM_COL32(195,222,241,int(40*g)),t);
        for(int j=0;j<3;++j)dl->AddBezierCubic(L(W-270,H-35-j*10,0,0).a,L(W-160,H-10-j*12,0,0).a,L(W-170,H-90-j*12,0,0).a,L(W-25,H-56-j*8,0,0).a,IM_COL32(164,213,243,int(24*g)),Px(.7f));
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        const auto inset=r.In(Px(7),Px(5));
        paint::Fill(dl,inset.a,inset.b,Px(7),IM_COL32(18,42,78,int((125+hover*70)*g)));
        paint::Stroke(dl,inset.a,inset.b,Px(7),IM_COL32(191,221,244,int((85+hover*110)*g)),Px(.8f));
        const ImU32 silver=IM_COL32(224,233,228,int((135+hover*90)*g));
        dl->AddBezierCubic({p.x-Px(11),p.y-Px(8)},{p.x-Px(3),p.y-Px(12)},{p.x+Px(3),p.y-Px(12)},{p.x+Px(11),p.y-Px(8)},silver,Px(.7f));
        col=Fade(paint::MixI(cx.C(cx.sk.accent),i==2?IM_COL32(251,186,207,255):IM_COL32(240,253,255,255),hover),g);
        if(i==0){dl->AddBezierCubic({p.x-z,p.y},{p.x-z*.3f,p.y+Px(1.4f)},{p.x+z*.3f,p.y+Px(1.4f)},{p.x+z,p.y},col,Px(1.2f));dl->AddCircleFilled({p.x,p.y+Px(4.2f)},Px(.7f),silver);}
        if(i==1){dl->AddRect({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1.6f),0,Px(1.1f));dl->AddLine({p.x-z*.5f,p.y-z-Px(2)},{p.x+z*.5f,p.y-z-Px(2)},silver,Px(.8f));}
        if(i==2){dl->AddLine({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1.2f));dl->AddLine({p.x-z,p.y+z},{p.x+z,p.y-z},col,Px(1.2f));}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Droplet(dl,L(140,mark.c().y/scale,0,0).a,Px(7.5f),Fade(cx.C(cx.sk.accent),.55f*g));
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        TopHat(dl,L(85,H-125,0,0).a,Px(28),Fade(cx.C(cx.sk.accent),.64f*g),t);
        Label(38,H-79,"枫丹 · 水色舞台",11.5f,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        TopHat(cx.dl,L(xx+cw-33,y+65,0,0).a,Px(27),IM_COL32(192,221,243,115),t);
        if(drawHydro&&m.navSel==4){const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;drawHydro(cx.dl,L(xx+36,y+31,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);}
        for(int j=0;j<3;++j)cx.dl->AddBezierCubic(L(xx+12,y+86+j*6,0,0).a,L(xx+cw*.4f,y+66+j*7,0,0).a,L(xx+cw*.6f,y+108+j*4,0,0).a,L(xx+cw-12,y+82+j*6,0,0).a,IM_COL32(162,217,248,32+j*9),Px(.8f));
        break;
    }
    }
}
}
