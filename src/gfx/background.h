#pragma once
#include <d3d11.h>
#include <string>
#include "gfx/texture.h"
#include "gfx/character.h"
#include "gfx/layered_wallpaper.h"
#include "imgui.h"

namespace hb {
class Background {
public:
    struct View {
        float zoom = 1.f, focusX = .5f, focusY = .5f;
        float dim = 0.f, sat = 1.f;
        float characterMix = 0.f;
        float yoimiyaMix = 0.f;
        float nahidaMix = 0.f;
        float furinaMix = 0.f;
        float faruzanMix = 0.f;
        float zhongliMix = 0.f;
    };
    bool Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, const std::string& imagePath);
    void Shutdown();
    void Render(ID3D11RenderTargetView* rtv, float width, float height, float t, const View& view);
    void DrawParticles(ImDrawList* dl, float width, float height, float t, const View& view) const;
    bool Valid() const { return valid_; }
    bool HasMips() const { return true; }
    int Width() const { return base_.w; }
    int Height() const { return base_.h; }
private:
    bool BuildPipeline();
    bool BuildTargets(int w, int h);
    struct Target { Texture image; ID3D11RenderTargetView* rtv = nullptr; };
    ID3D11Device* dev_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    ID3D11VertexShader* vs_ = nullptr;
    ID3D11PixelShader* ps_ = nullptr;
    ID3D11Buffer* cb_ = nullptr;
    ID3D11SamplerState* samp_ = nullptr;
    ID3D11RasterizerState* raster_ = nullptr;
    Texture base_;
    Texture yae_, overlay_, yoimiya_;
    Texture nahida_,nahidaWave_,nahidaShake_;
    Texture furina_,furinaFlow_,furinaIris_,furinaSway_,wallpaperNoise_;
    Texture faruzan_,faruzanFlow_;
    Character character_, yoimiyaCharacter_;
    LayeredWallpaper zhongliWallpaper_;
    Texture masks_[6];
    Target targets_[5];
    bool valid_ = false;
};
}
