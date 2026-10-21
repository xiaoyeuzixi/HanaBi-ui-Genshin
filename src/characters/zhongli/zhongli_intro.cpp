#include "intro/intro.h"
#include "intro/geo_geometry.h"
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
void Diamond(ImDrawList* dl,ImVec2 p,float r,float angle,float alpha){
    const float c=cosf(angle),s=sinf(angle);auto q=[&](float x,float y){return ImVec2{p.x+r*(x*c-y*s),p.y+r*(x*s+y*c)};};
    ImVec2 v[]={q(0,-1),q(.62f,0),q(0,1),q(-.62f,0)};
    dl->AddConvexPolyFilled(v,4,C(0xF5BF54,alpha*.30f));dl->AddPolyline(v,4,C(0xFFE5A0,alpha),ImDrawFlags_Closed,.8f);
    dl->AddTriangleFilled(v[0],v[1],v[2],C(0xFFEAB8,alpha*.18f));
}
std::vector<CrystalVertex> Vertices(ImVec2 p,float r,float t){
    std::vector<CrystalVertex> mesh;mesh.reserve(geo::Mesh().size());
    for(auto v:geo::Mesh()){
        const float settle=S(t,1.20f+v.seed*.40f,2.72f+v.seed*.36f),drift=1-settle;
        const float burst=S(t,.95f+v.seed*.10f,1.72f+v.seed*.14f);
        const float turn=drift*(v.seed-.5f)*1.15f,c=cosf(turn),s=sinf(turn);
        const float depth=Lerp(Lerp(.15f,1.30f,burst),1.f,settle),piece=Lerp(.46f,1.f,burst);
        const float x=((v.x-v.cx)*c-(v.y-v.cy)*s)*piece,y=((v.x-v.cx)*s+(v.y-v.cy)*c)*piece;
        const float tx=v.cx*depth+x+drift*(v.seed-.5f)*.12f,ty=v.cy*depth+y-drift*.04f;
        const float opacity=S(t,.95f+v.seed*.3f,1.4f+v.seed*.3f);
        mesh.push_back({p.x+tx*r,p.y+ty*r,v.nx,v.ny,v.edge,v.x,v.y,.5f,opacity,settle,v.seed});
    }
    return mesh;
}
void Meteor(ImDrawList* dl,ImVec2 p,float r,float t,float alpha){
    if(alpha<.002f)return;
    struct Point{float x,y,z;};struct Face{ImVec2 p[4];float depth;unsigned color;};std::vector<Face> faces;
    const float yaw=.61f+t*.18f,pitch=-.48f+t*.05f,roll=.24f;
    auto rotate=[&](Point p){float x=p.x*cosf(yaw)+p.z*sinf(yaw),z=-p.x*sinf(yaw)+p.z*cosf(yaw);float y=p.y*cosf(pitch)-z*sinf(pitch);z=p.y*sinf(pitch)+z*cosf(pitch);return Point{x*cosf(roll)-y*sinf(roll),x*sinf(roll)+y*cosf(roll),z};};
    const int indices[6][4]={{0,1,3,2},{4,6,7,5},{0,4,5,1},{2,3,7,6},{0,2,6,4},{1,5,7,3}};
    const unsigned colors[]={0x6D4B27,0xB7853F,0xF0C46E,0x554022,0xC79B52,0x8C622F};
    const float opening=S(t,1.04f,1.88f),gap=.052f+opening*.85f;
    Halo(dl,p,r*3.4f,0xE3A33B,alpha*.28f);
    for(int cube=0;cube<8;++cube){
        Point vertices[8];
        for(int k=0;k<8;++k){
            Point q{((cube&1)?1.f:-1.f)*(.46f+gap)+((k&1)?.43f:-.43f),((cube&2)?1.f:-1.f)*(.46f+gap)+((k&2)?.43f:-.43f),((cube&4)?1.f:-1.f)*(.46f+gap)+((k&4)?.43f:-.43f)};
            vertices[k]=rotate(q);
        }
        for(int j=0;j<6;++j){Face face{};face.color=colors[j];
            for(int k=0;k<4;++k){auto q=vertices[indices[j][k]];const float perspective=4/(4+q.z*.28f);face.p[k]={p.x+q.x*r*perspective,p.y+q.y*r*perspective};face.depth+=q.z*.25f;}
            const auto a=face.p[0],b=face.p[1],c=face.p[2];if((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)<0)continue;
            faces.push_back(face);
        }
    }
    std::sort(faces.begin(),faces.end(),[](const auto& a,const auto& b){return a.depth>b.depth;});
    for(const auto& f:faces){dl->AddConvexPolyFilled(f.p,4,C(f.color,alpha));dl->AddPolyline(f.p,4,C(0xFFE1A0,alpha*.75f),ImDrawFlags_Closed,std::max(.7f,r*.016f));
        ImVec2 line[3]={{Lerp(f.p[0].x,f.p[1].x,.26f),Lerp(f.p[0].y,f.p[1].y,.26f)},{Lerp(f.p[3].x,f.p[2].x,.26f),Lerp(f.p[3].y,f.p[2].y,.26f)},{Lerp(f.p[3].x,f.p[2].x,.55f),Lerp(f.p[3].y,f.p[2].y,.55f)}};
        dl->AddPolyline(line,3,C(0xFFF0BA,alpha*.62f),0,std::max(.6f,r*.012f));
    }
}
void Pillar(ImDrawList* dl,ImVec2 p,float h,float width,float alpha){
    if(alpha<.002f)return;const float top=p.y-h,w=width;
    ImVec2 left[]={{p.x-w,p.y},{p.x-w,top+w*.36f},{p.x,top+w*.74f},{p.x,p.y+w*.35f}};
    ImVec2 right[]={{p.x,p.y+w*.35f},{p.x,top+w*.74f},{p.x+w,top+w*.36f},{p.x+w,p.y}};
    ImVec2 cap[]={{p.x-w,top+w*.36f},{p.x,top},{p.x+w,top+w*.36f},{p.x,top+w*.74f}};
    dl->AddConvexPolyFilled(left,4,C(0x4B3522,alpha*.90f));dl->AddConvexPolyFilled(right,4,C(0x261E18,alpha*.95f));dl->AddConvexPolyFilled(cap,4,C(0x926037,alpha*.65f));
    dl->AddPolyline(left,4,C(0xD5A350,alpha*.38f),ImDrawFlags_Closed,.8f);dl->AddPolyline(right,4,C(0xC38A38,alpha*.30f),ImDrawFlags_Closed,.7f);
    for(int j=0;j<4;++j){float y=top+h*(.18f+j*.19f);const float pulse=.55f+.45f*sinf(j*1.7f);
        ImVec2 path[]={{p.x-w*.72f,y},{p.x-w*.72f,y+h*.055f},{p.x-w*.26f,y+h*.075f},{p.x-w*.26f,y+h*.025f},{p.x-w*.05f,y+h*.04f}};
        dl->AddPolyline(path,5,C(0xF4C36D,alpha*.55f*pulse),0,1.f);
    }
}
}
namespace geo {
bool SelfTest(){
    bool ok=!Mesh().empty()&&Mesh().size()<16000&&Mesh().size()%3==0;
    for(auto v:Mesh())ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.seed)&&v.seed>=0&&v.seed<=1&&v.edge>=0;
    for(float t:{0.f,1.f,1.5f,2.5f,3.1f,6.2f})for(const auto& v:Vertices({400,300},130,t))ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y)&&v.thickness>=0&&v.thickness<=1;
    return ok;
}
}
void Intro::DrawGeoEmblem(ImDrawList* dl,ImVec2 center,float radius,float time,float alpha){
    if(!dl||alpha<.001f)return;const float large=S(radius,22,65);
    const float descend=S(time,.06f,1.12f),opening=S(time,1.04f,1.88f);
    const ImVec2 core{center.x,center.y-radius*(1.65f*(1-descend))};
    const float meteorAlpha=S(time,.04f,.38f)*(1-S(time,1.27f,2.05f))*large*alpha;
    for(int j=0;j<3;++j){const float tail=.28f+j*.18f;Halo(dl,{core.x-radius*tail*.12f,core.y-radius*tail},radius*(.28f-j*.045f),0xD3922B,meteorAlpha*(1-descend)*(.15f-j*.035f));}
    Meteor(dl,core,radius*(.37f+.06f*opening),time,meteorAlpha);
    const float crest=expf(-powf((time-3.15f)/.25f,2))*alpha*large;
    Halo(dl,center,radius*2.3f,0xD79222,crest*.27f);
    geoMaterial.Queue(dl,Vertices(center,radius,time),center,radius,time,alpha);
    const float rupture=expf(-powf((time-1.48f)/.25f,2))*large*alpha;
    Halo(dl,center,radius*1.3f,0xFFCE73,rupture*.18f);
    const float release=S(time,2.8f,3.20f)*(1-S(time,4.1f,4.6f))*large*alpha;
    for(int layer=0;layer<2;++layer){
        const float u=S(time,2.82f+layer*.16f,4.25f+layer*.12f),r=radius*(.82f+u*(1.45f+layer*.28f));
        ImVec2 points[]={{center.x,center.y-r*.52f},{center.x+r,center.y},{center.x,center.y+r*.52f},{center.x-r,center.y}};
        const float k=release*(1-u)*.48f;dl->AddPolyline(points,4,C(0xE8B45B,k),ImDrawFlags_Closed,std::max(.6f,radius*.005f));
    }
    const float motes=S(time,.35f,1.2f)*(1-S(time,4.1f,4.65f))*large*alpha;
    for(int j=0;j<68;++j){
        const float u=std::fmod(Hash(j+10)+time*(.11f+Hash(j+20)*.06f),1.f),angle=Hash(j+30)*tau;
        const float distance=.35f+u*1.45f;
        ImVec2 p{center.x+cosf(angle)*radius*distance,center.y+sinf(angle)*radius*distance};
        float k=motes*sinf(u*pi)*(.15f+Hash(j+40)*.6f),r=radius*(.004f+Hash(j+50)*.010f);
        Halo(dl,p,r*5,0xF0AB34,k*.20f);Diamond(dl,p,r,angle+time*.15f,k*.7f);
    }
}
void Intro::DrawGeoAtmosphere(ImDrawList* dl,ImVec2 o){
    const float fade=1-S(t,kDockStart,5.55f),s=std::min(W,H),rise=S(t,.03f,2.4f);const auto pose=GeoPose(o);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},C(0x18140F,.985f*fade),C(0x221A11,.985f*fade),C(0x130F0C,.985f*fade),C(0x1D160F,.985f*fade));
    Halo(dl,{pose.center.x,pose.center.y+s*.06f},s*.76f,0xA46B20,fade*rise*.22f);
    for(int j=0;j<30;++j){const float x=o.x+W*Hash(j+301),y=o.y+H*(1.1f-std::fmod(Hash(j+309)+t*.018f,1.2f));
        const float r=s*(.001f+Hash(j+317)*.002f),k=fade*rise*(.12f+Hash(j+323)*.16f);Halo(dl,{x,y},r*8,0xDDA73E,k*.3f);Diamond(dl,{x,y},r,0,k);
    }
    dl->PopClipRect();
}
void Intro::DrawStoneScenery(ImDrawList* dl,ImVec2 o){
    const float fade=(1-S(t,kDockStart,5.55f))*S(t,.25f,1.6f),s=std::min(W,H),rise=S(t,.35f,3.f);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    for(int side:{-1,1})for(int j=0;j<3;++j){
        const float x=o.x+W*(.5f+side*(.34f+j*.09f)),y=o.y+H*(.91f+j*.06f);
        Pillar(dl,{x,y},s*(.32f+j*.18f)*rise,s*(.030f+j*.004f),fade*(.65f-j*.15f));
        Halo(dl,{x,y},s*.16f,0xBA7A1F,fade*.07f);
    }
    const float drift=S(t,1.15f,3.6f);
    for(int side:{-1,1})for(int j=0;j<5;++j){
        const ImVec2 p{o.x+W*.5f+side*s*(.31f+Hash(j+side+200)*.30f),o.y+H*.48f+s*(.26f-j*.12f)-drift*s*.04f};
        Diamond(dl,p,s*(.012f+Hash(j+180)*.022f),sinf(t*.3f+j)*.16f,fade*.28f);
    }
    dl->PopClipRect();
}
Intro::MarkPose Intro::GeoPose(ImVec2 o)const{
    const float dock=HandoffK(),arc=sinf(pi*dock),swell=1+.026f*expf(-powf((t-3.15f)/.29f,2));
    return {{Lerp(o.x+W*.5f,dockCenter.x,dock),Lerp(o.y+H*.45f,dockCenter.y,dock)-H*.045f*arc*arc},Lerp(std::min(W,H)*.235f*swell,dockRadius,dock),S(t,.03f,.23f)};
}
void Intro::DrawGeoMark(ImDrawList* dl,ImVec2 o,float alpha){const auto pose=GeoPose(o);dl->PushClipRect(o,{o.x+W,o.y+H},true);DrawGeoEmblem(dl,pose.center,pose.radius,finished?std::max(t,energyTime):t,pose.opacity*alpha);dl->PopClipRect();}
}
