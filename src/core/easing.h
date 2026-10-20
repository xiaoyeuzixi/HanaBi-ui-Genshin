#pragma once

#include <cmath>

namespace hb {

enum Ease { E_FAST, E_PAGE, E_TAP, E_COUNT };

inline float EaseRaw(int kind, float t) {
    if (t <= 0.f) return 0.f;
    if (t >= 1.f) return 1.f;
    switch (kind) {
        case E_FAST: { const float u = 1.f - t; return 1.f - u * u * u * u * u; }
        case E_PAGE: {
            return t < 0.5f ? 4.f * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 3.f) * 0.5f;
        }
        default: return t >= 1.f ? 1.f : 1.f - std::pow(2.f, -10.f * t);
    }
}

namespace detail {
constexpr int kLut = 256;
struct Lut {
    float v[E_COUNT][kLut + 1];
    constexpr Lut() : v{} {
        for (int k = 0; k < E_COUNT; ++k) {
            for (int i = 0; i <= kLut; ++i) {
                const float t = float(i) / float(kLut);

                float o;
                if (k == E_FAST) { const float u = 1.f - t; o = 1.f - u * u * u * u * u; }
                else if (k == E_PAGE) { o = t < 0.5f ? 4.f * t * t * t : 1.f - (2.f - 2.f * t) * (2.f - 2.f * t) * (2.f - 2.f * t) * 0.5f; }
                else { o = t >= 1.f ? 1.f : 1.f - 1.f / (1.f + t * (1.f + t * (1.f + t * 8.f))); }
                v[k][i] = o;
            }
        }
    }
};
inline constexpr Lut kLutData{};
}

inline float Ease(int kind, float t) {
    if (t <= 0.f) return 0.f;
    if (t >= 1.f) return 1.f;
    const float f = t * float(detail::kLut);
    const int i = int(f);
    const float a = f - float(i);
    const float* row = detail::kLutData.v[kind];
    return row[i] + (row[i + 1] - row[i]) * a;
}

inline float Clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float Smoothstep(float e0, float e1, float x) {
    const float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.f - 2.f * t);
}

inline float Win(float t, float start, float end) {
    if (t <= start) return 0.f;
    if (t >= end) return 1.f;
    return (t - start) / (end - start);
}

}
