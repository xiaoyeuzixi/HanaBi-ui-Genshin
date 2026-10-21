#include "intro/intro.h"
#include "intro/dendro_geometry.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb {
namespace {
constexpr float pi=3.14159265359f,tau=pi*2;
float S(float t,float a,float b){const float u=Clamp01((t-a)/(b-a));return u*u*u*(u*(u*6-15)+10);}
ImU32 C(unsigned color,float alpha){return IM_COL32(color>>16&255,color>>8&255,color&255,int(Clamp01(alpha)*255));}
float Hash(int i){uint32_t x=uint32_t(i)*747796405u+2891336453u;x=((x>>((x>>28)+4))^x)*277803737u;return float((x>>22^x)&0xffffff)/16777216.f;}
void Halo(ImDrawList* dl,ImVec2 p,float r,unsigned color,float alpha){
    if(alpha<.002f)return;
    constexpr int n=40;const auto uv=ImGui::GetFontTexUvWhitePixel();dl->PrimReserve(n*3,n+1);const auto base=dl->_VtxCurrentIdx;
    dl->PrimWriteVtx(p,uv,C(color,alpha));
    for(int i=0;i<n;++i){float a=i*tau/n;dl->PrimWriteVtx({p.x+cosf(a)*r,p.y+sinf(a)*r},uv,C(color,0));}
    for(int i=0;i<n;++i)for(auto j:{base,base+1+i,base+1+(i+1)%n})dl->PrimWriteIdx(ImDrawIdx(j));
}
void Star(ImDrawList* dl,ImVec2 p,float r,float alpha){
    if(alpha<.002f)return;
    Halo(dl,p,r*4,0x86F7AA,alpha*.32f);
    ImVec2 points[]={{p.x,p.y-r},{p.x+r*.18f,p.y-r*.18f},{p.x+r*.65f,p.y},{p.x+r*.18f,p.y+r*.18f},{p.x,p.y+r},{p.x-r*.18f,p.y+r*.18f},{p.x-r*.65f,p.y},{p.x-r*.18f,p.y-r*.18f}};
    const auto uv=ImGui::GetFontTexUvWhitePixel();dl->PrimReserve(24,9);const auto base=dl->_VtxCurrentIdx;
    dl->PrimWriteVtx(p,uv,C(0xFFFFE3,alpha));for(auto q:points)dl->PrimWriteVtx(q,uv,C(0xDDFFDE,alpha));
    for(int i=0;i<8;++i)for(auto j:{base,base+1+i,base+1+(i+1)%8})dl->PrimWriteIdx(ImDrawIdx(j));
}
void Leaf(ImDrawList* dl,ImVec2 a,ImVec2 b,float width,float bend,float alpha,float time){
    if(alpha<.002f)return;
    const float length=std::max(.001f,std::hypot(b.x-a.x,b.y-a.y)),nx=-(b.y-a.y)/length,ny=(b.x-a.x)/length;
    constexpr int n=28;const auto uv=ImGui::GetFontTexUvWhitePixel();dl->PrimReserve(n*24,(n+1)*5);const auto base=dl->_VtxCurrentIdx;
    constexpr float offsets[]={-1,-.45f,0,.45f,1};constexpr unsigned colors[]={0x59B889,0x85D99B,0xEEFFD2,0xB8F0A0,0x59B889};
    for(int j=0;j<=n;++j){const float u=j/float(n),profile=powf(std::max(0.f,sinf(pi*u)),.9f);
        const float shift=sinf(pi*u)*(bend+sinf(time*1.1f+u*3)*width*.10f);
        const ImVec2 p{Lerp(a.x,b.x,u)+nx*shift,Lerp(a.y,b.y,u)+ny*shift};
        for(int k=0;k<5;++k)dl->PrimWriteVtx({p.x+nx*width*profile*offsets[k],p.y+ny*width*profile*offsets[k]},uv,C(colors[k],alpha*(k==0||k==4?.08f:(k==2?1.f:.45f))));
    }
    for(int j=0;j<n;++j)for(int k=0;k<4;++k){const auto index=base+j*5+k;for(auto q:{index,index+1,index+5,index+1,index+6,index+5})dl->PrimWriteIdx(ImDrawIdx(q));}
    if(width>12)dl->AddBezierCubic(a,{a.x+(b.x-a.x)*.33f+nx*bend*.9f,a.y+(b.y-a.y)*.33f+ny*bend*.9f},{a.x+(b.x-a.x)*.66f+nx*bend*.9f,a.y+(b.y-a.y)*.66f+ny*bend*.9f},b,C(0xE6F7B8,alpha*.55f),std::max(.6f,width*.012f));
}
dendro::Step Along(const std::vector<dendro::Step>& path,float value){
    auto at=std::lower_bound(path.begin(),path.end(),value,[](const auto& a,float b){return a.travel<b;});
    if(at==path.begin())return path.front();if(at==path.end())return path.back();const auto a=*(at-1),b=*at;
    const float u=Clamp01((value-a.travel)/std::max(.00001f,b.travel-a.travel));return {Lerp(a.x,b.x,u),Lerp(a.y,b.y,u),value};
}
std::vector<CrystalVertex> Vertices(ImVec2 center,float radius){
    std::vector<CrystalVertex> mesh;mesh.reserve(dendro::Mesh().size());
    for(auto v:dendro::Mesh())mesh.push_back({center.x+v.x*radius,center.y+v.y*radius,0,0,v.edge,v.x,v.y,.5f,1,v.travel,0});
    return mesh;
}
}
namespace dendro {
bool SelfTest(){
    bool ok=!Mesh().empty()&&Mesh().size()%3==0&&Mesh().size()<16000&&Paths().size()==7;
    for(auto v:Mesh())ok=ok&&std::isfinite(v.travel)&&std::isfinite(v.edge)&&v.travel>=0&&v.travel<1.02f&&v.edge>=0;
    for(const auto& path:Paths()){float previous=-1;for(auto v:path){ok=ok&&v.travel>=previous;previous=v.travel;}}
    return ok;
}
}
void Intro::DrawDendroEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;
    const float large=S(radius,22,65),growth=S(time,.58f,2.95f)*1.09f;
    const float ignition=S(time,.03f,.2f)*(1-S(time,.65f,1.05f))*alpha*large;
    const ImVec2 seed{center.x,center.y+radius*Lerp(.26f,.97f,S(time,.12f,.58f))};
    Halo(dl,seed,radius*.32f,0x59DF82,ignition*.6f);
    Leaf(dl,{seed.x,seed.y+radius*.09f},{seed.x,seed.y-radius*.10f},radius*.045f,0,ignition,time);
    Star(dl,seed,radius*.026f,ignition);
    const float bloom=expf(-powf((time-3.22f)/.32f,2))*alpha*large;
    Halo(dl,center,radius*1.7f,0x74F29C,bloom*.19f);
    dendroMaterial.Queue(dl,Vertices(center,radius),center,radius,time,alpha);
    const float tracing=S(time,.55f,.8f)*(1-S(time,3.05f,3.5f))*alpha*large;
    if(tracing>.002f){int branch=0;for(const auto& path:dendro::Paths()){
        if(growth>path.back().travel+.07f){++branch;continue;}
        const float end=std::min(growth,path.back().travel),start=std::max(0.f,end-.10f);
        auto previous=Along(path,start);
        for(int j=1;j<=32;++j){const float u=j/32.f;auto next=Along(path,Lerp(start,end,u));
            ImVec2 a{center.x+previous.x*radius,center.y+previous.y*radius},b{center.x+next.x*radius,center.y+next.y*radius};
            dl->AddLine(a,b,C(0x66E99B,tracing*u*.25f),radius*.030f);dl->AddLine(a,b,C(0xEDFFCB,tracing*u),std::max(.7f,radius*.009f));previous=next;
        }
        Star(dl,{center.x+previous.x*radius,center.y+previous.y*radius},radius*(.020f+.006f*sinf(time*6+branch)),tracing);
        ++branch;
    }}
    const float release=S(time,2.8f,3.25f)*(1-S(time,3.8f,4.45f))*alpha*large;
    if(release>.002f)for(int i=0;i<20;++i){
        const float age=std::max(0.f,time-2.85f-Hash(i)*.22f),angle=i*2.399963f+age*.18f;
        const float distance=radius*(.94f+age*(.45f+Hash(i+31)*.55f));
        ImVec2 p{center.x+cosf(angle)*distance,center.y+sinf(angle)*distance*.78f-age*age*radius*.12f};
        const float length=radius*(.035f+Hash(i+71)*.055f),turn=angle+age*.9f;
        Leaf(dl,{p.x-cosf(turn)*length,p.y-sinf(turn)*length},{p.x+cosf(turn)*length,p.y+sinf(turn)*length},length*.36f,length*.25f,release*(.26f+Hash(i+29)*.4f),time+i);
        if(i%3==0)Star(dl,p,radius*.020f,release*.75f);
    }
}
void Intro::DrawDendroAtmosphere(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.55f);const auto pose=DendroPose(o);const float r=pose.radius;
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(0x071711,.98f*fade),C(0x0B221A,.98f*fade),C(0x182C18,.96f*fade),C(0x081A14,.98f*fade));
    const float awaken=S(t,.45f,2.4f)*(1-S(t,3.65f,4.55f)),open=S(t,1.2f,3.25f),bloom=expf(-powf((t-3.22f)/.38f,2));
    Halo(dl,pose.center,r*(2.5f+bloom*.25f),0x379E6C,(.23f*awaken+.26f*bloom)*fade);
    Halo(dl,{pose.center.x,pose.center.y+r*.8f},r*1.5f,0xA2D875,awaken*fade*.15f);
    const ImVec2 base{pose.center.x,pose.center.y+r*.98f};
    for(int side:{-1,1})for(int layer=0;layer<2;++layer){
        const float reach=Lerp(.25f,layer?2.f:1.25f,open);
        const ImVec2 tip{base.x+side*r*reach,base.y-r*(layer?1.f:2.15f)};
        Leaf(dl,base,tip,r*(layer?.48f:.56f)*open,side*r*.28f,(.19f+bloom*.13f)*awaken*fade,t+layer);
    }
    dl->PopClipRect();
}
void Intro::DrawSanctuaryScenery(ImDrawList* dl,ImVec2 o){
    const float fade=(1-S(t,kDockStart,5.6f))*S(t,.2f,1.3f),s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    const auto pose=DendroPose(o);
    for(int i=0;i<18;++i){
        const float u=S(t,.04f+Hash(i+321)*.28f,1.05f+Hash(i+357)*.45f),v=1-u;
        const float side=i%2?1.f:-1.f;
        const ImVec2 a{pose.center.x+side*s*(.18f+Hash(i+331)*.27f),pose.center.y+s*(Hash(i+349)-.45f)*.65f};
        const ImVec2 b{pose.center.x+side*s*.12f,pose.center.y+s*(Hash(i+371)-.5f)*.27f};
        const ImVec2 c{pose.center.x,pose.center.y+pose.radius*.97f};
        const ImVec2 p{v*v*a.x+2*v*u*b.x+u*u*c.x,v*v*a.y+2*v*u*b.y+u*u*c.y};
        const float light=sinf(u*pi)*.8f*(1-S(t,1.2f,1.6f));
        Halo(dl,p,s*.016f,0xA5FAC3,light*.24f);
        if(i%3==0)Star(dl,p,s*.0035f,light);else dl->AddCircleFilled(p,s*.0015f,C(0xF4FFE0,light),8);
    }
    for(int i=0;i<42;++i){
        const float speed=.025f+Hash(i+17)*.045f,phase=std::fmod(Hash(i+5)+t*speed,1.f);
        const float depth=.3f+Hash(i+91)*.7f,light=sinf(pi*phase)*fade*(.18f+.30f*depth);
        const ImVec2 p{o.x+(Hash(i+37)+sinf(t*.35f+i)*.014f)*W,o.y+(1.06f-phase*1.15f)*H};
        if(i%3==0){const float r=s*(.014f+depth*.027f);Halo(dl,p,r,0xB0E6A3,light*.12f);}
        else if(i%4==0)Star(dl,p,s*.0034f*depth,light);
        else{Halo(dl,p,s*.009f*depth,0x88EBA4,light*.5f);dl->AddCircleFilled(p,std::max(.6f,s*.0013f*depth),C(0xE9FFD0,light),8);}
    }
    for(int i=0;i<13;++i){
        const float phase=std::fmod(Hash(i+44)+t*(.035f+Hash(i+55)*.024f),1.f),depth=.4f+.6f*Hash(i+143);
        const ImVec2 p{o.x+W*(1.1f-phase*1.25f),o.y+H*Hash(i+68)+sinf(t*.55f+i)*s*.033f};
        const float a=sinf(pi*phase)*fade*.35f,angle=-.7f+sinf(t*.7f+i)*.8f,length=s*(.009f+depth*.014f);
        Leaf(dl,{p.x-cosf(angle)*length,p.y-sinf(angle)*length},{p.x+cosf(angle)*length,p.y+sinf(angle)*length},length*.40f,length*.18f,a,t+i);
    }
    dl->PopClipRect();
}
Intro::MarkPose Intro::DendroPose(ImVec2 o)const{
    const float dock=HandoffK(),arc=sinf(pi*dock);
    return {{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.45f,dockCenter.y,dock)-H*.055f*arc*arc},Lerp(std::min(W,H)*.215f,dockRadius,dock),S(t,.03f,.23f)};
}
void Intro::DrawDendroMark(ImDrawList* dl,ImVec2 o,float alpha){
    if(opening==Opening::NahidaDreamSanctuary){DrawCharacterMark(dl,o,alpha);return;}
    const auto pose=DendroPose(o);dl->PushClipRect(o,{o.x+W,o.y+H},true);DrawDendroEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);dl->PopClipRect();
}
}
