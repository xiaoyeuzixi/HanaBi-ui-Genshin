#include "intro/intro.h"
#include "intro/electro_geometry.h"
#include "core/easing.h"
#include <algorithm>
#include <cmath>

namespace hb {
namespace {
constexpr float pi=3.14159265359f,tau=pi*2;
float S(float t,float a,float b){return electro::Smooth(t,a,b);}
ImU32 C(unsigned rgb,float a){return IM_COL32((rgb>>16)&255,(rgb>>8)&255,rgb&255,int(std::clamp(a,0.f,1.f)*255));}
void Halo(ImDrawList* dl,ImVec2 c,float radius,unsigned color,float alpha){
    if(alpha<.001f||radius<.1f)return;constexpr int n=48;
    dl->PrimReserve(n*3,n+1);const auto index=dl->_VtxCurrentIdx;const auto uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimWriteVtx(c,uv,C(color,alpha));for(int i=0;i<n;++i){float a=i*tau/n;dl->PrimWriteVtx({c.x+cosf(a)*radius,c.y+sinf(a)*radius},uv,C(color,0));}
    for(int i=0;i<n;++i){dl->PrimWriteIdx((ImDrawIdx)index);dl->PrimWriteIdx((ImDrawIdx)(index+1+i));dl->PrimWriteIdx((ImDrawIdx)(index+1+(i+1)%n));}
}
float Hash(int seed){uint32_t v=uint32_t(seed)*747796405u+2891336453u;v=((v>>((v>>28)+4))^v)*277803737u;v=(v>>22)^v;return float(v&0xffffff)/16777216.f;}
}
Intro::MarkPose Intro::ElectroPose(ImVec2 o)const{
    const float dock=HandoffK(),arc=sinf(pi*dock);
    return{{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.45f,dockCenter.y,dock)-H*.055f*arc*arc},Lerp(std::min(W,H)*.17f,dockRadius,dock),S(t,.12f,.46f)};
}
void Intro::DrawElectroAtmosphere(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.60f),s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(0x100A20,.94f*fade),C(0x1A0C28,.91f*fade),C(0x26132F,.84f*fade),C(0x130C25,.94f*fade));
    const auto pose=ElectroPose(o);
    const float reveal=S(t,.7f,2.7f)*(1-S(t,4.20f,5.5f))*fade;
    const float surge=S(t,2.65f,2.95f)*(1-S(t,3.12f,3.65f));
    Halo(dl,pose.center,pose.radius*2.3f,0x9A41CB,reveal*(.11f+surge*.18f));
    for(int i=0;i<24;++i){
        const float age=t*(.08f+Hash(i+47)*.08f)+Hash(i+17)*tau;
        ImVec2 p{o.x+W*(.12f+.76f*Hash(i+41)),o.y+H*(.12f+.76f*Hash(i+79))-s*.018f*sinf(age)};
        const float alpha=reveal*(.07f+.17f*sinf(age)*sinf(age)),z=s*(.0006f+.0007f*Hash(i+121));
        Halo(dl,p,z*3.5f,0xCB82EE,alpha*.3f);
        dl->AddCircleFilled(p,z,C(0xEBC9F5,alpha),8);
    }
    dl->PopClipRect();
}
void Intro::DrawElectroEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;
    const float core=S(time,.08f,.4f)*(1-S(time,1.7f,2.8f))*alpha;
    Halo(dl,center,radius*.36f,0xBF75E7,core*.7f);
    Halo(dl,center,radius*.10f,0xEBC9F5,core*.95f);
    if(core>.001f)dl->AddCircleFilled(center,std::max(.7f,radius*.009f),C(0xF6E3FC,core),20);
    electroMaterial.Queue(dl,electro::Vertices(center,radius,time),center,radius,time,alpha);
    static const auto outline=electro::Blade();
    static const auto distances=[](){std::vector<float> d{0};const auto p=electro::Blade();for(size_t i=0;i<p.size();++i){auto q=p[(i+1)%p.size()];d.push_back(d.back()+std::hypot(p[i].x-q.x,p[i].y-q.y));}return d;}();
    auto contourPoint=[&](float u){
        const float distance=(u-floorf(u))*distances.back();
        const auto index=std::min(size_t(std::upper_bound(distances.begin(),distances.end(),distance)-distances.begin()-1),outline.size()-1);
        const float k=(distance-distances[index])/(distances[index+1]-distances[index]);
        const auto a=outline[index],b=outline[(index+1)%outline.size()];
        const auto v=electro::Rotate({Lerp(a.x,b.x,k),Lerp(a.y,b.y,k),.09f},time);
        return ImVec2(center.x+v.x*radius,center.y+v.y*radius);
    };
    const float ceremony=1-S(time,4.15f,5.15f);
    const float current=S(time,.22f,.65f)*alpha*(.35f+.65f*S(radius,14,50));
    for(int arm=0;arm<3;++arm){
        const float phase=time*(.105f+ceremony*.09f)+arm/3.f;
        const float length=Lerp(.035f,.12f,ceremony);
        for(int j=0;j<32;++j){
            const float u=j/32.f,light=std::pow(sinf(u*pi),.65f)*current;
            const auto a=contourPoint(phase+u*length),b=contourPoint(phase+(j+1)/32.f*length);
            dl->AddLine(a,b,C(0xB259DA,light*.24f),std::max(2.f,radius*(.025f+.055f*ceremony)));
            dl->AddLine(a,b,C(0xD89AF0,light*.6f),std::max(1.f,radius*(.011f+.014f*ceremony)));
            dl->AddLine(a,b,C(0xFBEAFC,light),std::max(.65f,radius*(.004f+.005f*ceremony)));
        }
    }
    const float sparks=S(time,.45f,1.0f)*(1-S(time,4.0f,4.65f))*alpha*S(radius,24,80);
    for(int i=0;i<21;++i){
        const float age=std::fmod(time*(.65f+Hash(i+51)*.35f)+Hash(i+13),1.f);
        const auto origin=contourPoint(Hash(i+97)+time*.025f);
        const float angle=Hash(i+173)*tau,distance=radius*age*(.12f+.24f*Hash(i+237));
        const ImVec2 p{origin.x+cosf(angle)*distance,origin.y+sinf(angle)*distance-radius*age*age*.13f};
        const float light=sparks*sinf(age*pi)*(.45f+.55f*Hash(i+307));
        Halo(dl,p,radius*.036f,0xCD85EC,light*.6f);
        dl->AddCircleFilled(p,std::max(.6f,radius*.005f),C(0xF5DDFB,light),10);
    }
}
void Intro::DrawElectroMark(ImDrawList* dl,ImVec2 origin,float alpha){
    if(opening==Opening::YaeSesshouSakura){DrawCharacterMark(dl,origin,alpha);return;}
    const auto pose=ElectroPose(origin);dl->PushClipRect(origin,{origin.x+W,origin.y+H},true);
    DrawElectroEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);
    dl->PopClipRect();
}
}
