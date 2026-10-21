#include "intro/intro.h"
#include "intro/anemo_geometry.h"
#include "core/easing.h"
#include <algorithm>
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
template<class Curve>void Silk(ImDrawList* dl,Curve curve,float lo,float hi,float width,float alpha,unsigned color){
    if(alpha<.002f||hi<=lo)return;constexpr int n=64;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(n*12,(n+1)*3);const auto base=dl->_VtxCurrentIdx;
    for(int j=0;j<=n;++j){const float u=j/float(n),v=Lerp(lo,hi,u);const auto p=curve(v),q=curve(v+.0005f);
        const float dx=q.x-p.x,dy=q.y-p.y,len=std::max(.001f,std::hypot(dx,dy));
        const float k=powf(std::max(0.f,sinf(pi*u)),1.3f),r=width*k;
        dl->PrimWriteVtx({p.x-dy/len*r,p.y+dx/len*r},uv,C(color,0));
        dl->PrimWriteVtx(p,uv,C(color,alpha*k));
        dl->PrimWriteVtx({p.x+dy/len*r,p.y-dx/len*r},uv,C(color,0));
    }
    for(int j=0;j<n;++j)for(int k=0;k<2;++k){const auto v=base+j*3+k;for(auto i:{v,v+1,v+3,v+1,v+4,v+3})dl->PrimWriteIdx(ImDrawIdx(i));}
}
anemo::Step Along(const std::vector<anemo::Step>& path,float value){
    auto at=std::lower_bound(path.begin(),path.end(),value,[](auto a,float b){return a.travel<b;});
    if(at==path.begin())return path.front();if(at==path.end())return path.back();const auto a=*(at-1),b=*at;
    const float k=Clamp01((value-a.travel)/std::max(.00001f,b.travel-a.travel));return {Lerp(a.x,b.x,k),Lerp(a.y,b.y,k),value};
}
std::vector<CrystalVertex> Vertices(ImVec2 p,float r){
    std::vector<CrystalVertex> mesh;mesh.reserve(anemo::Mesh().size());
    for(auto v:anemo::Mesh())mesh.push_back({p.x+v.x*r,p.y+v.y*r,v.nx,v.ny,v.edge,v.x,v.y,.5f,1,v.travel,0});
    return mesh;
}
void Prism(ImDrawList* dl,ImVec2 center,float r,float time,float alpha){
    if(alpha<.002f)return;
    ImVec2 top{center.x,center.y-r},bottom{center.x,center.y+r},ring[3];float depth[3];
    for(int j=0;j<3;++j){float a=time*.24f+j*tau/3;ring[j]={center.x+cosf(a)*r*.9f,center.y+sinf(a)*r*.24f};depth[j]=sinf(a);}
    for(int j=0;j<3;++j){int k=(j+1)%3;
        dl->AddTriangleFilled(top,ring[j],ring[k],C(j%2?0x73DAC0:0xD4E9CB,alpha*.10f));
        dl->AddTriangleFilled(bottom,ring[j],ring[k],C(0x4CBAAA,alpha*.08f));
        const auto line=C(depth[j]>.0f?0xDDCA91:0x77B8AD,alpha*(depth[j]>0?.65f:.22f));
        dl->AddLine(top,ring[j],line,.85f);dl->AddLine(bottom,ring[j],line,.85f);
        dl->AddLine(ring[j],ring[k],C(0xC2DEC3,alpha*.33f),.7f);
    }
    Halo(dl,center,r*.95f,0x61D5B4,alpha*.10f);
}
}
namespace anemo {
bool SelfTest(){
    bool ok=!Mesh().empty()&&Mesh().size()<16000&&Mesh().size()%3==0&&Paths().size()==7;
    for(auto v:Mesh())ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.travel)&&v.travel>=0&&v.travel<1.02f&&v.edge>=0;
    for(const auto& path:Paths()){float previous=-1;ok=ok&&!path.empty();for(auto v:path){ok=ok&&v.travel>=previous;previous=v.travel;}}
    for(float r:{12.f,130.f,240.f})for(auto v:Vertices({400,300},r))ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y);
    return ok;
}
}
void Intro::DrawAnemoEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;
    const float large=S(radius,22,65),growth=S(time,.50f,2.98f)*1.09f;
    const float gather=S(time,.06f,.28f)*(1-S(time,.60f,1.f))*alpha*large;
    Halo(dl,{center.x,center.y+radius*.43f},radius*.27f,0x80F7CF,gather*.65f);
    for(int side:{-1,1}){
        auto curve=[&](float u){const float distance=(1-u)*1.8f;
            return ImVec2{center.x+side*radius*(distance+.12f*sinf(u*pi*2)),center.y+radius*(.43f+.27f*sinf(u*pi*2)-distance*.12f)};};
        const float head=S(time,.04f,.57f);
        Silk(dl,curve,std::max(0.f,head-.55f),head,radius*.028f,gather*.9f,0xC1FFE7);
    }
    const float crest=expf(-powf((time-3.18f)/.30f,2))*large*alpha;
    Halo(dl,center,radius*1.9f,0x45D6A1,crest*.20f);
    anemoMaterial.Queue(dl,Vertices(center,radius),center,radius,time,alpha);
    const float tracing=S(time,.45f,.75f)*(1-S(time,3.0f,3.48f))*alpha*large;
    if(tracing>.002f)for(const auto& path:anemo::Paths()){
        if(growth>path.back().travel+.04f)continue;
        const float hi=std::min(growth,path.back().travel),lo=std::max(path.front().travel,hi-.13f);
        auto curve=[&](float u){const auto p=Along(path,u);return ImVec2{center.x+p.x*radius,center.y+p.y*radius};};
        Silk(dl,curve,lo,hi,radius*.026f,tracing*.72f,0xC3FFEB);
        const auto p=curve(hi);Halo(dl,p,radius*.10f,0xADFFDE,tracing*.28f);
    }
    const float release=S(time,2.7f,3.25f)*(1-S(time,4.2f,4.6f))*alpha*large;
    for(int side:{-1,1})for(int j=0;j<3;++j){
        auto curve=[&](float u){const float a=u*2.5f-.6f+j*.12f;
            return ImVec2{center.x+side*radius*(.50f+u*1.5f),center.y+radius*(.20f-j*.18f-.55f*sinf(a)-.30f*u)};};
        const float head=S(time,2.65f+j*.08f,4.4f+j*.04f);
        Silk(dl,curve,std::max(0.f,head-.72f),head,radius*(.012f+j*.003f),release*(.45f-j*.08f),0xB7FFE0);
    }
    const float motes=S(time,.28f,1.1f)*(1-S(time,4.25f,4.6f))*large*alpha;
    for(int j=0;j<46;++j){
        const float u=std::fmod(Hash(j+17)+time*(.12f+Hash(j+29)*.06f),1.f),side=j%2?1.f:-1.f;
        const float a=u*3.4f+Hash(j+39)*2,dist=.65f+Hash(j+47)*1.1f;
        const ImVec2 p{center.x+side*radius*(.25f+u*1.5f),center.y+radius*(cosf(a)*dist-.15f)};
        const float k=motes*sinf(u*pi)*(.25f+Hash(j+59)*.5f),r=radius*(.004f+Hash(j+71)*.006f);
        Halo(dl,p,r*4,0x86F0BF,k*.25f);
        dl->AddLine({p.x-side*r*2.8f,p.y+r},{p.x+side*r*2,p.y-r*.6f},C(0xE1FFED,k),std::max(.6f,r*.6f));
    }
}
void Intro::DrawAnemoAtmosphere(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.55f),s=std::min(W,H),rise=S(t,.05f,2.3f);
    const auto pose=AnemoPose(o);dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(0x0B211F,.98f*fade),C(0x0C2925,.98f*fade),C(0x071F1D,.98f*fade),C(0x091B1C,.98f*fade));
    Halo(dl,pose.center,pose.radius*3,0x23775D,rise*fade*.30f);
    for(int j=0;j<5;++j){
        auto curve=[&](float u){return ImVec2{o.x+W*(-.15f+u*1.3f),o.y+H*(.60f+j*.047f)+s*(.09f*sinf(u*5+t*.42f+j*.30f)-u*.16f)};};
        const float head=std::fmod(t*.13f+j*.18f,1.5f);
        Silk(dl,curve,std::max(0.f,head-.65f),std::min(1.f,head),s*(.0035f+j*.001f),fade*rise*.11f,0x94DDBA);
    }
    dl->PopClipRect();
}
void Intro::DrawArchiveScenery(ImDrawList* dl,ImVec2 o){
    const float fade=(1-S(t,kDockStart,5.55f))*S(t,.35f,1.8f),s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    Prism(dl,{o.x+W*.17f,o.y+H*.38f+sinf(t*.5f)*s*.018f},s*.068f,t,fade*.66f);
    Prism(dl,{o.x+W*.83f,o.y+H*.60f+cosf(t*.4f)*s*.018f},s*.048f,-t*.8f,fade*.44f);
    for(int j=0;j<18;++j){
        const float u=std::fmod(Hash(j+101)+t*(.04f+Hash(j+109)*.025f),1.f),x=o.x+W*(1.10f-u*1.2f),y=o.y+H*Hash(j+127)+sinf(t*.7f+j)*s*.025f;
        const float r=s*(.004f+Hash(j+139)*.008f),turn=t*(.3f+Hash(j+149)*.2f)+j,k=fade*sinf(u*pi)*.26f;
        auto q=[&](float a,float b){return ImVec2{x+r*(a*cosf(turn)-b*sinf(turn)),y+r*(a*sinf(turn)+b*cosf(turn))};};
        ImVec2 page[]={q(-.7f,-1),q(.7f,-.8f),q(.6f,1),q(-.8f,.8f)};
        dl->AddConvexPolyFilled(page,4,C(0xE9DCB6,k*.30f));dl->AddPolyline(page,4,C(0xDFCF9B,k),ImDrawFlags_Closed,.7f);
    }
    dl->PopClipRect();
}
Intro::MarkPose Intro::AnemoPose(ImVec2 o)const{
    const float dock=HandoffK(),arc=sinf(pi*dock),swell=1+.020f*expf(-powf((t-3.2f)/.32f,2));
    return {{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.45f,dockCenter.y,dock)-H*.055f*arc*arc},Lerp(std::min(W,H)*.235f*swell,dockRadius,dock),S(t,.03f,.23f)};
}
void Intro::DrawAnemoMark(ImDrawList* dl,ImVec2 o,float alpha){
    const auto pose=AnemoPose(o);dl->PushClipRect(o,{o.x+W,o.y+H},true);
    if(opening==Opening::FaruzanMechanism)DrawFaruzanEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);
    else DrawAnemoEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);
    dl->PopClipRect();
}
}
