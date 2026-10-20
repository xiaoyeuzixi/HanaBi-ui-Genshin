#pragma once

#include <algorithm>

namespace hb {

constexpr float kFixedDt = 1.0f / 240.0f;
constexpr int kMaxSteps = 8;

class FixedClock {
public:
    void Begin(float frameDt) {
        acc_ += std::clamp(frameDt, 0.f, 0.25f);
        steps_ = 0;
    }

    bool Next() {
        if (acc_ < kFixedDt || steps_ >= kMaxSteps) return false;
        acc_ -= kFixedDt;
        ++steps_;
        return true;
    }
    void End() {
        if (steps_ >= kMaxSteps) acc_ = 0.f;
    }

    float Alpha() const { return std::clamp(acc_ / kFixedDt, 0.f, 1.f); }
    int Steps() const { return steps_; }

private:
    float acc_ = 0.f;
    int steps_ = 0;
};

}
