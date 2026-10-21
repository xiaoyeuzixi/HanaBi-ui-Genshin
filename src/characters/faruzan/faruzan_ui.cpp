#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::faruzan {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() {
    Skin s=BaseCharacterSkin();
    s.backdrop0=C(0x102E31,30);s.backdrop1=C(0x397776,38);
    s.win0=C(0x0D3339,27);s.win1=C(0x143D43,43);
    s.sidebar0=C(0x123C43,210);s.sidebar1=C(0x102E36,224);
    s.winStroke=C(0xD3DCC5,84);s.hairline=C(0xE9D4A1,52);
    s.card0=C(0x1A484E,185);s.card1=C(0x123A43,206);
    s.cardStroke=C(0xB9D8C8,62);s.cardInnerHi=C(0xFFF2D2,24);
    s.text=C(0xF7F6E9,255);s.text2=C(0xD6E7DD,255);s.text3=C(0xABC8C1,255);
    s.accent=C(0xB1F0D6,255);s.accent2=C(0xE6CB94,255);
    s.accentSoft=C(0xA4E5CF,38);s.accentGlow=C(0x85DEC5,108);
    s.trackOn0=s.accent;s.trackOn1=C(0x70C9BC,255);s.segTrack=C(0x082C34,105);
    s.segPill=C(0xB4E7D6,42);s.field0=C(0xDAEEDC,18);s.fieldStroke=C(0xC6D8BD,39);
    s.good=C(0xB7E4C7,255);s.warn=C(0xE8CF9F,255);
    s.radius=13;s.cardRadius=10;
    return s;
}
}
ThemeDef MakeTheme() { return {"珐露珊", "FARUZAN", "ANEMO", MakeSkin(), "faruzan", true, Element::Anemo, Scenery::Archive, "风卷", Opening::FaruzanMechanism, {"F L O S C U L I   I M P L E X I", "风起，翻开新章", "青金机关 · 风阅古卷", "窗光与浮尘", "WISDOM ON THE WIND"}, IM_COL32(20,60,61,245), IM_COL32(41,87,78,245)}; }
}

namespace hb {
void Shell::DrawFaruzanUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        ArchiveBook(dl,L(W-80,H-74,0,0).a,Px(40),IM_COL32(227,211,163,int(43*g)),t);
        for(int j=0;j<3;++j){const float xx=W-240+j*21,yy=H-49-j*10;dl->AddLine(L(xx,yy,0,0).a,L(xx+30,yy-16,0,0).a,IM_COL32(173,225,203,int(28*g)),Px(.7f));dl->AddLine(L(xx+30,yy-16,0,0).a,L(xx+60,yy,0,0).a,IM_COL32(173,225,203,int(28*g)),Px(.7f));}
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        const auto inset=r.In(Px(7),Px(5));const float cut=Px(4);
        ImVec2 corners[]={{inset.a.x+cut,inset.a.y},{inset.b.x-cut,inset.a.y},{inset.b.x,inset.a.y+cut},{inset.b.x,inset.b.y-cut},{inset.b.x-cut,inset.b.y},{inset.a.x+cut,inset.b.y},{inset.a.x,inset.b.y-cut},{inset.a.x,inset.a.y+cut}};
        paint::Fill(dl,inset.a,inset.b,Px(4),IM_COL32(16,58,62,int((185+hover*50)*g)));
        const ImU32 gold=IM_COL32(232,210,156,int((125+hover*105)*g));dl->AddPolyline(corners,8,gold,ImDrawFlags_Closed,Px(.8f));
        col=Fade(paint::MixI(cx.C(cx.sk.accent),i==2?IM_COL32(255,197,163,255):IM_COL32(245,255,231,255),hover),g);
        dl->AddTriangleFilled({p.x-Px(2),inset.a.y+Px(1)},{p.x+Px(2),inset.a.y+Px(1)},{p.x,inset.a.y+Px(4)},Fade(gold,.75f));
        if(i==0)dl->AddLine({p.x-z,p.y},{p.x+z,p.y},col,Px(1.2f));
        if(i==1){dl->AddRect({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(.4f),0,Px(1.1f));dl->AddLine({p.x-z,p.y-z},{p.x+z,p.y+z},Fade(gold,.5f),Px(.65f));}
        if(i==2){dl->AddLine({p.x-z,p.y-z},{p.x+z,p.y+z},col,Px(1.2f));dl->AddLine({p.x-z,p.y+z},{p.x+z,p.y-z},col,Px(1.2f));}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Polyhedron(dl,L(141,mark.c().y/scale,0,0).a,Px(8.5f),Fade(cx.C(cx.sk.accent2),.65f*g),t);
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        ArchiveBook(dl,L(86,H-119,0,0).a,Px(27),Fade(cx.C(cx.sk.accent2),.65f*g),t);
        Label(38,H-79,"须弥 · 风阅古卷",11.5f,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        Polyhedron(cx.dl,L(xx+cw-29,y+42,0,0).a,Px(23),IM_COL32(233,215,169,100),t);
        if(drawAnemo&&m.navSel==4){const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;drawAnemo(cx.dl,L(xx+36,y+31,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);}
        ArchiveBook(cx.dl,L(xx+cw-33,y+84,0,0).a,Px(21),IM_COL32(179,222,201,65),t);
        break;
    }
    }
}
}
