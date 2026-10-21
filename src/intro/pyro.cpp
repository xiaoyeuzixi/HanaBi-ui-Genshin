#include "intro/intro.h"
#include "intro/pyro_geometry.h"
#include "intro/pyro_flow.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb::pyro {
namespace {
float UnitHash(int i){uint32_t x=uint32_t(i)*747796405u+2891336453u;x=((x>>((x>>28)+4))^x)*277803737u;return float((x>>22^x)&0xffffff)/16777216.f;}
float Smooth(float t,float a,float b){return electro::Smooth(t,a,b);}
struct Emitter{ImVec2 p,n;float birth,seed;};
const std::vector<Emitter>& Emitters(){
    static const auto emitters=[] {
        std::vector<Emitter> result;
        const auto& points=Points();int first=0;
        for(int end:Ends()){
            float area=0;
            for(int j=first;j<end;++j){const auto a=points[j],b=points[j+1<end?j+1:first];area+=a.x*b.y-b.x*a.y;}
            float carry=0;
            for(int j=first;j<end;++j){
                const auto a=points[j],b=points[j+1<end?j+1:first];const float dx=b.x-a.x,dy=b.y-a.y,length=std::hypot(dx,dy);
                if(length<.00001f)continue;
                const float sign=area>0?1.f:-1.f;const ImVec2 normal{dy/length*sign,-dx/length*sign};
                for(float d=carry;d<length;d+=.195f){
                    const ImVec2 p{a.x+dx*d/length,a.y+dy*d/length};
                    float distance=1e6f,travel=0;
                    for(const auto& v:FlowMesh()){float q=(p.x-v.x)*(p.x-v.x)+(p.y-v.y)*(p.y-v.y);if(q<distance){distance=q;travel=v.travel;}}
                    float lo=.30f,hi=3.30f;
                    for(int k=0;k<20;++k){float m=(lo+hi)*.5f;if(Smooth(m,.30f,3.30f)*1.07f<travel)lo=m;else hi=m;}
                    const int id=int(result.size());result.push_back({p,normal,(lo+hi)*.5f,UnitHash(id+217)});
                }
                carry=std::fmod(carry-length,.195f);if(carry<0)carry+=.195f;
            }
            first=end;
        }
        return result;
    }();return emitters;
}
void AppendFire(std::vector<CrystalVertex>& mesh,ImVec2 center,float radius,float time){
    const float outro=1-Smooth(time,3.4f,4.25f);
    if(outro<.002f||radius<26)return;
    for(const auto& e:Emitters()){
        const float age=time-e.birth,attack=Smooth(age,-.015f,.10f);
        if(attack<.001f)continue;
        const float front=expf(-std::max(0.f,age)*1.8f),crest=expf(-powf((time-3.16f)/.36f,2));
        const float energy=attack*outro*(.92f*front+.18f*Smooth(time,.9f,1.7f)+.42f*crest);
        const float pulse=.84f+.16f*sinf(time*(9+e.seed*7)+e.seed*31);
        const float height=radius*(.19f+.24f*e.seed)*(.62f+front*.42f+crest*.52f)*pulse;
        const float width=radius*(.065f+.045f*UnitHash(int(e.seed*1000)))*(1+crest*.28f);
        float dx=e.n.x*.56f+.12f*sinf(time*2+e.seed*23),dy=-.90f+e.n.y*.26f;
        const float length=std::hypot(dx,dy);dx/=length;dy/=length;
        const ImVec2 anchor{center.x+(e.p.x-e.n.x*.025f)*radius,center.y+(e.p.y-e.n.y*.025f)*radius};
        auto vertex=[&](float u,float v){return CrystalVertex{anchor.x+dx*height*v-dy*width*u,anchor.y+dy*height*v+dx*width*u,0,0,1,u,v,2,energy,0,e.seed};};
        for(auto uv:{ImVec2{-1,0},ImVec2{1,0},ImVec2{-1,1},ImVec2{1,0},ImVec2{1,1},ImVec2{-1,1}})mesh.push_back(vertex(uv.x,uv.y));
    }
}
}
V3 Transform(ImVec2 p,float z,float){return {p.x,p.y,z};}
std::vector<CrystalVertex> Vertices(ImVec2 center,float radius,float time){
    std::vector<CrystalVertex> result;result.reserve(FlowMesh().size()+Emitters().size()*6);
    AppendFire(result,center,radius,time);
    for(auto v:FlowMesh())result.push_back({center.x+v.x*radius,center.y+v.y*radius,v.dx,v.dy,1,v.x,v.y,.5f,1,v.travel,0});
    return result;
}
bool SelfTest(){
    bool ok=Triangles().size()+Ends().size()*2==Points().size()&&Ends().size()==2&&FlowPaths().size()==6;
    for(auto v:FlowMesh())ok=ok&&std::isfinite(v.travel)&&v.travel>=0&&v.travel<1.02f;
    for(float t:{0.f,.5f,1.f,2.f,3.16f,4.25f,6.2f}){
        const auto mesh=Vertices({400,300},100,t);ok=ok&&mesh.size()<16000&&mesh.size()%3==0;
        for(auto v:mesh)ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.opacity)&&v.opacity>=0;
        if(t>=4.25f)ok=ok&&mesh.size()==FlowMesh().size();
    }
    return ok;
}
}
namespace hb {
namespace {
constexpr float pi=3.14159265359f,tau=pi*2;
float S(float t,float a,float b){return electro::Smooth(t,a,b);}
ImU32 C(unsigned color,float alpha){return IM_COL32(color>>16&255,color>>8&255,color&255,int(std::clamp(alpha,0.f,1.f)*255));}
float Hash(int i){uint32_t x=uint32_t(i)*747796405u+2891336453u;x=((x>>((x>>28)+4))^x)*277803737u;return float((x>>22^x)&0xffffff)/16777216.f;}
void Glow(ImDrawList* dl,ImVec2 p,float radius,unsigned color,float alpha){
    if(alpha<.002f)return;constexpr int n=32;const auto base=dl->_VtxCurrentIdx;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(n*3,n+1);dl->PrimWriteVtx(p,uv,C(color,alpha));
    for(int i=0;i<n;++i){float a=i*tau/n;dl->PrimWriteVtx({p.x+cosf(a)*radius,p.y+sinf(a)*radius},uv,C(color,0));}
    for(int i=0;i<n;++i){dl->PrimWriteIdx((ImDrawIdx)base);dl->PrimWriteIdx((ImDrawIdx)(base+1+i));dl->PrimWriteIdx((ImDrawIdx)(base+1+(i+1)%n));}
}


pyro::FlowVertex Along(const std::vector<pyro::FlowVertex>& path,float travel){
    auto it=std::lower_bound(path.begin(),path.end(),travel,[](const auto& v,float t){return v.travel<t;});
    if(it==path.begin())return path.front();
    if(it==path.end())return path.back();
    const auto a=*(it-1),b=*it;const float k=Clamp01((travel-a.travel)/std::max(.0001f,b.travel-a.travel));
    return {Lerp(a.x,b.x,k),Lerp(a.y,b.y,k),travel,Lerp(a.dx,b.dx,k),Lerp(a.dy,b.dy,k)};
}
void BurningTongue(ImDrawList* dl,ImVec2 center,float radius,const std::vector<pyro::FlowVertex>& path,float growth,float time,float alpha,int seed){
    if(growth<.008f||growth>path.back().travel+.055f||alpha<.002f)return;
    constexpr int n=36;
    const auto base=dl->_VtxCurrentIdx;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(n*24,(n+1)*5);
    const float head=std::min(growth,path.back().travel),tail=std::max(0.f,head-.075f);
    for(int j=0;j<=n;++j){const float u=j/float(n);auto v=Along(path,Lerp(tail,head,u));
        float dx=v.dx,dy=v.dy,length=std::max(.1f,std::hypot(dx,dy));dx/=length;dy/=length;
        const float lift=std::max(0.f,u-.58f)*.16f;
        const float flicker=sinf(u*9-time*15+seed)*(.006f+.018f*u*u);
        const float x=center.x+(v.x+dx*lift-dy*flicker)*radius,y=center.y+(v.y+dy*lift+dx*flicker)*radius;
        const float profile=powf(sinf(u*pi),.65f),width=radius*(.026f+.012f*sinf(time*10+seed))*profile;
        constexpr float offsets[]={-1.8f,-.50f,0,.50f,1.8f};
        constexpr unsigned colors[]={0xF55232,0xFF9D57,0xFFF3CB,0xFF9D57,0xF55232};
        for(int k=0;k<5;++k)dl->PrimWriteVtx({x-dy*width*offsets[k],y+dx*width*offsets[k]},uv,C(colors[k],alpha*profile*(k==0||k==4?0.f:1.f)));
    }
    for(int j=0;j<n;++j)for(int k=0;k<4;++k){const auto index=base+j*5+k;for(auto i:{index,index+1,index+5,index+1,index+6,index+5})dl->PrimWriteIdx(ImDrawIdx(i));}
    const auto tip=Along(path,head);Glow(dl,{center.x+tip.x*radius,center.y+tip.y*radius},radius*.095f,0xFF8753,alpha*.25f);
}

}
void Intro::DrawPyroEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;
    const ImVec2 seed{center.x,center.y+radius*.16f};
    const float ignition=S(time,.04f,.2f)*(1-S(time,.7f,1.45f))*alpha;
    Glow(dl,seed,radius*(.12f+.025f*sinf(time*11)),0xFF6C46,ignition*.8f);
    Glow(dl,seed,radius*.038f,0xFFE5B6,ignition);
    if(ignition>.002f)dl->AddCircleFilled(seed,std::max(.8f,radius*.012f),C(0xFFF4D5,ignition),16);
    pyroMaterial.Queue(dl,pyro::Vertices(center,radius,time),center,radius,time,alpha);
    const float growth=S(time,.30f,3.30f)*1.07f;
    const float fire=S(time,.2f,.5f)*(1-S(time,3.1f,3.65f))*alpha*S(radius,22,65);
    int branch=0;
    for(const auto& path:pyro::FlowPaths()){
        BurningTongue(dl,center,radius,path,growth,time,fire,branch);
        if(growth<path.back().travel+.05f){
            const auto head=Along(path,std::min(growth,path.back().travel));
            for(int i=0;i<4;++i){
                const float age=std::fmod(time*(1.3f+Hash(i+branch*17)*.6f)+Hash(i+branch*31),1.f);
                const float travel=age*.15f;
                const ImVec2 at{center.x+(head.x+head.dx*travel+sinf(time*3+i)*age*.025f)*radius,center.y+(head.y+head.dy*travel-age*age*.08f)*radius};
                const float light=fire*sinf(age*pi)*.75f;
                Glow(dl,at,radius*.022f,0xFF824C,light*.45f);dl->AddCircleFilled(at,std::max(.65f,radius*.004f),C(0xFFEDBE,light),8);
            }
        }++branch;
    }
    const float cinderFade=(1-S(time,3.65f,4.4f))*alpha*S(radius,26,65);
    if(cinderFade>.002f)for(const auto& e:pyro::Emitters())for(int flight=0;flight<5;++flight){
        const float age=time-e.birth-flight*.24f-e.seed*.12f;
        if(age<=0||age>=1.05f)continue;
        const float light=S(age,0,.055f)*(1-S(age,.30f,1.05f))*cinderFade;
        const float speed=.17f+.24f*e.seed;
        auto point=[&](float a){return ImVec2{center.x+(e.p.x+e.n.x*speed*a+sinf(a*6+e.seed*31)*a*.045f)*radius,center.y+(e.p.y+e.n.y*.08f*a-speed*a-a*a*.19f)*radius};};
        const auto head=point(age),tail=point(std::max(0.f,age-.07f));
        Glow(dl,head,radius*.026f,0xFA683B,light*.34f);
        dl->AddLine(tail,head,C(0xFFAD69,light*.85f),std::max(.7f,radius*.008f));
        dl->AddCircleFilled(head,std::max(.6f,radius*.004f),C(0xFFF1CA,light),6);
    }
}
void Intro::DrawPyroAtmosphere(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.6f);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(0x1A0D0D,.96f*fade),C(0x26120E,.94f*fade),C(0x3C1C12,.90f*fade),C(0x241110,.96f*fade));
    const auto pose=PyroPose(o);
    const float rise=S(t,.3f,2.f)*(1-S(t,4.3f,5.3f));
    const float crest=expf(-powf((t-3.16f)/.34f,2));
    const float breathing=.88f+.08f*sinf(t*7)+.04f*sinf(t*13);
    Glow(dl,{pose.center.x,pose.center.y+pose.radius*.10f},pose.radius*(2.2f+crest*.35f),0xE55C37,rise*fade*(.28f*breathing+crest*.25f));
    Glow(dl,pose.center,pose.radius*1.35f,0xFFAA65,crest*fade*.13f);
    dl->PopClipRect();
}
void Intro::DrawFireworkScenery(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.6f),s=std::min(W,H);
    const float rise=S(t,.3f,2.f)*(1-S(t,4.3f,5.3f));
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    for(int burst=0;burst<3;++burst){
        const float launch=burst==0?1.55f:(burst==1?2.65f:3.15f),age=t-launch;
        const float x=o.x+W*(burst==0?.23f:(burst==1?.78f:.69f)),y=o.y+H*(burst==0?.35f:(burst==1?.27f:.67f));
        if(age>-.45f&&age<0){const float k=(age+.45f)/.45f;ImVec2 head{x,y+(1-k)*H*.24f};Glow(dl,head,s*.014f,0xFFBA7E,k*.5f*fade);dl->AddLine(head,{x,y+(1-k)*H*.24f+s*.035f},C(0xFFD6A1,k*.7f*fade),1.f);}
        if(age<0||age>1.85f)continue;
        Glow(dl,{x,y},s*.032f,0xFFD391,expf(-age*13)*fade*.75f);
        for(int j=0;j<64;++j){
            const float angle=j*tau/64+Hash(j+burst*101)*.04f,speed=s*(.10f+.17f*Hash(j+burst*131));
            auto point=[&](float a){const float flight=(1-expf(-a*1.1f))/1.1f;return ImVec2(x+cosf(angle)*speed*flight+sinf(float(j))*s*.003f*a*a,y+sinf(angle)*speed*flight+s*.046f*a*a);};
            const auto head=point(age);
            const float light=S(age,0,.045f)*(1-S(age,.65f+Hash(j+47)*.4f,1.85f))*fade;
            for(int trail=0;trail<12;++trail){const float u=trail/12.f,v=(trail+1)/12.f;
                const float a=std::max(0.f,age-.32f*(1-u)),b=std::max(0.f,age-.32f*(1-v));
                const auto p=point(a),q=point(b);const float flicker=.7f+.3f*sinf(j*13+t*31+u*8);
                dl->AddLine(p,q,C(0xE85029,light*u*.15f),s*.006f);
                dl->AddLine(p,q,C(j%3?0xFFC182:0xFFF2D6,light*u*flicker),s*.0012f);
            }
            Glow(dl,head,s*.006f,0xFF9D63,light*.32f);
            dl->AddCircleFilled(head,s*.0014f,C(0xFFF5DB,light),8);
        }
    }
    for(int i=0;i<35;++i){const float age=std::fmod(t*.13f+Hash(i+81),1.f);ImVec2 p{o.x+W*Hash(i+53)+sinf(t*.8f+i)*s*.007f,o.y+H*(1.08f-age*1.2f)};const float light=sinf(age*pi)*rise*fade*(.10f+.30f*Hash(i+197));Glow(dl,p,s*.004f,0xFF8C5D,light*.5f);dl->AddCircleFilled(p,s*.001f,C(0xFFD6A1,light),8);}
    dl->PopClipRect();
}
Intro::MarkPose Intro::PyroPose(ImVec2 o)const{
    const float dock=HandoffK(),arc=sinf(pi*dock);
    return {{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.45f,dockCenter.y,dock)-H*.055f*arc*arc},Lerp(std::min(W,H)*.225f,dockRadius,dock),S(t,.04f,.25f)};
}
void Intro::DrawPyroMark(ImDrawList* dl,ImVec2 o,float alpha){if(opening==Opening::YoimiyaGoldfish){DrawCharacterMark(dl,o,alpha);return;}const auto pose=PyroPose(o);dl->PushClipRect(o,{o.x+W,o.y+H},true);DrawPyroEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);dl->PopClipRect();}
}
