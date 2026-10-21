#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>
#include "intro/crystal_material.h"

namespace hb::electro {
constexpr float pi=3.14159265359f;
struct V3 {float x,y,z;V3 operator+(V3 b)const{return{x+b.x,y+b.y,z+b.z};}V3 operator-(V3 b)const{return{x-b.x,y-b.y,z-b.z};}V3 operator*(float k)const{return{x*k,y*k,z*k};}};
inline V3 Cross(V3 a,V3 b){return{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline float Area(ImVec2 a,ImVec2 b,ImVec2 c){return(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
inline float Smooth(float t,float a,float b){float x=std::clamp((t-a)/(b-a),0.f,1.f);return x*x*x*(x*(x*6-15)+10);}
inline std::vector<ImVec2> Blade(){
    return {
        {0.195312f,-0.864583f},{0.252604f,-0.848958f},{0.260417f,-0.841146f},{0.322917f,-0.817708f},
        {0.197917f,-0.820312f},{0.132812f,-0.812500f},{0.028646f,-0.791667f},{-0.026042f,-0.776042f},
        {-0.052083f,-0.760417f},{0.031250f,-0.747396f},{0.119792f,-0.721354f},{0.203125f,-0.679688f},
        {0.260417f,-0.635417f},{0.338542f,-0.544271f},{0.367188f,-0.494792f},{0.408854f,-0.382812f},
        {0.419271f,-0.325521f},{0.416667f,-0.236979f},{0.393229f,-0.151042f},{0.359375f,-0.221354f},
        {0.333333f,-0.247396f},{0.289062f,-0.268229f},{0.223958f,-0.268229f},{0.190104f,-0.257812f},
        {0.119792f,-0.210938f},{0.104167f,-0.210938f},{0.093750f,-0.195312f},{0.078125f,-0.135417f},
        {0.065104f,-0.041667f},{0.070312f,0.041667f},{0.093750f,0.098958f},{0.179688f,0.226562f},
        {0.213542f,0.260417f},{0.210938f,0.239583f},{0.218750f,0.239583f},{0.263021f,0.260417f},
        {0.304688f,0.270833f},{0.403646f,0.270833f},{0.450521f,0.263021f},{0.536458f,0.234375f},
        {0.585938f,0.208333f},{0.622396f,0.182292f},{0.679688f,0.125000f},{0.736979f,0.023438f},
        {0.770833f,-0.109375f},{0.781250f,-0.205729f},{0.776042f,-0.411458f},{0.796875f,-0.390625f},
        {0.835938f,-0.322917f},{0.888021f,-0.166667f},{0.908854f,-0.018229f},{0.908854f,0.132812f},
        {0.901042f,0.200521f},{0.885417f,0.276042f},{0.859375f,0.361979f},{0.809896f,0.468750f},
        {0.773438f,0.528646f},{0.734375f,0.572917f},{0.705729f,0.617188f},{0.622396f,0.695312f},
        {0.567708f,0.736979f},{0.604167f,0.682292f},{0.664062f,0.559896f},{0.697917f,0.453125f},
        {0.705729f,0.395833f},{0.713542f,0.380208f},{0.710938f,0.372396f},{0.677083f,0.421875f},
        {0.622396f,0.481771f},{0.549479f,0.539062f},{0.473958f,0.578125f},{0.421875f,0.596354f},
        {0.341146f,0.611979f},{0.247396f,0.611979f},{0.179688f,0.601562f},{0.114583f,0.580729f},
        {0.052083f,0.549479f},{0.015625f,0.526042f},{-0.039062f,0.466146f},{-0.039062f,0.460938f},
        {0.000000f,0.468750f},{0.059896f,0.463542f},{0.091146f,0.447917f},{0.117188f,0.421875f},
        {0.138021f,0.390625f},{0.151042f,0.354167f},{0.153646f,0.296875f},{0.143229f,0.236979f},
        {0.171875f,0.257812f},{0.179688f,0.255208f},{0.174479f,0.244792f},{0.101562f,0.179688f},
        {0.023438f,0.119792f},{-0.049479f,0.085938f},{-0.122396f,0.080729f},{-0.223958f,0.085938f},
        {-0.299479f,0.098958f},{-0.296875f,0.106771f},{-0.283854f,0.109375f},{-0.325521f,0.140625f},
        {-0.361979f,0.177083f},{-0.408854f,0.263021f},{-0.445312f,0.385417f},{-0.447917f,0.471354f},
        {-0.434896f,0.533854f},{-0.411458f,0.596354f},{-0.382812f,0.645833f},{-0.328125f,0.713542f},
        {-0.252604f,0.783854f},{-0.208333f,0.815104f},{-0.093750f,0.882812f},{-0.010417f,0.924479f},
        {-0.010417f,0.929688f},{-0.023438f,0.932292f},{-0.187500f,0.927083f},{-0.346354f,0.880208f},
        {-0.453125f,0.828125f},{-0.528646f,0.783854f},{-0.596354f,0.734375f},{-0.695312f,0.643229f},
        {-0.697917f,0.632812f},{-0.734375f,0.593750f},{-0.778646f,0.528646f},{-0.833333f,0.419271f},
        {-0.872396f,0.302083f},{-0.893229f,0.169271f},{-0.843750f,0.265625f},{-0.789062f,0.343750f},
        {-0.710938f,0.429688f},{-0.661458f,0.466146f},{-0.661458f,0.450521f},{-0.695312f,0.372396f},
        {-0.697917f,0.338542f},{-0.708333f,0.307292f},{-0.713542f,0.218750f},{-0.705729f,0.143229f},
        {-0.677083f,0.049479f},{-0.645833f,-0.013021f},{-0.585938f,-0.096354f},{-0.520833f,-0.156250f},
        {-0.440104f,-0.200521f},{-0.380208f,-0.218750f},{-0.351562f,-0.221354f},{-0.395833f,-0.156250f},
        {-0.403646f,-0.135417f},{-0.403646f,-0.083333f},{-0.377604f,-0.028646f},{-0.343750f,0.007812f},
        {-0.320312f,0.026042f},{-0.257812f,0.049479f},{-0.265625f,0.059896f},{-0.286458f,0.067708f},
        {-0.291667f,0.078125f},{-0.250000f,0.070312f},{-0.226562f,0.057292f},{-0.106771f,0.015625f},
        {-0.052083f,-0.015625f},{-0.018229f,-0.046875f},{0.039062f,-0.140625f},{0.075521f,-0.216146f},
        {0.085938f,-0.252604f},{0.072917f,-0.268229f},{0.052083f,-0.361979f},{0.005208f,-0.442708f},
        {-0.059896f,-0.513021f},{-0.125000f,-0.562500f},{-0.166667f,-0.583333f},{-0.252604f,-0.609375f},
        {-0.380208f,-0.606771f},{-0.455729f,-0.588542f},{-0.557292f,-0.549479f},{-0.635417f,-0.510417f},
        {-0.763021f,-0.427083f},{-0.768229f,-0.427083f},{-0.768229f,-0.437500f},{-0.739583f,-0.494792f},
        {-0.695312f,-0.557292f},{-0.609375f,-0.651042f},{-0.533854f,-0.713542f},{-0.414062f,-0.789062f},
        {-0.289062f,-0.846354f},{-0.171875f,-0.880208f},{-0.088542f,-0.890625f},{0.052083f,-0.890625f},
    };
}
inline std::vector<ImVec2> Inset(const std::vector<ImVec2>& p){
    std::vector<ImVec2> result;result.reserve(p.size());
    for(size_t i=0;i<p.size();++i){auto prev=p[(i+p.size()-1)%p.size()],a=p[i],next=p[(i+1)%p.size()];
        float ax=a.x-prev.x,ay=a.y-prev.y,bx=next.x-a.x,by=next.y-a.y;
        float al=std::hypot(ax,ay),bl=std::hypot(bx,by);ax/=al;ay/=al;bx/=bl;by/=bl;
        float nx=-ay-by,ny=ax+bx,len=std::hypot(nx,ny);if(len<.001f){nx=-ay;ny=ax;len=1;}
        nx/=len;ny/=len;float offset=.0025f/std::max(.35f,nx*(-ay)+ny*ax);offset=std::min(offset,.00625f);
        result.push_back({a.x+nx*offset,a.y+ny*offset});
    }return result;
}
inline std::vector<std::array<int,3>> Triangulate(const std::vector<ImVec2>& p){
    std::vector<int> indices(p.size());std::iota(indices.begin(),indices.end(),0);std::vector<std::array<int,3>> triangles;
    while(indices.size()>2){bool clipped=false;
        for(size_t i=0;i<indices.size();++i){int a=indices[(i+indices.size()-1)%indices.size()],b=indices[i],c=indices[(i+1)%indices.size()];if(Area(p[a],p[b],p[c])<1e-8f)continue;bool inside=false;
            for(int j:indices)if(j!=a&&j!=b&&j!=c&&Area(p[a],p[b],p[j])>-.0000001f&&Area(p[b],p[c],p[j])>-.0000001f&&Area(p[c],p[a],p[j])>-.0000001f){inside=true;break;}
            if(inside)continue;triangles.push_back({a,b,c});indices.erase(indices.begin()+i);clipped=true;break;}
        if(!clipped)break;
    }return triangles;
}
struct Face{V3 p[3];float material,seed;};
inline const std::vector<Face>& Mesh(){
    static const auto mesh=[](){std::vector<Face> faces;const auto outline=Blade(),inset=Inset(outline);const auto triangles=Triangulate(inset);
        auto front=[](ImVec2 p){return V3{p.x,p.y,.105f+.018f*(p.x+p.y*.3f)};};
        {
            auto add=[&](V3 a,V3 b,V3 c,float material){faces.push_back({{a,b,c},material,0.f});};
            for(auto tri:triangles){add(front(inset[tri[0]]),front(inset[tri[1]]),front(inset[tri[2]]),.5f);auto a=front(inset[tri[0]]),b=front(inset[tri[1]]),c=front(inset[tri[2]]);a.z=b.z=c.z=-.085f;add(c,b,a,0);}
            for(size_t i=0;i<outline.size();++i){size_t j=(i+1)%outline.size();V3 a{outline[i].x,outline[i].y,.055f},b{outline[j].x,outline[j].y,.055f},c=front(inset[j]),d=front(inset[i]);
                add(a,b,c,1);add(a,c,d,1);V3 aa=a,bb=b;aa.z=bb.z=-.085f;add(a,aa,bb,0);add(a,bb,b,0);}
        }return faces;
    }();return mesh;
}
inline V3 Rotate(V3 p,float time){
    float settle=1-Smooth(time,2.5f,4.2f),y=.22f*settle*sinf(time*.7f+.4f),x=-.10f*settle,z=-.20f*settle;
    p={p.x,p.y*cosf(x)-p.z*sinf(x),p.y*sinf(x)+p.z*cosf(x)};
    p={p.x*cosf(y)+p.z*sinf(y),p.y,-p.x*sinf(y)+p.z*cosf(y)};
    return{p.x*cosf(z)-p.y*sinf(z),p.x*sinf(z)+p.y*cosf(z),p.z};
}
inline std::vector<CrystalVertex> Vertices(ImVec2 center,float radius,float time){
    struct Sorted{const Face* face;float depth;};std::vector<Sorted> sorted;sorted.reserve(Mesh().size());
    for(const auto& face:Mesh()){float z=0;for(auto p:face.p)z+=Rotate(p,time).z;sorted.push_back({&face,z});}
    std::stable_sort(sorted.begin(),sorted.end(),[](auto a,auto b){return a.depth<b.depth;});
    std::vector<CrystalVertex> vertices;vertices.reserve(sorted.size()*3);
    for(auto item:sorted){const auto& face=*item.face;V3 n=Cross(face.p[1]-face.p[0],face.p[2]-face.p[0]);float len=std::sqrt(n.x*n.x+n.y*n.y+n.z*n.z);if(len<1e-8f)continue;n=Rotate(n*(1/len),time);
        for(auto original:face.p){auto p=Rotate(original,time);vertices.push_back({center.x+p.x*radius,center.y+p.y*radius,n.x,n.y,n.z,original.x,original.y,face.material,1,.15f,face.seed});}
    }return vertices;
}
inline bool SelfTest(){const auto outline=Blade(),inset=Inset(outline);bool ok=Triangulate(inset).size()==inset.size()-2&&Mesh().size()>500;
    for(float time:{0.f,.8f,2.f,4.f,6.2f})for(auto v:Vertices({400,300},100,time))ok=ok&&std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.nz);
    return ok;
}
}
