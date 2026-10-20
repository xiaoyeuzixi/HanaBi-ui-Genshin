#pragma once

#include <cstddef>

namespace hb {

struct AssetBlob {
    const void* data = nullptr;
    size_t size = 0;
    bool Valid() const { return data != nullptr && size > 0; }
};

AssetBlob LoadAsset(const char* path);

int AssetCount();

}
