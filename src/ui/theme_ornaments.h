#pragma once
#include "ui/theme.h"
namespace hb::ornament {
void StoneSeal(ImDrawList* dl,ImVec2 p,float r,ImU32 col);
void Flower(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation=0.f);
void Sakura(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation=0.f);
void Firework(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float rotation=0.f);
void Goldfish(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time);
void Sprig(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time=0);
void Polyhedron(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time);
void ArchiveBook(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time);
void Droplet(ImDrawList* dl,ImVec2 p,float r,ImU32 col);
void TopHat(ImDrawList* dl,ImVec2 p,float r,ImU32 col,float time);
void ChineseKnot(ImDrawList* dl,ImVec2 top,float scale,float time);
void Cryo(ImDrawList* dl,ImVec2 p,float r,ImU32 col);
}
