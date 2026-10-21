#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::zhongli {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() {
    Skin s=BaseCharacterSkin();
    s.backdrop0=C(0x241C12,28);s.backdrop1=C(0x59412A,42);
    s.win0=C(0x251E16,30);s.win1=C(0x33281B,50);
    s.sidebar0=C(0x241F19,210);s.sidebar1=C(0x1A1815,232);
    s.winStroke=C(0xDAB77A,92);s.winStroke2=C(0xFFF0CE,28);
    s.hairline=C(0xEAC78F,50);s.divider=C(0xC0A177,35);
    s.card0=C(0x382E22,184);s.card1=C(0x29231C,207);
    s.cardStroke=C(0xCEAC74,66);s.cardInnerHi=C(0xFFF1CE,24);s.cardInnerLo=C(0x1D1812,32);
    s.text=C(0xFFF4DF,255);s.text2=C(0xE3CFAC,255);s.text3=C(0xBCA788,255);
    s.accent=C(0xF5C969,255);s.accent2=C(0xDDA556,255);
    s.accentSoft=C(0xE7B85C,36);s.accentGlow=C(0xF0B645,110);
    s.trackOn0=s.accent;s.trackOn1=C(0xCF943F,255);s.trackOff0=C(0x191710,175);s.trackOff1=C(0x473A26,120);
    s.segTrack=C(0x1F1B16,115);s.segPill=C(0xF3D390,36);s.segStroke=C(0xCCAB72,38);
    s.field0=C(0xF2D9A5,16);s.field1=C(0x1D1913,28);s.fieldStroke=C(0xD3B77D,40);
    s.good=C(0xD6D7A2,255);s.warn=C(0xF5BF71,255);s.radius=12;s.cardRadius=9;
    return s;
}
}
ThemeDef MakeTheme() { return {"钟离", "ZHONGLI", "GEO", MakeSkin(), "zhongli", true, Element::Geo, Scenery::Stone, "磐岩", Opening::ZhongliMeteor, {"L A P I S   D E I", "磐石亦有温度", "玄金磐石 · 岩契回纹", "鎏金岩尘", "A CONTRACT IN STONE"}, IM_COL32(52,39,24,245), IM_COL32(95,68,36,245)}; }
}

namespace hb {
void Shell::DrawZhongliUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        StoneSeal(dl,L(W-75,H-76,0,0).a,Px(43),IM_COL32(230,184,104,int(48*g)));
        for(int j=0;j<3;++j){ImVec2 path[]={L(W-260+j*26,H-28,0,0).a,L(W-260+j*26,H-56,0,0).a,L(W-232+j*26,H-56,0,0).a,L(W-232+j*26,H-38,0,0).a};dl->AddPolyline(path,4,IM_COL32(222,178,98,int(26*g)),0,Px(.8f));}
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        const auto box=r.In(Px(7),Px(6));const float cut=Px(4);
        ImVec2 corners[]={{box.a.x+cut,box.a.y},{box.b.x-cut,box.a.y},{box.b.x,box.a.y+cut},{box.b.x,box.b.y-cut},{box.b.x-cut,box.b.y},{box.a.x+cut,box.b.y},{box.a.x,box.b.y-cut},{box.a.x,box.a.y+cut}};
        dl->AddConvexPolyFilled(corners,8,IM_COL32(42,32,20,int((175+hover*65)*g)));
        const ImU32 gold=IM_COL32(241,196,117,int((100+hover*130)*g));dl->AddPolyline(corners,8,gold,ImDrawFlags_Closed,Px(.85f));
        dl->AddLine({box.a.x+Px(5),box.b.y-Px(2)},{box.b.x-Px(5),box.b.y-Px(2)},Fade(gold,.65f),Px(.7f));
        col=Fade(paint::MixI(cx.C(cx.sk.accent),i==2?IM_COL32(255,185,131,255):IM_COL32(255,244,211,255),hover),g);
        if(i==0)dl->AddLine({p.x-z,p.y},{p.x+z,p.y},col,Px(1.25f));
        if(i==1){ImVec2 seal[]={{p.x,p.y-z-Px(1)},{p.x+z+Px(1),p.y},{p.x,p.y+z+Px(1)},{p.x-z-Px(1),p.y}};dl->AddPolyline(seal,4,col,ImDrawFlags_Closed,Px(1.1f));}
        if(i==2){dl->AddLine({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1.2f));dl->AddLine({p.x-z,p.y+z},{p.x+z,p.y-z},col,Px(1.2f));}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Flower(dl,L(140,mark.c().y/scale,140,mark.c().y/scale).a,Px(9),Fade(cx.C(cx.sk.accent),.5f*g));
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        StoneSeal(dl,L(86,H-122,0,0).a,Px(29),Fade(cx.C(cx.sk.accent2),.67f*g));
        Label(38,H-79,"璃月 · 岩契千年",11.5f,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        StoneSeal(cx.dl,L(xx+cw-35,y+61,0,0).a,Px(36),IM_COL32(249,205,121,105));
        if(drawGeo&&m.navSel==4){const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;drawGeo(cx.dl,L(xx+35,y+31,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);}
        for(int j=0;j<3;++j){const float a=xx+20+j*28;ImVec2 path[]={L(a,y+91,0,0).a,L(a,y+80,0,0).a,L(a+16,y+80,0,0).a,L(a+16,y+88,0,0).a,L(a+25,y+88,0,0).a};cx.dl->AddPolyline(path,5,IM_COL32(235,193,113,55),0,Px(.8f));}
        break;
    }
    }
}
}
