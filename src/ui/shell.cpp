#include "ui/shell.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "core/easing.h"
#include "ui/paint.h"
#include "ui/widgets.h"

namespace hb {
int VtxBegin(ImDrawList* dl) { return dl->VtxBuffer.Size; }
void VtxApply(ImDrawList* dl,int start,ImVec2 pivot,float scale,ImVec2 shift,float alpha) {
    alpha=std::clamp(alpha,0.f,1.f);
    for(int i=start;i<dl->VtxBuffer.Size;i++) {
        auto& vertex=dl->VtxBuffer[i];
        vertex.pos.x=pivot.x+(vertex.pos.x-pivot.x)*scale+shift.x;
        vertex.pos.y=pivot.y+(vertex.pos.y-pivot.y)*scale+shift.y;
        const int a=int(((vertex.col>>IM_COL32_A_SHIFT)&255)*alpha+.5f);
        vertex.col=(vertex.col&~IM_COL32_A_MASK)|(ImU32(a)<<IM_COL32_A_SHIFT);
    }
}
namespace {
constexpr float pi=3.14159265359f;
const char* const navNames[]={"自瞄","视觉","调节","杂项","主题"};
const char* const navEn[]={"AIM ASSIST","VISUALS","FINE TUNING","WORKSPACE","APPEARANCE"};
const char* const descriptions[]={"让每一次操作，都恰到好处。","清晰的信息，安静的视野。","找到属于你的操作节奏。","收好习惯，随时回到熟悉的状态。","给工作空间，一点自己的气质。"};
const int icons[]={paint::ICON_AIM,paint::ICON_EYE,paint::ICON_TUNE,paint::ICON_GRID,paint::ICON_PALETTE};
float Clamp(float v){return std::clamp(v,0.f,1.f);}
void Flower(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation=0.f) {

    for(int i=0;i<6;i++) {
        float a=rotation+i*pi/3.f;
        auto pt=[&](float x,float y){return ImVec2(p.x+(cosf(a)*x-sinf(a)*y)*r,p.y+(sinf(a)*x+cosf(a)*y)*r);};
        dl->AddBezierCubic(pt(.12f,0),pt(.65f,-.44f),pt(1.2f,-.23f),pt(1,0),col,1.f);
        dl->AddBezierCubic(pt(1,0),pt(1.2f,.23f),pt(.65f,.44f),pt(.12f,0),col,1.f);
    }
    dl->AddCircle(p,r*.12f,col,12,1.f);
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
const ThemeTokens& Tokens(int theme) {
    static ThemeTokens tok[3]; static bool init=false;
    if(!init){init=true;for(int i=0;i<3;i++){const auto& s=ThemeAt(i).skin;tok[i]={ThemeAt(i).nameCn,ThemeAt(i).nameEn,Col(s.win0),Col(s.winStroke),Col(s.sidebar0),Col(s.card0),Col(s.cardStroke),Col(s.text),Col(s.text2),Col(s.text3),Col(s.accent),Col(s.accentSoft),Col(s.good),s.radius};}}
    return tok[std::clamp(theme,0,2)];
}
void Shell::ApplyFonts(ImFont* ui,ImFont* display,ImFont* caps) {
    cx.fUi=cx.fSm=cx.fXs=cx.fLg=cx.fXl=ui;cx.fDisp=display;cx.fCaps=caps?caps:display;
}
void Shell::Layout(float sw,float sh,float uiScale) {
    screenW=sw;screenH=sh;
    scale=std::max(.1f,std::min({uiScale,sw/980.f,sh/650.f}));
    W=sw/scale;H=sh/scale;originX=originY=0;centerX=sw*.5f;centerY=sh*.5f;
    cx.sc=scale;cx.pool=pool;cx.poolN=kMaxWid;
}
void Shell::ReplayEnter() {
    entering=true;enterT=0;ready=true;
    for(int i=0;i<5;i++)pageWeight[i].Snap(m.navSel==i?1.f:0.f);
    navPosition.Snap(float(m.navSel));ganyuMix.Snap(theme==0?1.f:0.f);wallpaperMix.Snap(m.wallpaper);
    for(auto& w:pool)w=WState{};
}
void Shell::SetTheme(int tint) {
    tint=std::clamp(tint,0,2);if(theme==tint)return;
    float e=themeT*.42f;
    themeFrom=BuildSkin(themeFrom,ThemeAt(theme).skin,Ease(E_PAGE,Clamp(e/.26f)),Ease(E_PAGE,Clamp((e-.08f)/.26f)),Ease(E_PAGE,Clamp((e-.14f)/.28f)));
    oldTheme=theme;theme=tint;themeT=0;m.themeSel=tint;
}
void Shell::Begin(float dt) {
    cx.dt=std::clamp(dt,0.f,.05f);t+=cx.dt;uptime+=cx.dt;cx.t=t;
    m.fontPercent=std::clamp(m.fontPercent,100.f,125.f);
    cx.textScale=m.fontPercent*.01f;
    enterT+=cx.dt;entering=enterT<.85f;announce=Clamp((enterT-.5f)/.5f);
    themeT=std::min(1.f,themeT+cx.dt/.42f);
    for(int i=0;i<5;i++) {pageWeight[i].Target(m.navSel==i?1.f:0.f);if(m.animations)pageWeight[i].Update(cx.dt,springs::S_CRIT);else pageWeight[i].Snap(m.navSel==i?1.f:0.f);}
    navPosition.Target(float(m.navSel));navPosition.Update(cx.dt,springs::S_UI);
    ganyuMix.Target(theme==0?1.f:0.f);ganyuMix.Update(cx.dt,springs::S_CRIT);sceneMix=Clamp(ganyuMix.V(1));
    wallpaperMix.Target(m.wallpaper);wallpaperMix.Update(cx.dt,springs::S_CRIT);
    toastTime=std::max(0.f,toastTime-cx.dt);
}
void Shell::Notify(const char* message){snprintf(toast,sizeof(toast),"%s",message);toastTime=2.8f;}
void Shell::Label(float x,float y,const char* text,float size,int tone){
    paint::Text(cx.dl,cx.fUi,Fs(size),L(x,y,x,y).a,cx.C(tone==0?cx.sk.text:(tone==1?cx.sk.text2:cx.sk.text3)),text);
}
bool Shell::Button(int id,Rc r,const char* text,bool selected) {
    auto& st=cx.St(id);bool hit=cx.Claim(id,r);bool active=cx.IsActive(id)&&cx.down;
    st.hov.Target(cx.Hover(r)?1.f:0.f);st.hov.Update(cx.dt);st.prs.Target(active?1.f:0.f);st.prs.Update(cx.dt,springs::S_SNAP);
    st.act.Target(selected?1.f:0.f);st.act.Update(cx.dt);
    float hv=st.hov.V(),sel=Clamp(st.act.V()),pv=st.prs.V();
    Rc box=r.In(Px(.7f)*pv);
    paint::Fill(cx.dl,box.a,box.b,Px(9),paint::MixI(cx.C(cx.sk.field0),cx.C(cx.sk.accentSoft),Clamp(sel+hv*.25f)));
    paint::Stroke(cx.dl,box.a,box.b,Px(9),paint::MixI(cx.C(cx.sk.fieldStroke),Fade(cx.C(cx.sk.accent),.65f),Clamp(sel+hv*.25f)),1);
    paint::TextC(cx.dl,cx.fUi,Fs(12.5f),box.c(),cx.C(selected?cx.sk.accent:cx.sk.text2),text);
    if(cx.Hover(r))ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    return hit;
}
void Shell::Panel(int id,float x,float y,float w,float h,const char* title,const char* sub) {
    Card(cx,id,L(x,y,x+w,y+h),false);
    if(sceneMix>.001f) {

        auto* dl=cx.dl;
        const float end=x+std::min(w-38.f,112.f);
        dl->AddBezierCubic(L(x+20,y+1,0,0).a,L(x+48,y+1,0,0).a,
            L(end-16,y+4,0,0).a,L(end,y+1,0,0).a,
            IM_COL32(162,219,236,int(66*sceneMix)),Px(.8f));
        dl->AddCircleFilled(L(x+w-14,y+14,0,0).a,Px(1.3f),IM_COL32(209,178,126,int(100*sceneMix)),12);
    }
    Label(x+20,y+18,title,15);
    if(sub)Label(x+20,y+41,sub,11.5f,2);
}
void Shell::RowToggle(int id,float x,float y,float w,const char* label,bool& value,const char* detail) {
    Label(x,y+(detail?0:3),label,13,1);if(detail)Label(x,y+21,detail,10.5f,2);
    Toggle(cx,id,L(x+w-38,y+3,x+w,y+24),value);
}
void Shell::Range(int id,float x,float y,float w,const char* label,float& value,float lo,float hi,const char* unit,int precision) {
    Slider(cx,id,L(x,y,x+w,y+20),L(x,y+30,x+w,y+34),value,lo,hi,label,unit,precision);
}
void Shell::DrawChrome() {
    auto* dl=cx.dl;const float g=sceneMix;

    paint::FillGrad(dl,L(0,0,W,H).a,L(0,0,W,H).b,0,cx.C(cx.sk.win0),cx.C(cx.sk.win1));
    if(g>.001f) {
        ImU32 dark=IM_COL32(6,16,32,int(64*g)),clear=IM_COL32(6,16,32,0);
        dl->AddRectFilledMultiColor(L(176,0,176,0).a,L(W,0,W,H).b,dark,clear,clear,dark);
        Flower(dl,L(W-45,H-74,W-45,H-74).a,Px(61),IM_COL32(176,217,235,int(28*g)),.1f);
    }

    paint::TextC(dl,cx.fCaps,Fs(9),L(W*.5f,20,W*.5f,20).a,cx.C(cx.sk.text3),theme==0?"S I N U S   U N I C O R N I S":"H A N A B I   /   S T U D I O");
    for(int i=0;i<3;i++) {
        Rc r=L(W-132+i*44,0,W-88+i*44,38);
        auto& s=cx.St(1+i);s.hov.Target(cx.Hover(r)?1.f:0.f);s.hov.Update(cx.dt);
        ImVec2 p=r.c();ImU32 col=cx.C(cx.sk.text2);float z=Px(4.5f);
        const float hover=Clamp(s.hov.V());
        if(g>.001f){
            const float half=Px(13),cut=Px(5);
            ImVec2 points[]={{p.x-half+cut,p.y-half},{p.x+half-cut,p.y-half},{p.x+half,p.y-half+cut},{p.x+half,p.y+half-cut},{p.x+half-cut,p.y+half},{p.x-half+cut,p.y+half},{p.x-half,p.y+half-cut},{p.x-half,p.y-half+cut}};
            ImU32 tint=paint::MixI(IM_COL32(151,219,240,255),i==2?IM_COL32(223,140,159,255):IM_COL32(220,238,249,255),hover);
            dl->AddConvexPolyFilled(points,8,IM_COL32(12,37,57,int((75+hover*55)*g)));
            dl->AddPolyline(points,8,Fade(tint,(.3f+hover*.5f)*g),ImDrawFlags_Closed,Px(.8f));
            dl->AddTriangleFilled(points[0],points[1],ImVec2(p.x,p.y-2),Fade(tint,.07f*g));
            dl->AddLine(points[0],points[1],IM_COL32(218,191,137,int((80+hover*90)*g)),Px(.8f));
            col=Fade(tint,g);
            if(i==0){dl->AddLine(ImVec2(p.x-z,p.y),ImVec2(p.x+z,p.y),col,Px(1.1f));dl->AddQuadFilled(ImVec2(p.x+z+1,p.y-2),ImVec2(p.x+z+3,p.y),ImVec2(p.x+z+1,p.y+2),ImVec2(p.x+z-1,p.y),col);}
            if(i==1){ImVec2 diamond[]={{p.x,p.y-z-1},{p.x+z+1,p.y},{p.x,p.y+z+1},{p.x-z-1,p.y}};dl->AddPolyline(diamond,4,col,ImDrawFlags_Closed,Px(1.1f));dl->AddCircleFilled(p,Px(1),col,12);}
            if(i==2){dl->AddLine(ImVec2(p.x-z,p.y-z),ImVec2(p.x+z,p.y+z),col,Px(1.2f));dl->AddLine(ImVec2(p.x-z,p.y+z),ImVec2(p.x+z,p.y-z),col,Px(1.2f));dl->AddCircle(p,Px(1.7f),col,12,Px(.7f));}
        }
        if(g<.999f){
            paint::Fill(dl,r.In(Px(4)).a,r.In(Px(4)).b,Px(6),Fade(i==2?IM_COL32(219,76,94,210):cx.C(cx.sk.fieldStroke),hover*(1-g)));
            col=Fade(cx.C(cx.sk.text2),1-g);
            if(i==0)dl->AddLine(ImVec2(p.x-z,p.y),ImVec2(p.x+z,p.y),col,Px(1.2f));
            if(i==1)dl->AddRect(ImVec2(p.x-z,p.y-z),ImVec2(p.x+z,p.y+z),col,Px(1),0,Px(1.1f));
            if(i==2){dl->AddLine(ImVec2(p.x-z,p.y-z),ImVec2(p.x+z,p.y+z),col,Px(1.2f));dl->AddLine(ImVec2(p.x-z,p.y+z),ImVec2(p.x+z,p.y-z),col,Px(1.2f));}
        }
        if(cx.Claim(1+i,r))windowAction=i+1;
        if(cx.Hover(r))ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    dl->AddLine(L(176,H-30,176,H-30).a,L(W,H-30,W,H-30).a,cx.C(cx.sk.divider));
    Label(204,H-22,"HANABI  /  PERSONAL WORKSPACE",9,2);
    Label(W-155,H-22,"本地工作空间",10,2);

    for(int i=0;i<3;i++)dl->AddLine(L(W-5-i*4,H-5,W-5-i*4,H-5).a,L(W-5,H-5-i*4,W-5,H-5-i*4).a,Fade(cx.C(cx.sk.text3),.5f));
}
void Shell::DrawSidebar() {
    auto* dl=cx.dl;float g=sceneMix;
    paint::FillGrad(dl,L(0,0,176,H).a,L(0,0,176,H).b,0,cx.C(cx.sk.sidebar0),cx.C(cx.sk.sidebar1));
    dl->AddLine(L(176,46,176,46).a,L(176,H-20,176,H-20).a,cx.C(cx.sk.divider));
    Rc emblem=L(24,58,66,100);
    if(g<.999f)paint::Fill(dl,emblem.a,emblem.b,Px(13),Fade(cx.C(cx.sk.accentSoft),1-g));

    if(g<.999f)paint::DrawIcon(dl,paint::ICON_GRID,emblem.c(),Px(18),Fade(cx.C(cx.sk.accent),1-g),Px(1.5f));
    Label(78,59,"HanaBi",23);Label(79,87,theme==0?"循霜而行":"你的工作空间",10.5f,2);
    Label(25,125,"工作空间",10.5f,2);
    Rc mark=L(14,154+navPosition.V()*54,162,200+navPosition.V()*54);
    paint::Fill(dl,mark.a,mark.b,Px(11),cx.C(cx.sk.accentSoft));
    if(g>.001f) {
        dl->AddLine(L(15,mark.a.y/scale+12,15,mark.a.y/scale+12).a,L(15,mark.a.y/scale+32,15,mark.a.y/scale+32).a,Fade(cx.C(cx.sk.accent),g),Px(2));
        Flower(dl,L(140,mark.c().y/scale,140,mark.c().y/scale).a,Px(9),Fade(cx.C(cx.sk.accent),.5f*g));
    }
    for(int i=0;i<5;i++) {
        Rc r=L(14,154.f+float(i)*54,162,200.f+float(i)*54);auto& s=cx.St(10+i);
        bool over=cx.Hover(r);s.hov.Target(over?1.f:0.f);s.hov.Update(cx.dt);
        paint::Fill(dl,r.a,r.b,Px(11),Fade(cx.C(cx.sk.field0),s.hov.V()*.8f));
        ImU32 col=cx.C(m.navSel==i?cx.sk.accent:cx.sk.text2);
        paint::DrawIcon(dl,icons[i],L(36,177.f+float(i)*54,36,177.f+float(i)*54).a,Px(17),col,Px(1.4f));
        Label(57.f,166.f+float(i)*54.f,navNames[i],14,m.navSel==i?0:1);
        if(cx.Claim(10+i,r)){m.navSel=i;recording=-1;for(auto& rec:m.rec)rec=false;}
        if(over)ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if(g>.01f) {
        Flower(dl,L(82,H-125,82,H-125).a,Px(29),IM_COL32(188,218,232,int(90*g)),-pi*.5f);
        Label(43,H-79,"璃月 · 海灯夜",12,1);
        paint::TextC(dl,cx.fCaps,Fs(8.5f),L(88,H-49,88,H-49).a,Fade(cx.C(cx.sk.text3),g),"A QUIET MOMENT");
    } else {
        Label(26,H-91,"HanaBi Studio",13,1);Label(26,H-65,"简单，自在，专注。",10.5f,2);
    }
}
void Shell::DrawKeys(int id,float x,float y,float w,float h) {
    Panel(id,x,y,w,h,"快捷操作","点击键位，按下新的快捷键");
    const char* names[]={"启用自瞄","触发操作","临时切换"};
    float space=(h-76)/3.f;
    for(int i=0;i<3;i++) {
        float yy=y+67+i*space;Label(x+20,yy+5,names[i],12.5f,1);
        const char* key=recording==i?"按键…":ImGui::GetKeyName((ImGuiKey)m.hk[i]);
        if(!key||!*key)key="未设置";
        if(Button(id+1+i,L(x+w-120,yy,x+w-20,yy+28),key,recording==i)) {
            recording=recording==i?-1:i;for(int j=0;j<3;j++)m.rec[j]=recording==j;
        }
    }
}
void Shell::DrawAim(float x,float y,float w,float h) {
    float rw=std::clamp(w*.37f,270.f,360.f),lw=w-rw-18,xx=x+lw+18;
    float top=282,low=h-top-14;
    Panel(40,x,y,lw,top,"自瞄设置","目标与触发方式");
    Toggle(cx,41,L(x+lw-60,y+20,x+lw-20,y+42),m.aimMaster);
    const char* bones[]={"头部","胸部","最近"};const char* trigger[]={"按住","切换","始终"};
    Label(x+20,y+80,"目标部位",12,1);Segment(cx,42,L(x+126,y+72,x+lw-20,y+104),m.boneSel,bones,3);
    Label(x+20,y+124,"触发模式",12,1);Segment(cx,43,L(x+126,y+116,x+lw-20,y+148),m.trigSel,trigger,3);
    Range(44,x+20,y+163,lw-40,"视野范围",m.fov,1,60,"°",1);
    Range(45,x+20,y+219,lw-40,"平滑程度",m.smooth,.1f,3.f,"",2);
    Panel(46,x,y+top+14,lw,low,"弹道与预测");
    float by=y+top+14;
    float col=(lw-52)*.5f;
    RowToggle(47,x+20,by+55,col,"移动预测",m.predOn);
    RowToggle(48,x+32+col,by+55,col,"下坠补偿",m.fallOn);
    RowToggle(49,x+20,by+91,col,"静默模式",m.silentOn);
    Range(50,x+20,by+126,lw-40,"弹速",m.bulletSpeed,100,1600,"m/s");
    float kh=192,hero=h-kh-16;
    Panel(51,xx,y,rw,hero,"目标部位","点击模型，选择头部、胸部或最近部位");
    DrawMannequin(xx+16,y+65,rw-32,hero-80,true);
    DrawKeys(55,xx,y+h-kh,rw,kh);
}
void Shell::DrawMannequin(float x,float y,float w,float h,bool selectTarget) {
    auto* dl=cx.dl;Rc clip=L(x,y,x+w,y+h);dl->PushClipRect(clip.a,clip.b,true);
    const float unit=std::min(h/2.02f,w/1.58f);
    const float center=x+w*(selectTarget?.43f:.5f),top=y+3;
    struct V3 {float x,y,z;};
    auto project=[&](V3 v){return L(center+v.x*unit,top+v.y*unit,0,0).a;};
    ImVec2 floor=project({0,1.82f,.03f});
    dl->AddEllipseFilled(floor,ImVec2(Px(unit*.38f),Px(unit*.085f)),Fade(cx.C(cx.sk.accent),.055f));
    dl->AddEllipse(floor,ImVec2(Px(unit*.38f),Px(unit*.085f)),Fade(cx.C(cx.sk.accent),.24f),0,64,Px(.7f));
    const float figureH=unit*1.95f,figureW=figureH*320.f/720.f;
    Rc artwork=L(center-figureW*.5f,top,center+figureW*.5f,top+figureH);
    if(targetFigure.Valid())dl->AddImage((ImTextureID)(intptr_t)targetFigure.srv,artwork.a,artwork.b);
    if(selectTarget) {
        const char* parts[]={"头部","胸部","最近部位"};
        for(int i=0;i<3;i++){
            float yy=top+unit*(i==0?.15f:(i==1?.48f:1.13f));
            Rc hit=L(center-unit*.3f,yy-unit*(i==0?.15f:.22f),center+unit*.3f,yy+unit*(i==0?.15f:.22f));
            if(cx.Claim(60+i,hit))m.boneSel=i;
            if(cx.Hover(hit))ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImVec2 p=project({i==0?0.f:.04f,i==0?.15f:(i==1?.47f:1.12f),.14f});
            float end=x+w-11;
            ImU32 ink=Fade(cx.C(m.boneSel==i?cx.sk.accent:cx.sk.text2),m.boneSel==i?1.f:.6f);
            dl->AddCircle(p,Px(m.boneSel==i?5.f:2.5f),ink,24,Px(1));
            float elbow=center+unit*.34f;
            dl->AddLine(ImVec2(p.x+Px(6),p.y),L(elbow,yy,0,0).a,ink,Px(.7f));
            dl->AddLine(L(elbow,yy,0,0).a,L(end,yy,0,0).a,Fade(ink,.5f),Px(.7f));
            paint::TextR(dl,cx.fUi,Fs(10.5f),L(end,yy-10,0,0).a,ink,parts[i]);
            Rc labelHit=L(elbow,yy-22,end,yy+10);
            if(cx.Claim(60+i,labelHit))m.boneSel=i;
        }
        Label(x+5,y+h-15,"BODY TARGET",8.5f,2);
    } else if(m.visualOn){
        ImVec2 a=project({-.36f,.045f,0}),b=project({.36f,1.88f,0});
        ImU32 ink=cx.C(cx.sk.accent);float corner=Px(12);
        if(m.boxes)for(int side:{0,1})for(int end:{0,1}){
            ImVec2 p(side?b.x:a.x,end?b.y:a.y);
            dl->AddLine(p,ImVec2(p.x+(side?-corner:corner),p.y),ink,Px(m.lineWidth));
            dl->AddLine(p,ImVec2(p.x,p.y+(end?-corner:corner)),ink,Px(m.lineWidth));
        }
        if(m.bones){
            V3 joints[]={{0,.18f,.13f},{0,.38f,.14f},{0,.7f,.13f},{0,.88f,.13f}};
            for(int i=0;i<3;i++)dl->AddLine(project(joints[i]),project(joints[i+1]),Fade(ink,.75f),Px(m.lineWidth*.7f));
            for(int side:{-1,1}){
                V3 shoulder{side*.15f,.39f,.07f},elbow{side*.21f,.63f,.07f},hand{side*.27f,.91f,.07f};
                V3 hip{side*.085f,.91f,.12f},knee{side*.08f,1.31f,.08f},ankle{side*.08f,1.66f,.08f};
                for(auto joint:{shoulder,elbow,hand,hip,knee,ankle})dl->AddCircleFilled(project(joint),Px(1.5f),ink,12);
                dl->AddLine(project(shoulder),project(elbow),Fade(ink,.75f),Px(.8f));dl->AddLine(project(elbow),project(hand),Fade(ink,.75f),Px(.8f));
                dl->AddLine(project(hip),project(knee),Fade(ink,.75f),Px(.8f));dl->AddLine(project(knee),project(ankle),Fade(ink,.75f),Px(.8f));
            }
        }
        if(m.health){Rc rail(a.x-Px(8),a.y,a.x-Px(5),b.y);paint::Fill(dl,rail.a,rail.b,Px(1),cx.C(cx.sk.fieldStroke));rail.a.y+=rail.h()*.22f;paint::Fill(dl,rail.a,rail.b,Px(1),cx.C(cx.sk.good));}
        if(m.names)paint::TextC(dl,cx.fUi,Fs(10.5f),L(x+w*.5f+unit*.33f,y+7,0,0).a,cx.C(cx.sk.text2),"训练目标");
        if(m.distance)paint::TextR(dl,cx.fUi,Fs(10),ImVec2(clip.b.x-Px(5),clip.b.y-Px(10)),cx.C(cx.sk.text2),"48 m");
        if(m.tracers)dl->AddLine(ImVec2(clip.c().x,clip.b.y),project({0,1.86f,.07f}),Fade(ink,.65f),Px(1));
    }
    dl->PopClipRect();
}
void Shell::DrawPreview(float x,float y,float w,float h,bool tuning) {
    auto* dl=cx.dl;Rc box=L(x,y,x+w,y+h);dl->PushClipRect(box.a,box.b,true);
    paint::FillGrad(dl,box.a,box.b,Px(10),cx.C(cx.sk.field0),cx.C(cx.sk.field1));
    for(int i=1;i<7;i++){float yy=y+h*i/7.f;dl->AddLine(L(x,yy,x,yy).a,L(x+w,yy,x+w,yy).a,Fade(cx.C(cx.sk.divider),.5f));}
    for(int i=1;i<7;i++){float xx=x+w*i/7.f;dl->AddLine(L(xx,y,xx,y).a,L(xx,y+h,xx,y+h).a,Fade(cx.C(cx.sk.divider),.5f));}
    if(!tuning){dl->PopClipRect();DrawMannequin(x,y,w,h,false);return;}

    float drift=m.animations?sinf(t*.6f)*w*.07f:0;
    float tx=x+w*.52f+drift,ty=y+h*.47f,body=std::min(h*.43f,95.f),bw=body*.55f;
    ImU32 a=cx.C(cx.sk.accent),ink=Fade(cx.C(cx.sk.text2),.5f);
    if(tuning) {
        float radius=Px(m.reticleSize);ImVec2 p=L(x+w*.5f,y+h*.5f,0,0).a;
        if(m.reticle==0||m.reticle==2)paint::Ring(dl,p,radius,a,Px(1.3f),64);
        if(m.reticle==1||m.reticle==2){dl->AddLine(ImVec2(p.x-radius*1.5f,p.y),ImVec2(p.x+radius*1.5f,p.y),a,Px(1));dl->AddLine(ImVec2(p.x,p.y-radius*1.5f),ImVec2(p.x,p.y+radius*1.5f),a,Px(1));}
        dl->AddCircleFilled(p,Px(2),a);
    }else {
        ImVec2 head=L(tx,ty-body*.37f,0,0).a;
        dl->AddCircleFilled(head,Px(body*.09f),ink,24);
        dl->AddLine(L(tx,ty-body*.24f,0,0).a,L(tx,ty+body*.18f,0,0).a,ink,Px(3));
        for(int side:{-1,1}){
            dl->AddLine(L(tx,ty-body*.2f,0,0).a,L(tx+side*bw*.4f,ty,0,0).a,ink,Px(2));
            dl->AddLine(L(tx,ty+body*.15f,0,0).a,L(tx+side*bw*.3f,ty+body*.47f,0,0).a,ink,Px(2));
        }
        if(m.visualOn){
            if(m.boxes)paint::Stroke(dl,L(tx-bw*.65f,ty-body*.58f,0,0).a,L(tx+bw*.65f,ty+body*.57f,0,0).a,Px(3),a,Px(m.lineWidth));
            if(m.bones){dl->AddLine(L(tx,ty-body*.25f,0,0).a,L(tx,ty+body*.2f,0,0).a,a,Px(m.lineWidth));for(int side:{-1,1})dl->AddLine(L(tx,ty-body*.18f,0,0).a,L(tx+side*bw*.4f,ty,0,0).a,a,Px(m.lineWidth));}
            if(m.names)paint::TextC(dl,cx.fUi,Fs(11.5f),L(tx,ty-body*.73f,0,0).a,cx.C(cx.sk.text),"训练目标");
            if(m.health){paint::Fill(dl,L(tx-bw*.65f-7,ty-body*.58f,tx-bw*.65f-4,ty+body*.57f).a,L(tx-bw*.65f-7,ty-body*.58f,tx-bw*.65f-4,ty+body*.57f).b,Px(1),cx.C(cx.sk.fieldStroke));paint::Fill(dl,L(tx-bw*.65f-7,ty-body*.32f,tx-bw*.65f-4,ty+body*.57f).a,L(tx-bw*.65f-7,ty-body*.32f,tx-bw*.65f-4,ty+body*.57f).b,Px(1),cx.C(cx.sk.good));}
            if(m.distance)paint::TextC(dl,cx.fUi,Fs(10),L(tx,ty+body*.76f,0,0).a,cx.C(cx.sk.text2),"48 m");
            if(m.tracers)dl->AddLine(L(x+w*.5f,y+h,0,0).a,L(tx,ty+body*.57f,0,0).a,Fade(a,.6f),Px(1));
        }
        if(m.navSel==0)paint::Ring(dl,L(x+w*.5f,y+h*.5f,0,0).a,Px(std::min(h*.42f,15+m.fov*1.4f)),Fade(a,.5f),Px(1),80);
    }
    dl->PopClipRect();
}
void Shell::DrawVisual(float x,float y,float w,float h) {
    float rw=std::clamp(w*.44f,310.f,440.f),lw=w-rw-18,xx=x+lw+18;
    Panel(80,x,y,lw,h,"显示内容","只保留你需要的信息");Toggle(cx,81,L(x+lw-60,y+21,x+lw-20,y+43),m.visualOn);
    const char* labels[]={"目标方框","骨骼轮廓","目标名称","生命状态","距离标记","引导连线","过滤队友"};
    bool* values[]={&m.boxes,&m.bones,&m.names,&m.health,&m.distance,&m.tracers,&m.teamFilter};
    float step=std::min(46.f,(h-100)/7.f);
    for(int i=0;i<7;i++){float yy=y+79+i*step;RowToggle(82+i,x+20,yy,lw-40,labels[i],*values[i]);if(i<6)cx.dl->AddLine(L(x+20,yy+34,0,0).a,L(x+lw-20,yy+34,0,0).a,Fade(cx.C(cx.sk.divider),.65f));}
    float bottom=153;
    Panel(90,xx,y,rw,h-bottom-14,"实时预览","选项会立即反映在预览中");
    DrawPreview(xx+20,y+74,rw-40,h-bottom-108);
    Panel(91,xx,y+h-bottom,rw,bottom,"细节控制");
    Range(92,xx+20,y+h-bottom+51,rw-40,"显示距离",m.maxDistance,50,500,"m");
    Range(93,xx+20,y+h-bottom+101,rw-40,"线条宽度",m.lineWidth,1,3,"px",1);
}
void Shell::DrawTuning(float x,float y,float w,float h) {
    float rw=std::clamp(w*.44f,310.f,440.f),lw=w-rw-18,xx=x+lw+18;
    Panel(120,x,y,lw,h,"手感调节","细微的改变，也有恰好的反馈");
    Range(121,x+20,y+78,lw-40,"基础灵敏度",m.sensitivity,.1f,3,"×",2);
    Range(122,x+20,y+140,lw-40,"响应强度",m.response,0,1,"",2);
    Range(123,x+20,y+202,lw-40,"缩放倍率",m.zoom,.5f,2,"×",2);
    Label(x+20,y+270,"响应曲线",12,1);const char* curves[]={"线性","柔和","迅捷"};Segment(cx,124,L(x+20,y+297,x+lw-20,y+330),m.curve,curves,3);
    Label(x+20,y+354,"准星样式",12,1);const char* styles[]={"圆环","十字","组合"};Segment(cx,125,L(x+20,y+381,x+lw-20,y+414),m.reticle,styles,3);
    Panel(126,xx,y,rw,h*.48f,"响应曲线","移动指针，查看当前响应");
    Rc graph=L(xx+24,y+78,xx+rw-24,y+h*.48f-24);
    paint::Fill(cx.dl,graph.a,graph.b,Px(8),cx.C(cx.sk.field0));
    for(int i=1;i<4;i++){float k=i/4.f;cx.dl->AddLine(ImVec2(graph.a.x,graph.a.y+graph.h()*k),ImVec2(graph.b.x,graph.a.y+graph.h()*k),cx.C(cx.sk.divider));}
    float exponent=m.curve==0?1.f:(m.curve==1?1.15f+m.response*2:.8f-m.response*.45f);
    ImVec2 prev=graph.a;for(int i=0;i<=80;i++){float k=i/80.f;ImVec2 p(graph.a.x+graph.w()*k,graph.b.y-graph.h()*powf(k,exponent));if(i)cx.dl->AddLine(prev,p,cx.C(cx.sk.accent),Px(2));prev=p;}
    float pos=cx.Hover(graph)?Clamp((cx.mouse.x-graph.a.x)/graph.w()):.65f;
    ImVec2 point(graph.a.x+graph.w()*pos,graph.b.y-graph.h()*powf(pos,exponent));paint::Glow(cx.dl,point,Px(12),Fade(cx.C(cx.sk.accent),.15f));cx.dl->AddCircleFilled(point,Px(4),cx.C(cx.sk.accent));
    float yy=y+h*.48f+14,hh=h*.52f-14;
    Panel(127,xx,yy,rw,hh,"准星预览");DrawPreview(xx+20,yy+55,rw-40,hh-127,true);
    Range(128,xx+20,yy+hh-58,rw-40,"准星大小",m.reticleSize,4,28,"px");
}
void Shell::DrawMisc(float x,float y,float w,float h) {
    float gap=14,cw=(w-gap*2)/3.f;
    const char* names[]={"日常","专注","自定义"};const char* desc[]={"熟悉的日常手感","更干净的操作空间","留给你的偏好"};
    for(int i=0;i<3;i++) {
        float xx=x+i*(cw+gap);Panel(160+i,xx,y,cw,151,names[i]);
        Label(xx+20,y+47,desc[i],11,2);Label(xx+20,y+73,presetsSaved[i]?"已保存设置":"尚未保存",11,1);
        if(Button(164+i,L(xx+20,y+105,xx+cw-20,y+133),"选择配置",m.slotSel==i))m.slotSel=i;
    }
    float lw=(w-18)*.54f,rw=w-lw-18,xx=x+lw+18;
    Panel(169,x,y+167,lw,h-167,"配置管理","当前会话内保存与恢复你的界面设置");
    Label(x+20,y+245,"当前配置",12,2);Label(x+20,y+270,names[m.slotSel],24);
    if(Button(170,L(x+20,y+319,x+lw*.5f-6,y+355),"保存当前设置")){presets[m.slotSel]=m;presetsSaved[m.slotSel]=true;Notify("当前设置已保存");}
    if(Button(171,L(x+lw*.5f+6,y+319,x+lw-20,y+355),"载入配置")){
        if(presetsSaved[m.slotSel]){int page=m.navSel,slot=m.slotSel;UiModel saved=presets[slot];int th=saved.themeSel;m=saved;m.navSel=page;m.slotSel=slot;recording=-1;for(auto& rec:m.rec)rec=false;SetTheme(th);Notify("已载入所选配置");}
        else Notify("这个配置还没有保存内容");
    }
    if(Button(172,L(x+20,y+h-57,x+lw-20,y+h-23),"恢复默认设置")) {int page=m.navSel; m=UiModel{};m.navSel=page;recording=-1;SetTheme(0);Notify("已恢复默认设置");}
    DrawKeys(175,xx,y+167,rw,h-167);
}
void Shell::DrawThemes(float x,float y,float w,float h) {
    const char* names[]={"甘雨 · 循霜","云母 · 晨光","曜石 · 静夜"};
    const char* subtitles[]={"海灯映雪，清心入梦。","温润纸面，轻盈有序。","收敛光线，专注此刻。"};
    float gap=14,cw=(w-gap*2)/3.f,th=205;
    for(int i=0;i<3;i++) {
        float xx=x+i*(cw+gap);Rc card=L(xx,y,xx+cw,y+th);
        auto& st=cx.St(200+i);st.hov.Target(cx.Hover(card)?1.f:0.f);st.hov.Update(cx.dt);st.act.Target(theme==i?1.f:0.f);st.act.Update(cx.dt);
        paint::Fill(cx.dl,card.a,card.b,Px(14),cx.C(cx.sk.card0));
        Rc preview=L(xx+8,y+8,xx+cw-8,y+123);
        ImU32 bg=i==0?IM_COL32(19,43,68,255):(i==1?IM_COL32(235,238,242,255):IM_COL32(19,22,27,255));
        paint::FillGrad(cx.dl,preview.a,preview.b,Px(9),bg,i==0?IM_COL32(44,80,101,255):bg);

        if(i==0){
            Flower(cx.dl,L(xx+cw-47,y+49,0,0).a,Px(29),IM_COL32(176,223,236,90),-.3f);
            for(int j=0;j<3;j++) {float yy=y+111-j*11;cx.dl->AddBezierCubic(L(xx+12,yy,0,0).a,L(xx+cw*.4f,yy-24,0,0).a,L(xx+cw*.7f,yy+15,0,0).a,L(xx+cw-12,yy-15,0,0).a,IM_COL32(156,211,233,30+j*8),Px(1));}
            Cryo(cx.dl,L(xx+36,y+39,0,0).a,Px(14),IM_COL32(165,230,247,230));
            paint::Text(cx.dl,cx.fDisp,Fs(27),L(xx+21,y+64,0,0).a,IM_COL32(226,241,251,235),"Ganyu");
            cx.dl->AddLine(L(xx+cw-25,y+18,0,0).a,L(xx+cw-25,y+46,0,0).a,IM_COL32(215,178,114,180));
            cx.dl->AddLine(L(xx+cw-25,y+46,0,0).a,L(xx+cw-25,y+67,0,0).a,IM_COL32(167,65,87,220),Px(2));
        }else {
            ImU32 panel=i==1?IM_COL32(255,255,255,255):IM_COL32(36,40,48,255);
            paint::Fill(cx.dl,L(xx+20,y+21,xx+cw-20,y+111).a,L(xx+20,y+21,xx+cw-20,y+111).b,Px(6),panel);
            paint::Fill(cx.dl,L(xx+20,y+21,xx+65,y+111).a,L(xx+20,y+21,xx+65,y+111).b,Px(6),i==1?IM_COL32(243,245,248,255):IM_COL32(28,31,37,255));
            for(int j=0;j<3;j++)paint::Fill(cx.dl,L(xx+29,y+38+j*21,xx+54,y+43+j*21).a,L(xx+29,y+38+j*21,xx+54,y+43+j*21).b,Px(2),i==1?IM_COL32(187,197,211,255):IM_COL32(75,83,97,255));
            paint::Fill(cx.dl,L(xx+79,y+37,xx+cw-34,y+45).a,L(xx+79,y+37,xx+cw-34,y+45).b,Px(3),i==1?IM_COL32(198,207,218,255):IM_COL32(96,110,130,255));
            for(int j=0;j<2;j++)paint::Fill(cx.dl,L(xx+79,y+58+j*25,xx+cw-34,y+75+j*25).a,L(xx+79,y+58+j*25,xx+cw-34,y+75+j*25).b,Px(4),i==1?IM_COL32(230,236,243,255):IM_COL32(48,55,68,255));
        }
        Label(xx+17,y+138,names[i],15);Label(xx+17,y+168,subtitles[i],10.5f,2);
        paint::Stroke(cx.dl,card.a,card.b,Px(14),paint::MixI(cx.C(cx.sk.cardStroke),cx.C(cx.sk.accent),Clamp(st.act.V()+st.hov.V()*.28f)),Px(1+st.act.V()));
        if(theme==i){cx.dl->AddCircleFilled(L(xx+cw-21,y+148,0,0).a,Px(8),cx.C(cx.sk.accent));paint::DrawIcon(cx.dl,paint::ICON_CHECK,L(xx+cw-21,y+148,0,0).a,Px(9),cx.C(cx.sk.sidebar0),Px(1.6f));}
        if(cx.Claim(200+i,card))SetTheme(i);
        if(cx.Hover(card))ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    float yy=y+th+16,hh=h-th-16,lw=(w-18)*.54f,rw=w-lw-18,xx=x+lw+18;
    Panel(205,x,yy,lw,hh,"场景与文字",theme==0?"冰蓝绸缎 · 清心花纹":"按自己的喜好，调整空间的细节");
    Range(206,x+20,yy+74,lw-40,"壁纸显现",m.wallpaper,.2f,1.f,"",2);
    RowToggle(207,x+20,yy+126,lw-40,"背景流动",m.backgroundMotion);
    RowToggle(208,x+20,yy+164,lw-40,"微光粒子",m.particles);
    Range(209,x+20,yy+202,lw-110,"整体字号",m.fontPercent,100.f,125.f,"%");
    m.fontPercent=std::round(m.fontPercent);
    if(Button(218,L(x+lw-82,yy+206,x+lw-20,yy+238),"默认"))m.fontPercent=112.f;
    Panel(210,xx,yy,rw,hh,"交互与强调色");
    RowToggle(211,xx+20,yy+54,rw-40,"流畅动效",m.animations,"连续过渡，跟随每一次操作");
    Label(xx+20,yy+116,"强调色",12,1);
    const Rgba accent[]={ThemeAt(theme).skin.accent,Hex(0x9CB9F4),Hex(0xB6A7DF),Hex(0xD3B78C)};
    for(int i=0;i<4;i++){
        Rc r=L(xx+20+i*45,yy+145,xx+52+i*45,yy+177);
        cx.dl->AddCircleFilled(r.c(),Px(10),Col(accent[i]),32);
        if(m.accentSel==i)paint::Ring(cx.dl,r.c(),Px(14),cx.C(cx.sk.text2),Px(1),32);
        if(cx.Claim(214+i,r))m.accentSel=i;
        if(cx.Hover(r))ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
}
void Shell::DrawPage(int page) {
    Label(204,57,navNames[page],29);
    float titleWidth=paint::TextSize(cx.fUi,Fs(29),navNames[page]).x/scale;
    paint::TextTracked(cx.dl,cx.fCaps,Fs(9.5f),L(204+titleWidth+16,70,0,0).a,cx.C(cx.sk.text3),navEn[page],Px(1.5f));
    Label(205,100,descriptions[page],12,1);
    float x=204,y=136,w=W-232,h=H-184;
    switch(page){case 0:DrawAim(x,y,w,h);break;case 1:DrawVisual(x,y,w,h);break;case 2:DrawTuning(x,y,w,h);break;case 3:DrawMisc(x,y,w,h);break;case 4:DrawThemes(x,y,w,h);break;}
}
void Shell::Draw(ImDrawList* dl) {
    cx.dl=dl;auto& io=ImGui::GetIO();cx.mouse=io.MousePos;cx.down=ImGui::IsMouseDown(0);cx.pressed=ImGui::IsMouseClicked(0);cx.released=ImGui::IsMouseReleased(0);cx.hotId=-1;
    if(cx.pressed)cx.activeId=-1;
    float e=themeT*.42f;cx.sk=BuildSkin(themeFrom,ThemeAt(theme).skin,Ease(E_PAGE,Clamp(e/.26f)),Ease(E_PAGE,Clamp((e-.08f)/.26f)),Ease(E_PAGE,Clamp((e-.14f)/.28f)));
    if(m.accentSel>0){const unsigned colors[]={0x9CB9F4,0xB6A7DF,0xD3B78C};cx.sk=WithAccent(cx.sk,Hex(colors[m.accentSel-1]),ThemeAt(theme).skin.accent2);}
    cx.interactive=true;
    int base=VtxBegin(dl);DrawChrome();DrawSidebar();
    if(recording>=0) {
        if(ImGui::IsKeyPressed(ImGuiKey_Escape)){m.rec[recording]=false;recording=-1;}
        else for(int key=ImGuiKey_NamedKey_BEGIN;key<ImGuiKey_NamedKey_END;key++) {
            if(key==ImGuiKey_MouseLeft)continue;
            if(ImGui::IsKeyPressed((ImGuiKey)key,false)){m.hk[recording]=key;m.rec[recording]=false;recording=-1;Notify("快捷键已更新");break;}
        }
    }

    for(int i=0;i<5;i++) {
        float a=Clamp(pageWeight[i].V(m.navSel==i?1.f:0.f));if(a<.002f)continue;
        cx.interactive=i==m.navSel&&a>.65f;
        dl->PushClipRect(L(185,43,0,0).a,L(W-12,H-32,0,0).a,true);
        int mark=VtxBegin(dl);DrawPage(i);
        VtxApply(dl,mark,L(204,136,W-28,H-48).c(),1,ImVec2(0,Px(10)*(1-a)),a);
        dl->PopClipRect();
    }
    cx.interactive=true;
    if(!cx.down)cx.activeId=-1;
    if(toastTime>0){
        float alpha=std::min(Clamp((2.8f-toastTime)/.15f),Clamp(toastTime/.25f));
        int mark=VtxBegin(dl);Rc r=L(W*.5f-140,H-77,W*.5f+140,H-38);
        paint::Shadow(dl,r.a,r.b,Px(12),cx.C(cx.sk.shadow),Px(4),Px(12),.5f);
        paint::Fill(dl,r.a,r.b,Px(12),paint::MixI(cx.C(cx.sk.sidebar0),cx.C(cx.sk.text),.06f));
        paint::Stroke(dl,r.a,r.b,Px(12),cx.C(cx.sk.cardStroke),1);
        paint::TextC(dl,cx.fUi,Fs(12.5f),r.c(),cx.C(cx.sk.text),toast);
        VtxApply(dl,mark,r.c(),1,ImVec2(0,Px(5)*(1-alpha)),alpha);
    }
    if(entering){float k=Ease(E_PAGE,Clamp(enterT/.8f));VtxApply(dl,base,L(0,0,W,H).c(),1,ImVec2(0,Px(12)*(1-k)),k);}
}
}
