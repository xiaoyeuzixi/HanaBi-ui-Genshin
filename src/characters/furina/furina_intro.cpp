#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "intro/hydro_geometry.h"

namespace hb::furina {
using namespace opening_draw;
void Curtains(ImDrawList* dl,ImVec2 o,float W,float H,float t,float a){
    if(a<.002f)return;
    const float open=S(t,.10f,1.86f);const auto uv=ImGui::GetFontTexUvWhitePixel();
    constexpr int nx=34,ny=36;
    for(int side:{-1,1}){const auto base=dl->_VtxCurrentIdx;dl->PrimReserve(nx*ny*6,(nx+1)*(ny+1));
        for(int iy=0;iy<=ny;++iy)for(int ix=0;ix<=nx;++ix){const float u=ix/float(nx),v=iy/float(ny);
            const float inner=W*(.485f-open*(.39f+.06f*sinf(v*pi)));
            const float x=side<0?inner*u:W-inner*u,y=H*v;
            const float fold=powf(.5f+.5f*cosf(u*11*tau+.30f*sinf(v*4+t)),2);
            const float edge=S(u,.91f,1),shade=.28f+.63f*fold;
            const unsigned red=unsigned(8+shade*19+edge*29),green=unsigned(15+shade*37+edge*48),blue=unsigned(32+shade*68+edge*61);
            dl->PrimWriteVtx({o.x+x,o.y+y},uv,IM_COL32(red,green,blue,int(a*(.91f-.10f*fold)*255)));
        }
        for(int iy=0;iy<ny;++iy)for(int ix=0;ix<nx;++ix){const auto v=base+iy*(nx+1)+ix;for(auto i:{v,v+1,v+nx+1,v+1,v+nx+2,v+nx+1})dl->PrimWriteIdx(ImDrawIdx(i));}
        ImVec2 edge[81];for(int j=0;j<=80;++j){const float v=j/80.f,inner=W*(.485f-open*(.39f+.06f*sinf(v*pi)));edge[j]={o.x+(side<0?inner:W-inner),o.y+H*v};}
        LightLine(dl,edge,81,0xB5C8E3,a*.32f,.85f);
    }
}

void WaterStage(ImDrawList* dl,ImVec2 center,float r,float t,float a){
    const float assemble=S(t,.25f,1.25f);if(a<.002f||assemble<=0)return;
    const auto uv=ImGui::GetFontTexUvWhitePixel();constexpr int n=160;
    Halo(dl,{center.x,center.y+r*.18f},r*2.75f,r*.65f,0x0B1B32,a*.65f);
    for(int tier=2;tier>=0;--tier){const float rx=r*(1.87f+tier*.13f)*assemble,ry=r*(.31f+tier*.038f)*assemble;
        const float y=center.y+tier*r*.075f,depth=r*.045f;const auto base=dl->_VtxCurrentIdx;
        dl->PrimReserve(n*6,(n+1)*2);
        for(int j=0;j<=n;++j){const float angle=j*tau/n,x=cosf(angle)*rx,yy=sinf(angle)*ry;
            const float front=S(sinf(angle),-.1f,.45f),shine=powf(.5f+.5f*cosf(angle+.7f),6);
            dl->PrimWriteVtx({center.x+x,y+yy},uv,C(tier?0x55738E:0x83BDD5,a*(.20f+shine*.18f)*(.4f+.6f*front)));
            dl->PrimWriteVtx({center.x+x,y+yy+depth},uv,C(0x061426,a*(.42f+.30f*front)));
        }
        for(int j=0;j<n;++j){const auto v=base+j*2;for(auto i:{v,v+1,v+2,v+1,v+3,v+2})dl->PrimWriteIdx(ImDrawIdx(i));}
        ImVec2 rim[n+1];for(int j=0;j<=n;++j){const float angle=j*tau/n;rim[j]={center.x+cosf(angle)*rx,y+sinf(angle)*ry};}
        LightLine(dl,rim,n+1,tier?0x6F9FBE:0xB4D7E8,a*(tier?.17f:.32f),tier?.65f:.9f);
    }
    const float rx=r*1.87f*assemble,ry=r*.31f*assemble;
    const auto base=dl->_VtxCurrentIdx;dl->PrimReserve(n*3,n+1);
    dl->PrimWriteVtx(center,uv,C(0x3A7397,a*.28f));
    for(int j=0;j<n;++j){const float angle=j*tau/n;dl->PrimWriteVtx({center.x+cosf(angle)*rx,center.y+sinf(angle)*ry},uv,C(0x102A43,a*.55f));}
    for(int j=0;j<n;++j)for(auto i:{base,base+1+j,base+1+(j+1)%n})dl->PrimWriteIdx(ImDrawIdx(i));
    for(int lane=0;lane<6;++lane){const float y=.02f+lane*.045f,w=sqrtf(std::max(0.f,1-powf(y/.31f,2)))*1.78f;
        Ribbon(dl,P(center,r,-w,y),P(center,r,-.5f,y-.035f+sinf(t*.8f+lane)*.014f),P(center,r,.65f,y+.026f),P(center,r,w,y),r*.005f,0x7DB6D2,a*.12f);
    }
    for(int side:{-1,1}){const auto p=P(center,r,side*1.86f,.03f);
        ImVec2 ornament[]={P(p,r,0,-.09f),P(p,r,side*.07f,0),P(p,r,0,.09f),P(p,r,-side*.07f,0)};
        LightLine(dl,ornament,4,0xC9DAE7,a*.44f,.75f,true);
        Star(dl,p,r*.016f,0xD7EDF6,a*.64f);
    }
}

void DrawScene(Intro&,ImDrawList* dl,ImVec2 p,float r,float t,float a){
    const float stage=Window(t,.30f,1.10f,4.02f,4.65f)*a;
    const auto water=P(p,r,0,1.08f);
    Halo(dl,water,r*2.4f,r*.42f,0x2D81B5,stage*.31f);
    WaterStage(dl,water,r,t,stage);
    for(int j=0;j<3;++j){const float age=t-.99f-j*.19f;
        if(age<0)continue;const float fade=stage*S(age,0,.25f)*(1-S(age,1.25f,3.15f));
        Ring(dl,water,r*(.16f+age*.59f),r*(.025f+age*.095f),0xA1E8FB,fade*(.27f-j*.045f));
    }
    const float drop=Window(t,.15f,.38f,1.02f,1.17f)*a,fall=S(t,.15f,1.07f);
    const auto q=P(p,r,0,Lerp(-1.6f,1.08f,fall));
    Pearl(dl,q,r*.08f,t,drop,true);
    Ribbon(dl,P(q,r,0,-.55f),P(q,r,-.03f,-.29f),P(q,r,.02f,-.15f),q,r*.028f,0xBFEFFF,drop*.3f);
    for(int side:{-1,1}){const float k=Window(t,.65f,1.20f,3.85f,4.55f)*a;
        Ribbon(dl,P(p,r,side*1.85f,-2),P(p,r,side*1.15f,-.7f),P(p,r,side*1.2f,.7f),P(p,r,side*.25f,1.1f),r*.23f,0x77BCED,k*.10f);
    }
    const float burst=Window(t,1.05f,1.22f,2.0f,2.85f)*a;
    for(int j=0;j<40;++j){const float age=std::max(0.f,t-1.07f),angle=j*tau/40;
        const float d=age*(.65f+Hash(j+802)*.65f),h=-sinf(angle)*d*.75f+age*age*.40f;
        const auto q2=P(water,r,cosf(angle)*d,h);
        Halo(dl,q2,r*.023f,0x9BE3FF,burst*.35f);dl->AddCircleFilled(q2,std::max(.6f,r*.008f),C(0xC2F1FF,burst*.7f),10);
    }
    Halo(dl,p,r*1.8f,0x65C0EC,expf(-powf((t-2.7f)/.43f,2))*a*.21f);
}
std::vector<CrystalVertex> BuildGlyph(ImVec2 p,float r,float t){
    std::vector<CrystalVertex> mesh;
        mesh.reserve(hydro::Mesh().size());const float rise=S(t,1.06f,2.93f);
        for(auto v:hydro::Mesh()){
            const float y=v.y+(1-rise)*1.92f,x=v.x+sinf(v.y*6-t*3)*.065f*(1-rise);
            const float show=(1-S(y,1.02f,1.14f))*S(t,1.08f,1.5f);
            mesh.push_back({p.x+x*r,p.y+y*r,v.nx,v.ny,v.edge,v.x,v.y,.5f,show,v.travel,0});
        }

    return mesh;
}
void DrawBackdrop(Intro& intro,ImDrawList* dl,ImVec2 o){
    const float t=intro.t,fade=1-S(t,Intro::kDockStart,5.55f);
    Curtains(dl,o,intro.W,intro.H,t,fade);
}
CharacterOpeningDef OpeningInfo(){return {Opening::FurinaWaterStage,Element::Hydro,.215f,0x07111F,0x10243D,0x7EC3EC,DrawScene,BuildGlyph,DrawBackdrop};}
}
