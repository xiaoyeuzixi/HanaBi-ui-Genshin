#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "intro/pyro_geometry.h"

namespace hb::yoimiya {
using namespace opening_draw;
void Firework(ImDrawList* dl,ImVec2 p,float r,float age,float alpha,int seed,int count=64){
    if(age<0||age>2.6f||alpha<.002f)return;
    const float onset=S(age,0,.045f),life=1-S(age,.70f,2.4f);
    Halo(dl,p,r*(.25f+age*.6f),0xFF9C58,alpha*expf(-age*8)*.55f);
    for(int j=0;j<count;++j){const float angle=j*tau/count+Hash(j+seed)*.035f,speed=.65f+Hash(j+seed+77)*.5f;
        auto point=[&](float v){const float d=(1-expf(-v*1.6f))*r*speed;return ImVec2{p.x+cosf(angle)*d,p.y+sinf(angle)*d+v*v*r*.15f};};
        for(int k=0;k<12;++k){const float u=k/12.f,v=(k+1)/12.f;
            const auto x=point(std::max(0.f,age-.30f*(1-u))),y=point(std::max(0.f,age-.30f*(1-v)));
            const float a=alpha*onset*life*u*(.65f+.35f*sinf(age*8+j*2));
            dl->AddLine(x,y,C(j%4?0xFFCF86:0xE96A4C,a*.12f),r*.016f);
            dl->AddLine(x,y,C(j%4?0xFFE7A4:0xFFA46E,a),std::max(.65f,r*.0035f));
        }
        const auto q=point(age);if(j%4==0)Star(dl,q,r*.009f,0xFFF4CD,alpha*onset*life*.7f);
    }
}

void Goldfish(ImDrawList* dl,ImVec2 p,float r,float angle,float a){
    if(a<.002f)return;const auto uv=ImGui::GetFontTexUvWhitePixel();
    auto turn=[&](float x,float y){return P(p,r,x*cosf(angle)-y*sinf(angle),x*sinf(angle)+y*cosf(angle));};
    constexpr int n=42;const auto base=dl->_VtxCurrentIdx;dl->PrimReserve(n*3,n+1);
    dl->PrimWriteVtx(turn(-.1f,0),uv,C(0xFFF2BA,a));
    for(int j=0;j<n;++j){const float u=j*tau/n;dl->PrimWriteVtx(turn(cosf(u)*.45f,sinf(u)*.19f),uv,C(j<n/2?0xFFC36C:0xED784E,a*.45f));}
    for(int j=0;j<n;++j)for(auto i:{base,base+1+j,base+1+(j+1)%n})dl->PrimWriteIdx(ImDrawIdx(i));
    for(int side:{-1,1}){
        const auto x=turn(-.33f,0),y=turn(-1.f,side*.50f),z=turn(-.78f,0);
        Ribbon(dl,x,turn(-.64f,side*.08f),turn(-.80f,side*.43f),y,r*.08f,0xFFCF83,a*.7f);
        ImVec2 tail[]={x,y,z};LightLine(dl,tail,3,0xFFD69A,a*.6f,.7f);
        Petal(dl,turn(0,side*.12f),r*.42f,angle+side*1.6f,0xFFE0A1,a*.5f,.30f);
    }
    Halo(dl,p,r*1.6f,0xFF9C54,a*.16f);
}
ImVec2 FireMorph(float x,float y,float t){
    const float ascent=S(t,.16f,1.40f),assemble=S(t,1.32f,3.16f),plume=.22f+.78f*S(t,1.35f,2.08f);
    const float sourceX=(sinf(y*5+x*2-t*2)*.13f+x*.22f)*plume+sinf(ascent*pi)*.19f;
    const float sourceY=(y*.38f+cosf(x*4+t)*.08f)*plume+2.4f*(1-ascent);
    return {Lerp(sourceX,x,assemble),Lerp(sourceY,y,assemble)};
}
std::vector<CrystalVertex> FlameVeils(ImVec2 p,float r,float t,float a){
    std::vector<CrystalVertex> mesh;const float active=Window(t,1.40f,1.72f,3.12f,4.1f)*a;
    if(active<.002f)return mesh;
    const float scale=.13f+.87f*S(t,1.32f,3.16f);const auto& outline=pyro::Points();const int end=pyro::Ends()[0];
    for(int j=0;j<26;++j){const auto anchor=FireMorph(outline[j*end/26].x,outline[j*end/26].y,t);const float x=anchor.x,y=anchor.y;
        const float w=r*scale*(.075f+Hash(j+19)*.055f),h=r*scale*(.18f+Hash(j+33)*.17f);
        for(auto q:{ImVec2{-1,0},ImVec2{1,0},ImVec2{-1,1},ImVec2{1,0},ImVec2{1,1},ImVec2{-1,1}})
            mesh.push_back({p.x+x*r+q.x*w,p.y+y*r-q.y*h,0,0,1,q.x,q.y,2,active*.65f,0,Hash(j+52)});
    }
    return mesh;
}
void DrawScene(Intro& intro,ImDrawList* dl,ImVec2 p,float r,float t,float a){
    const float ascent=S(t,.16f,1.40f),launch=Window(t,.1f,.35f,1.38f,1.62f)*a;
    auto path=[&](float u){return P(p,r,sinf(u*pi)*.19f,2.4f*(1-u));};
    const auto seed=path(ascent);
    for(int j=0;j<32;++j){const float u=j/32.f,v=(j+1)/32.f;
        const auto x=path(std::max(0.f,ascent-.24f*(1-u))),y=path(std::max(0.f,ascent-.24f*(1-v)));
        dl->AddLine(x,y,C(0xFFB66D,launch*u*.13f),r*.035f);dl->AddLine(x,y,C(0xFFF0C1,launch*u),1.1f);
    }
    Star(dl,seed,r*.035f,0xFFF3D0,launch,pi*.25f);
    Firework(dl,p,r*1.72f,t-1.40f,a*.83f,73,88);
    const float orbit=Window(t,.75f,1.30f,2.72f,3.55f)*a;
    for(int side:{-1,1}){const float angle=t*1.25f+(side<0?pi:0),d=1.38f-.25f*S(t,1.3f,2.8f);
        const auto fish=P(p,r,cosf(angle)*d,sinf(angle)*d*.57f);
        Goldfish(dl,fish,r*.34f,atan2f(cosf(angle)*.57f,-sinf(angle)),orbit*.85f);
        for(int k=0;k<38;++k){const float u=k/38.f,ang=angle-u*1.4f;
            const auto x=P(p,r,cosf(ang)*d,sinf(ang)*d*.57f),y=P(p,r,cosf(ang-.025f)*d,sinf(ang-.025f)*d*.57f);
            dl->AddLine(x,y,C(0xFFC26D,orbit*(1-u)*.40f),std::max(.7f,r*.009f*(1-u)));
        }
    }
    Firework(dl,P(p,r,-2.1f,-.95f),r*.85f,t-2.52f,a*.50f,170,44);
    Firework(dl,P(p,r,2.0f,-.60f),r*.72f,t-2.90f,a*.48f,299,38);
    Halo(dl,p,r*1.8f,0xF77E3F,expf(-powf((t-2.15f)/.35f,2))*a*.27f);
    auto fire=FlameVeils(p,r,t,a);if(!fire.empty())intro.pyroMaterial.Queue(dl,std::move(fire),p,r,t,1.f);
}
std::vector<CrystalVertex> BuildGlyph(ImVec2 p,float r,float t){
    std::vector<CrystalVertex> mesh;
        mesh=pyro::Vertices({0,0},1,6.2f);
        for(auto& v:mesh){const float x=v.x,y=v.y;
            const auto q=FireMorph(x,y,t);v.x=p.x+q.x*r;v.y=p.y+q.y*r;
            v.opacity*=S(t,.10f,.38f);
        }

    return mesh;
}
void DrawBackdrop(Intro&,ImDrawList*,ImVec2){}
CharacterOpeningDef OpeningInfo(){return {Opening::YoimiyaGoldfish,Element::Pyro,.218f,0x150F19,0x30201D,0xF8B365,DrawScene,BuildGlyph,DrawBackdrop};}
}
