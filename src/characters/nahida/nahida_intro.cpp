#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "intro/dendro_geometry.h"

namespace hb::nahida {
using namespace opening_draw;
void DreamPalace(ImDrawList* dl,ImVec2 p,float r,float t,float alpha){
    const float active=Window(t,.62f,1.72f,3.74f,4.62f)*alpha,open=S(t,.62f,2.04f);
    if(active<.002f)return;
    auto world=[&](float az,float u){const float radial=cosf(u*pi*.5f)*2.00f*open,height=sinf(u*pi*.5f)*2.62f*open;
        return P(p,r,cosf(az)*radial,1.02f-height+sinf(az)*radial*.22f);
    };
    Halo(dl,P(p,r,0,-.32f),r*1.85f,r*1.65f,0x8EB781,active*.045f);
    constexpr int panels=8;std::array<int,panels> order;for(int j=0;j<panels;++j)order[j]=j;
    std::sort(order.begin(),order.end(),[](int a,int b){return sinf(a*tau/panels)<sinf(b*tau/panels);});
    const auto uv=ImGui::GetFontTexUvWhitePixel();
    for(int j:order){const float az=j*tau/panels,front=.24f+.76f*(sinf(az)*.5f+.5f),k=active*front;
        constexpr int n=48;std::array<ImVec2,n+1> left,right,mid;
        for(int v=0;v<=n;++v){const float u=v/float(n),latitude=.035f+u*.94f,width=powf(std::max(0.f,sinf(u*pi)),.85f)*.335f;
            left[v]=world(az-width,latitude);right[v]=world(az+width,latitude);mid[v]=world(az,latitude);
        }
        dl->PrimReserve(n*12,(n+1)*3);const auto base=dl->_VtxCurrentIdx;
        for(int v=0;v<=n;++v){const float u=v/float(n),light=powf(std::max(0.f,sinf(u*pi)),.5f)*k;
            dl->PrimWriteVtx(left[v],uv,C(0x70B68B,light*.07f));
            dl->PrimWriteVtx(mid[v],uv,C(0xBCE2AB,light*.22f));
            dl->PrimWriteVtx(right[v],uv,C(0x70B68B,light*.07f));
        }
        for(int v=0;v<n;++v)for(int side=0;side<2;++side){const auto index=base+v*3+side;for(auto i:{index,index+1,index+3,index+1,index+4,index+3})dl->PrimWriteIdx(ImDrawIdx(i));}
        LightLine(dl,left.data(),n+1,0xD4E5B7,k*.27f,.85f);
        LightLine(dl,right.data(),n+1,0xD4E5B7,k*.27f,.85f);
        for(int inset=0;inset<2;++inset){ImVec2 innerL[41],innerR[41];
            for(int v=0;v<=40;++v){const float u=v/40.f,latitude=.16f+u*.67f,width=sinf(u*pi)*(.24f-inset*.055f);
                innerL[v]=world(az-width,latitude);innerR[v]=world(az+width,latitude);
            }
            LightLine(dl,innerL,41,inset?0xCEE0A8:0x94CAA2,k*(inset?.20f:.16f),.65f);
            LightLine(dl,innerR,41,inset?0xCEE0A8:0x94CAA2,k*(inset?.20f:.16f),.65f);
        }
        for(int vein=0;vein<5;++vein)for(int side:{-1,1}){const float u=.23f+vein*.105f,width=sinf((u-.035f)/.94f*pi)*.27f;
            ImVec2 stem[17];for(int v=0;v<=16;++v){const float f=v/16.f;stem[v]=world(az+side*width*f,u+.065f*sinf(f*pi*.5f));}
            LightLine(dl,stem,17,0xAACD9E,k*.12f,.6f);
        }
        const auto foot=world(az,.015f);Star(dl,foot,r*.017f,0xD4E5B1,k*.58f,pi*.25f);
    }
    for(int lane=0;lane<2;++lane){ImVec2 pattern[241];
        for(int j=0;j<=240;++j){const float angle=j*tau/240,rad=(1.88f+lane*.09f)+.055f*cosf(angle*panels);
            pattern[j]=P(p,r,cosf(angle)*rad*open,1.02f+sinf(angle)*rad*.22f*open);
        }
        LightLine(dl,pattern,241,0xC5D9A6,active*(lane?.17f:.24f),.7f);
    }
    const auto crown=P(p,r,0,1.02f-2.62f*open);
    for(int j=0;j<6;++j)Petal(dl,crown,r*.14f,j*tau/6-pi*.5f,0xCDE8AF,active*.42f,.22f);
    Star(dl,crown,r*.026f,0xEEFFD5,active*.65f);
}

void DrawScene(Intro&,ImDrawList* dl,ImVec2 p,float r,float t,float a){
    const float awake=Window(t,.12f,.65f,3.8f,4.65f)*a;
    const ImVec2 base=P(p,r,0,1.04f);
    Lotus(dl,base,r*1.15f,t*.93f,awake,true);
    const float lift=S(t,.48f,2.75f),seed=Window(t,.18f,.6f,1.65f,2.45f)*a;
    Halo(dl,P(p,r,0,.88f*(1-lift)),r*.30f,0xADDDA1,seed*.25f);
    DreamPalace(dl,p,r,t,a);
    const float orbit=Window(t,1.0f,1.6f,3.45f,4.45f)*a;
    for(int j=0;j<10;++j){const float angle=j*tau/10+t*.12f;
        const auto q=P(p,r,cosf(angle)*1.46f,sinf(angle)*1.08f-.05f);
        Petal(dl,q,r*.25f,angle-.6f,0xB9E89C,orbit*.50f,.32f);
        if(j%2==0)Star(dl,q,r*.018f,0xE5FBC4,orbit*.7f,pi*.25f);
    }
    Halo(dl,p,r*1.65f,0x81D29C,expf(-powf((t-2.42f)/.4f,2))*a*.30f);
    for(int j=0;j<55;++j){const float u=std::fmod(Hash(j+700)+t*.13f,1.f),angle=Hash(j+724)*tau+t*.15f;
        const float d=.35f+Hash(j+733)*1.8f;const auto q=P(p,r,cosf(angle)*d,1.3f-u*3.1f);
        const float k=awake*sinf(u*pi)*(.18f+Hash(j+749)*.5f);
        if(j%6==0)Star(dl,q,r*.018f,0xE9FBC7,k,pi*.25f);
        else{Halo(dl,q,r*.025f,0xBCF4A4,k*.35f);dl->AddCircleFilled(q,std::max(.65f,r*.005f),C(0xDEF8C2,k),8);}
    }
}
std::vector<CrystalVertex> BuildGlyph(ImVec2 p,float r,float t){
    std::vector<CrystalVertex> mesh;
        mesh.reserve(dendro::Mesh().size());const float bloom=S(t,.78f,3.14f),lift=S(t,.48f,2.75f);
        for(auto v:dendro::Mesh()){
            const float fold=(1-bloom)*1.43f,perspective=1/(1+std::abs(v.x)*sinf(fold)*.26f);
            const float x=v.x*cosf(fold)*perspective,y=.88f*(1-lift)+v.y*(.23f+.77f*bloom);
            mesh.push_back({p.x+x*r,p.y+y*r,0,0,v.edge,v.x,v.y,.5f,S(t,.04f,.45f),v.travel,0});
        }

    return mesh;
}
void DrawBackdrop(Intro& intro,ImDrawList* dl,ImVec2 o){intro.DrawSanctuaryScenery(dl,o);}
CharacterOpeningDef OpeningInfo(){return {Opening::NahidaDreamSanctuary,Element::Dendro,.215f,0x091A1A,0x14352A,0xAFD994,DrawScene,BuildGlyph,DrawBackdrop};}
}
