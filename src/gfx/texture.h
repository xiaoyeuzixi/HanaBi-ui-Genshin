#pragma once

#include <d3d11.h>
#include <cstdint>
#include <string>
#include <vector>

namespace hb {

struct Texture {
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11Texture2D* tex = nullptr;
    int w = 0, h = 0;

    bool Valid() const { return srv != nullptr; }
    void Release();
};

Texture LoadTexture(ID3D11Device* dev, const std::string& path);
Texture LoadTexture(ID3D11Device* dev, const void* bytes, size_t size);

bool LoadImageRGBA(const std::string& path, std::vector<uint8_t>& out, int& w, int& h);

bool LoadImageMono(const std::string& path, std::vector<uint8_t>& out, int& w, int& h);

}
