#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "intro/dendro_geometry.h"
#include "intro/hydro_geometry.h"
#include <cstdio>

namespace hb {
using namespace opening_draw;
namespace {
std::vector<CrystalVertex> CharacterGlyph(Opening opening,ImVec2 p,float r,float t){const auto* def=FindCharacterOpening(opening);return def?def->glyph(p,r,t):std::vector<CrystalVertex>{};}
}
Intro::MarkPose Intro::CharacterPose(ImVec2 o)const{
    const auto* def=FindCharacterOpening(opening);const float size=def?def->radius:.215f,dock=HandoffK(),arc=sinf(pi*dock);
    return {{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.44f,dockCenter.y,dock)-H*.055f*arc*arc},Lerp(std::min(W,H)*size,dockRadius,dock),1};
}
void Intro::DrawCharacterAtmosphere(ImDrawList* dl,ImVec2 o){
    const auto* def=FindCharacterOpening(opening);if(!def)return;
    const float fade=1-S(t,kDockStart,5.55f),s=std::min(W,H);const auto pose=CharacterPose(o);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(def->dark,fade*.97f),C(def->dark,fade*.97f),C(def->base,fade*.94f),C(def->base,fade*.94f));
    Halo(dl,pose.center,s*.65f,s*.55f,def->glow,Window(t,.1f,1.6f,4.2f,5.3f)*fade*.11f);
    def->backdrop(*this,dl,o);
    if(def->element!=Element::Cryo)for(int j=0;j<28;++j){const float u=std::fmod(Hash(j+110)+t*.038f,1.f);
        const auto q=ImVec2{o.x+W*Hash(j+146)+sinf(t*.5f+j)*s*.008f,o.y+H*(1-u)};
        const float k=fade*S(t,.3f,1.4f)*sinf(u*pi)*(.10f+Hash(j+171)*.17f);
        Halo(dl,q,s*.006f,def->glow,k*.25f);dl->AddCircleFilled(q,std::max(.6f,s*.0011f),C(def->glow,k),8);
    }
    dl->PopClipRect();
}
void Intro::DrawCharacterMark(ImDrawList* dl,ImVec2 o,float alpha){
    const auto* def=FindCharacterOpening(opening);if(!def||!dl||alpha<.001f)return;
    const auto pose=CharacterPose(o);const auto p=pose.center;const float r=pose.radius,ceremony=(1-S(t,4.10f,kDockStart))*alpha;
    dl->PushClipRect(o,{o.x+W,o.y+H},true);def->scene(*this,dl,p,r,t,ceremony);
    auto mesh=def->glyph(p,r,t);
    if(def->element==Element::Cryo){for(auto& v:mesh)v.opacity*=alpha;material.Queue(dl,std::move(mesh),p,r,t,0,{p.x+r*2,p.y-r*3});}
    else {
        auto* mat=def->element==Element::Electro?&electroMaterial:def->element==Element::Pyro?&pyroMaterial:def->element==Element::Dendro?&dendroMaterial:&hydroMaterial;
        mat->QueueFormation(dl,std::move(mesh),p,r,finished?std::max(t,energyTime):t,alpha,def->element==Element::Hydro?.50f*ceremony:0.f,p.y+r*1.08f);
    }
    dl->PopClipRect();
}
bool CharacterIntroSelfTest(){
    bool ok=true;
    for(auto opening:{Opening::GanyuFrostLotus,Opening::YaeSesshouSakura,Opening::YoimiyaGoldfish,Opening::NahidaDreamSanctuary,Opening::FurinaWaterStage}){
        bool meshOk=true,dockOk=true,finalOk=true;
        for(float t:{0.f,.5f,1.1f,1.8f,2.4f,3.2f,4.65f,6.2f}){
            const auto mesh=CharacterGlyph(opening,{400,300},120,t);
            meshOk&=mesh.size()<16000&&mesh.size()%3==0;
            for(auto v:mesh)meshOk&=std::isfinite(v.x)&&std::isfinite(v.y)&&v.opacity>=0&&v.opacity<=1.00001f;
        }
        for(auto size:{ImVec2{840,544},ImVec2{1600,900},ImVec2{2560,1440}}){
            Intro intro;intro.Reset(size.x,size.y);intro.opening=opening;intro.dockCenter={45,79};intro.dockRadius=16;
            for(float skip:{.1f,1.40f,2.7f,4.9f}){
                intro.Reset(size.x,size.y);intro.t=skip;intro.Skip();float previous=intro.t;
                for(int j=0;j<120;++j){intro.Update(1.f/120);dockOk&=intro.t>=previous;previous=intro.t;}
                const auto end=intro.CharacterPose({0,0});
                dockOk&=intro.finished&&std::abs(end.center.x-45)<.0001f&&std::abs(end.center.y-79)<.0001f&&std::abs(end.radius-16)<.0001f;
            }
        }
        const auto final=CharacterGlyph(opening,{0,0},1,6.2f);
        for(auto v:final)finalOk&=v.opacity==1.f;
        if(opening==Opening::NahidaDreamSanctuary){const auto& reference=dendro::Mesh();finalOk&=final.size()==reference.size();for(size_t j=0;j<final.size();++j)finalOk&=std::abs(final[j].x-reference[j].x)<.00001f&&std::abs(final[j].y-reference[j].y)<.00001f;}
        if(opening==Opening::FurinaWaterStage){const auto& reference=hydro::Mesh();finalOk&=final.size()==reference.size();for(size_t j=0;j<final.size();++j)finalOk&=std::abs(final[j].x-reference[j].x)<.00001f&&std::abs(final[j].y-reference[j].y)<.00001f;}
        if(!meshOk||!dockOk||!finalOk)printf("character opening %d: mesh=%d dock=%d settled=%d vertices=%zu FAIL\n",int(opening),meshOk,dockOk,finalOk,final.size());
        ok&=meshOk&&dockOk&&finalOk;
    }
    return ok;
}
}
