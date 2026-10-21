#pragma once
#include <cstdint>
#include "imgui.h"
#include "intro/crystal_material.h"
#include "gfx/texture.h"
#include "intro/electro_material.h"
#include "ui/theme.h"

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
    ElectroMaterial electroMaterial;
    ElectroMaterial pyroMaterial;
    ElectroMaterial dendroMaterial;
    ElectroMaterial hydroMaterial;
    ElectroMaterial anemoMaterial;
    ElectroMaterial geoMaterial;
    ElectroMaterial faruzanMaterial;
    float energyTime=0.f;
    Element element=Element::Cryo;
    Scenery scenery=Scenery::Lanterns;
    Opening opening=Opening::Element;
    struct MarkPose { ImVec2 center; float radius, opacity; };
    ImVec2 dockCenter{45.f,79.f};
    float dockRadius=16.f;
    void Shutdown() { material.Shutdown(); electroMaterial.Shutdown(); pyroMaterial.Shutdown(); dendroMaterial.Shutdown(); hydroMaterial.Shutdown(); anemoMaterial.Shutdown(); geoMaterial.Shutdown(); faruzanMaterial.Shutdown(); }
    void Reset(float w, float h);
    void Bind(ImFont* brand, ImFont*, ImFont*) { fBrand = brand; }
    void Update(float dt);
    void Draw(ImDrawList* dl, ImVec2 origin);
    void DrawMark(ImDrawList* dl, ImVec2 origin, float alpha=1.f);
    void DrawElectroMark(ImDrawList* dl, ImVec2 origin, float alpha=1.f);
    void DrawPyroMark(ImDrawList*,ImVec2,float alpha=1.f);
    void DrawPyroEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawPyroAtmosphere(ImDrawList*,ImVec2);
    void DrawFireworkScenery(ImDrawList*,ImVec2);
    void DrawDendroMark(ImDrawList*,ImVec2,float alpha=1.f);
    void DrawDendroEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawDendroAtmosphere(ImDrawList*,ImVec2);
    void DrawSanctuaryScenery(ImDrawList*,ImVec2);
    MarkPose DendroPose(ImVec2) const;
    void DrawHydroMark(ImDrawList*,ImVec2,float alpha=1.f);
    void DrawHydroEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawHydroAtmosphere(ImDrawList*,ImVec2);
    void DrawOperaScenery(ImDrawList*,ImVec2);
    MarkPose HydroPose(ImVec2) const;
    void DrawAnemoMark(ImDrawList*,ImVec2,float alpha=1.f);
    void DrawAnemoEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawAnemoAtmosphere(ImDrawList*,ImVec2);
    void DrawArchiveScenery(ImDrawList*,ImVec2);
    MarkPose AnemoPose(ImVec2) const;
    void DrawGeoMark(ImDrawList*,ImVec2,float alpha=1.f);
    void DrawGeoEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawGeoAtmosphere(ImDrawList*,ImVec2);
    void DrawStoneScenery(ImDrawList*,ImVec2);
    MarkPose GeoPose(ImVec2) const;
    void DrawFaruzanEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawFaruzanAtmosphere(ImDrawList*,ImVec2);
    void DrawCharacterAtmosphere(ImDrawList*,ImVec2);
    void DrawLanternScenery(ImDrawList*,ImVec2);
    void DrawCharacterMark(ImDrawList*,ImVec2,float alpha=1.f);
    static std::vector<CrystalVertex> CryoMesh();
    MarkPose CharacterPose(ImVec2) const;
    void DrawElectroEmblem(ImDrawList*,ImVec2,float,float,float);
    void DrawCryoEmblem(ImDrawList*,ImVec2,float,float,float);
    MarkPose ElectroPose(ImVec2 origin) const;
    MarkPose PyroPose(ImVec2 origin) const;
    void DrawElectroAtmosphere(ImDrawList* dl, ImVec2 origin);
    MarkPose Pose(ImVec2 origin) const;
    void Skip();
    float Progress() const;
    float HandoffK() const;
    bool CurtainDone() const { return finished; }
private:
    void DrawBrand(ImDrawList* dl, ImVec2 origin);
    float skipElapsed = 0.f, skipStart = 0.f;
    float Opacity() const;
};
bool IntroSelfTest(char* out, int cap);
}
