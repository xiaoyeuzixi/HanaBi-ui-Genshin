#include "ui/theme_ornaments.h"
#include <algorithm>
#include <cmath>
#include <vector>
namespace hb::ornament {
namespace { constexpr float pi=3.14159265359f; }
void StoneSeal(ImDrawList* dl,ImVec2 p,float r,ImU32 col){
    auto q=[&](float x,float y){return ImVec2{p.x+r*x,p.y+r*y};};
    ImVec2 outer[]={q(0,-1),q(.72f,-.28f),q(.72f,.28f),q(0,1),q(-.72f,.28f),q(-.72f,-.28f)};
    dl->AddConvexPolyFilled(outer,6,Fade(col,.06f));dl->AddPolyline(outer,6,Fade(col,.65f),ImDrawFlags_Closed,.8f);
    for(float side:{-1.f,1.f}){
        ImVec2 path[]={q(side*.42f,-.30f),q(0,-.72f),q(-side*.42f,-.30f),q(side*.17f,.28f),q(side*.39f,.06f),q(0,-.33f)};
        dl->AddPolyline(path,6,col,0,1.f);
    }
    ImVec2 end[]={q(-.28f,.48f),q(0,.76f),q(.28f,.48f)};dl->AddPolyline(end,3,Fade(col,.65f),0,.8f);
}
void Flower(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation) {

    for(int i=0;i<6;i++) {
        float a=rotation+i*pi/3.f;
        auto pt=[&](float x,float y){return ImVec2(p.x+(cosf(a)*x-sinf(a)*y)*r,p.y+(sinf(a)*x+cosf(a)*y)*r);};
        dl->AddBezierCubic(pt(.12f,0),pt(.65f,-.44f),pt(1.2f,-.23f),pt(1,0),col,1.f);
        dl->AddBezierCubic(pt(1,0),pt(1.2f,.23f),pt(.65f,.44f),pt(.12f,0),col,1.f);
    }
    dl->AddCircle(p,r*.12f,col,12,1.f);
}
void Sakura(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation) {
    for(int i=0;i<5;++i) {
        float a=rotation+i*pi*.4f-pi*.5f;
        auto pt=[&](float x,float y){return ImVec2(p.x+(cosf(a)*x-sinf(a)*y)*r,p.y+(sinf(a)*x+cosf(a)*y)*r);};
        dl->AddBezierCubic(pt(.13f,0),pt(.65f,-.49f),pt(1.06f,-.38f),pt(.93f,-.03f),col,.9f);
        dl->AddBezierCubic(pt(.93f,-.03f),pt(.78f,-.03f),pt(.80f,.05f),pt(1.0f,.11f),col,.9f);
        dl->AddBezierCubic(pt(1,.11f),pt(.96f,.50f),pt(.40f,.28f),pt(.13f,0),col,.9f);
    }
    dl->AddCircle(p,r*.09f,col,12,.8f);
}
void Firework(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation) {
    for(int i=0;i<12;++i){const float a=rotation+i*pi/6.f;const float length=i%2?.74f:1.f;
        auto pt=[&](float x,float y){return ImVec2(p.x+(cosf(a)*x-sinf(a)*y)*r,p.y+(sinf(a)*x+cosf(a)*y)*r);};
        dl->AddBezierCubic(pt(.23f,0),pt(.45f,-.03f),pt(length*.83f,-.08f),pt(length,-.15f),col,1.f);
        dl->AddCircleFilled(pt(length,-.15f),std::max(.7f,r*.027f),col,8);
    }
    dl->AddCircleFilled(p,std::max(.8f,r*.07f),col,12);
}
void Goldfish(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time) {
    const float sway=sinf(time*.7f)*.05f;
    auto q=[&](float x,float y){return ImVec2(p.x+(x*cosf(sway)-y*sinf(sway))*r,p.y+(x*sinf(sway)+y*cosf(sway))*r);};
    dl->AddBezierCubic(q(-.65f,0),q(-.6f,-.58f),q(.15f,-.62f),q(.44f,-.12f),col,1.1f);
    dl->AddBezierCubic(q(.44f,-.12f),q(.10f,.50f),q(-.5f,.55f),q(-.65f,0),col,1.1f);
    dl->AddBezierCubic(q(.44f,-.12f),q(.6f,-.50f),q(1.15f,-.7f),q(.96f,-.18f),col,1.f);
    dl->AddBezierCubic(q(.96f,-.18f),q(1.26f,.38f),q(.65f,.51f),q(.44f,-.12f),col,1.f);
    dl->AddBezierCubic(q(-.2f,-.4f),q(.02f,-.92f),q(.38f,-.68f),q(.18f,-.35f),col,.8f);
    dl->AddCircleFilled(q(-.42f,-.10f),r*.044f,col,10);
    dl->AddBezierCubic(q(-.07f,.0f),q(.18f,.0f),q(.15f,.4f),q(-.07f,.26f),col,.8f);
}
void Sprig(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time){
    const float angle=sinf(time*.5f)*.035f;
    auto q=[&](float x,float y){return ImVec2(p.x+(x*cosf(angle)-y*sinf(angle))*r,p.y+(x*sinf(angle)+y*cosf(angle))*r);};
    dl->AddBezierCubic(q(0,.95f),q(.12f,.25f),q(-.10f,-.4f),q(0,-1.05f),col,1.f);
    for(int side:{-1,1})for(int i=0;i<2;++i){float y=.38f-i*.62f;
        dl->AddBezierCubic(q(0,y),q(side*.15f,y-.52f),q(side*.83f,y-.75f),q(side*.77f,y-.56f),col,.85f);
        dl->AddBezierCubic(q(side*.77f,y-.56f),q(side*.90f,y-.06f),q(side*.18f,y+.06f),q(0,y),col,.85f);
        dl->AddBezierCubic(q(0,y),q(side*.22f,y-.1f),q(side*.52f,y-.25f),q(side*.77f,y-.56f),Fade(col,.6f),.65f);
    }
    dl->AddBezierCubic(q(0,-.62f),q(-.39f,-.92f),q(-.15f,-1.25f),q(0,-1.36f),col,.85f);
    dl->AddBezierCubic(q(0,-1.36f),q(.15f,-1.25f),q(.39f,-.92f),q(0,-.62f),col,.85f);
}
void Polyhedron(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time){
    const ImVec2 top{p.x,p.y-r},bottom{p.x,p.y+r};ImVec2 ring[3];
    for(int j=0;j<3;++j){float a=time*.23f+j*pi*2/3;ring[j]={p.x+cosf(a)*r*.84f,p.y+sinf(a)*r*.22f};}
    for(int j=0;j<3;++j){dl->AddLine(top,ring[j],col,.9f);dl->AddLine(bottom,ring[j],col,.9f);dl->AddLine(ring[j],ring[(j+1)%3],Fade(col,.55f),.75f);}
    dl->AddTriangleFilled(top,ring[0],ring[1],Fade(col,.07f));
}
void ArchiveBook(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time){
    const float wave=sinf(time*.55f)*.035f;
    auto q=[&](float x,float y){return ImVec2(p.x+x*r,p.y+(y+wave*x*x)*r);};
    for(int side:{-1,1}){
        dl->AddBezierCubic(q(0,-.35f),q(side*.3f,-.65f),q(side*.72f,-.63f),q(side*1.04f,-.45f),col,1.f);
        dl->AddLine(q(side*1.04f,-.45f),q(side*1.04f,.60f),col,1.f);
        dl->AddBezierCubic(q(side*1.04f,.60f),q(side*.65f,.40f),q(side*.35f,.43f),q(0,.70f),col,1.f);
        for(int j=0;j<3;++j)dl->AddBezierCubic(q(side*.17f,-.10f+j*.20f),q(side*.4f,-.25f+j*.20f),q(side*.6f,-.23f+j*.20f),q(side*.86f,-.10f+j*.20f),Fade(col,.4f),.65f);
    }
    dl->AddLine(q(0,-.35f),q(0,.70f),col,1.f);
    Polyhedron(dl,q(0,-1.25f),r*.35f,Fade(col,.85f),time);
}
void Droplet(ImDrawList* dl,ImVec2 p,float r,ImU32 col){
    auto q=[&](float x,float y){return ImVec2(p.x+x*r,p.y+y*r);};
    dl->AddBezierCubic(q(0,-1),q(-.15f,-.43f),q(-.85f,-.08f),q(-.62f,.48f),col,.9f);
    dl->AddBezierCubic(q(-.62f,.48f),q(-.33f,1.08f),q(.33f,1.08f),q(.62f,.48f),col,.9f);
    dl->AddBezierCubic(q(.62f,.48f),q(.85f,-.08f),q(.15f,-.43f),q(0,-1),col,.9f);
    dl->AddBezierCubic(q(-.34f,.20f),q(-.42f,.47f),q(-.20f,.66f),q(-.06f,.61f),Fade(col,.7f),.8f);
}
void TopHat(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time){
    const float a=sinf(time*.4f)*.018f;
    auto q=[&](float x,float y){return ImVec2(p.x+(x*cosf(a)-y*sinf(a))*r,p.y+(x*sinf(a)+y*cosf(a))*r);};
    dl->AddBezierCubic(q(-.60f,-.68f),q(-.31f,-.82f),q(.30f,-.82f),q(.60f,-.68f),col,1.f);
    dl->AddLine(q(-.60f,-.68f),q(-.46f,.27f),col,1.f);dl->AddLine(q(.60f,-.68f),q(.46f,.27f),col,1.f);
    dl->AddBezierCubic(q(-.46f,.27f),q(-1.55f,-.06f),q(-1.22f,.65f),q(0,.60f),col,1.f);
    dl->AddBezierCubic(q(0,.60f),q(1.22f,.65f),q(1.55f,-.06f),q(.46f,.27f),col,1.f);
    dl->AddBezierCubic(q(-.50f,-.02f),q(-.18f,.12f),q(.18f,.12f),q(.50f,-.02f),col,1.3f);
    dl->AddBezierCubic(q(.38f,-.26f),q(.34f,-1.05f),q(.61f,-1.45f),q(.99f,-1.55f),col,.8f);
    dl->AddBezierCubic(q(.99f,-1.55f),q(1.21f,-1.04f),q(.85f,-.62f),q(.38f,-.26f),col,.8f);
    dl->AddBezierCubic(q(.38f,-.26f),q(.65f,-.62f),q(.74f,-1.12f),q(.99f,-1.55f),Fade(col,.55f),.65f);
    dl->AddBezierCubic(q(-.3f,.55f),q(-.55f,.82f),q(-.14f,.88f),q(-.40f,1.18f),col,.8f);
    dl->AddBezierCubic(q(.3f,.55f),q(.55f,.82f),q(.14f,.88f),q(.40f,1.18f),col,.8f);
    Droplet(dl,q(0,.11f),r*.13f,col);
}
void ChineseKnot(ImDrawList* dl,ImVec2 top,float scale,float time) {
    const float sway=sinf(time*.75f)*.018f;
    auto p=[&](float x,float y){return ImVec2(top.x+(x*cosf(sway)-(y-13)*sinf(sway))*scale,top.y+(13+x*sinf(sway)+(y-13)*cosf(sway))*scale);};
    const ImU32 shade=IM_COL32(65,27,49,245),silk=IM_COL32(197,67,90,255),shine=IM_COL32(250,149,144,230),gold=IM_COL32(229,195,133,240);
    auto rope=[&](const std::vector<ImVec2>& points,float width=2.3f){
        dl->AddPolyline(points.data(),int(points.size()),shade,0,(width+1.25f)*scale);
        dl->AddPolyline(points.data(),int(points.size()),silk,0,width*scale);
        std::vector<ImVec2> highlight=points;for(auto& v:highlight){v.x-=.35f*scale;v.y-=.32f*scale;}
        dl->AddPolyline(highlight.data(),int(highlight.size()),shine,0,.55f*scale);
    };
    dl->AddBezierCubic(p(-1,0),p(-2,6),p(-2,10),p(0,14),gold,1.15f*scale);
    dl->AddBezierCubic(p(1,0),p(2,6),p(2,10),p(0,14),gold,.7f*scale);
    dl->AddCircleFilled(p(0,15),2.2f*scale,gold,12);
    auto q=[&](float x,float y){return p((x-y)*.70710678f,31+(x+y)*.70710678f);};
    for(int axis=0;axis<2;++axis){
        auto v=[&](float x,float y){return axis?q(y,x):q(x,y);};
        std::vector<ImVec2> strand;
        auto line=[&](float x,float y){strand.push_back(v(x,y));};
        auto curve=[&](ImVec2 a,ImVec2 b,ImVec2 c){auto start=strand.back();for(int j=1;j<=10;++j){float t=j/10.f,u=1-t;strand.push_back({u*u*u*start.x+3*u*u*t*a.x+3*u*t*t*b.x+t*t*t*c.x,u*u*u*start.y+3*u*u*t*a.y+3*u*t*t*b.y+t*t*t*c.y});}};
        line(-7,-6);line(7,-6);curve(v(13,-6),v(13,0),v(7,0));line(-7,0);curve(v(-13,0),v(-13,6),v(-7,6));line(7,6);
        rope(strand);
    }
    for(int row=0;row<3;++row)for(int col=0;col<3;++col){
        const float x=-6.f+col*6,y=-6.f+row*6;
        const auto a=(row+col)%2?q(x-2,y):q(x,y-2),b=(row+col)%2?q(x+2,y):q(x,y+2);
        rope({a,b},2.2f);
    }
    dl->AddCircleFilled(p(0,47),2.4f*scale,silk,12);
    dl->AddRectFilled(p(-2.5f,49),p(2.5f,53),gold,1.f*scale);
    for(int i=0;i<9;++i){
        const float x=(i-4)*.85f,tip=72.f-std::abs(i-4)*.65f;
        const float drift=sinf(time*.9f+i*.12f)*.5f;
        dl->AddBezierCubic(p(x*.50f,53),p(x,59),p(x+drift,66),p(x+drift,tip),i%3?silk:shine,.85f*scale);
    }
    dl->AddLine(p(-3.1f,55),p(3.1f,55),gold,.75f*scale);
}
void Cryo(ImDrawList* dl,ImVec2 p,float r,ImU32 col) {
    for(int i=0;i<6;i++) {
        float a=i*pi/3.f-pi*.5f,dx=cosf(a),dy=sinf(a);
        dl->AddLine(p,ImVec2(p.x+dx*r,p.y+dy*r),col,1.2f);
        for(int side:{-1,1}) {
            float b=a+side*pi*.27f;
            dl->AddLine(ImVec2(p.x+dx*r*.55f,p.y+dy*r*.55f),ImVec2(p.x+dx*r*.55f-cosf(b)*r*.3f,p.y+dy*r*.55f-sinf(b)*r*.3f),col,1.f);
        }
    }
}
}
