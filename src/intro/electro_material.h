#pragma once
#include <deque>
#include "intro/crystal_material.h"

namespace hb {
class ElectroMaterial {
public:
    bool Init(ID3D11Device*,ID3D11DeviceContext*,int style=0);
    void Shutdown();
    void BeginFrame() { requests_.clear(); }
    void Queue(ImDrawList*,std::vector<CrystalVertex>&&,ImVec2,float,float,float,float reflection=0.f,float waterY=0.f,float motionTime=0.f);
    void QueueFormation(ImDrawList*,std::vector<CrystalVertex>&&,ImVec2,float,float,float,float reflection=0.f,float waterY=0.f);
private:
    struct Request {
        ElectroMaterial* owner;
        std::vector<CrystalVertex> vertices;
        ImVec2 center;
        float radius,time,opacity;
        ImVec4 clip;
        float reflection,waterY,motionTime;
    };
    static void Callback(const ImDrawList*,const ImDrawCmd*);
    void Render(const Request&);
    bool Snapshot(ID3D11Texture2D*);
    bool SetParams(const float*);
    void DrawGlow(const Request&,ID3D11RenderTargetView*,ID3D11DepthStencilView*);
    ID3D11Device* device_=nullptr;
    ID3D11DeviceContext* context_=nullptr;
    ID3D11VertexShader* vs_=nullptr;
    ID3D11PixelShader* ps_=nullptr;
    ID3D11VertexShader* glowVs_=nullptr;
    ID3D11PixelShader* glowPs_=nullptr;
    ID3D11Texture2D* glowTextures_[2]{};
    ID3D11RenderTargetView* glowTargets_[2]{};
    ID3D11ShaderResourceView* glowViews_[2]{};
    ID3D11InputLayout* layout_=nullptr;
    ID3D11Buffer *vertices_=nullptr,*params_=nullptr;
    ID3D11SamplerState* sampler_=nullptr;
    ID3D11RasterizerState* raster_=nullptr;
    ID3D11BlendState* blend_=nullptr;
    ID3D11DepthStencilState* depth_=nullptr;
    ID3D11Texture2D* scene_=nullptr;
    ID3D11ShaderResourceView* sceneView_=nullptr;
    UINT width_=0,height_=0;
    int style_=0;
    std::deque<Request> requests_;
};
}
