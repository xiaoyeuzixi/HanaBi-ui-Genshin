#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::ganyu {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() { return BaseCharacterSkin(); }
}
ThemeDef MakeTheme() { return {"甘雨", "GANYU", "CRYO", MakeSkin(), "ganyu", true, Element::Cryo, Scenery::Lanterns, "循霜", Opening::GanyuFrostLotus, {"S I N U S   U N I C O R N I S", "循霜而行", "冰蓝绸缎 · 清心花纹", "微光粒子", "A QUIET MOMENT"}, IM_COL32(17,38,60,240), IM_COL32(39,70,88,245)}; }
}

namespace hb {
void Shell::DrawGanyuUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        Flower(dl,L(W-45,H-74,W-45,H-74).a,Px(61),IM_COL32(176,217,235,int(28*g)),.1f);
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        const float half=Px(13),cut=Px(5);
        ImVec2 points[]={{p.x-half+cut,p.y-half},{p.x+half-cut,p.y-half},{p.x+half,p.y-half+cut},{p.x+half,p.y+half-cut},{p.x+half-cut,p.y+half},{p.x-half+cut,p.y+half},{p.x-half,p.y+half-cut},{p.x-half,p.y-half+cut}};
        ImU32 tint=paint::MixI(IM_COL32(151,219,240,255),i==2?IM_COL32(223,140,159,255):IM_COL32(220,238,249,255),hover);
        dl->AddConvexPolyFilled(points,8,IM_COL32(12,37,57,int((75+hover*55)*g)));
        dl->AddPolyline(points,8,Fade(tint,(.3f+hover*.5f)*g),ImDrawFlags_Closed,Px(.8f));
        dl->AddTriangleFilled(points[0],points[1],ImVec2(p.x,p.y-2),Fade(tint,.07f*g));
        dl->AddLine(points[0],points[1],IM_COL32(218,191,137,int((80+hover*90)*g)),Px(.8f));
        col=Fade(tint,g);
        if(i==0){dl->AddLine(ImVec2(p.x-z,p.y),ImVec2(p.x+z,p.y),col,Px(1.1f));dl->AddQuadFilled(ImVec2(p.x+z+1,p.y-2),ImVec2(p.x+z+3,p.y),ImVec2(p.x+z+1,p.y+2),ImVec2(p.x+z-1,p.y),col);}
        if(i==1){ImVec2 diamond[]={{p.x,p.y-z-1},{p.x+z+1,p.y},{p.x,p.y+z+1},{p.x-z-1,p.y}};dl->AddPolyline(diamond,4,col,ImDrawFlags_Closed,Px(1.1f));dl->AddCircleFilled(p,Px(1),col,12);}
        if(i==2){dl->AddLine(ImVec2(p.x-z,p.y-z),ImVec2(p.x+z,p.y+z),col,Px(1.2f));dl->AddLine(ImVec2(p.x-z,p.y+z),ImVec2(p.x+z,p.y-z),col,Px(1.2f));dl->AddCircle(p,Px(1.7f),col,12,Px(.7f));}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Flower(dl,L(140,mark.c().y/scale,140,mark.c().y/scale).a,Px(9),Fade(cx.C(cx.sk.accent),.5f*g));
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        Flower(dl,L(82,H-125,82,H-125).a,Px(29),IM_COL32(188,218,232,int(90*g)),-pi*.5f);
        Label(43,H-79,"璃月 · 海灯夜",12,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        Flower(cx.dl,L(xx+cw-81,y+57,0,0).a,Px(36),IM_COL32(176,223,236,90),-.3f+sinf(t*.25f)*.025f);
        for(int j=0;j<3;j++) {float yy=y+90-j*11;cx.dl->AddBezierCubic(L(xx+12,yy,0,0).a,L(xx+cw*.4f,yy-24,0,0).a,L(xx+cw*.7f,yy+15,0,0).a,L(xx+cw-12,yy-15,0,0).a,IM_COL32(156,211,233,30+j*8),Px(1));}
        if(drawCryo&&m.navSel==4) {
            const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;
            drawCryo(cx.dl,L(xx+36,y+31,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);
        }
        ChineseKnot(cx.dl,L(xx+cw-29,y+12,0,0).a,scale,t);
        break;
    }
    }
}
}
