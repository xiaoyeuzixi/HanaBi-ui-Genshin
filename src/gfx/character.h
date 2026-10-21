#pragma once
#include "gfx/texture.h"
#include <array>

namespace hb {
class Character {
public:
    bool Init(ID3D11Device*,ID3D11DeviceContext*,const char* asset="tex/yae-character.hbc");
    void Draw(float time);
    void Shutdown();
private:
    struct Vertex {float x,y,u,v;};
    struct Layer {
        Texture image,mask;
        std::vector<Vertex> base;
        std::vector<int16_t> poses;
        std::vector<uint16_t> topology;
        ID3D11Buffer* indices=nullptr;
        UINT indexCount=0,id=0,kind=0;
        std::array<float,4> wave{};
    };
    std::vector<Layer> layers_;
    ID3D11DeviceContext* context_=nullptr;
    ID3D11VertexShader* vs_=nullptr;
    ID3D11PixelShader* ps_=nullptr;
    ID3D11InputLayout* layout_=nullptr;
    ID3D11Buffer *vertices_=nullptr,*params_=nullptr;
    ID3D11SamplerState* sampler_=nullptr;
    ID3D11BlendState* blend_=nullptr;
    UINT frameCount_=0;
    float period_=12;
};
}
