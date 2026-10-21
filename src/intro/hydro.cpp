#include "intro/intro.h"
#include "intro/hydro_geometry.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb {
namespace {
constexpr float pi=3.14159265359f,tau=pi*2;
float S(float t,float a,float b){const float u=Clamp01((t-a)/(b-a));return u*u*u*(u*(u*6-15)+10);}
ImU32 C(unsigned color,float a){return IM_COL32(color>>16&255,color>>8&255,color&255,int(Clamp01(a)*255));}
float Hash(int i){uint32_t x=uint32_t(i)*747796405u+2891336453u;x=((x>>((x>>28)+4))^x)*277803737u;return float((x>>22^x)&0xffffff)/16777216.f;}
void Halo(ImDrawList* dl,ImVec2 p,float r,unsigned color,float alpha){
    if(alpha<.002f)return;constexpr int n=40;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(n*3,n+1);const auto base=dl->_VtxCurrentIdx;dl->PrimWriteVtx(p,uv,C(color,alpha));
    for(int j=0;j<n;++j){const float a=j*tau/n;dl->PrimWriteVtx({p.x+cosf(a)*r,p.y+sinf(a)*r},uv,C(color,0));}
    for(int j=0;j<n;++j)for(auto i:{base,base+1+j,base+1+(j+1)%n})dl->PrimWriteIdx(ImDrawIdx(i));
}
ImVec2 Bezier(ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d,float t){const float u=1-t;return {a.x*u*u*u+3*b.x*u*u*t+3*c.x*u*t*t+d.x*t*t*t,a.y*u*u*u+3*b.y*u*u*t+3*c.y*u*t*t+d.y*t*t*t};}
void Veil(ImDrawList* dl,ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d,float width,float time,float alpha,unsigned color){
    if(alpha<.002f)return;constexpr int n=54;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(n*24,(n+1)*5);const auto base=dl->_VtxCurrentIdx;
    constexpr float offsets[]={-1,-.4f,0,.4f,1};
    for(int j=0;j<=n;++j){const float u=j/float(n);const auto p=Bezier(a,b,c,d,u),q=Bezier(a,b,c,d,std::min(1.f,u+.001f));
        const float dx=q.x-p.x,dy=q.y-p.y,length=std::max(.001f,std::hypot(dx,dy));
        const float taper=powf(std::max(0.f,sinf(pi*u)),.8f),w=width*taper*(.85f+.15f*sinf(time*1.3f+u*8));
        for(int k=0;k<5;++k)dl->PrimWriteVtx({p.x-dy/length*w*offsets[k],p.y+dx/length*w*offsets[k]},uv,C(color,alpha*taper*(k==0||k==4?0.f:k==2?1.f:.26f)));
    }
    for(int j=0;j<n;++j)for(int k=0;k<4;++k){const auto v=base+j*5+k;for(auto i:{v,v+1,v+5,v+1,v+6,v+5})dl->PrimWriteIdx(ImDrawIdx(i));}
}
hydro::Step Along(const std::vector<hydro::Step>& path,float value){
    auto at=std::lower_bound(path.begin(),path.end(),value,[](const auto& a,float b){return a.travel<b;});
    if(at==path.begin())return path.front();if(at==path.end())return path.back();const auto a=*(at-1),b=*at;
    const float k=Clamp01((value-a.travel)/std::max(.00001f,b.travel-a.travel));return {Lerp(a.x,b.x,k),Lerp(a.y,b.y,k),value};
}
void Drop(std::vector<CrystalVertex>& mesh,ImVec2 center,float x,float y,float rx,float ry,float angle,float alpha){
    if(alpha<.001f)return;const float c=cosf(angle),s=sinf(angle);
    for(auto uv:{ImVec2{-1,-1},ImVec2{1,-1},ImVec2{-1,1},ImVec2{1,-1},ImVec2{1,1},ImVec2{-1,1}})
        mesh.push_back({center.x+x+c*uv.x*rx-s*uv.y*ry,center.y+y+s*uv.x*rx+c*uv.y*ry,0,0,1,uv.x,uv.y,2,alpha,0,0});
}
std::vector<CrystalVertex> Vertices(ImVec2 center,float radius,float time){
    std::vector<CrystalVertex> mesh;mesh.reserve(hydro::Mesh().size()+600);
    for(auto v:hydro::Mesh())mesh.push_back({center.x+v.x*radius,center.y+v.y*radius,v.nx,v.ny,v.edge,v.x,v.y,.5f,1,v.travel,0});
    const float large=S(radius,22,65),growth=S(time,.42f,3.08f)*1.08f;
    const float seed=S(time,.03f,.16f)*(1-S(time,.45f,.80f))*large;
    Drop(mesh,center,radius*.109f*S(time,.1f,.42f),radius*.362f*S(time,.1f,.42f),radius*.067f,radius*.078f,0,seed);
    const float flow=S(time,.3f,.65f)*(1-S(time,3.10f,3.55f))*large;
    for(const auto& path:hydro::Paths()){
        if(growth>path.back().travel+.06f)continue;
        const float head=std::min(growth,path.back().travel);const auto p=Along(path,head),q=Along(path,std::max(0.f,head-.008f));
        const float r=radius*(.042f+.010f*sinf(time*7+head*9));
        Drop(mesh,center,p.x*radius,p.y*radius,r*1.6f,r,atan2f(p.y-q.y,p.x-q.x),flow);
    }
    const float ambient=S(time,.22f,.8f)*(1-S(time,3.9f,4.5f))*large;
    for(int i=0;i<18;++i){
        const float u=std::fmod(time*.095f+Hash(i+11),1.f),x=(Hash(i+29)-.5f)*4.0f+sinf(time*.55f+i)*.09f;
        const float y=1.9f-u*3.8f,r=radius*(.019f+Hash(i+73)*.044f);
        Drop(mesh,center,x*radius,y*radius,r,r*(1+.09f*sinf(time*2+i)),0,ambient*sinf(u*pi)*(.35f+Hash(i+91)*.5f));
    }
    const float burst=S(time,3.05f,3.3f)*(1-S(time,3.8f,4.45f))*large;
    for(int i=0;i<24;++i){
        const auto& path=hydro::Paths()[i%hydro::Paths().size()];const auto p=Along(path,path.back().travel*(.15f+.85f*Hash(i+187)));
        const float age=std::max(0.f,time-3.06f-Hash(i+203)*.16f),angle=atan2f(p.y,p.x)+(Hash(i+213)-.5f)*.6f;
        const float speed=.45f+Hash(i+241)*.65f,x=p.x+cosf(angle)*speed*age,y=p.y+sinf(angle)*speed*age+age*age*.52f;
        const float r=radius*(.014f+Hash(i+271)*.028f);
        Drop(mesh,center,x*radius,y*radius,r,r*(1+age*.35f),-angle,burst*(.5f+Hash(i+293)*.5f));
    }
    return mesh;
}
}
namespace hydro {
bool SelfTest(){
    bool ok=!Mesh().empty()&&Paths().size()==5&&Mesh().size()%3==0;
    for(auto v:Mesh())ok=ok&&std::isfinite(v.travel)&&std::isfinite(v.edge)&&v.travel>=0&&v.travel<1.02f&&v.edge>=0;
    for(const auto& path:Paths()){float previous=-1;for(auto v:path){ok=ok&&v.travel>=previous;previous=v.travel;}}
    for(float t:{0.f,.5f,1.f,2.f,3.28f,4.5f,6.2f}){
        const auto vertices=Vertices({400,300},132,t);ok=ok&&vertices.size()<16000&&vertices.size()%3==0;
        for(auto v:vertices)ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.opacity)&&v.opacity>=0;
        if(t>=4.5f)ok=ok&&vertices.size()==Mesh().size();
    }
    return ok;
}
}
void Intro::DrawHydroEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;
    const float large=S(radius,22,65),growth=S(time,.42f,3.08f)*1.08f;
    const float ignition=S(time,.03f,.16f)*(1-S(time,.6f,1.f))*alpha*large;
    Halo(dl,{center.x+radius*.109f,center.y+radius*.362f},radius*.3f,0x6CE9FB,ignition*.5f);
    const float crest=expf(-powf((time-3.28f)/.32f,2))*large*alpha;
    Halo(dl,center,radius*1.85f,0x4BCFF4,crest*.20f);
    hydroMaterial.Queue(dl,Vertices(center,radius,time),center,radius,time,alpha);
    const float tracing=S(time,.38f,.65f)*(1-S(time,3.1f,3.55f))*alpha*large;
    if(tracing>.002f)for(const auto& path:hydro::Paths()){
        if(growth>path.back().travel+.06f)continue;
        const float end=std::min(growth,path.back().travel),start=std::max(0.f,end-.085f);auto previous=Along(path,start);
        for(int j=1;j<=32;++j){const float u=j/32.f;auto next=Along(path,Lerp(start,end,u));
            ImVec2 a{center.x+previous.x*radius,center.y+previous.y*radius},b{center.x+next.x*radius,center.y+next.y*radius};
            dl->AddLine(a,b,C(0x44D9F1,tracing*u*.22f),radius*.044f);
            dl->AddLine(a,b,C(0xE9FFFF,tracing*u*.85f),std::max(.8f,radius*.012f));previous=next;
        }
        Halo(dl,{center.x+previous.x*radius,center.y+previous.y*radius},radius*.13f,0x9EF8FF,tracing*.30f);
    }
}
void Intro::DrawHydroAtmosphere(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.55f),s=std::min(W,H);const auto pose=HydroPose(o);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(0x071A29,.98f*fade),C(0x0A2538,.98f*fade),C(0x0A2232,.97f*fade),C(0x061621,.98f*fade));
    const float rise=S(t,.3f,2.35f)*(1-S(t,4.2f,4.7f)),crest=expf(-powf((t-3.28f)/.38f,2));
    Halo(dl,pose.center,pose.radius*(2.6f+crest*.2f),0x238EB6,rise*fade*(.25f+crest*.20f));
    for(int j=0;j<3;++j){
        const float y=o.y+H*(.76f+j*.045f)+sinf(t*.8f+j)*s*.012f;
        Veil(dl,{o.x-W*.05f,y+s*.1f},{o.x+W*.3f,y-s*.18f},{o.x+W*.62f,y+s*.16f},{o.x+W*1.05f,y-s*.14f},s*(.015f+j*.004f),t+j,rise*fade*(.12f-j*.025f),0x7ADCF0);
    }
    dl->PopClipRect();
}
void Intro::DrawOperaScenery(ImDrawList* dl,ImVec2 o){
    const float fade=(1-S(t,kDockStart,5.55f))*S(t,.2f,1.5f),s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    for(int side:{-1,1}){
        const float x=o.x+W*(side<0?.08f:.92f),lean=sinf(t*.5f)*s*.025f;
        Veil(dl,{x-side*s*.06f,o.y-H*.1f},{x+side*s*.12f,o.y+H*.30f},{x-side*s*.08f+lean,o.y+H*.68f},{x+side*s*.20f,o.y+H*1.1f},s*.13f,t+side,fade*.14f,0x436AB8);
        Veil(dl,{x-side*s*.06f,o.y-H*.1f},{x+side*s*.12f,o.y+H*.30f},{x-side*s*.08f+lean,o.y+H*.68f},{x+side*s*.20f,o.y+H*1.1f},s*.008f,t+side,fade*.10f,0xC4E7EF);
    }
    for(int j=0;j<24;++j){
        const float u=std::fmod(Hash(j+319)+t*(.035f+Hash(j+337)*.028f),1.f);
        const ImVec2 p{o.x+W*Hash(j+347)+sinf(t*.6f+j)*s*.01f,o.y+H*(1.05f-u*1.15f)};
        const float light=sinf(u*pi)*fade*(.08f+Hash(j+367)*.18f);
        Halo(dl,p,s*(.005f+Hash(j+383)*.008f),0xB2DFF2,light);
    }
    dl->PopClipRect();
}
Intro::MarkPose Intro::HydroPose(ImVec2 o)const{
    const float dock=HandoffK(),arc=sinf(pi*dock),swell=1+.022f*expf(-powf((t-3.30f)/.31f,2));
    return {{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.45f,dockCenter.y,dock)-H*.055f*arc*arc},Lerp(std::min(W,H)*.22f*swell,dockRadius,dock),S(t,.03f,.23f)};
}
void Intro::DrawHydroMark(ImDrawList* dl,ImVec2 o,float alpha){
    if(opening==Opening::FurinaWaterStage){DrawCharacterMark(dl,o,alpha);return;}
    const auto pose=HydroPose(o);dl->PushClipRect(o,{o.x+W,o.y+H},true);DrawHydroEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);dl->PopClipRect();
}
}
