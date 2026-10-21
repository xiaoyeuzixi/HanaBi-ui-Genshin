#pragma once
#include <d3d11.h>
#include <vector>
#include <deque>
#include "imgui.h"

namespace hb {

struct CrystalVertex {
    float x, y, nx, ny, nz, u, v, material, opacity, thickness, seed;
};
class CrystalMaterial {
public:
    bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
    void SetStyle(int style) { style_=style; }
    void Shutdown();
    void BeginFrame() { requests_.clear(); }
    void Queue(ImDrawList* list, std::vector<CrystalVertex>&& vertices,
               ImVec2 center, float radius, float time, float dissolve, ImVec2 lantern);
private:
    struct Request {
        CrystalMaterial* owner;
        std::vector<CrystalVertex> vertices;
        ImVec2 center,lantern;
        float radius,time,dissolve;
        ImVec4 clip;
    };
    static void Callback(const ImDrawList*, const ImDrawCmd* cmd);
    void Render(const Request&);
    bool Snapshot(ID3D11Texture2D* target);
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    ID3D11VertexShader* vs_ = nullptr;
    ID3D11PixelShader* ps_ = nullptr;
    ID3D11InputLayout* layout_ = nullptr;
    ID3D11Buffer* vertices_ = nullptr;
    ID3D11Buffer* params_ = nullptr;
    ID3D11SamplerState* sampler_ = nullptr;
    ID3D11RasterizerState* raster_ = nullptr;
    ID3D11BlendState* blend_ = nullptr;
    ID3D11DepthStencilState* depth_ = nullptr;
    ID3D11Texture2D* scene_ = nullptr;
    ID3D11ShaderResourceView* sceneView_ = nullptr;
    UINT width_ = 0, height_ = 0;
    std::deque<Request> requests_;
    int style_ = 2;
};
}
