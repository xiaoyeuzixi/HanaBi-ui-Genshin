#include "gfx/background.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "core/easing.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace hb {
namespace {
const char* kVS = R"(
struct Output { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };
Output main(uint id : SV_VertexID) {
    Output o;
    float2 p = float2(id == 2 ? 3.0 : -1.0, id == 1 ? 3.0 : -1.0);
    o.pos = float4(p, 0, 1);
    o.uv = float2((p.x + 1) * .5, (1 - p.y) * .5);
    return o;
}
)";
const char* kPS = R"(
Texture2D image : register(t0);
Texture2D baseImage : register(t1);
Texture2D overlayAlpha : register(t2);
Texture2D flowMap : register(t3);
Texture2D phaseMap : register(t4);
Texture2D shakeOpacity : register(t5);
Texture2D waveA : register(t6);
Texture2D waveB : register(t7);
Texture2D yaeImage : register(t8);
Texture2D overlayImage : register(t9);
SamplerState linearClamp : register(s0);
cbuffer Params : register(b0) {
    float4 frame;
    float4 view;
    float4 source;
};
float4 Original(float2 uv) {
    return float4(overlayImage.SampleLevel(linearClamp, uv, 0).rgb,
                  overlayAlpha.SampleLevel(linearClamp, uv, 0).r);
}
float2 WaveUV(float2 uv, float direction, float scale, float speed, float strength, float mask) {

    float2 d = float2(-sin(direction), cos(direction));
    float phase = frame.z * speed + dot(uv, d) * scale;
    return uv + sin(phase) * float2(d.y, -d.x) * strength * strength * mask;
}
float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET {
    if(frame.w>3.5)return yaeImage.SampleLevel(linearClamp,uv,0);
    if (frame.w < .5) {

        float phase = phaseMap.SampleLevel(linearClamp, uv, 0).r * 6.283185307;
        float offset = (sin(frame.z * 1.3 + phase) * .498 + .5) * 2 - 1;
        float2 flow = (flowMap.SampleLevel(linearClamp, uv, 0).rg - .498) * 2;
        float2 delta = offset * .033 * .033 * flow;
        float mask = shakeOpacity.SampleLevel(linearClamp, uv + delta, 0).r;
        return lerp(Original(uv), Original(uv + delta), mask);
    }
    if (frame.w < 1.5) {
        float mask = waveA.SampleLevel(linearClamp, uv, 0).r;
        return image.SampleLevel(linearClamp,
            WaveUV(uv, 1.5707963, 62.51, 2.2, .04, mask), 0);
    }
    if (frame.w < 2.5) {
        float mask = waveB.SampleLevel(linearClamp, uv, 0).r;
        return image.SampleLevel(linearClamp,
            WaveUV(uv, .69881376, 9.91, 1.25, .05, mask), 0);
    }

    float outAspect = frame.x / frame.y;
    float2 cover = outAspect > source.x ? float2(1, source.x / outAspect)
                                        : float2(outAspect / source.x, 1);
    float2 p = (uv - .5) * cover / max(view.y, .25) + view.zw;
    float3 base = baseImage.SampleLevel(linearClamp, p, 0).rgb;
    float4 top = image.SampleLevel(linearClamp, p, 0);
    float3 color = lerp(base, top.rgb, top.a);

    float4 blurred = image.SampleLevel(linearClamp, p, 4);
    float3 light = lerp(base, blurred.rgb, blurred.a);
    float brightness = max(light.r, max(light.g, light.b));
    color += light * saturate((brightness - .65) / .35) * .2;
    if(source.z > .0001) {
        float3 sakura = yaeImage.SampleLevel(linearClamp,p,0).rgb;
        float shaft = pow(saturate(sin((p.x+p.y*.32)*14+sin(frame.z*.17)*.25)),12);
        sakura += float3(.050,.028,.043)*shaft*(.6+.4*sin(frame.z*.3));
        color = lerp(color,sakura,source.z);
    }
    float luma = dot(color, float3(.299, .587, .114));
    color = lerp(luma.xxx, color, source.y) * (1 - saturate(view.x));
    return float4(saturate(color), 1);
}
)";

void Particle(ImDrawList* dl, ImVec2 c, float radius, uint32_t rgb, float alpha) {
    if (alpha <= .004f) return;
    constexpr int n = 16;
    const auto col = [rgb](float a) {
        return IM_COL32((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255,
                        int(Clamp01(a) * 255.f));
    };
    dl->PrimReserve(n * 3, n + 1);
    const ImDrawIdx base = (ImDrawIdx)dl->_VtxCurrentIdx;
    const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
    dl->PrimWriteVtx(c, uv, col(alpha));
    for (int i = 0; i < n; ++i) {
        const float a = i * 6.283185307f / n;
        dl->PrimWriteVtx(ImVec2(c.x + std::cos(a) * radius, c.y + std::sin(a) * radius),
                         uv, col(0.f));
    }
    for (int i = 0; i < n; ++i) {
        dl->PrimWriteIdx(base);
        dl->PrimWriteIdx((ImDrawIdx)(base + 1 + i));
        dl->PrimWriteIdx((ImDrawIdx)(base + 1 + (i + 1) % n));
    }
}
float Hash(int i, int j) {
    uint32_t v = uint32_t(i * 12539 + j * 8641 + 7937);
    v = (v ^ (v >> 16)) * 0x45d9f3bu;
    return float(v & 0xFFFFFF) / 16777216.f;
}
}

bool Background::Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, const std::string& path) {
    dev_ = dev; ctx_ = ctx;
    base_ = LoadTexture(dev, path);
    yae_=LoadTexture(dev,"tex/yae-bg-4k.jpg");
    overlay_=LoadTexture(dev,"tex/ganyu-overlay.jpg");
    const char* names[] = {"ganyu-overlay-alpha.png", "ganyu-flow-rg.png", "ganyu-phase.png",
                          "ganyu-shake-opacity.png", "ganyu-wave-a.png", "ganyu-wave-b.png"};
    bool ok = base_.Valid() && yae_.Valid() && overlay_.Valid() && character_.Init(dev,ctx);
    for (int i = 0; i < 6; ++i) {
        masks_[i] = LoadTexture(dev, std::string("tex/") + names[i]);
        ok = ok && masks_[i].Valid();
    }
    valid_ = ok && BuildPipeline();
    printf("[bg] original %dx%d + six source masks: %s\n",
            base_.w, base_.h, valid_ ? "ok" : "FAILED");
    if (!valid_) Shutdown();
    return valid_;
}

bool Background::BuildPipeline() {
    ID3DBlob* code = nullptr;
    ID3DBlob* errors = nullptr;
    auto compile = [&](const char* src, const char* profile) {
        const HRESULT hr = D3DCompile(src, strlen(src), "wallpaper", nullptr, nullptr,
            "main", profile, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &errors);
        if (errors) {
            printf("[bg] %s\n", (const char*)errors->GetBufferPointer());
            errors->Release(); errors = nullptr;
        }
        return SUCCEEDED(hr);
    };
    if (!compile(kVS, "vs_5_0")) return false;
    HRESULT hr = dev_->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &vs_);
    code->Release(); code = nullptr;
    if (FAILED(hr) || !compile(kPS, "ps_5_0")) return false;
    hr = dev_->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &ps_);
    code->Release();
    if (FAILED(hr)) return false;
    D3D11_BUFFER_DESC cb{};
    cb.ByteWidth = 48; cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER; cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(dev_->CreateBuffer(&cb, nullptr, &cb_))) return false;
    D3D11_SAMPLER_DESC s{};
    s.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    s.AddressU = s.AddressV = s.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    s.ComparisonFunc = D3D11_COMPARISON_NEVER; s.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(dev_->CreateSamplerState(&s, &samp_))) return false;
    D3D11_RASTERIZER_DESC r{};
    r.FillMode = D3D11_FILL_SOLID; r.CullMode = D3D11_CULL_NONE; r.DepthClipEnable = TRUE;
    return SUCCEEDED(dev_->CreateRasterizerState(&r, &raster_));
}

bool Background::BuildTargets(int w, int h) {
    if (targets_[0].image.w == w && targets_[0].image.h == h &&
        targets_[0].rtv && targets_[0].image.Valid() &&
        targets_[1].rtv && targets_[1].image.Valid() && targets_[2].rtv && targets_[2].image.Valid()) return true;
    ctx_->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* empty[8]{};
    ctx_->PSSetShaderResources(0, 8, empty);
    for (auto& target : targets_) {
        if (target.rtv) { target.rtv->Release(); target.rtv = nullptr; }
        target.image.Release();
        target.image.w = target.image.h = 0;
    }
    for (auto& target : targets_) {
        D3D11_TEXTURE2D_DESC d{};
        d.Width = w; d.Height = h; d.MipLevels = 0; d.ArraySize = 1;
        d.Format = DXGI_FORMAT_R8G8B8A8_UNORM; d.SampleDesc.Count = 1;
        d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        d.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        if (FAILED(dev_->CreateTexture2D(&d, nullptr, &target.image.tex)) ||
            FAILED(dev_->CreateShaderResourceView(target.image.tex, nullptr, &target.image.srv)) ||
            FAILED(dev_->CreateRenderTargetView(target.image.tex, nullptr, &target.rtv))) return false;
        target.image.w = w; target.image.h = h;
    }
    return true;
}

void Background::Render(ID3D11RenderTargetView* rtv, float w, float h, float t, const View& v) {
    if (!valid_ || w <= 0 || h <= 0) return;
    const int effectW = std::min(base_.w, std::max(1280, int(std::max(w, h * base_.w / base_.h))));
    const int effectH = effectW * base_.h / base_.w;
    if (!BuildTargets(effectW, effectH)) {
        const float clear[] = {.03f, .05f, .09f, 1.f};
        ctx_->ClearRenderTargetView(rtv, clear);
        return;
    }
    ctx_->RSSetState(raster_);
    ctx_->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    ctx_->OMSetDepthStencilState(nullptr, 0);
    ctx_->IASetInputLayout(nullptr);
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_->VSSetShader(vs_, nullptr, 0);
    ctx_->GSSetShader(nullptr, nullptr, 0);
    ctx_->PSSetShader(ps_, nullptr, 0);
    ctx_->PSSetConstantBuffers(0, 1, &cb_);
    ctx_->PSSetSamplers(0, 1, &samp_);
    if(v.characterMix>.0001f){
        ID3D11ShaderResourceView* empty[10]{};ctx_->PSSetShaderResources(0,10,empty);
        ctx_->OMSetRenderTargets(1,&targets_[2].rtv,nullptr);
        D3D11_VIEWPORT viewport{0,0,float(effectW),float(effectH),0,1};ctx_->RSSetViewports(1,&viewport);
        const float params[]={w,h,t,4,v.dim,v.zoom,v.focusX,v.focusY,float(base_.w)/base_.h,v.sat,v.characterMix,0};
        D3D11_MAPPED_SUBRESOURCE map{};if(FAILED(ctx_->Map(cb_,0,D3D11_MAP_WRITE_DISCARD,0,&map)))return;memcpy(map.pData,params,sizeof(params));ctx_->Unmap(cb_,0);
        ctx_->PSSetShaderResources(8,1,&yae_.srv);ctx_->Draw(3,0);
        character_.Draw(t);
        ctx_->OMSetRenderTargets(0,nullptr,nullptr);ctx_->PSSetShaderResources(0,10,empty);
        ctx_->GenerateMips(targets_[2].image.srv);
        ctx_->IASetInputLayout(nullptr);ctx_->VSSetShader(vs_,nullptr,0);ctx_->PSSetShader(ps_,nullptr,0);ctx_->PSSetConstantBuffers(0,1,&cb_);ctx_->PSSetSamplers(0,1,&samp_);ctx_->OMSetBlendState(nullptr,nullptr,0xffffffff);
    }
    for (int pass = v.characterMix >= .9999f ? 3 : 0; pass < 4; ++pass) {
        const int outIndex = pass % 2;
        ID3D11ShaderResourceView* empty[10]{};
        ctx_->PSSetShaderResources(0, 10, empty);
        ID3D11RenderTargetView* target = pass == 3 ? rtv : targets_[outIndex].rtv;
        ctx_->OMSetRenderTargets(1, &target, nullptr);
        const float tw = pass == 3 ? w : float(effectW);
        const float th = pass == 3 ? h : float(effectH);
        D3D11_VIEWPORT viewport{0, 0, tw, th, 0, 1};
        ctx_->RSSetViewports(1, &viewport);
        float params[12] = {w, h, t, float(pass), v.dim, v.zoom, v.focusX, v.focusY,
                            float(base_.w) / base_.h, v.sat, v.characterMix, 0};
        D3D11_MAPPED_SUBRESOURCE map{};
        if (FAILED(ctx_->Map(cb_, 0, D3D11_MAP_WRITE_DISCARD, 0, &map))) return;
        memcpy(map.pData, params, sizeof(params)); ctx_->Unmap(cb_, 0);
        ID3D11ShaderResourceView* resources[10] = {
            pass ? targets_[(pass - 1) % 2].image.srv : base_.srv, base_.srv,
            masks_[0].srv, masks_[1].srv, masks_[2].srv, masks_[3].srv, masks_[4].srv, masks_[5].srv,targets_[2].image.srv,overlay_.srv
        };
        ctx_->PSSetShaderResources(0, 10, resources);
        ctx_->Draw(3, 0);
        if (pass == 2) {
            ctx_->OMSetRenderTargets(0, nullptr, nullptr);
            ctx_->PSSetShaderResources(0, 10, empty);
            ctx_->GenerateMips(targets_[0].image.srv);
        }
    }
    ID3D11ShaderResourceView* empty[10]{};
    ctx_->PSSetShaderResources(0, 10, empty);
}

void Background::DrawParticles(ImDrawList* dl, float w, float h, float t, const View& v) const {
    if (!valid_ || !dl) return;
    const float cover = std::max(w / base_.w, h / base_.h) * v.zoom;
    auto screen = [&](float x, float y) {
        return ImVec2((x - v.focusX * base_.w) * cover + w * .5f,
                      (y - v.focusY * base_.h) * cover + h * .5f);
    };
    constexpr float origins[4][2] = {{169.72144f, 548.18933f}, {739.14343f, 1634.87567f},
                                    {1830.05762f, 1738.57819f}, {3082.74438f, 1650.83331f}};
    dl->PushClipRect(ImVec2(0, 0), ImVec2(w, h), true);
    if(v.characterMix>.001f) {
        for(int i=0;i<32;++i) {
            const float depth=.35f+.65f*Hash(i,30),life=16.f+Hash(i,31)*15.f;
            const float age=std::fmod(t+Hash(i,32)*life,life),u=age/life;
            const float x=std::fmod(Hash(i,33)+u*.18f+std::sin(t*.17f+i)*.025f+1.f,1.f)*w;
            const float y=(-.08f+u*1.16f)*h;
            const float a=Smoothstep(0,.08f,u)*(1-Smoothstep(.88f,1,u))*v.characterMix*(1-v.dim);
            const float r=(3.f+depth*4.f)*w/1600.f,turn=t*(.35f+Hash(i,34)*.5f)+i;
            auto pt=[&](float px,float py){return ImVec2(x+(cosf(turn)*px-sinf(turn)*py)*r,y+(sinf(turn)*px+cosf(turn)*py)*r);};
            const ImU32 col=IM_COL32(255,195+int(depth*30),228,int(a*(80+depth*65)));
            dl->PathLineTo(pt(-1,0));dl->PathBezierCubicCurveTo(pt(-.7f,-1.15f),pt(.4f,-.9f),pt(1,0));
            dl->PathBezierCubicCurveTo(pt(.1f,.8f),pt(-.5f,.7f),pt(-1,0));dl->PathFillConvex(col);
            if(i%4==0)Particle(dl,{x,y},r*4,0xDC92ED,a*.065f);
        }
    }
    for (int emitter = 0; emitter < 4; ++emitter) {
        for (int i = 0; i < 24; ++i) {
            const int seed = i + emitter * 37;
            const float life = 3.f + Hash(seed, 1) * 7.f;
            const float age = std::fmod(t + Hash(seed, 2) * life, life);
            const float u = age / life;
            const float a = Smoothstep(0.f, .30f, u) * (1.f - Smoothstep(.70f, 1.f, u));
            const float x = origins[emitter][0] + (Hash(seed, 3) - .5f) * 1440.f
                          + std::sin(t * .7f + seed) * 10.f + (Hash(seed, 4) - .5f) * age * 20.f;
            const float y = origins[emitter][1] + (Hash(seed, 5) - .5f) * 1024.f
                          + std::cos(t * .8f + seed) * 10.f + (Hash(seed, 6) - .5f) * age * 20.f;
            const float radius = (5.f + Hash(seed, 7) * 20.f) * 1.5f * cover;
            const float twinkle = .55f + .35f * std::sin(t * (3.f + Hash(seed, 8) * 4.f) + seed);
            Particle(dl, screen(x, y), radius, emitter == 3 ? 0x2252C1 : 0x163FFB,
                     a * twinkle * .62f * (1.f - v.dim)*(1-v.characterMix));
        }
    }
    for (int i = 0; i < 6; ++i) {
        const float life = 3.f + Hash(i, 10) * 2.f;
        const float age = std::fmod(t * 1.2f + Hash(i, 11) * life, life);
        const float u = age / life;
        const float x = 2556.69189f + (Hash(i, 12) - .5f) * 1228.f - age * age * 32.f;
        const float y = 920.54199f + (Hash(i, 13) - .5f) * 614.f - age * 18.f;
        const float a = Smoothstep(0.f, .1f, u) * (1.f - Smoothstep(.65f, 1.f, u));
        Particle(dl, screen(x, y), (7.f + Hash(i, 14) * 12.f) * cover, 0xF4C585,
                 a * .38f * (1.f - v.dim)*(1-v.characterMix));
    }
    dl->PopClipRect();
}

void Background::Shutdown() {
    valid_ = false;
    base_.Release();
    yae_.Release();overlay_.Release();character_.Shutdown();
    for (auto& mask : masks_) mask.Release();
    for (auto& target : targets_) {
        if (target.rtv) { target.rtv->Release(); target.rtv = nullptr; }
        target.image.Release(); target.image.w = target.image.h = 0;
    }
    if (vs_) { vs_->Release(); vs_ = nullptr; }
    if (ps_) { ps_->Release(); ps_ = nullptr; }
    if (cb_) { cb_->Release(); cb_ = nullptr; }
    if (samp_) { samp_->Release(); samp_ = nullptr; }
    if (raster_) { raster_->Release(); raster_ = nullptr; }
}
}
