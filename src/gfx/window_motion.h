#pragma once
#include <algorithm>
#include <cmath>
#include "gfx/texture.h"
#include "imgui.h"

namespace hb {

struct WindowMotion {
    enum Phase { Idle, Leaving, Minimized, Returning } phase=Idle;
    float amount=0,elapsed=0,from=0,duration=.36f;
    struct Pose { float width,height,drop,opacity; };
    static float Ease(float input) {
        const double x=std::clamp(double(input),0.,1.);
        return float(x*x*x*(x*(x*6.-15.)+10.));
    }
    bool BeginMinimize(bool animate=true) {
        if(phase==Leaving||phase==Minimized)return false;
        from=amount;elapsed=0;duration=.36f*(1.f-from);phase=Leaving;
        if(!animate){amount=1;phase=Minimized;}
        return true;
    }
    bool BeginRestore(bool animate=true) {
        if(phase==Idle||phase==Returning)return false;
        from=amount;elapsed=0;duration=.30f*from;phase=Returning;
        if(!animate){amount=0;phase=Idle;}
        return true;
    }
    void Update(float dt) {
        if(phase!=Leaving&&phase!=Returning)return;
        elapsed+=std::max(0.f,dt);
        const float k=duration>.0001f?Ease(elapsed/duration):1.f;
        amount=from+((phase==Leaving?1.f:0.f)-from)*k;
        if(elapsed>=duration){amount=phase==Leaving?1.f:0.f;phase=phase==Leaving?Minimized:Idle;}
    }
    Pose GetPose() const {
        const float fade=Ease((amount-.12f)/.88f);
        return {1.f-.11f*amount,1.f-.16f*amount,.075f*amount,1.f-fade};
    }
};

struct WindowSnapshot {
    Texture image;
    bool Capture(ID3D11Device* device,ID3D11DeviceContext* context,ID3D11RenderTargetView* view) {
        image.Release();
        ID3D11Resource* resource=nullptr;ID3D11Texture2D* source=nullptr;
        view->GetResource(&resource);
        const HRESULT result=resource->QueryInterface(IID_PPV_ARGS(&source));resource->Release();
        if(FAILED(result))return false;
        D3D11_TEXTURE2D_DESC desc{};source->GetDesc(&desc);
        image.w=int(desc.Width);image.h=int(desc.Height);
        const bool multisample=desc.SampleDesc.Count>1;
        desc.SampleDesc={1,0};desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags=desc.MiscFlags=0;
        bool ok=SUCCEEDED(device->CreateTexture2D(&desc,nullptr,&image.tex));
        if(ok){
            context->OMSetRenderTargets(0,nullptr,nullptr);
            if(multisample)context->ResolveSubresource(image.tex,0,source,0,desc.Format);
            else context->CopyResource(image.tex,source);
            ok=SUCCEEDED(device->CreateShaderResourceView(image.tex,nullptr,&image.srv));
        }
        source->Release();
        if(!ok)image.Release();
        return ok;
    }
    void Draw(ImDrawList* dl,ImVec2 a,ImVec2 b,float opacity=1.f) const {
        if(image.Valid())dl->AddImage((ImTextureID)(intptr_t)image.srv,a,b,{0,0},{1,1},
                                     IM_COL32(255,255,255,int(std::clamp(opacity,0.f,1.f)*255.f+.5f)));
    }
};

inline bool WindowMotionSelfTest() {
    bool ok=true;
    for(float fps:{30.f,60.f,120.f,240.f}) {
        WindowMotion motion;ok&=motion.BeginMinimize();
        float previous=0,elapsed=0;
        while(motion.phase==WindowMotion::Leaving&&elapsed<1.f){
            ok&=!motion.BeginMinimize();
            motion.Update(1.f/fps);elapsed+=1.f/fps;
            const auto pose=motion.GetPose();
            ok&=motion.amount>=previous&&pose.opacity>=0&&pose.opacity<=1&&pose.width>0&&pose.height>0;
            previous=motion.amount;
        }
        ok&=motion.phase==WindowMotion::Minimized&&motion.amount==1&&motion.GetPose().opacity==0;
        ok&=elapsed>=.36f-.0001f&&elapsed<=.36f+1.f/fps+.0001f;
        ok&=motion.BeginRestore();previous=motion.amount;
        for(int i=0;i<int(fps);++i){motion.Update(1.f/fps);ok&=motion.amount<=previous;previous=motion.amount;}
        ok&=motion.phase==WindowMotion::Idle&&motion.amount==0&&motion.GetPose().opacity==1;
    }
    WindowMotion disabled;disabled.BeginMinimize(false);ok&=disabled.phase==WindowMotion::Minimized;
    disabled.BeginRestore(false);ok&=disabled.phase==WindowMotion::Idle;
    return ok;
}
}
