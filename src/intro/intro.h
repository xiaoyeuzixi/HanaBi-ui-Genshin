#pragma once
#include <cstdint>
#include "imgui.h"
#include "intro/crystal_material.h"

namespace hb {
struct Intro {
    static constexpr float kDuration = 6.20f;
    static constexpr float kTurnStart = 1.45f, kTurnEnd = 3.65f;
    static constexpr float kDockStart = 4.65f;
    static constexpr int kSnowCount = 32, kLantCount = 20;
    struct Snow { float x, y, size, depth, ph; };
    struct Lantern { float x, y0, size, spd, ph; };
    Snow snow[kSnowCount]{};
    Lantern lant[kLantCount]{};
    float t = 0.f;
    bool finished = false, skipped = false;
    float W = 1600.f, H = 900.f;
    ImFont* fBrand = nullptr;
    CrystalMaterial material;
    struct MarkPose { ImVec2 center; float radius, opacity; };
    ImVec2 dockCenter{45.f,79.f};
    float dockRadius=16.f;
    void Shutdown() { material.Shutdown(); }
    void Reset(float w, float h);
    void Bind(ImFont* brand, ImFont*, ImFont*) { fBrand = brand; }
    void Update(float dt);
    void Draw(ImDrawList* dl, ImVec2 origin);
    void DrawMark(ImDrawList* dl, ImVec2 origin, float alpha=1.f);
    MarkPose Pose(ImVec2 origin) const;
    void Skip();
    float Progress() const;
    float HandoffK() const;
    bool CurtainDone() const { return finished; }
private:
    float skipElapsed = 0.f, skipStart = 0.f;
    float Opacity() const;
};
bool IntroSelfTest(char* out, int cap);
}
