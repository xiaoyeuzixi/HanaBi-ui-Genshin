#include "characters/characters.h"
#include "ui/shell.h"
#include "ui/paint.h"
#include "ui/theme_ornaments.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::yae_miko {
namespace {
Rgba C(uint32_t hex,int a){return Hex(hex,float(a)/255.f);}
Skin MakeSkin() {
    Skin s = BaseCharacterSkin();
    s.backdrop0=C(0x231326,30); s.backdrop1=C(0x35192D,40);
    s.win0=C(0x30192F,18); s.win1=C(0x27142B,38);
    s.sidebar0=C(0x301C35,161); s.sidebar1=C(0x241328,197);
    s.winStroke=C(0xE6B4D7,62); s.hairline=C(0xEDC9A3,38);
    s.card0=C(0x3E2541,122); s.card1=C(0x2C1934,154);
    s.cardStroke=C(0xF1C1DD,53); s.cardInnerHi=C(0xFFE9F6,28);
    s.text=C(0xFFF0F8,255); s.text2=C(0xE7CBDF,255); s.text3=C(0xBE9DB9,255);
    s.accent=C(0xF2B8DF,255); s.accent2=C(0xBA90F5,255);
    s.accentSoft=C(0xE6A1D0,37); s.accentGlow=C(0xC978F0,118);
    s.trackOn0=s.accent; s.trackOn1=s.accent2;
    s.segTrack=C(0x1F102B,76); s.segPill=C(0xE8B5D7,37);
    s.field0=C(0xFCE5F6,14); s.fieldStroke=C(0xE3BED5,30);
    s.good=C(0xD8C19A,255); s.warn=C(0xF0C995,255);
    s.cardRadius=12.f; s.radius=18.f;
    return s;
}
}
ThemeDef MakeTheme() { return {"八重神子", "YAE MIKO", "ELECTRO", MakeSkin(), "yae", true, Element::Electro, Scenery::Sakura, "神樱", Opening::YaeSesshouSakura, {"D I V I N A   V U L P E S", "浮世笑百姿", "绯樱薄绢 · 鸣神金饰", "落樱与微光", "A FLEETING BLOSSOM"}, IM_COL32(56,28,55,240), IM_COL32(85,44,73,245)}; }
}

namespace hb {
void Shell::DrawYaeUi(CharacterUiPart part,const CharacterUiArgs& args) {
    using namespace ornament;
    constexpr float pi=3.14159265359f;
    const auto Clamp=[](float v){return std::clamp(v,0.f,1.f);};
    switch(part) {
    case CharacterUiPart::Chrome: {
        auto* dl=cx.dl; const float g=sceneMix;
        Sakura(dl,L(W-51,H-78,0,0).a,Px(57),Fade(cx.C(cx.sk.accent),.15f*g),.15f);
        for(int j=0;j<3;++j)dl->AddBezierCubic(L(W-260+j*14,H-30,0,0).a,L(W-150,H-48-j*13,0,0).a,L(W-82,H-24-j*22,0,0).a,L(W-15,H-122-j*9,0,0).a,IM_COL32(223,188,144,int(20*g)),Px(.7f));
        break;
    }
    case CharacterUiPart::WindowControl: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc r=args.box; const float hover=args.hover; const int i=args.index; const ImVec2 p=r.c(); const float z=Px(4.5f); ImU32 col=cx.C(cx.sk.text2);
        Rc inset=r.In(Px(7),Px(5));
        paint::Fill(dl,inset.a,inset.b,Px(7),IM_COL32(69,33,66,int((80+hover*58)*g)));
        paint::Stroke(dl,inset.a,inset.b,Px(7),IM_COL32(236,177,215,int((50+hover*110)*g)),Px(.7f));
        dl->AddBezierCubic({p.x-Px(12),p.y-Px(9)},{p.x-Px(7),p.y-Px(7)},{p.x+Px(7),p.y-Px(7)},{p.x+Px(12),p.y-Px(9)},IM_COL32(227,193,146,int((125+hover*90)*g)),Px(.9f));
        col=Fade(paint::MixI(cx.C(cx.sk.accent),i==2?IM_COL32(255,160,193,255):IM_COL32(255,237,252,255),hover),g);
        if(i==0){dl->AddLine({p.x-z,p.y+1},{p.x+z,p.y+1},col,Px(1.2f));dl->AddCircleFilled({p.x,p.y+Px(5)},Px(.8f),col);}
        if(i==1){ImVec2 diamond[]={{p.x,p.y-z},{p.x+z,p.y},{p.x,p.y+z},{p.x-z,p.y}};dl->AddPolyline(diamond,4,col,ImDrawFlags_Closed,Px(1));}
        if(i==2){dl->AddBezierCubic({p.x-z,p.y-z},{p.x-z*.2f,p.y-z*.2f},{p.x+z*.2f,p.y+z*.2f},{p.x+z,p.y+z},col,Px(1.2f));dl->AddLine({p.x-z,p.y+z},{p.x+z,p.y-z},col,Px(1.2f));}
        break;
    }
    case CharacterUiPart::Nav: {
        auto* dl=cx.dl; const float g=sceneMix; const Rc mark=args.box;
        Sakura(dl,L(140,mark.c().y/scale,0,0).a,Px(9),Fade(cx.C(cx.sk.accent),.5f*g));
        break;
    }
    case CharacterUiPart::Sidebar: {
        auto* dl=cx.dl; const float g=sceneMix;
        const auto p=L(85,H-126,0,0).a;const float z=Px(27);
        ImU32 gold=IM_COL32(226,191,151,int(118*g));
        auto pt=[&](float x,float y){return ImVec2(p.x+x*z,p.y+y*z);};
        dl->AddBezierCubic(pt(-.86f,-.17f),pt(-1,-1.35f),pt(-.58f,-1.18f),pt(-.33f,-.55f),gold,Px(1));
        dl->AddBezierCubic(pt(.86f,-.17f),pt(1,-1.35f),pt(.58f,-1.18f),pt(.33f,-.55f),gold,Px(1));
        dl->AddBezierCubic(pt(-.86f,-.17f),pt(-.80f,.55f),pt(-.3f,.72f),pt(0,.98f),gold,Px(1));
        dl->AddBezierCubic(pt(.86f,-.17f),pt(.80f,.55f),pt(.3f,.72f),pt(0,.98f),gold,Px(1));
        for(int side:{-1,1})dl->AddBezierCubic(pt(side*.68f,.0f),pt(side*.55f,.15f),pt(side*.28f,.19f),pt(side*.17f,.06f),Fade(cx.C(cx.sk.accent),.7f*g),Px(1.2f));
        Sakura(dl,pt(0,-.15f),Px(6),Fade(cx.C(cx.sk.accent),.5f*g),.3f);
        Label(43,H-79,"稻妻 · 鸣神樱",12,1);
        break;
    }
    case CharacterUiPart::Card: {
        const float xx=args.x,y=args.y,cw=args.width,contentAlpha=args.alpha;
        Sakura(cx.dl,L(xx+cw-83,y+56,0,0).a,Px(35),IM_COL32(239,181,214,106),.1f+sinf(t*.25f)*.025f);
        Sakura(cx.dl,L(xx+cw-125,y+27,0,0).a,Px(10),IM_COL32(227,193,149,120),-.2f);
        if(drawElectro&&m.navSel==4) {
            const float page=Clamp(pageWeight[4].V(1.f)),enter=entering?Ease(E_PAGE,Clamp(enterT/.8f)):1.f;
            drawElectro(cx.dl,L(xx+36,y+31,0,0).a,Px(17),6.2f+t,page*enter*contentAlpha);
        }
        for(int j=0;j<3;++j)cx.dl->AddBezierCubic(L(xx+12,y+90-j*8,0,0).a,L(xx+cw*.4f,y+51-j*7,0,0).a,L(xx+cw*.7f,y+98-j*10,0,0).a,L(xx+cw-12,y+69-j*9,0,0).a,IM_COL32(229,168,207,30+j*10),Px(.8f));
        const ImU32 gold=IM_COL32(224,188,142,185);
        cx.dl->AddLine(L(xx+cw-25,y+17,0,0).a,L(xx+cw-25,y+66,0,0).a,gold,Px(.8f));
        ImVec2 charm[]={L(xx+cw-25,y+40,0,0).a,L(xx+cw-19,y+49,0,0).a,L(xx+cw-25,y+58,0,0).a,L(xx+cw-31,y+49,0,0).a};
        cx.dl->AddPolyline(charm,4,gold,ImDrawFlags_Closed,Px(.8f));
        cx.dl->AddLine(L(xx+cw-28,y+66,0,0).a,L(xx+cw-22,y+72,0,0).a,gold,Px(1));
        cx.dl->AddLine(L(xx+cw-22,y+72,0,0).a,L(xx+cw-28,y+78,0,0).a,gold,Px(1));
        break;
    }
    }
}
}
