#pragma once
#include "imgui.h"
#include "ui/theme.h"
#include "ui/ui_ctx.h"
#include "gfx/texture.h"
#include <functional>

namespace hb {
enum UiTheme { UI_THEME_GANYU=0, UI_THEME_LIGHT=1, UI_THEME_DARK=2, UI_THEME_YAE=3 };
constexpr int kUiThemeCount=kThemeCount;
struct ThemeTokens {
    const char* nameCn; const char* nameEn;
    ImU32 winFill,winStroke,sidebarFill,cardFill,cardStroke,text,text2,text3,accent,accentSoft,good;
    float radius;
};
const ThemeTokens& Tokens(int theme);
struct UiModel {
    bool aimMaster=true; int boneSel=0,trigSel=1;
    float fov=12.f,smooth=1.35f;
    bool predOn=true,fallOn=true,silentOn=false; float bulletSpeed=940.f;
    int hk[3]={ImGuiKey_LeftAlt,ImGuiKey_MouseRight,ImGuiKey_LeftShift}; bool rec[3]{};
    int navSel=0,themeSel=0,accentSel=0,slotSel=0;
    bool visualOn=true,boxes=true,bones=true,names=true,health=true,distance=true,tracers=false,teamFilter=true;
    float maxDistance=250.f,lineWidth=1.5f;
    float sensitivity=1.f,response=.45f,zoom=1.f,reticleSize=12.f;
    int curve=1,reticle=0;
    bool backgroundMotion=true,particles=true,animations=true;
    float wallpaper=.82f;
    float fontPercent=112.f;
    int startupMode=0, startupTheme=0;
};
struct Shell {
    int theme=0,oldTheme=0; Skin themeFrom=ThemeAt(0).skin; float themeT=1.f;
    bool entering=true,ready=false; float enterT=0.f,announce=0.f,t=0.f,uptime=0.f;
    float W=1050,H=680,scale=1,originX=0,originY=0,screenW=1050,screenH=680,centerX=525,centerY=340;
    UiModel m;
    Texture targetFigure;
    std::function<void(ImDrawList*,ImVec2,float,float,float)> drawElectro;
    std::function<void(ImDrawList*,ImVec2,float,float,float)> drawCryo;
    UiCtx cx; WState pool[kMaxWid];
    Smooth pageWeight[5],navPosition,ganyuMix,yaeMix,wallpaperMix;
    float sceneMix=1.f;
    int windowAction=0;
    int recording=-1; bool presetsSaved[3]{}; UiModel presets[3];
    char toast[128]{}; float toastTime=0.f;
    void Layout(float sw,float sh,float uiScale);
    void ApplyFonts(ImFont* ui,ImFont* display,ImFont* caps=nullptr);
    void Begin(float dt); void Draw(ImDrawList* dl); void SetTheme(int tint); void ReplayEnter();
    Rc L(float x0,float y0,float x1,float y1) const {return Rc(originX+x0*scale,originY+y0*scale,originX+x1*scale,originY+y1*scale);}
    float Px(float v) const {return v*scale;}
    float Fs(float v) const {return cx.Fs(v);}
    ImVec2 LogoCenter() const {return L(45,79,45,79).a;}
    float LogoRadius() const {return Px(16);}
    void Notify(const char* message);
    void Label(float x,float y,const char* text,float size=13.5f,int tone=0);
    bool Button(int id,Rc box,const char* text,bool selected=false);
    void Panel(int id,float x,float y,float w,float h,const char* title,const char* subtitle=nullptr);
    void RowToggle(int id,float x,float y,float w,const char* label,bool& value,const char* detail=nullptr);
    void Range(int id,float x,float y,float w,const char* label,float& value,float lo,float hi,const char* unit="",int precision=0);
    void DrawChrome(); void DrawSidebar(); void DrawPage(int page);
    void DrawAim(float x,float y,float w,float h);
    void DrawVisual(float x,float y,float w,float h);
    void DrawTuning(float x,float y,float w,float h);
    void DrawMisc(float x,float y,float w,float h);
    void DrawThemes(float x,float y,float w,float h);
    void DrawKeys(int id,float x,float y,float w,float h);
    void DrawPreview(float x,float y,float w,float h,bool tuning=false);
    void DrawMannequin(float x,float y,float w,float h,bool selectTarget);
};
}
