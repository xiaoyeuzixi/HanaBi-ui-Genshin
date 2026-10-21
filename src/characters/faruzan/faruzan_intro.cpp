#include "intro/intro.h"
#include "intro/anemo_geometry.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace hb {
namespace {
constexpr float pi=3.14159265359f,tau=pi*2;
float S(float t,float a,float b){float u=Clamp01((t-a)/(b-a));return u*u*u*(u*(u*6-15)+10);}
ImU32 C(unsigned rgb,float a){return IM_COL32(rgb>>16&255,rgb>>8&255,rgb&255,int(Clamp01(a)*255));}
float Hash(int i){uint32_t x=uint32_t(i)*747796405u+2891336453u;x=((x>>((x>>28)+4))^x)*277803737u;return float((x>>22^x)&0xffffff)/16777216.f;}
void Halo(ImDrawList* dl,ImVec2 p,float r,unsigned color,float alpha){
    if(alpha<.002f)return;constexpr int n=48;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(n*3,n+1);const auto base=dl->_VtxCurrentIdx;dl->PrimWriteVtx(p,uv,C(color,alpha));
    for(int j=0;j<n;++j){const float a=j*tau/n;dl->PrimWriteVtx({p.x+cosf(a)*r,p.y+sinf(a)*r},uv,C(color,0));}
    for(int j=0;j<n;++j)for(auto i:{base,base+1+j,base+1+(j+1)%n})dl->PrimWriteIdx(ImDrawIdx(i));
}
struct V3{float x,y,z;V3 operator+(V3 b)const{return{x+b.x,y+b.y,z+b.z};}V3 operator-(V3 b)const{return{x-b.x,y-b.y,z-b.z};}V3 operator*(float k)const{return{x*k,y*k,z*k};}};
float Dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 Cross(V3 a,V3 b){return{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
V3 Unit(V3 p){return p*(1/std::max(.0001f,sqrtf(Dot(p,p))));}
V3 Rotate(V3 p,V3 axis,float angle){const float c=cosf(angle),s=sinf(angle);return p*c+Cross(axis,p)*s+axis*(Dot(axis,p)*(1-c));}
struct Facet{std::array<ImVec2,3> p;float depth,light,opacity;};
std::array<Facet,6> Mechanism(ImVec2 center,float r,float t){
    std::array<Facet,6> faces{};
    const float awake=S(t,.06f,1.1f),open=S(t,1.06f,2.45f);
    V3 ring[3];for(int j=0;j<3;++j){const float a=j*tau/3-pi*.5f;ring[j]={cosf(a)*.87f,0,sinf(a)*.87f};}
    auto world=[&](V3 p){p=Rotate(p,{0,1,0},.45f+t*.37f);return Rotate(p,{1,0,0},-.18f);};
    for(int j=0;j<6;++j){const int side=j<3?-1:1,edge=j%3;V3 a=ring[edge],b=ring[(edge+1)%3],pole{0,side*1.12f,0};
        const auto hinge=(a+b)*.5f,axis=Unit(b-a);
        pole=hinge+Rotate(pole-hinge,axis,float(side)*open*1.70f);
        const V3 normal=Unit(Cross(b-a,pole-a));
        const V3 separate=Unit(hinge+V3{0,side*.4f,0})*((1-awake)*1.5f+open*.25f);
        V3 v[]={world(pole+separate),world(a+separate),world(b+separate)};
        auto& f=faces[j];f.depth=(v[0].z+v[1].z+v[2].z)/3;f.light=std::abs(Dot(world(normal),Unit({-.5f,-.6f,.8f})));
        f.opacity=S(t,.04f+j*.035f,.44f+j*.055f)*(1-S(t,2.05f+j*.035f,2.90f+j*.035f));
        for(int k=0;k<3;++k){const float perspective=4/(4+v[k].z*.32f);f.p[k]={center.x+v[k].x*r*perspective,center.y+v[k].y*r*perspective};}
    }
    std::sort(faces.begin(),faces.end(),[](auto a,auto b){return a.depth>b.depth;});return faces;
}
void DrawMechanism(ImDrawList* dl,ImVec2 p,float r,float t,float alpha){
    const auto faces=Mechanism(p,r,t);
    const float glow=S(t,.15f,.85f)*(1-S(t,2.15f,2.85f))*alpha;
    Halo(dl,p,r*2,0x47D8AC,glow*.25f);
    for(const auto& f:faces){const float k=f.opacity*alpha;if(k<.002f)continue;
        const unsigned teal=f.light>.55f?0x82D4B7:0x285E58;
        const ImVec2 c{(f.p[0].x+f.p[1].x+f.p[2].x)/3,(f.p[0].y+f.p[1].y+f.p[2].y)/3};
        dl->AddTriangleFilled(f.p[0],f.p[1],f.p[2],C(teal,k*.80f));
        dl->AddPolyline(f.p.data(),3,C(0x56CFAA,k*.14f),ImDrawFlags_Closed,std::max(1.f,r*.055f));
        dl->AddPolyline(f.p.data(),3,C(0xE7D19D,k*.88f),ImDrawFlags_Closed,std::max(.8f,r*.012f));
        ImVec2 inner[3];for(int j=0;j<3;++j)inner[j]={Lerp(c.x,f.p[j].x,.72f),Lerp(c.y,f.p[j].y,.72f)};
        dl->AddPolyline(inner,3,C(0xC0F2D8,k*.63f),ImDrawFlags_Closed,std::max(.65f,r*.008f));
        const ImVec2 tip{Lerp(c.x,f.p[0].x,.53f),Lerp(c.y,f.p[0].y,.53f)};
        dl->AddTriangleFilled(inner[0],{Lerp(inner[0].x,inner[1].x,.26f),Lerp(inner[0].y,inner[1].y,.26f)},{Lerp(inner[0].x,inner[2].x,.26f),Lerp(inner[0].y,inner[2].y,.26f)},C(0xE7D5A6,k*.6f));
        dl->AddLine(tip,c,C(0xF5E9BF,k*.55f),std::max(.6f,r*.008f));
        Halo(dl,c,r*.10f,0xB8FADB,k*.12f);
    }
    const float core=glow*(1-S(t,1.8f,2.55f));
    Halo(dl,p,r*.34f,0xA5FFDC,core*.65f);
    ImVec2 diamond[]={{p.x,p.y-r*.16f},{p.x+r*.10f,p.y},{p.x,p.y+r*.16f},{p.x-r*.10f,p.y}};
    dl->AddConvexPolyFilled(diamond,4,C(0xDEFFEC,core));
}
void WindRibbons(ImDrawList* dl,ImVec2 center,float r,float t,float alpha,bool front){
    const float active=S(t,.55f,1.2f)*(1-S(t,3.55f,4.3f))*alpha;if(active<.002f)return;
    constexpr int n=96;const auto uv=ImGui::GetFontTexUvWhitePixel();
    for(int lane=0;lane<3;++lane){
        const float radius=r*(1.18f+lane*.17f+.27f*S(t,2.6f,3.8f)),head=t*(1.7f+lane*.11f)+lane*tau/3;
        auto point=[&](float u){const float a=head-u*4.9f,x=cosf(a)*radius,y=sinf(a)*radius*.28f;return ImVec2{center.x+x,center.y+y+x*(lane==1?-.26f:.22f)};};
        dl->PrimReserve(n*12,(n+1)*3);const auto base=dl->_VtxCurrentIdx;
        for(int j=0;j<=n;++j){const float u=j/float(n),a=head-u*4.9f;const auto p=point(u),q=point(u+.001f);const float dx=q.x-p.x,dy=q.y-p.y,len=std::max(.001f,std::hypot(dx,dy));
            float k=powf(std::max(0.f,sinf(pi*u)),1.3f)*active*(.55f-lane*.09f);if((sinf(a)>0)!=front)k=0;
            const float width=r*(.025f-lane*.003f)*(.4f+.6f*sinf(u*pi));
            dl->PrimWriteVtx({p.x-dy/len*width,p.y+dx/len*width},uv,C(0x4EC9A0,0));
            dl->PrimWriteVtx(p,uv,C(lane==1?0xE1EBD1:0xB1FFE1,k));
            dl->PrimWriteVtx({p.x+dy/len*width,p.y-dx/len*width},uv,C(0x4EC9A0,0));
        }
        for(int j=0;j<n;++j)for(int k=0;k<2;++k){const auto v=base+j*3+k;for(auto i:{v,v+1,v+3,v+1,v+4,v+3})dl->PrimWriteIdx(ImDrawIdx(i));}
    }
}
std::vector<CrystalVertex> WindMesh(ImVec2 p,float r,float t){
    std::vector<CrystalVertex> mesh;mesh.reserve(anemo::Mesh().size());
    const float wind=S(t,1.25f,1.75f)*(1-S(t,2.35f,3.25f));
    for(auto v:anemo::Mesh()){
        const float x=v.x+wind*.055f*sinf(v.y*5-t*3.1f),y=v.y+wind*.025f*sinf(v.x*7+t*3.4f);
        mesh.push_back({p.x+x*r,p.y+y*r,v.nx,v.ny,v.edge,v.x,v.y,.5f,1,0,0});
    }return mesh;
}
}
bool FaruzanIntroSelfTest(){
    bool ok=true;
    for(float t:{0.f,.5f,1.f,1.5f,2.f,2.5f,3.f,4.65f,6.2f}){
        const auto faces=Mechanism({400,300},130,t);
        for(const auto& f:faces){for(auto p:f.p)ok&=std::isfinite(p.x)&&std::isfinite(p.y);ok&=f.opacity>=0&&f.opacity<=1;}
        for(auto v:WindMesh({400,300},130,t))ok&=std::isfinite(v.x)&&std::isfinite(v.y);
    }
    const auto final=WindMesh({0,0},1,6.2f);const auto& reference=anemo::Mesh();
    for(size_t j=0;j<final.size();++j)ok&=final[j].x==reference[j].x&&final[j].y==reference[j].y;
    return ok;
}
void Intro::DrawFaruzanEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;const float large=S(radius,22,65),ceremony=alpha*large;
    const float awaken=S(time,.04f,.55f)*(1-S(time,2.4f,3.25f))*ceremony;
    Halo(dl,center,radius*1.7f,0x3DAA87,awaken*.18f);
    WindRibbons(dl,center,radius,time,ceremony,false);
    DrawMechanism(dl,center,radius*.68f,time,ceremony);
    const float crest=expf(-powf((time-3.24f)/.28f,2))*ceremony;
    Halo(dl,center,radius*2.15f,0x57D5AB,crest*.24f);
    faruzanMaterial.Queue(dl,WindMesh(center,radius,time),center,radius,time,alpha);
    WindRibbons(dl,center,radius,time,ceremony,true);
    const float motes=S(time,.5f,1.25f)*(1-S(time,4.0f,4.6f))*ceremony;
    for(int j=0;j<70;++j){
        const float u=std::fmod(Hash(j+51)+time*(.14f+Hash(j+61)*.10f),1.f),angle=u*tau+Hash(j+71)*tau;
        const float distance=(1.62f-u*.95f)*(1+.30f*S(time,3.f,4.f));
        const ImVec2 p{center.x+cosf(angle)*radius*distance,center.y+sinf(angle)*radius*distance*.60f};
        const float k=motes*sinf(u*pi)*(.2f+Hash(j+91)*.6f),r=radius*(.003f+Hash(j+101)*.006f);
        Halo(dl,p,r*5,0x90E4BD,k*.24f);
        const float a=angle+pi*.5f;dl->AddLine({p.x-cosf(a)*r*2,p.y-sinf(a)*r},{p.x+cosf(a)*r,p.y+sinf(a)*r*.5f},C(0xDDFFEC,k),std::max(.65f,r*.7f));
    }
}
void Intro::DrawFaruzanAtmosphere(ImDrawList* dl,ImVec2 o){
    DrawAnemoAtmosphere(dl,o);DrawArchiveScenery(dl,o);
    const float fade=(1-S(t,kDockStart,5.55f))*S(t,.18f,1.4f),s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    for(int side:{-1,1}){
        const float x=o.x+W*.5f+side*W*.32f,y=o.y+H*.76f;
        const float r=s*.13f;
        ImVec2 slab[]={{x-side*r*.60f,y-r*.12f},{x,y-r*.75f},{x+side*r*.60f,y-r*.12f},{x,y+r*.24f}};
        dl->AddConvexPolyFilled(slab,4,C(0xA0C4A4,fade*.035f));dl->AddPolyline(slab,4,C(0xD9C994,fade*.22f),ImDrawFlags_Closed,.7f);
        for(int j=0;j<5;++j){const float yy=y-r*.45f+j*r*.13f,xx=x+side*r*.15f;
            ImVec2 rune[]={{xx,yy},{xx+side*r*.18f,yy},{xx+side*r*.18f,yy-r*.06f},{xx+side*r*.26f,yy-r*.06f}};
            dl->AddPolyline(rune,4,C(0xC4D5B1,fade*(.12f+.06f*sinf(t*.6f+j))),0,.7f);
        }
    }
    dl->PopClipRect();
}
}
