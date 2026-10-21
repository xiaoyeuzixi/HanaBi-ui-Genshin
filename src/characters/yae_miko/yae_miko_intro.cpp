#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "intro/electro_geometry.h"

namespace hb::yae_miko {
using namespace opening_draw;
void FoxMask(ImDrawList* dl,ImVec2 p,float r,float a){
    if(a<.003f)return;
    const ImVec2 outline[]={P(p,r,0,-.50f),P(p,r,-.33f,-.69f),P(p,r,-.73f,-1.03f),P(p,r,-.64f,-.20f),P(p,r,-.48f,.24f),P(p,r,-.23f,.53f),P(p,r,0,.74f),P(p,r,.23f,.53f),P(p,r,.48f,.24f),P(p,r,.64f,-.20f),P(p,r,.73f,-1.03f),P(p,r,.33f,-.69f)};
    const auto uv=ImGui::GetFontTexUvWhitePixel();const auto base=dl->_VtxCurrentIdx;
    dl->PrimReserve(36,13);dl->PrimWriteVtx(P(p,r,0,-.08f),uv,C(0xE8BEE8,a*.38f));
    for(int j=0;j<12;++j)dl->PrimWriteVtx(outline[j],uv,C(j<6?0x7C4A9E:0xC78DBD,a*(j<6?.26f:.42f)));
    for(int j=0;j<12;++j)for(auto v:{base,base+1+j,base+1+(j+1)%12})dl->PrimWriteIdx(ImDrawIdx(v));
    LightLine(dl,outline,12,0xE9BEEA,a*.59f,.85f,true);
    for(int side:{-1,1}){
        ImVec2 eye[]={P(p,r,side*.13f,-.06f),P(p,r,side*.32f,-.12f),P(p,r,side*.48f,-.24f),P(p,r,side*.40f,-.03f),P(p,r,side*.24f,.06f)};
        dl->AddTriangleFilled(eye[0],eye[2],eye[4],C(0xD9A1EA,a*.42f));
        LightLine(dl,eye,5,0xF2D3F0,a*.82f,1.1f);
        ImVec2 ear[]={P(p,r,side*.44f,-.50f),P(p,r,side*.60f,-.77f),P(p,r,side*.56f,-.35f)};
        LightLine(dl,ear,3,0xC775C8,a*.5f,.8f);
        const auto x=P(p,r,side*.36f,.22f);Petal(dl,x,r*.20f,side<0?.4f:pi-.4f,0xEA8ECB,a*.7f,.16f);
    }
    const ImVec2 mark[]={P(p,r,0,-.38f),P(p,r,-.055f,-.22f),P(p,r,0,-.12f),P(p,r,.055f,-.22f)};
    dl->AddConvexPolyFilled(mark,4,C(0xF1CADF,a*.6f));
    const ImVec2 nose[]={P(p,r,-.06f,.29f),P(p,r,.06f,.29f),P(p,r,0,.38f)};
    dl->AddTriangleFilled(nose[0],nose[1],nose[2],C(0xEDBCE3,a*.8f));
}

void SakuraTotem(ImDrawList* dl,ImVec2 p,float r,float t,float a){
    if(a<.002f)return;
    Halo(dl,P(p,r,0,-.13f),r*.75f,0xB664D3,a*.22f);
    ImVec2 stem[]={P(p,r,-.12f,.70f),P(p,r,-.09f,-.25f),P(p,r,0,-.52f),P(p,r,.09f,-.25f),P(p,r,.12f,.70f)};
    dl->AddConvexPolyFilled(stem,5,C(0x7E459A,a*.63f));LightLine(dl,stem,5,0xD19AD8,a*.65f,.8f,true);
    dl->AddTriangleFilled(P(p,r,0,-.52f),P(p,r,0,.70f),P(p,r,.12f,.70f),C(0xE4ABD8,a*.22f));
    dl->AddLine(P(p,r,0,-.18f),P(p,r,0,.61f),C(0xF1D8EE,a*.58f),.75f);
    for(int level=0;level<3;++level){const float y=.15f+level*.17f;
        const ImVec2 seal[]={P(p,r,0,y-.035f),P(p,r,.035f,y),P(p,r,0,y+.035f),P(p,r,-.035f,y)};
        dl->AddConvexPolyFilled(seal,4,C(0xEAC9E7,a*.52f));
    }
    for(int side:{-1,1}){
        ImVec2 branch[]={P(p,r,0,.15f),P(p,r,side*.25f,-.02f),P(p,r,side*.36f,-.36f),P(p,r,side*.25f,-.56f)};
        LightLine(dl,branch,4,0xC891DC,a*.7f,1.1f);
        ImVec2 roof[]={P(p,r,side*.48f,-.38f),P(p,r,side*.25f,-.48f),P(p,r,0,-.45f)};
        LightLine(dl,roof,3,0xF1D5ED,a*.8f,1.2f);
    }
    FoxMask(dl,P(p,r,0,-.31f),r*.22f,a*.9f);
    for(int j=0;j<5;++j)Petal(dl,P(p,r,0,-.85f),r*.25f,j*tau/5+t*.12f,0xE8AAD9,a*.82f,.37f);
    Star(dl,P(p,r,0,-.85f),r*.06f,0xF8E5FA,a);
    const ImVec2 foot[]={P(p,r,-.24f,.74f),P(p,r,0,.67f),P(p,r,.24f,.74f)};
    LightLine(dl,foot,3,0xDDA6DF,a*.60f,.8f);
}

void Lightning(ImDrawList* dl,ImVec2 a,ImVec2 b,float age,float alpha,int seed){
    const float grown=S(age,0,.30f);if(alpha<.002f||grown<=0)return;
    ImVec2 points[33];const float dx=b.x-a.x,dy=b.y-a.y,len=std::max(.01f,std::hypot(dx,dy));
    for(int j=0;j<=32;++j){const float u=j/32.f*grown;
        const float offset=(sinf(u*43+seed)*.043f+sinf(u*91+seed*4)*.025f)*sinf(u*pi)*len;
        points[j]={a.x+dx*u-dy/len*offset,a.y+dy*u+dx/len*offset};
    }
    LightLine(dl,points,33,0xE9B7FF,alpha,1.0f);
    for(int branch:{11,21}){const auto p=points[branch];const float direction=branch==11?1.f:-1.f;
        ImVec2 fork[]={p,{p.x+dx*.10f-dy*.14f*direction,p.y+dy*.10f+dx*.14f*direction},{p.x+dx*.21f-dy*.13f*direction,p.y+dy*.21f+dx*.13f*direction}};
        LightLine(dl,fork,3,0xC88CE8,alpha*.48f,.7f);
    }
}
CrystalVertex Interpolate(CrystalVertex a,CrystalVertex b,float k){
    return {Lerp(a.x,b.x,k),Lerp(a.y,b.y,k),Lerp(a.nx,b.nx,k),Lerp(a.ny,b.ny,k),Lerp(a.nz,b.nz,k),Lerp(a.u,b.u,k),Lerp(a.v,b.v,k),Lerp(a.material,b.material,k),Lerp(a.opacity,b.opacity,k),Lerp(a.thickness,b.thickness,k),a.seed};
}
struct SakuraBlade{std::vector<CrystalVertex> mesh;ImVec2 center;};
const std::array<SakuraBlade,3>& SakuraBlades(){
    static const auto blades=[](){
        std::array<SakuraBlade,3> result;const auto reference=electro::Vertices({0,0},1,6.2f);
        for(int sector=0;sector<3;++sector){const float angle=-pi*.5f+sector*tau/3,lo=angle-pi/3,hi=angle+pi/3;
            const ImVec2 normals[]={{-sinf(lo),cosf(lo)},{sinf(hi),-cosf(hi)}};auto& blade=result[sector];
            for(size_t j=0;j<reference.size();j+=3){std::vector<CrystalVertex> polygon{reference[j],reference[j+1],reference[j+2]};
                for(auto normal:normals){if(polygon.empty())break;std::vector<CrystalVertex> clipped;auto a=polygon.back();float da=a.x*normal.x+a.y*normal.y;
                    for(auto b:polygon){const float db=b.x*normal.x+b.y*normal.y;
                        if((da>=0)!=(db>=0))clipped.push_back(Interpolate(a,b,da/(da-db)));
                        if(db>=0)clipped.push_back(b);a=b;da=db;
                    }polygon=std::move(clipped);
                }
                for(size_t k=1;k+1<polygon.size();++k)for(auto v:{polygon[0],polygon[k],polygon[k+1]})blade.mesh.push_back(v);
            }
            float area=0,x=0,y=0;
            for(size_t j=0;j<blade.mesh.size();j+=3){const auto a=blade.mesh[j],b=blade.mesh[j+1],c=blade.mesh[j+2];const float w=std::abs((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x));area+=w;x+=(a.x+b.x+c.x)*w/3;y+=(a.y+b.y+c.y)*w/3;}
            blade.center={x/std::max(.001f,area),y/std::max(.001f,area)};
        }return result;
    }();return blades;
}

void DrawScene(Intro&,ImDrawList* dl,ImVec2 p,float r,float t,float a){
    constexpr ImVec2 locations[]={{0,-1.12f},{1.42f,.72f},{-1.42f,.72f}};
    for(int j=0;j<3;++j){const auto q=P(p,r,locations[j].x,locations[j].y);const float birth=.14f+j*.22f,up=S(t,birth,birth+.7f);
        const float k=up*(1-S(t,2.7f+j*.13f,3.7f+j*.13f))*a;
        SakuraTotem(dl,{q.x,q.y+r*.25f*(1-up)},r*.49f,t,k);
        const float discharge=Window(t,1.42f+j*.15f,1.77f+j*.15f,2.6f+j*.13f,3.4f+j*.13f)*a;
        Lightning(dl,P(q,r,0,-.34f),p,t-1.42f-j*.15f,discharge*.82f,j+7);
    }
    const float pulse=expf(-powf((t-2.38f)/.32f,2))*a;
    Halo(dl,p,r*2.0f,0xAA5BCC,pulse*.33f);
    for(int j=0;j<38;++j){const float age=std::max(0.f,t-.40f-Hash(j+40)*1.25f),angle=Hash(j+48)*tau+age*.26f;
        const float d=.6f+Hash(j+90)*1.6f,fade=Window(t,.6f,1.3f,3.75f,4.6f)*a;
        const auto q=P(p,r,cosf(angle)*d,sinf(angle)*d*.64f+age*.065f);
        Petal(dl,q,r*(.035f+Hash(j+65)*.035f),angle+t*.4f,0xD6A2DF,fade*(.22f+Hash(j+72)*.5f),.42f);
    }
}
std::vector<CrystalVertex> BuildGlyph(ImVec2 p,float r,float t){
    std::vector<CrystalVertex> mesh;
        constexpr ImVec2 source[]={{0,-1.12f},{1.42f,.72f},{-1.42f,.72f}};
        const auto& blades=SakuraBlades();
        for(int j=0;j<3;++j){const auto& blade=blades[j];const float k=S(t,1.06f+j*.12f,3.02f+j*.12f),angle=(1-k)*.5f,c=cosf(angle),s=sinf(angle),scale=.23f+.77f*k;
            const float x=Lerp(source[j].x,blade.center.x,k)+sinf(k*pi)*(j==0?.20f:j==1?.12f:-.12f),y=Lerp(source[j].y-.34f,blade.center.y,k)-sinf(k*pi)*.16f;
            for(auto v:blade.mesh){const float localX=v.x-blade.center.x,localY=v.y-blade.center.y;
                v.x=p.x+(x+(localX*c-localY*s)*scale)*r;v.y=p.y+(y+(localX*s+localY*c)*scale)*r;
                v.opacity*=S(t,.10f+j*.20f,.52f+j*.20f);mesh.push_back(v);
            }
        }

    return mesh;
}
void DrawBackdrop(Intro&,ImDrawList*,ImVec2){}
CharacterOpeningDef OpeningInfo(){return {Opening::YaeSesshouSakura,Element::Electro,.182f,0x130A20,0x2A1531,0xCA87D7,DrawScene,BuildGlyph,DrawBackdrop};}
}
