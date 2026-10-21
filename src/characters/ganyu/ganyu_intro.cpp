#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace hb::ganyu {
using namespace opening_draw;


void DrawScene(Intro&,ImDrawList* dl,ImVec2 p,float r,float t,float a){
    const float lift=S(t,.85f,3.25f),pearl=Window(t,.08f,.48f,1.35f,2.45f)*a;
    const ImVec2 orb=P(p,r,0,Lerp(.28f,0.f,lift));
    Pearl(dl,orb,r*(.12f+.08f*S(t,.1f,1.1f)),t,pearl,true);
    Halo(dl,P(p,r,0,(1-lift)*.72f),r*1.45f,r*.28f,0x2979B8,Window(t,.12f,.6f,2.2f,3.3f)*a*.3f);
    for(int lane=0;lane<3;++lane){const float age=t-.8f-lane*.10f,k=Window(t,.8f,1.35f,2.7f,3.4f)*a;
        const float y=.65f-age*.32f;Ribbon(dl,P(p,r,-1.5f,y+.18f),P(p,r,-.5f,y-.3f),P(p,r,.7f,y+.4f),P(p,r,1.5f,y-.12f),r*.032f,0x8ED5FF,k*.28f);
    }
    const float release=Window(t,1.95f,2.18f,3.7f,4.5f)*a;
    for(int j=0;j<70;++j){const float age=std::max(0.f,t-1.98f-Hash(j+9)*.26f),angle=Hash(j+18)*tau;
        const float d=(.20f+age*(.6f+Hash(j+31)*.6f)),x=cosf(angle)*d,y=sinf(angle)*d+age*age*.30f;
        const float k=release*(1-S(age,.65f,2.2f))*(.35f+.65f*Hash(j+41));
        if(j%4==0)Star(dl,P(p,r,x,y),r*(.012f+Hash(j+52)*.012f),0xC7F3FF,k,pi*.25f);
        else{const auto q=P(p,r,x,y);dl->AddLine(q,{q.x+r*.006f,q.y+r*.035f},C(0xB5E5FF,k),.8f);}
    }
    const float flash=expf(-powf((t-2.42f)/.30f,2))*a;
    Halo(dl,p,r*1.9f,0x7CCBFF,flash*.20f);
}
std::vector<CrystalVertex> BuildGlyph(ImVec2 center,float radius,float time) {
    auto vertices=Intro::CryoMesh();
    const float bloom=S(time,.85f,3.25f),presence=S(time,.03f,.40f);
    for(auto& vertex:vertices){
        const float x=vertex.x,y=vertex.y,angle=std::atan2(x,-y);
        const float arm=std::round(angle/(tau/6))*(tau/6),c=std::cos(arm),s=std::sin(arm);
        const float along=x*s-y*c,across=x*c+y*s;
        const float fold=(1-bloom)*1.22f,radial=along*std::cos(fold),height=along*std::sin(fold);
        const float floorX=s*radial+c*across,floorY=c*radial-s*across;
        vertex.x=center.x+floorX*(.68f+.32f*bloom);
        vertex.y=center.y-floorY*(.34f+.66f*bloom)-height*.64f+(1-bloom)*.72f;
        vertex.x=center.x+(vertex.x-center.x)*radius;
        vertex.y=center.y+(vertex.y-center.y)*radius;
        vertex.opacity*=presence;
    }
    return vertices;
}
void DrawBackdrop(Intro& intro,ImDrawList* dl,ImVec2 o){
    const float W=intro.W,H=intro.H,t=intro.t,s=std::min(W,H),fade=1-S(t,Intro::kDockStart,5.55f);

        intro.DrawLanternScenery(dl,o);
        for(int j=0;j<55;++j){const float u=std::fmod(Hash(j+7)+t*(.04f+Hash(j+13)*.04f),1.f);
            const auto q=ImVec2{o.x+W*Hash(j+21)+sinf(t*.4f+j)*s*.012f,o.y+u*H};const float k=fade*S(t,.6f,1.7f)*(.12f+Hash(j+43)*.45f);
            if(j%7==0)Star(dl,q,s*.003f,0xBFE4F7,k,pi*.25f);else dl->AddCircleFilled(q,s*(.0006f+Hash(j+56)*.0010f),C(0xD0ECFF,k),8);
        }

}
CharacterOpeningDef OpeningInfo(){return {Opening::GanyuFrostLotus,Element::Cryo,.168f,0x071522,0x102B3A,0x68BFEA,DrawScene,BuildGlyph,DrawBackdrop};}
}
