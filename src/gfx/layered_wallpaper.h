#pragma once
#include "gfx/texture.h"
#include <array>

namespace hb {
class LayeredWallpaper {
public:
    bool Init(ID3D11Device*,ID3D11DeviceContext*,const char*);
    void Draw(float time);
    void Shutdown();
private:
    struct Layer {Texture image,mask;std::array<float,44> params{};};
    std::vector<Layer> layers_;
    Texture noise_,normal_;
    ID3D11DeviceContext* context_=nullptr;
    ID3D11VertexShader* vs_=nullptr;
    ID3D11PixelShader* ps_=nullptr;
    ID3D11Buffer* params_=nullptr;
    ID3D11SamplerState *border_=nullptr,*repeat_=nullptr,*clamp_=nullptr;
    ID3D11BlendState* blend_=nullptr;
};
}
