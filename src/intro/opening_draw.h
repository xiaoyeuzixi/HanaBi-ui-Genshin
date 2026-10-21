#pragma once
#include "imgui.h"
namespace hb::opening_draw {
inline constexpr float pi=3.14159265359f,tau=pi*2;
float S(float t,float a,float b);
float Window(float t,float a,float b,float c,float d);
float Hash(int i);
ImU32 C(unsigned rgb,float a);
ImVec2 P(ImVec2 c,float r,float x,float y);
void Halo(ImDrawList* dl,ImVec2 p,float rx,float ry,unsigned rgb,float alpha);
void Halo(ImDrawList* dl,ImVec2 p,float r,unsigned rgb,float alpha);
void LightLine(ImDrawList* dl,const ImVec2* p,int count,unsigned rgb,float a,float w,bool closed=false);
void Ring(ImDrawList* dl,ImVec2 c,float rx,float ry,unsigned rgb,float a,float phase=0);
void Star(ImDrawList* dl,ImVec2 p,float r,unsigned rgb,float a,float rotation=0);
ImVec2 Bezier(ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d,float t);
void Ribbon(ImDrawList* dl,ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d,float width,unsigned rgb,float alpha);
void Petal(ImDrawList* dl,ImVec2 p,float r,float angle,unsigned color,float a,float width=.45f);
void Lotus(ImDrawList* dl,ImVec2 p,float r,float time,float alpha,bool jade);
void Pearl(ImDrawList* dl,ImVec2 center,float r,float t,float a,bool ice);
}
