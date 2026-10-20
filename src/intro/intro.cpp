#include "intro/intro.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include "core/easing.h"

namespace hb {
namespace {
constexpr float pi=3.14159265359f, tau=2.f*pi;

float S(float t,float a,float b) {
    const double x=Win(t,a,b);
    return float(x*x*x*(x*(x*6.-15.)+10.));
}
float Random(uint32_t& s) {s=s*1664525u+1013904223u;return float(s>>8)/16777216.f;}
ImU32 Color(uint32_t rgb,float a=1.f) {
    return IM_COL32((rgb>>16)&255,(rgb>>8)&255,rgb&255,int(Clamp01(a)*255.f+.5f));
}
struct V3 {
    float x,y,z;
    V3 operator+(V3 b)const{return{x+b.x,y+b.y,z+b.z};}
    V3 operator-(V3 b)const{return{x-b.x,y-b.y,z-b.z};}
    V3 operator*(float k)const{return{x*k,y*k,z*k};}
};
float Dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V3 Cross(V3 a,V3 b){return{a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
V3 Unit(V3 a){return a*(1.f/std::max(.00001f,std::sqrt(Dot(a,a))));}
V3 Rotate(V3 p,float yaw,float pitch,float roll) {
    const float cy=std::cos(yaw),sy=std::sin(yaw),cp=std::cos(pitch),sp=std::sin(pitch);
    const float cr=std::cos(roll),sr=std::sin(roll);
    const V3 a{p.x*cy+p.z*sy,p.y,p.z*cy-p.x*sy};
    const V3 b{a.x,a.y*cp-a.z*sp,a.y*sp+a.z*cp};
    return{b.x*cr-b.y*sr,b.x*sr+b.y*cr,b.z};
}
ImVec2 Project(V3 p,ImVec2 c,float r) {
    const float q=6.f/(6.f-p.z);return{c.x+p.x*q*r,c.y+p.y*q*r};
}
struct Face{V3 p[3];float material;};
struct LitFace{ImVec2 p[3];float z;ImU32 col;};
void Halo(ImDrawList* dl,ImVec2 c,float rx,float ry,uint32_t rgb,float alpha) {
    if(alpha<.004f)return;
    constexpr int rings=8,sides=32;
    const ImVec2 uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve((rings-1)*sides*6,rings*sides);
    const ImDrawIdx base=(ImDrawIdx)dl->_VtxCurrentIdx;
    for(int j=0;j<rings;++j) {
        const float r=float(j)/(rings-1),a=alpha*(std::exp(-r*r*6.f)-std::exp(-6.f));
        for(int i=0;i<sides;++i) {
            const float th=tau*i/sides;
            dl->PrimWriteVtx({c.x+std::cos(th)*r*rx,c.y+std::sin(th)*r*ry},uv,Color(rgb,a));
        }
    }
    for(int j=0;j<rings-1;++j)for(int i=0;i<sides;++i) {
        const int ni=(i+1)%sides;
        for(int id:{j*sides+i,(j+1)*sides+i,(j+1)*sides+ni,j*sides+i,(j+1)*sides+ni,j*sides+ni})
            dl->PrimWriteIdx((ImDrawIdx)(base+id));
    }
}

const std::vector<V3> outer={
    {0,-1.10f,0},{-.258f,-.548f,0},{-.159f,-.591f,0},{-.074f,-.404f,0},
    {-.042f,-.246f,0},{-.057f,-.225f,0},{-.186f,-.323f,0},{-.135f,-.196f,0},
    {0,.012f,0},{.135f,-.196f,0},{.186f,-.323f,0},{.057f,-.225f,0},
    {.042f,-.246f,0},{.074f,-.404f,0},{.159f,-.591f,0},{.258f,-.548f,0}
};
const std::vector<V3> hole={{0,-.735f,0},{.079f,-.612f,0},{0,-.352f,0},{-.079f,-.612f,0}};
constexpr float kOuterBevel=.018f,kHoleBevel=.012f;
float BackDepth(V3 p){return .055f+.080f*Clamp01(1.f+p.y);}
float Area(const std::vector<V3>& loop) {
    float a=0;
    for(size_t i=0;i<loop.size();++i){const auto p=loop[i],q=loop[(i+1)%loop.size()];a+=p.x*q.y-q.x*p.y;}
    return a*.5f;
}
std::vector<V3> Inset(const std::vector<V3>& loop,float d) {
    std::vector<V3> result;
    const float winding=Area(loop)>0?1.f:-1.f;
    for(size_t i=0;i<loop.size();++i) {
        const V3 p=loop[i],a=Unit(p-loop[(i+loop.size()-1)%loop.size()]),b=Unit(loop[(i+1)%loop.size()]-p);
        const V3 na{-a.y*winding,a.x*winding,0},nb{-b.y*winding,b.x*winding,0};
        const V3 bisector=Unit(na+nb);
        const V3 previous=loop[(i+loop.size()-1)%loop.size()],next=loop[(i+1)%loop.size()];
        const float shortest=std::min(std::sqrt(Dot(p-previous,p-previous)),std::sqrt(Dot(next-p,next-p)));
        const float point=Clamp01((1.f-Dot(a,b))*.5f);
        const float width=d*(1.f-.85f*point*point);
        const float limit=std::min(.021f,shortest*.28f);
        result.push_back(p+bisector*std::clamp(width/std::max(.15f,Dot(bisector,nb)),-limit,limit));
    }
    return result;
}

constexpr V3 cuts[]={{1,0,0},{.70f,1,.60f},{-.60f,1,.26f}};
float CutDistance(V3 p,V3 cut){return p.x*cut.x+p.y*cut.y+cut.z;}
float FrontHeight(V3 p) {
    return .105f-.25f*std::sqrt(p.x*p.x+.0004f)
                 -.070f*std::sqrt(std::pow(CutDistance(p,cuts[1]),2.f)+.0009f)
                 -.040f*std::sqrt(std::pow(CutDistance(p,cuts[2]),2.f)+.0009f);
}
V3 FrontNormal(V3 p) {
    constexpr float step=.0005f;
    const float dx=(FrontHeight({p.x+step,p.y,0})-FrontHeight({p.x-step,p.y,0}))/(2*step);
    const float dy=(FrontHeight({p.x,p.y+step,0})-FrontHeight({p.x,p.y-step,0}))/(2*step);
    return Unit({-dx,-dy,1.f});
}
std::vector<V3> ClipPlane(const std::vector<V3>& polygon,V3 cut,float sign) {
    std::vector<V3> clipped;
    for(size_t i=0;i<polygon.size();++i) {
        const V3 a=polygon[i],b=polygon[(i+1)%polygon.size()];
        const float da=CutDistance(a,cut)*sign,db=CutDistance(b,cut)*sign;
        if(da>=0)clipped.push_back(a);
        if((da<0&&db>0)||(da>0&&db<0))clipped.push_back(a+(b-a)*(da/(da-db)));
    }
    return clipped;
}
std::vector<Face> BuildCrystal() {
    std::vector<Face> mesh;
    auto tri=[&](V3 a,V3 b,V3 c,float material){
        const V3 cross=Cross(b-a,c-a);
        if(Dot(cross,cross)>1.e-14f)mesh.push_back({{a,b,c},material});
    };
    const auto inside=Inset(outer,kOuterBevel),outsideHole=Inset(hole,-kHoleBevel);
    auto walls=[&](const std::vector<V3>& edge,const std::vector<V3>& inset) {
        for(size_t i=0;i<edge.size();++i) {
            const size_t j=(i+1)%edge.size();

            std::vector<float> stops{0,1};
            for(V3 cut:cuts) {
                const float a=CutDistance(inset[i],cut),b=CutDistance(inset[j],cut);
                if(a*b<0)stops.push_back(a/(a-b));
            }
            std::sort(stops.begin(),stops.end());
            for(size_t k=1;k<stops.size();++k) {
                V3 e=inset[i]+(inset[j]-inset[i])*stops[k-1],f=inset[i]+(inset[j]-inset[i])*stops[k];
                V3 a=edge[i]+(edge[j]-edge[i])*stops[k-1],b=edge[i]+(edge[j]-edge[i])*stops[k];
                const float depthA=BackDepth(a),depthB=BackDepth(b);
                a.z=-depthA;b.z=-depthB;
                V3 c{b.x,b.y,FrontHeight(f)-.018f},d{a.x,a.y,FrontHeight(e)-.018f};
                e.z=FrontHeight(e);f.z=FrontHeight(f);
                tri(a,b,c,.15f);tri(a,c,d,.15f);tri(d,c,f,.95f);tri(d,f,e,.95f);
            }
        }
    };
    walls(outer,inside);walls(hole,outsideHole);

    std::vector<float> ys;
    for(const auto* loop:{&inside,&outsideHole})for(V3 p:*loop)ys.push_back(p.y);
    std::sort(ys.begin(),ys.end());ys.erase(std::unique(ys.begin(),ys.end()),ys.end());
    struct Edge{V3 a,b;float mid;float At(float y)const{return Lerp(a.x,b.x,(y-a.y)/(b.y-a.y));}};
    for(size_t row=1;row<ys.size();++row) {
        const float lo=ys[row-1],hi=ys[row],mid=(lo+hi)*.5f;
        if(hi-lo<.00001f)continue;
        std::vector<Edge> edges;
        for(const auto* loop:{&inside,&outsideHole})for(size_t i=0;i<loop->size();++i) {
            const V3 a=(*loop)[i],b=(*loop)[(i+1)%loop->size()];
            if(mid>std::min(a.y,b.y)&&mid<std::max(a.y,b.y)){Edge e{a,b,0};e.mid=e.At(mid);edges.push_back(e);}
        }
        std::sort(edges.begin(),edges.end(),[](Edge a,Edge b){return a.mid<b.mid;});
        for(size_t i=1;i<edges.size();i+=2) {
            const V3 a{edges[i-1].At(lo),lo,0},b{edges[i].At(lo),lo,0};
            const V3 c{edges[i].At(hi),hi,0},d{edges[i-1].At(hi),hi,0};
            std::vector<std::vector<V3>> pieces{{a,b,c,d}};
            for(V3 cut:cuts) {
                std::vector<std::vector<V3>> split;
                for(const auto& piece:pieces)for(float sign:{-1.f,1.f}) {
                    auto polygon=ClipPlane(piece,cut,sign);
                    if(polygon.size()>=3)split.push_back(std::move(polygon));
                }
                pieces=std::move(split);
            }
            for(auto& piece:pieces) {
                for(V3& p:piece)p.z=FrontHeight(p);
                for(size_t k=2;k<piece.size();++k)tri(piece[0],piece[k-1],piece[k],.5f);
            }
        }
    }
    return mesh;
}
const std::vector<Face>& Crystal(){static const auto mesh=BuildCrystal();return mesh;}
int ClipGrowth(const V3* p,V3* result,float front) {
    int n=0;
    for(int i=0;i<3;++i) {
        const V3 a=p[i],b=p[(i+1)%3];const bool ia=a.y>=front,ib=b.y>=front;
        if(ia)result[n++]=a;
        if(ia!=ib)result[n++]=a+(b-a)*((front-a.y)/(b.y-a.y));
    }
    return n;
}
struct GlassFace { CrystalVertex p[3]; float z; };
void IceArm(std::vector<GlassFace>& output,ImVec2 center,float radius,float angle,float growth,float alpha,float t,float seed) {
    if(growth<=0||alpha<.003f)return;

    const float pose=S(t,.20f,Intro::kTurnEnd);
    const float yaw=-.14f+.08f*pose,pitch=-.10f;
    for(const Face& f:Crystal()) {
        V3 clip[4];const int count=ClipGrowth(f.p,clip,-growth*1.10f);
        for(int j=2;j<count;++j) {
            V3 source[3]={clip[0],clip[j-1],clip[j]},p[3];
            for(int k=0;k<3;++k)p[k]=Rotate(Rotate(source[k],0,0,angle),yaw,pitch,0);
            V3 n=Unit(Cross(p[1]-p[0],p[2]-p[0]));if(n.z<0)n=n*-1.f;
            GlassFace face{};
            face.z=(p[0].z+p[1].z+p[2].z)/3.f;
            for(int k=0;k<3;++k) {
                const ImVec2 screen=Project(p[k],center,radius);
                const float backDepth=BackDepth(source[k]);
                const float thickness=std::max(.012f,source[k].z+backDepth);
                const V3 normal=f.material==.5f?Rotate(Rotate(FrontNormal(source[k]),0,0,angle),yaw,pitch,0):n;
                face.p[k]={screen.x,screen.y,normal.x,normal.y,normal.z,source[k].x,source[k].y,f.material,alpha,thickness,seed};
            }
            output.push_back(face);
        }
    }
}
void CrystalDetails(std::vector<GlassFace>& output,ImVec2 center,float radius,float t,float alpha) {
    const float dissolve=1.f-S(t,4.65f,5.65f);
    auto face=[&](V3 a,V3 b,V3 c,float yaw,float pitch,float roll,ImVec2 origin,float size,float opacity) {
        V3 source[3]={a,b,c},p[3];
        for(int k=0;k<3;++k)p[k]=Rotate(source[k],yaw,pitch,roll);
        V3 n=Unit(Cross(p[1]-p[0],p[2]-p[0]));if(n.z<0)n=n*-1.f;
        GlassFace f{};f.z=(p[0].z+p[1].z+p[2].z)/3.f+.15f;
        for(int k=0;k<3;++k) {
            const ImVec2 point=Project(p[k],origin,size);
            const float localScale=size<radius*.1f?.25f:1.f;
            const float localOffset=size<radius*.1f?-.5f:0.f;
            f.p[k]={point.x,point.y,n.x,n.y,n.z,source[k].x*localScale,
                    source[k].y*localScale+localOffset,.5f,opacity,.085f};
        }
        output.push_back(f);
    };
    const float hub=S(t,3.25f,Intro::kTurnEnd)*alpha;
    if(hub>.001f)for(int j=0;j<6;++j) {
        const float a=j*tau/6.f,b=(j+1)*tau/6.f;
        face({0,0,.15f},{std::sin(a)*.092f,-std::cos(a)*.092f,.065f},
             {std::sin(b)*.092f,-std::cos(b)*.092f,.065f},-.06f,-.10f,0,center,radius,hub);
    }
    for(int i=0;i<9;++i) {
        const float phase=i*2.399963f,a=phase+.025f*std::sin(t*.75f+phase);
        const float visible=S(t,2.1f+i*.115f,2.9f+i*.08f)*alpha*dissolve*.62f;
        if(visible<.001f)continue;
        const float distance=radius*(1.24f+.18f*std::sin(phase*1.7f)+.025f*std::sin(t*.80f+phase));
        const ImVec2 origin{center.x+std::sin(a)*distance,center.y-std::cos(a)*distance};
        const float size=radius*(.017f+.009f*(.5f+.5f*std::sin(phase)));
        const V3 ring[]={{-.46f,0,0},{0,.24f,.36f},{.46f,0,0},{0,-.24f,-.36f}};
        for(int j=0;j<4;++j) {
            const V3 a=ring[j],b=ring[(j+1)%4];
            face({0,-1.5f,0},a,b,.4f+phase*.1f,.3f,phase+t*.20f,origin,size,visible);
            face({0,1.5f,0},b,a,.4f+phase*.1f,.3f,phase+t*.20f,origin,size,visible);
        }
    }
}
const std::vector<CrystalVertex>& SettledCrystal() {

    static const auto vertices=[] {
        std::vector<GlassFace> faces;
        IceArm(faces,{0,0},1.f,5.f*tau/6.f,1.f,1.f,Intro::kDuration,5.f);
        for(int i=0;i<5;++i)IceArm(faces,{0,0},1.f,i*tau/6.f,1.f,1.f,Intro::kDuration,float(i));
        CrystalDetails(faces,{0,0},1.f,Intro::kDuration,1.f);
        std::stable_sort(faces.begin(),faces.end(),[](const GlassFace& a,const GlassFace& b){return a.z<b.z;});
        std::vector<CrystalVertex> result;result.reserve(faces.size()*3);
        for(const auto& face:faces)for(const auto& vertex:face.p)result.push_back(vertex);
        return result;
    }();
    return vertices;
}
void ColdMist(ImDrawList* dl,ImVec2 center,float radius,float t,float alpha) {
    const float visible=S(t,2.5f,3.8f)*(1.f-S(t,4.5f,5.6f))*alpha;
    if(visible<.001f)return;
    constexpr int rows=40,columns=7;
    const ImVec2 uv=ImGui::GetFontTexUvWhitePixel();
    for(float side:{-1.f,1.f}) {
        dl->PrimReserve(rows*(columns-1)*6,(rows+1)*columns);
        const ImDrawIdx base=(ImDrawIdx)dl->_VtxCurrentIdx;
        for(int row=0;row<=rows;++row) {
            const float u=float(row)/rows,wave=std::sin(u*6.f-t*.8f+side);
            const float x=center.x+side*radius*(.82f+u*.40f);
            const float y=center.y+radius*(.57f-u*.22f+wave*.035f);
            const float width=radius*(.008f+.040f*u);
            for(int column=0;column<columns;++column) {
                const float v=float(column)/(columns-1)*2.f-1.f;
                const float density=std::pow(std::sin(u*pi),2.f)*(std::exp(-v*v*5.f)-std::exp(-5.f));
                dl->PrimWriteVtx({x,y+v*width},uv,Color(0xBCEAF5,visible*density*.065f));
            }
        }
        for(int row=0;row<rows;++row)for(int col=0;col<columns-1;++col)
            for(int id:{row*columns+col,(row+1)*columns+col,(row+1)*columns+col+1,
                        row*columns+col,(row+1)*columns+col+1,row*columns+col+1})
                dl->PrimWriteIdx((ImDrawIdx)(base+id));
    }
}
void CoreGlint(ImDrawList* dl,ImVec2 center,float radius,float t,float alpha) {
    const float visible=S(t,3.45f,3.75f)*(1.f-S(t,4.3f,4.9f))*alpha;
    if(visible<.001f)return;
    const float pulse=.42f+.58f*std::exp(-std::pow((t-3.80f)/.18f,2.f));
    const ImVec2 uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(12,12);
    for(int j=0;j<4;++j) {
        const float a=tau*j/4.f;
        const V3 d{std::cos(a),std::sin(a),0},n{-d.y,d.x,0};
        const float length=radius*(j%2?.072f:.095f);
        const float width=std::max(.55f,radius*.004f);
        const ImDrawIdx base=(ImDrawIdx)dl->_VtxCurrentIdx;
        dl->PrimWriteVtx({center.x+n.x*width,center.y+n.y*width},uv,Color(0xF6FDFF,visible*pulse));
        dl->PrimWriteVtx({center.x+d.x*length,center.y+d.y*length},uv,Color(0x99DFFC,0));
        dl->PrimWriteVtx({center.x-n.x*width,center.y-n.y*width},uv,Color(0xF6FDFF,visible*pulse));
        for(int k=0;k<3;++k)dl->PrimWriteIdx((ImDrawIdx)(base+k));
    }
}
void DrawFaces(ImDrawList* dl,std::vector<LitFace>& faces) {
    std::stable_sort(faces.begin(),faces.end(),[](const LitFace& a,const LitFace& b){return a.z<b.z;});
    const ImVec2 uv=ImGui::GetFontTexUvWhitePixel();
    dl->PrimReserve(int(faces.size())*3,int(faces.size())*3);
    for(const auto& f:faces) {
        const ImDrawIdx base=(ImDrawIdx)dl->_VtxCurrentIdx;
        for(int i=0;i<3;++i)dl->PrimWriteVtx(f.p[i],uv,f.col);
        for(int i=0;i<3;++i)dl->PrimWriteIdx((ImDrawIdx)(base+i));
    }
}

void Lamp(ImDrawList* dl,ImVec2 center,float size,float phase,float t,float alpha) {
    if(alpha<.003f)return;
    const float yaw=phase+t*.13f,tilt=.075f*std::sin(t*.72f+phase),pitch=.12f*std::cos(phase+t*.48f);
    auto world=[&](V3 p){return Rotate(p,yaw,pitch,tilt);};
    auto screen=[&](V3 p){return Project(world(p),center,size);};
    auto ring=[](float a,float y,float r){return V3{std::cos(a)*r,y,std::sin(a)*r};};
    const float flicker=.95f+.025f*std::sin(t*5.1f+phase)+.018f*std::sin(t*8.3f+phase);
    Halo(dl,screen({0,.22f,0}),size*2.7f,size*3.1f,0xF5A64B,alpha*.19f*flicker);
    std::vector<LitFace> faces;faces.reserve(840);
    constexpr int panels=6,rows=14,cols=5;
    for(int side=0;side<panels;++side)for(int row=0;row<rows;++row)for(int col=0;col<cols;++col) {
        auto vertex=[&](int u,int v) {
            const float h=float(v)/rows,k=float(u)/cols,a=tau*side/panels,b=tau*(side+1)/panels;
            const float radius=.43f+.16f*std::sin(h*pi)-.10f*h;
            V3 p=ring(a,Lerp(-.85f,.60f,h),radius)*(1.f-k)+ring(b,Lerp(-.85f,.60f,h),radius)*k;
            const float pleat=.009f*std::sin(k*pi*6.f)*std::sin(h*pi);p.x*=1.f+pleat;p.z*=1.f+pleat;return p;
        };
        V3 raw[4]={vertex(col,row),vertex(col+1,row),vertex(col+1,row+1),vertex(col,row+1)};
        const V3 mid=(raw[0]+raw[1]+raw[2]+raw[3])*.25f;
        const float warmth=std::exp(-std::pow((mid.y-.30f)*1.6f,2.f));
        const float grain=.015f*std::sin(row*13.7f+col*18.2f+side*9.1f),lit=.38f+.40f*warmth*flicker+grain;
        const float facing=.88f+.12f*std::sin(yaw+(side+.5f)*tau/panels);
        const ImU32 color=IM_COL32(int((171+80*lit)*facing),int((74+124*lit)*facing),int((37+102*lit)*facing),int(alpha*255));
        V3 p[4];for(int i=0;i<4;++i)p[i]=world(raw[i]);
        for(int k=0;k<2;++k) {
            const int b=k?2:1,c=k?3:2;
            faces.push_back({{Project(p[0],center,size),Project(p[b],center,size),Project(p[c],center,size)},
                             (p[0].z+p[b].z+p[c].z)/3.f,color});
        }
    }
    for(int side=0;side<panels;++side) {
        const V3 a=world({0,-.97f,0});
        const V3 b=world(ring(tau*side/panels,-.85f,.43f));
        const V3 c=world(ring(tau*(side+1)/panels,-.85f,.43f));
        faces.push_back({{Project(a,center,size),Project(b,center,size),Project(c,center,size)},
                         (a.z+b.z+c.z)/3.f,Color(side%2?0xDAB077:0xBE8D58,alpha)});
    }
    DrawFaces(dl,faces);

    for(int i=0;i<panels;++i) {
        const float angle=tau*i/panels;if(world(ring(angle,0,1)).z<-.05f)continue;
        ImVec2 last=screen(ring(angle,-.85f,.43f));
        for(int j=1;j<=14;++j) {
            const float h=j/14.f;
            const ImVec2 next=screen(ring(angle,Lerp(-.85f,.60f,h),.43f+.16f*std::sin(h*pi)-.10f*h));
            dl->AddLine(last,next,Color(0xFFE4A2,alpha*.65f),std::max(.6f,size*.018f));last=next;
        }
        const float a=tau*i/panels,b=tau*(i+1)/panels;
        dl->AddLine(screen(ring(a,-.85f,.43f)),screen(ring(b,-.85f,.43f)),Color(0xBE925B,alpha),std::max(.7f,size*.026f));
        dl->AddLine(screen(ring(a,.60f,.33f)),screen(ring(b,.60f,.33f)),Color(0x805039,alpha),std::max(.8f,size*.034f));
        if(size>12.f) {
            ImVec2 motif[5];
            for(int j=0;j<5;++j) {
                const float k=.20f+j*.15f;
                V3 p=ring(a,-.22f,.58f)*(1.f-k)+ring(b,-.22f,.58f)*k;p.y+=std::abs(j-2)*.055f;motif[j]=screen(p);
            }
            dl->AddPolyline(motif,5,Color(0xFFE2A1,alpha*.50f),0,std::max(.7f,size*.016f));
        }
    }
    const ImVec2 flame=screen({0,.58f,.14f});
    Halo(dl,flame,size*.42f,size*.58f,0xFFF3C6,alpha*.75f*flicker);
    dl->AddTriangleFilled(screen({-.065f,.64f,.14f}),screen({.035f*std::sin(t*4.f+phase),.32f,.14f}),
                          screen({.065f,.64f,.14f}),Color(0xFFF8D8,alpha*.9f));
    dl->AddLine(screen({.18f,.62f,0}),screen({.18f+.11f*std::sin(t*.9f+phase),.99f,0}),Color(0xC8AA75,alpha*.8f),std::max(.6f,size*.014f));
}
void Trail(ImDrawList* dl,ImVec2 c,float r,float angle,float turn,float alpha) {
    const float strength=S(turn,0,.15f)*(1.f-S(turn,.78f,1.f))*alpha;if(strength<.001f)return;
    for(int strand=0;strand<3;++strand) {
        ImVec2 prev{};
        for(int i=0;i<=44;++i) {
            const float k=i/44.f,a=angle-.58f+k*.58f,radius=r*(.96f+.018f*strand+.02f*std::sin(k*pi));
            const ImVec2 p{c.x+std::sin(a)*radius,c.y-std::cos(a)*radius};
            if(i)dl->AddLine(prev,p,Color(0xBCEFFB,strength*k*k*(.18f-.04f*strand)),std::max(.65f,r*.005f));prev=p;
        }
    }
}
void IceSpark(ImDrawList* dl,ImVec2 p,float size,float alpha,float roll=0.f) {
    if(alpha<.004f)return;
    const float sn=std::sin(roll),cs=std::cos(roll);
    const ImVec2 a{sn*size*2.f,-cs*size*2.f},b{cs*size*.6f,sn*size*.6f};
    const ImVec2 tip{p.x+a.x,p.y+a.y},tail{p.x-a.x,p.y-a.y};
    dl->AddTriangleFilled(tip,{p.x+b.x,p.y+b.y},tail,Color(0xA0DFFF,alpha*.65f));
    dl->AddTriangleFilled(tip,tail,{p.x-b.x,p.y-b.y},Color(0xEEFCFF,alpha));
}

void IceAtmosphere(ImDrawList* dl,ImVec2 c,float r,float t,float alpha) {

    const float gather=S(t,1.25f,1.85f)*(1.f-S(t,3.42f,3.90f))*alpha;
    if(gather>.004f)for(int strand=0;strand<2;++strand) {
        constexpr int steps=72;
        const float turn=S(t,Intro::kTurnStart,Intro::kTurnEnd);
        const float collapse=S(t,3.38f,3.90f);
        const float head=turn*tau*5.f/6.f+strand*pi;
        const ImVec2 uv=ImGui::GetFontTexUvWhitePixel();
        dl->PrimReserve(steps*12,(steps+1)*3);
        const ImDrawIdx base=(ImDrawIdx)dl->_VtxCurrentIdx;
        for(int j=0;j<=steps;++j) {
            const float u=j/float(steps),angle=head-(1.f-u)*2.4f;
            const float d=r*(1.24f+(1.f-u)*.45f)*(1.f-collapse*.78f);
            const ImVec2 p{c.x+std::sin(angle)*d,
                           c.y-std::cos(angle)*d*.72f+std::sin(angle)*r*.15f};
            const float width=r*(.018f+.014f*u)*std::sin(u*pi);
            const float intensity=std::pow(std::sin(u*pi),1.5f)*gather;
            const ImVec2 n{std::sin(angle),-std::cos(angle)};
            dl->PrimWriteVtx({p.x-n.x*width,p.y-n.y*width},uv,Color(0x4BAFFF,0));
            dl->PrimWriteVtx(p,uv,Color(strand?0xA5EDFF:0x64C8FF,intensity*.48f));
            dl->PrimWriteVtx({p.x+n.x*width,p.y+n.y*width},uv,Color(0x4BAFFF,0));
        }
        for(int j=0;j<steps;++j)for(int side=0;side<2;++side) {
            const int a=j*3+side,b=(j+1)*3+side;
            for(int id:{a,b,b+1,a,b+1,a+1})dl->PrimWriteIdx((ImDrawIdx)(base+id));
        }
    }

    for(int i=0;i<54;++i) {
        const float phase=i*2.399963f,start=.55f+(i%18)*.092f+(i/18)*.13f;
        const float age=(t-start)/1.45f;
        const float visible=S(age,0,.20f)*(1.f-S(age,.72f,1.f))*alpha;
        if(visible<.004f)continue;
        auto path=[&](float u) {
            u=Clamp01(u);const float k=S(u,0,1),a=phase+u*2.65f;
            const float radius=r*Lerp(2.20f+(i%5)*.09f,.27f,k);
            return ImVec2{c.x+std::sin(a)*radius,
                          c.y-std::cos(a)*radius*.70f+std::sin(a*.7f)*r*.12f};
        };
        ImVec2 previous=path(age-.09f);
        for(int j=1;j<=8;++j) {
            const float k=j/8.f;const ImVec2 p=path(age-(1.f-k)*.09f);
            dl->AddLine(previous,p,Color(0x8EDFFF,visible*k*k*.60f),std::max(.75f,r*.006f));previous=p;
        }
        IceSpark(dl,path(age),r*(.005f+(i%4)*.0023f),visible,phase+age*3.f);
    }

    const float age=Win(t,3.53f,4.48f),expand=S(age,0,1);
    const float wave=S(age,0,.14f)*(1.f-S(age,.30f,1.f))*alpha;
    if(wave>.004f) {
        const float radius=r*Lerp(.30f,2.25f,expand);
        for(int j=0;j<120;++j) {
            const float a=tau*j/120.f,b=tau*(j+1)/120.f;
            const float broken=.15f+.85f*std::pow(.5f+.5f*std::sin(a*7.f+.8f),2.f);
            auto point=[&](float angle) {
                const float warp=1.f+.012f*std::sin(angle*11.f+age*3.f);
                return ImVec2{c.x+std::sin(angle)*radius*warp,c.y-std::cos(angle)*radius*.78f*warp};
            };
            dl->AddLine(point(a),point(b),Color(0xB3EDFF,wave*broken*.68f),std::max(.85f,r*.009f));
        }
        for(int i=0;i<24;++i) {
            const float a=i*2.399963f,d=radius*(.91f+(i%4)*.045f);
            const ImVec2 p{c.x+std::sin(a)*d,c.y-std::cos(a)*d*.78f};
            IceSpark(dl,p,r*(.006f+(i%3)*.003f),wave,a+age);
        }
    }
}
void IceFinish(ImDrawList* dl,ImVec2 c,float r,float t,float alpha) {

    const float impact=S(t,3.55f,3.72f)*(1.f-S(t,3.76f,4.18f))*alpha;
    if(impact>.004f) {
        Halo(dl,c,r*.70f,r*.70f,0x66CFFF,impact*.20f);
        const float age=Win(t,3.56f,4.20f);
        for(int i=0;i<42;++i) {
            const float phase=i*2.399963f;
            const float arm=(i%6)*tau/6.f;
            const float spread=.065f*std::sin(phase);
            const float a=arm+spread,d=r*(.33f+(1.05f+(i%7)*.095f)*S(age,0,1));
            const float flicker=.68f+.32f*std::sin(phase+age*5.f);
            const ImVec2 p{c.x+std::sin(a)*d,c.y-std::cos(a)*d*.93f};
            const float tail=r*(.035f+(i%4)*.022f)*std::sin(age*pi);
            dl->AddLine({p.x-std::sin(a)*tail,p.y+std::cos(a)*tail*.93f},p,
                        Color(0xA0E9FF,impact*flicker*.65f),std::max(.8f,r*.006f));
            IceSpark(dl,p,r*(.006f+(i%3)*.003f),impact*flicker,phase+age*2.f);
        }
    }

    for(int i=0;i<6;++i) {
        const float start=3.60f+i*.105f,age=Win(t,start,start+.42f);
        const float visible=S(age,0,.20f)*(1.f-S(age,.25f,1.f))*alpha;
        if(visible<.004f)continue;
        const V3 tip=Rotate(Rotate({0,-1.10f,.055f},0,0,i*tau/6.f),-.06f,-.10f,0);
        const ImVec2 p=Project(tip,c,r);
        const float length=r*(.035f+.045f*std::sin(age*pi));
        Halo(dl,p,length*1.9f,length*1.9f,0x89DFFF,visible*.13f);
        dl->AddLine({p.x-length,p.y},{p.x+length,p.y},Color(0xE2F8FF,visible*.72f),.8f);
        dl->AddLine({p.x,p.y-length*.65f},{p.x,p.y+length*.65f},Color(0xE2F8FF,visible*.58f),.8f);
        IceSpark(dl,p,std::max(.7f,r*.006f),visible);
    }
}
}

void Intro::Reset(float w,float h) {
    t=0;finished=false;skipped=false;skipElapsed=0;W=std::max(w,1.f);H=std::max(h,1.f);
    uint32_t seed=0x85A16B3Du;
    for(auto& p:snow)p={Random(seed),Random(seed),.0005f+Random(seed)*.0013f,Random(seed),Random(seed)*tau};
    for(int i=0;i<kLantCount;++i) {
        const float depth=float(i)/(kLantCount-1),side=i%2?1.f:-1.f;
        lant[i]={.5f+side*(.20f+Random(seed)*.27f),.18f+Random(seed)*.95f,
                 .0028f+depth*depth*.027f,.007f+depth*.019f,Random(seed)*tau};
    }
    lant[kLantCount-2]={.075f,.76f,.066f,.038f,.7f};
    lant[kLantCount-1]={.91f,.40f,.050f,.030f,2.1f};
}
void Intro::Skip(){if(finished||skipped)return;skipped=true;skipStart=t;skipElapsed=0;}
void Intro::Update(float dt) {
    if(finished)return;dt=std::clamp(dt,0.f,.20f);
    if(skipped){

        const float duration=std::min(.9f,kDuration-skipStart);
        skipElapsed+=dt;
        t=std::min(kDuration,skipStart+skipElapsed+(kDuration-skipStart-duration)*S(skipElapsed,0,duration));
        finished=t>=kDuration;
    }
    else{t=std::min(kDuration,t+dt);finished=t>=kDuration;}
}
float Intro::Opacity()const{return 1.f-S(t,kDockStart,5.55f);}
float Intro::HandoffK()const{return S(t,kDockStart,kDuration);}
float Intro::Progress()const{return Clamp01(t/kDuration);}
Intro::MarkPose Intro::Pose(ImVec2 o)const {
    const float rise=S(t,.20f,kTurnStart),dock=HandoffK();
    const ImVec2 start{o.x+W*.5f,o.y+H*Lerp(.66f,.45f,rise)};
    const float impact=1.f-.025f*S(t,3.33f,3.58f)+.065f*S(t,3.58f,3.78f)-.040f*S(t,3.78f,4.18f);

    const float arc=std::sin(pi*dock);
    return {{Lerp(start.x,dockCenter.x,dock),Lerp(start.y,dockCenter.y,dock)-H*.055f*arc*arc},
            Lerp(std::min(W,H)*.155f*impact,dockRadius,dock),S(t,.18f,.67f)};
}
void Intro::Draw(ImDrawList* dl,ImVec2 o) {
    if(!dl||finished)return;
    const float fade=Opacity(),s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);

    const float atmosphere=1.f-.10f*S(t,.25f,3.5f);
    dl->AddRectFilledMultiColor(o,{o.x+W,o.y+H},Color(0x08141E,.93f*fade*atmosphere),Color(0x08141E,.93f*fade*atmosphere),
                               Color(0x132328,.71f*fade*atmosphere),Color(0x132328,.71f*fade*atmosphere));
    const float sideW=W*.22f;
    dl->AddRectFilledMultiColor(o,{o.x+sideW,o.y+H},Color(0x040B12,.28f*fade),Color(0x040B12,0),Color(0x040B12,0),Color(0x040B12,.28f*fade));
    dl->AddRectFilledMultiColor({o.x+W-sideW,o.y},{o.x+W,o.y+H},Color(0x040B12,0),Color(0x040B12,.28f*fade),Color(0x040B12,.28f*fade),Color(0x040B12,0));
    for(const auto& p:lant) {
        const float y=p.y0-t*p.spd,x=p.x+.007f*std::sin(t*.40f+p.ph);
        const float edge=S(y,-.1f,.05f)*(1.f-S(y,1.05f,1.2f));
        Lamp(dl,{o.x+W*x,o.y+H*y},s*p.size,p.ph,t,fade*edge*S(t,0,.70f)*.85f);
    }
    for(const auto& p:snow) {
        const float x=p.x+.006f*std::sin(t*.40f+p.ph),y=std::fmod(p.y-t*(.009f+.012f*p.depth)+2.f,1.f);
        const float a=S(t,.80f,2.2f)*fade*(.10f+.25f*p.depth)*S(y,0,.08f)*(1.f-S(y,.92f,1));
        const ImVec2 c{o.x+x*W,o.y+y*H};
        dl->AddLine(c,{c.x+s*p.size*.45f,c.y-s*p.size*1.8f},Color(0xD0EBF3,a),std::max(.65f,s*p.size*.45f));
    }
    if(fBrand) {
        const float reveal=S(t,3.75f,4.45f)*(1.f-S(t,4.90f,5.70f))*fade;
        const float fs=20.f,tracking=4.f,margin=std::clamp(s*.055f,24.f,48.f);
        constexpr char title[]="HANABI";
        float width=tracking*5.f;
        for(int j=0;j<6;++j)width+=fBrand->CalcTextSizeA(fs,FLT_MAX,0,title+j,title+j+1).x;
        const float y=std::round(o.y+H-margin-fs);
        float x=std::round(o.x+W-margin-width);
        dl->AddLine({x-35.f,y+fs*.55f},{x-14.f,y+fs*.55f},Color(0xD0E5F2,reveal*.38f),1.f);
        for(int j=0;j<6;++j) {
            dl->AddText(fBrand,fs,{x,y+1.f},Color(0x000000,reveal*.16f),title+j,title+j+1);
            dl->AddText(fBrand,fs,{x,y},Color(0xD0E5F2,reveal*.90f),title+j,title+j+1);
            x+=fBrand->CalcTextSizeA(fs,FLT_MAX,0,title+j,title+j+1).x+tracking;
        }
    }
    dl->PopClipRect();
}
void Intro::DrawMark(ImDrawList* dl,ImVec2 o,float alpha) {
    if(!dl||alpha<.001f)return;
    const auto pose=Pose(o);
    const ImVec2 center=pose.center;
    const float radius=pose.radius,turn=S(t,kTurnStart,kTurnEnd),angle=(5.f*tau/6.f)*turn;
    const float presence=pose.opacity*alpha,s=std::min(W,H);
    dl->PushClipRect(o,{o.x+W,o.y+H},true);
    const auto& keyLamp=lant[kLantCount-1];
    const ImVec2 lantern{o.x+W*(keyLamp.x+.007f*std::sin(t*.40f+keyLamp.ph)),
                         o.y+H*(keyLamp.y0-t*keyLamp.spd)};
    if(t>=5.65f) {
        auto vertices=SettledCrystal();
        for(auto& vertex:vertices){vertex.x=center.x+vertex.x*radius;vertex.y=center.y+vertex.y*radius;vertex.opacity*=presence;}
        material.Queue(dl,std::move(vertices),center,radius,t,0.f,lantern);
        dl->PopClipRect();
        return;
    }
    ColdMist(dl,center,radius,t,presence);
    const float impactLight=S(t,3.50f,3.70f)*(1.f-S(t,3.74f,4.16f));
    Halo(dl,center,radius*2.3f,radius*1.7f,0x55BDFF,presence*impactLight*.115f);
    IceAtmosphere(dl,center,radius,t,presence);
    Trail(dl,center,radius,angle,turn,presence*(1.f-S(t,4.65f,5.65f)));
    std::vector<GlassFace> ice;ice.reserve(Crystal().size()*8);

    IceArm(ice,center,radius,angle,1.f,presence,t,5.f);
    for(int i=0;i<5;++i) {
        float growth=S(turn,i/5.f,i/5.f+.10f);
        if(i==0)growth=S(turn,.015f,.12f);
        if(turn>=1.f)growth=1.f;
        IceArm(ice,center,radius,i*tau/6.f,growth,presence,t,float(i));
    }
    CrystalDetails(ice,center,radius,t,presence);
    std::stable_sort(ice.begin(),ice.end(),[](const GlassFace& a,const GlassFace& b){return a.z<b.z;});
    std::vector<CrystalVertex> vertices;vertices.reserve(ice.size()*3);
    for(const auto& face:ice)for(const auto& vertex:face.p)vertices.push_back(vertex);

    material.Queue(dl,std::move(vertices),center,radius,t,0.f,lantern);
    CoreGlint(dl,center,radius,t,presence);
    IceFinish(dl,center,radius,t,presence);

    for(int i=0;i<30;++i) {
        const int arm=i/5;
        const float start=arm==5?.90f:arm/5.f;
        const float age=Win(turn,start,std::min(1.f,start+.15f));
        const float visible=S(age,0,.18f)*(1.f-S(age,.55f,1.f));
        const float a=arm*tau/6.f+(1.f-age)*.15f*std::sin(float(i)*2.7f);
        const float d=radius*(1.f+(1.f-age)*(.18f+(i%5)*.04f));
        const ImVec2 p{center.x+std::sin(a)*d,center.y-std::cos(a)*d};const float w=s*.0008f;
        dl->AddQuadFilled({p.x,p.y-w*2},{p.x+w,p.y},{p.x,p.y+w*2},{p.x-w,p.y},Color(0xCEEFF9,visible*presence*.65f));
    }
    dl->PopClipRect();
}
bool IntroSelfTest(char* out,int cap) {
    bool ok=!Crystal().empty();
    float frontArea=0,minDepth=1,maxDepth=0;
    for(const Face& f:Crystal()) {
        const V3 normal=Cross(f.p[1]-f.p[0],f.p[2]-f.p[0]);
        ok&=Dot(normal,normal)>1.e-14f;
        if(f.material==.5f) {
            frontArea+=std::abs(normal.z)*.5f;
            for(V3 p:f.p) {
                ok&=std::abs(p.z-FrontHeight(p))<.00001f;
                minDepth=std::min(minDepth,p.z);maxDepth=std::max(maxDepth,p.z);
            }
        }
        for(V3 p:f.p)ok&=std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
    }
    const float expectedArea=std::abs(Area(Inset(outer,kOuterBevel)))-std::abs(Area(Inset(hole,-kHoleBevel)));
    ok&=std::abs(frontArea-expectedArea)<.00001f&&maxDepth-minDepth>.025f;
    V3 fullTip[4];const V3 tipTriangle[]={{0,-1.10f,0},{-.02f,-1.05f,0},{.02f,-1.05f,0}};
    ok&=ClipGrowth(tipTriangle,fullTip,-1.10f)==3;
    for(float at:{.2f,1.8f,3.8f,5.4f}) {
        Intro test;test.Reset(1600,900);
        for(int i=0;i<int(at*120);++i)test.Update(1.f/120);
        test.Skip();float previous=test.HandoffK();
        for(int i=0;i<140;++i){test.Update(1.f/120);const float a=test.HandoffK();ok&=a>=previous-.00001f;previous=a;}
        ok&=test.finished;
    }

    for(ImVec2 size:{ImVec2(840,544),ImVec2(1050,680),ImVec2(1600,900)}) {
        Intro test;test.Reset(size.x,size.y);
        const float scale=std::min(size.x/1050.f,size.y/680.f);
        test.dockCenter={45.f*scale,79.f*scale};test.dockRadius=16.f*scale;
        float previousX=size.x*.5f,previousRadius=std::min(size.x,size.y)*.155f;
        for(int i=0;i<=240;++i) {
            test.t=Lerp(Intro::kDockStart,Intro::kDuration,i/240.f);
            const auto pose=test.Pose({0,0});
            ok&=std::isfinite(pose.center.x)&&std::isfinite(pose.center.y)&&pose.opacity==1.f;
            ok&=pose.center.x<=previousX+.0001f&&pose.radius<=previousRadius+.0001f;
            previousX=pose.center.x;previousRadius=pose.radius;
        }
        const auto end=test.Pose({0,0});
        test.finished=true;
        const auto settled=test.Pose({0,0});
        ok&=std::abs(end.center.x-test.dockCenter.x)<.0001f&&std::abs(end.center.y-test.dockCenter.y)<.0001f;
        ok&=std::abs(end.radius-test.dockRadius)<.0001f&&settled.radius==end.radius;
    }
    const float eps=.0001f;
    ok&=std::abs(S(eps,0,1)/eps)<.001f;
    ok&=std::abs((1.f-S(1.f-eps,0,1))/eps)<.01f;
    snprintf(out,cap,"intro %.2fs geometry=%zu faces/arm area=%.5f sweep=300deg C2-dock continuous-logo skip<=900ms %s",
             Intro::kDuration,Crystal().size(),frontArea,ok?"PASS":"FAIL");
    return ok;
}
}
