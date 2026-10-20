#pragma once

#include <cmath>
#include <cstdint>

namespace hb {

struct Rgba {
    float r, g, b, a;
};

constexpr uint32_t kIce   = 0x7FE6F5;
constexpr uint32_t kEmber = 0xFFC978;
constexpr uint32_t kDeep  = 0x0A1220;
constexpr uint32_t kNight = 0x1B1830;
constexpr uint32_t kInk   = 0x070B14;

inline Rgba Hex(uint32_t hex, float a = 1.f) {
    return Rgba{((hex >> 16) & 0xFF) / 255.f, ((hex >> 8) & 0xFF) / 255.f, (hex & 0xFF) / 255.f, a};
}

inline Rgba WithA(Rgba c, float a) { c.a = a; return c; }
inline Rgba MulA(Rgba c, float k) { c.a *= k; return c; }

inline float SrgbToLinear(float c) {
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}
inline float LinearToSrgb(float c) {
    return c <= 0.0031308f ? c * 12.92f : 1.055f * std::pow(c, 1.f / 2.4f) - 0.055f;
}

inline Rgba MixLinear(Rgba a, Rgba b, float t) {
    return Rgba{
        LinearToSrgb(SrgbToLinear(a.r) + (SrgbToLinear(b.r) - SrgbToLinear(a.r)) * t),
        LinearToSrgb(SrgbToLinear(a.g) + (SrgbToLinear(b.g) - SrgbToLinear(a.g)) * t),
        LinearToSrgb(SrgbToLinear(a.b) + (SrgbToLinear(b.b) - SrgbToLinear(a.b)) * t),
        a.a + (b.a - a.a) * t};
}

inline Rgba Mix(Rgba a, Rgba b, float t) {
    return Rgba{a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t};
}

}
