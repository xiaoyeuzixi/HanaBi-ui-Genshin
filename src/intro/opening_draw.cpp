#include "characters/characters.h"
#include "intro/intro.h"
#include "intro/opening_draw.h"
#include "core/easing.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace hb::opening_draw {
float S(float t,float a,float b){const float u=Clamp01((t-a)/(b-a));return Clamp01(u*u*u*(u*(u*6-15)+10));}
float Window(float t,float a,float b,float c,float d){return S(t,a,b)*(1-S(t,c,d));}
float Hash(int i){uint32_t x=uint32_t(i)*747796405u+2891336453u;x=((x>>((x>>28)+4))^x)*277803737u;return float((x>>22^x)&0xffffff)/16777216.f;}
ImU32 C(unsigned rgb,float a){return IM_COL32(rgb>>16&255,rgb>>8&255,rgb&255,int(Clamp01(a)*255+.5f));}
ImVec2 P(ImVec2 c,float r,float x,float y){return {c.x+r*x,c.y+r*y};}
void Halo(ImDrawList* dl,ImVec2 p,float rx,float ry,unsigned rgb,float alpha){
    if(alpha<.002f||rx<.1f||ry<.1f)return;
    constexpr int n=64;const auto uv=ImGui::GetFontTexUvWhitePixel();const auto base=dl->_VtxCurrentIdx;
    dl->PrimReserve(n*3,n+1);dl->PrimWriteVtx(p,uv,C(rgb,alpha));
    for(int j=0;j<n;++j){const float a=j*tau/n;dl->PrimWriteVtx({p.x+cosf(a)*rx,p.y+sinf(a)*ry},uv,C(rgb,0));}
    for(int j=0;j<n;++j)for(auto i:{base,base+1+j,base+1+(j+1)%n})dl->PrimWriteIdx(ImDrawIdx(i));
}
void Halo(ImDrawList* dl,ImVec2 p,float r,unsigned rgb,float alpha){Halo(dl,p,r,r,rgb,alpha);}
void LightLine(ImDrawList* dl,const ImVec2* p,int count,unsigned rgb,float a,float w,bool closed){
    if(a<.003f)return;
    const ImDrawFlags flags=closed?ImDrawFlags_Closed:ImDrawFlags_None;
    dl->AddPolyline(p,count,C(rgb,a*.075f),flags,w*9);
    dl->AddPolyline(p,count,C(rgb,a*.22f),flags,w*3.3f);
    dl->AddPolyline(p,count,C(rgb,a),flags,std::max(.65f,w));
}
void Ring(ImDrawList* dl,ImVec2 c,float rx,float ry,unsigned rgb,float a,float phase){
    ImVec2 points[129];
    for(int j=0;j<=128;++j){const float u=j*tau/128;points[j]={c.x+cosf(u+phase)*rx,c.y+sinf(u+phase)*ry};}
    LightLine(dl,points,129,rgb,a,.8f);
}
void Star(ImDrawList* dl,ImVec2 p,float r,unsigned rgb,float a,float rotation){
    Halo(dl,p,r*3,rgb,a*.15f);
    ImVec2 v[8];for(int j=0;j<8;++j){const float angle=j*tau/8+rotation,d=j%2?r*.17f:r;v[j]={p.x+cosf(angle)*d,p.y+sinf(angle)*d};}
    for(int j=0;j<8;++j)dl->AddTriangleFilled(p,v[j],v[(j+1)%8],C(rgb,a));
}
ImVec2 Bezier(ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d,float t){const float u=1-t;return {a.x*u*u*u+3*b.x*u*u*t+3*c.x*u*t*t+d.x*t*t*t,a.y*u*u*u+3*b.y*u*u*t+3*c.y*u*t*t+d.y*t*t*t};}
void Ribbon(ImDrawList* dl,ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d,float width,unsigned rgb,float alpha){
    if(alpha<.002f)return;constexpr int n=72;const auto uv=ImGui::GetFontTexUvWhitePixel();
    const auto base=dl->_VtxCurrentIdx;dl->PrimReserve(n*12,(n+1)*3);
    for(int j=0;j<=n;++j){const float u=j/float(n);const auto p=Bezier(a,b,c,d,u),q=Bezier(a,b,c,d,std::min(.99999f,u+.0001f));
        const float dx=q.x-p.x,dy=q.y-p.y,len=std::max(.001f,std::hypot(dx,dy)),k=powf(std::max(0.f,sinf(u*pi)),.7f);
        for(int side=-1;side<=1;++side)dl->PrimWriteVtx({p.x-dy/len*width*side*k,p.y+dx/len*width*side*k},uv,C(rgb,side?0:alpha*k));
    }
    for(int j=0;j<n;++j)for(int k=0;k<2;++k){const auto v=base+j*3+k;for(auto i:{v,v+1,v+3,v+1,v+4,v+3})dl->PrimWriteIdx(ImDrawIdx(i));}
}
void Petal(ImDrawList* dl,ImVec2 p,float r,float angle,unsigned color,float a,float width){
    if(a<.002f)return;
    const float c=cosf(angle),s=sinf(angle);const auto uv=ImGui::GetFontTexUvWhitePixel();
    constexpr int n=20;const auto base=dl->_VtxCurrentIdx;dl->PrimReserve(n*12,(n+1)*3);
    for(int j=0;j<=n;++j){const float u=j/float(n),x=u*r,y=powf(std::max(0.f,sinf(u*pi)),.8f)*r*width;
        for(int side=-1;side<=1;++side)dl->PrimWriteVtx({p.x+c*x-s*y*side,p.y+s*x+c*y*side},uv,C(side?color:0xF1FDE8,a*(side?.18f:.7f)*sinf(u*pi*.85f+.12f)));
    }
    for(int j=0;j<n;++j)for(int k=0;k<2;++k){const auto v=base+j*3+k;for(auto i:{v,v+1,v+3,v+1,v+4,v+3})dl->PrimWriteIdx(ImDrawIdx(i));}
}
struct Petal3D{std::array<ImVec2,27> left,right;float z,light;};
void Lotus(ImDrawList* dl,ImVec2 p,float r,float time,float alpha,bool jade){
    if(alpha<.002f)return;
    const float bloom=S(time,.18f,1.72f);const int outer=jade?8:6,count=outer+6;std::vector<Petal3D> petals(count);
    for(int j=0;j<count;++j){auto& petal=petals[j];const int n=j<outer?outer:6,index=j<outer?j:j-outer;
        const float a=index*tau/n+(j<outer?0.f:pi/6),layer=j<outer?1.f:.72f;
        const float tilt=Lerp(1.48f,j<outer?.15f:.48f,bloom),len=layer*(.50f+.50f*bloom);
        auto project=[&](float u,float side){const float along=u*len,wide=powf(std::max(0.f,sinf(u*pi)),.80f)*len*.27f*side;
            const float radial=along*cosf(tilt),height=along*sinf(tilt)+sinf(u*pi)*.11f;
            const float x=cosf(a)*radial-sinf(a)*wide,z=sinf(a)*radial+cosf(a)*wide;
            return P(p,r,x,z*.37f-height);
        };
        for(int k=0;k<=26;++k){const float u=k/26.f;petal.left[k]=project(u,-1);petal.right[k]=project(u,1);}
        petal.z=sinf(a)*layer;petal.light=.55f+.45f*cosf(a-.8f);
    }
    std::sort(petals.begin(),petals.end(),[](const auto& a,const auto& b){return a.z<b.z;});
    const auto uv=ImGui::GetFontTexUvWhitePixel();
    for(const auto& petal:petals){const auto base=dl->_VtxCurrentIdx;dl->PrimReserve(26*12,27*3);
        for(int j=0;j<=26;++j){const auto a=petal.left[j],b=petal.right[j];
            const float u=j/26.f,k=alpha*(.26f+.30f*petal.light)*(.5f+.5f*sinf(u*pi));
            dl->PrimWriteVtx(a,uv,C(jade?0x54B98C:0x236FBB,k*.75f));
            dl->PrimWriteVtx({(a.x+b.x)*.5f,(a.y+b.y)*.5f},uv,C(jade?0xD5FBC5:0xB5EDFF,k));
            dl->PrimWriteVtx(b,uv,C(jade?0x83CE9C:0x4789CD,k*.75f));
        }
        for(int j=0;j<26;++j)for(int k=0;k<2;++k){const auto v=base+j*3+k;for(auto i:{v,v+1,v+3,v+1,v+4,v+3})dl->PrimWriteIdx(ImDrawIdx(i));}
        LightLine(dl,petal.left.data(),27,jade?0xB9EB9C:0x9EDFFE,alpha*(.25f+.30f*petal.light),.7f);
        LightLine(dl,petal.right.data(),27,jade?0xB9EB9C:0x9EDFFE,alpha*.25f,.65f);
    }
    Halo(dl,p,r*.55f,r*.14f,jade?0x8CD9A0:0x68BCF7,alpha*.4f);
}
void Pearl(ImDrawList* dl,ImVec2 center,float r,float t,float a,bool ice){
    if(a<.002f||r<.2f)return;
    Halo(dl,center,r*2.9f,ice?0x68BFEA:0x86D9AD,a*.25f);
    constexpr int rings=14,n=72;const auto uv=ImGui::GetFontTexUvWhitePixel();const auto base=dl->_VtxCurrentIdx;
    dl->PrimReserve(rings*n*6,(rings+1)*(n+1));
    for(int k=0;k<=rings;++k)for(int j=0;j<=n;++j){const float radius=k/float(rings),angle=j*tau/n,x=cosf(angle)*radius,y=sinf(angle)*radius,z=sqrtf(std::max(0.f,1-radius*radius));
        const float light=Clamp01(-x*.35f-y*.48f+z*.65f),shine=expf(-((x+.3f)*(x+.3f)+(y+.36f)*(y+.36f))*38);
        const float caustic=expf(-powf((sinf(x*13+y*6-t*.8f)+sinf(x*7-y*11+t*.6f))/.20f,2));
        const float rim=powf(1-z,3),brightness=Clamp01(.18f+light*.5f+shine*.65f+caustic*.20f+rim*.3f);
        const unsigned red=unsigned((ice?30:65)+brightness*(ice?180:170)),green=unsigned(100+brightness*148),blue=unsigned((ice?174:101)+brightness*(ice?81:120));
        dl->PrimWriteVtx({center.x+x*r,center.y+y*r},uv,IM_COL32(red,green,blue,int(a*255)));
    }
    for(int k=0;k<rings;++k)for(int j=0;j<n;++j){const auto v=base+k*(n+1)+j;for(auto i:{v,v+1,v+n+1,v+1,v+n+2,v+n+1})dl->PrimWriteIdx(ImDrawIdx(i));}
    Ring(dl,center,r,r,ice?0xC5F4FF:0xDBFFCA,a*.65f);
}

}
