#pragma once

#include <cmath>

namespace hb {

struct SpringSpec {
    float zeta;
    float w;
    float w2;
};

constexpr SpringSpec MakeSpring(float zeta, float hz) {
    const float w = 6.28318530718f * hz;
    return SpringSpec{zeta, w, w * w};
}

namespace springs {

constexpr SpringSpec S_UI   = MakeSpring(0.72f, 3.20f);

constexpr SpringSpec S_SNAP = MakeSpring(0.65f, 4.00f);

constexpr SpringSpec S_SOFT = MakeSpring(0.80f, 2.40f);

constexpr SpringSpec S_CRIT = MakeSpring(1.00f, 4.00f);
}

struct Chan {
    float x = 0.f;
    float v = 0.f;
    float target = 0.f;
    float prev = 0.f;
};

inline void Step(Chan& c, const SpringSpec& s, float dt) {
    const float a = -s.w2 * (c.x - c.target) - 2.f * s.zeta * s.w * c.v;
    c.v += a * dt;
    c.x += c.v * dt;
}

inline void Retarget(Chan& c, float t) { c.target = t; }

inline void SnapTo(Chan& c, float v) {
    c.x = c.target = c.prev = v;
    c.v = 0.f;
}

inline float Render(const Chan& c, float alpha) {
    return c.prev + (c.x - c.prev) * alpha;
}

inline float StepResponse(const SpringSpec& s, float t) {
    if (t <= 0.f) return 0.f;
    const float z = s.zeta, w = s.w;
    if (z >= 1.f - 1e-4f) {
        return 1.f - std::exp(-w * t) * (1.f + w * t);
    }
    const float wd = w * std::sqrt(1.f - z * z);
    return 1.f - std::exp(-z * w * t) * (std::cos(wd * t) + (z * w / wd) * std::sin(wd * t));
}

inline float SettleEnvelope(const SpringSpec& s, float tol = 0.02f) {
    return std::log(1.f / tol) / (s.zeta * s.w);
}

}
