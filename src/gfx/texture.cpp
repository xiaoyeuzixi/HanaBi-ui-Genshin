#include "gfx/texture.h"

#include "assets/embedded.h"

#include <cstdio>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

namespace hb {
namespace {

bool DecodeRGBA(const unsigned char* bytes, int size, std::vector<uint8_t>& out, int& w, int& h) {
    if (!bytes || size <= 0) return false;
    int comp = 0;
    stbi_set_flip_vertically_on_load(0);
    unsigned char* px = stbi_load_from_memory(bytes, size, &w, &h, &comp, 4);
    if (!px) {
        printf("[img] decode failed (%s)\n", stbi_failure_reason());
        return false;
    }
    out.assign(px, px + (size_t)w * (size_t)h * 4);
    stbi_image_free(px);
    return true;
}

}

void Texture::Release() {
    if (srv) { srv->Release(); srv = nullptr; }
    if (tex) { tex->Release(); tex = nullptr; }
}

bool LoadImageRGBA(const std::string& path, std::vector<uint8_t>& out, int& w, int& h) {
    AssetBlob b = LoadAsset(path.c_str());
    if (!b.Valid()) {
        printf("[img] embedded miss: %s\n", path.c_str());
        return false;
    }
    return DecodeRGBA(static_cast<const unsigned char*>(b.data), (int)b.size, out, w, h);
}

bool LoadImageMono(const std::string& path, std::vector<uint8_t>& out, int& w, int& h) {

    std::vector<uint8_t> rgba;
    int rw = 0, rh = 0;
    if (!LoadImageRGBA(path, rgba, rw, rh)) return false;
    out.resize((size_t)rw * (size_t)rh);
    for (size_t i = 0, n = (size_t)rw * (size_t)rh; i < n; ++i) out[i] = rgba[i * 4];
    w = rw;
    h = rh;
    return true;
}

Texture LoadTexture(ID3D11Device* dev, const std::string& path) {
    const auto asset=LoadAsset(path.c_str());
    return LoadTexture(dev,asset.data,asset.size);
}
Texture LoadTexture(ID3D11Device* dev, const void* bytes, size_t size) {
    Texture t;
    std::vector<uint8_t> px;
    int w = 0, h = 0;
    if (size>INT_MAX||!DecodeRGBA(static_cast<const unsigned char*>(bytes),int(size),px,w,h)) return t;

    D3D11_TEXTURE2D_DESC d{};
    d.Width = (UINT)w;
    d.Height = (UINT)h;
    d.MipLevels = 1;
    d.ArraySize = 1;
    d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d.SampleDesc.Count = 1;
    d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = px.data();
    sd.SysMemPitch = (UINT)(w * 4);

    if (FAILED(dev->CreateTexture2D(&d, &sd, &t.tex))) return t;

    D3D11_SHADER_RESOURCE_VIEW_DESC sv{};
    sv.Format = d.Format;
    sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    sv.Texture2D.MipLevels = 1;
    if (FAILED(dev->CreateShaderResourceView(t.tex, &sv, &t.srv))) {
        t.Release();
        return t;
    }
    t.w = w;
    t.h = h;
    return t;
}

}
