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
Texture2D yoimiyaImage : register(t10);
Texture2D nahidaImage : register(t11);
Texture2D nahidaWave : register(t12);
Texture2D nahidaShake : register(t13);
Texture2D furinaImage : register(t14);
Texture2D furinaFlow : register(t15);
Texture2D furinaIris : register(t16);
Texture2D furinaSway : register(t17);
Texture2D wallpaperNoise : register(t18);
Texture2D faruzanImage : register(t19);
Texture2D faruzanFlow : register(t20);
Texture2D zhongliImage : register(t21);
SamplerState linearClamp : register(s0);
cbuffer Params : register(b0) {
    float4 frame;
    float4 view;
    float4 source;
    float4 nature;
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
float3 Furina(float2 p){
    float t=frame.z;
    p+=float2(sin(t*.83)+sin(t*.37)*.3,cos(t*.71)+sin(t*.43)*.3)*.00012;
    float noise=wallpaperNoise.SampleLevel(linearClamp,p*.05,0).g;
    float phase=(noise*6.2831853+p.x*10+p.y*5)*.5;
    float4 sines=sin(phase+t*1.5*float4(1,-.16161616,.0083333,-.00019841));
    float4 csines=sin(.4+phase+t*1.5*float4(-.5,.041666666,-.0013888889,.000024801587));
    float amp=.0008*furinaSway.SampleLevel(linearClamp,p,0).r;
    float2 q=p+float2(1.875*dot(sines,amp.xxxx),.5333333*dot(csines,amp.xxxx));
    float clock=t*.5,cell=floor(clock),k=smoothstep(.8,1,-cos(frac(clock)*3.14159265)*.5+.5);
    float2 start=sin(cell*1.9)+sin(cell*2.5+float2(1,2));
    float2 end=sin((cell+1)*1.9)+sin((cell+1)*2.5+float2(1,2));
    float2 gaze=(lerp(start,end,k)+float2(sin(clock),cos(clock))*.5)*.001;
    q+=gaze*furinaIris.SampleLevel(linearClamp,q,0).r;
    q+=sin(t)*.00996*(furinaFlow.SampleLevel(linearClamp,q,0).rg-.498)*2;
    float3 color=furinaImage.SampleLevel(linearClamp,q,0).rgb;
    float3 light=furinaImage.SampleLevel(linearClamp,q,3).rgb;
    color+=light*saturate((max(light.r,max(light.g,light.b))-.75)/.25)*.1;
    return color;
}
float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET {
    if(frame.w>4.5)return yoimiyaImage.SampleLevel(linearClamp,uv,0);
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
    color*=max(0,1-source.z-source.w-nature.x-nature.y-nature.z-nature.w);
    if(source.z > .0001) {
        float3 sakura = yaeImage.SampleLevel(linearClamp,p,0).rgb;
        float shaft = pow(saturate(sin((p.x+p.y*.32)*14+sin(frame.z*.17)*.25)),12);
        sakura += float3(.050,.028,.043)*shaft*(.6+.4*sin(frame.z*.3));
        color += sakura*source.z;
    }
    if(source.w>.0001)color+=yoimiyaImage.SampleLevel(linearClamp,p,0).rgb*source.w;
    if(nature.x>.0001){
        float2 flow=(nahidaShake.SampleLevel(linearClamp,p,0).rg-.498)*2;
        float2 q=p+sin(frame.z)*.00996*flow;
        float mask=nahidaWave.SampleLevel(linearClamp,q,0).r;
        q.x+=sin(frame.z*2.75+q.y*22)*.0064*mask;
        color+=nahidaImage.SampleLevel(linearClamp,q,0).rgb*nature.x;
    }
    if(nature.y>.0001)color+=Furina(p)*nature.y;
    if(nature.z>.0001){
        float2 flow=(faruzanFlow.SampleLevel(linearClamp,p,0).rg-.498)*2;
        float2 q=p+sin(frame.z)*.00996*flow;
        float3 archive=faruzanImage.SampleLevel(linearClamp,q,0).rgb;
        float2 ray=p-float2(.73,-.05);
        float angle=atan2(ray.x,max(.02,ray.y));
        float fan=pow(saturate(.5+.5*sin(angle*24+sin(frame.z*.15)*.18)),18);
        float warm=fan*smoothstep(.03,.25,ray.y)*(1-smoothstep(.6,1.2,length(ray)))*(.6+.4*sin(frame.z*.22+angle*3));
        archive+=float3(.055,.044,.025)*warm;
        color+=archive*nature.z;
    }
    if(nature.w>.0001){
        float aspect=2559.0/1428.0;
        float2 geoCover=outAspect>aspect?float2(1,aspect/outAspect):float2(outAspect/aspect,1);
        float2 q=(uv-.5)*geoCover/max(view.y,.25)+view.zw;
        color+=zhongliImage.SampleLevel(linearClamp,q,0).rgb*nature.w;
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
    yoimiya_=LoadTexture(dev,"tex/yoimiya-bg.jpg");
    nahida_=LoadTexture(dev,"tex/nahida-bg.jpg");
    nahidaWave_=LoadTexture(dev,"tex/nahida-wave.png");
    nahidaShake_=LoadTexture(dev,"tex/nahida-shake.png");
    furina_=LoadTexture(dev,"tex/furina-bg.jpg");furinaFlow_=LoadTexture(dev,"tex/furina-flow.png");
    furinaIris_=LoadTexture(dev,"tex/furina-iris.png");furinaSway_=LoadTexture(dev,"tex/furina-sway.png");
    wallpaperNoise_=LoadTexture(dev,"tex/wallpaper-noise.png");
    faruzan_=LoadTexture(dev,"tex/faruzan-bg.jpg");faruzanFlow_=LoadTexture(dev,"tex/faruzan-flow.png");
    overlay_=LoadTexture(dev,"tex/ganyu-overlay.jpg");
    const char* names[] = {"ganyu-overlay-alpha.png", "ganyu-flow-rg.png", "ganyu-phase.png",
                          "ganyu-shake-opacity.png", "ganyu-wave-a.png", "ganyu-wave-b.png"};
    bool ok = base_.Valid() && yae_.Valid() && overlay_.Valid() && yoimiya_.Valid() && nahida_.Valid() && nahidaWave_.Valid() && nahidaShake_.Valid() && character_.Init(dev,ctx) && yoimiyaCharacter_.Init(dev,ctx,"tex/yoimiya-character.hbc");
    ok=ok&&furina_.Valid()&&furinaFlow_.Valid()&&furinaIris_.Valid()&&furinaSway_.Valid()&&wallpaperNoise_.Valid();
    ok=ok&&faruzan_.Valid()&&faruzanFlow_.Valid();
    ok=ok&&zhongliWallpaper_.Init(dev,ctx,"tex/zhongli-wallpaper.hbw");
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
    cb.ByteWidth = 64; cb.Usage = D3D11_USAGE_DYNAMIC;
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
        targets_[1].rtv && targets_[1].image.Valid() && targets_[2].rtv && targets_[2].image.Valid() && targets_[3].rtv && targets_[3].image.Valid() && targets_[4].rtv && targets_[4].image.Valid()) return true;
    ctx_->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* empty[22]{};
    ctx_->PSSetShaderResources(0, 22, empty);
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
    if(v.yoimiyaMix>.0001f){
        ID3D11ShaderResourceView* empty[11]{};ctx_->PSSetShaderResources(0,11,empty);
        ctx_->OMSetRenderTargets(1,&targets_[3].rtv,nullptr);
        D3D11_VIEWPORT viewport{0,0,float(effectW),float(effectH),0,1};ctx_->RSSetViewports(1,&viewport);
        const float params[]={w,h,t,5,v.dim,v.zoom,v.focusX,v.focusY,float(base_.w)/base_.h,v.sat,v.characterMix,v.yoimiyaMix};
        D3D11_MAPPED_SUBRESOURCE map{};if(FAILED(ctx_->Map(cb_,0,D3D11_MAP_WRITE_DISCARD,0,&map)))return;memcpy(map.pData,params,sizeof(params));ctx_->Unmap(cb_,0);
        ctx_->PSSetShaderResources(10,1,&yoimiya_.srv);ctx_->Draw(3,0);
        yoimiyaCharacter_.Draw(t);
        ctx_->OMSetRenderTargets(0,nullptr,nullptr);ctx_->PSSetShaderResources(0,11,empty);
        ctx_->GenerateMips(targets_[3].image.srv);
        ctx_->IASetInputLayout(nullptr);ctx_->VSSetShader(vs_,nullptr,0);ctx_->PSSetShader(ps_,nullptr,0);ctx_->PSSetConstantBuffers(0,1,&cb_);ctx_->PSSetSamplers(0,1,&samp_);ctx_->OMSetBlendState(nullptr,nullptr,0xffffffff);
    }
    if(v.zhongliMix>.0001f){
        ID3D11ShaderResourceView* empty[22]{};ctx_->PSSetShaderResources(0,22,empty);
        ctx_->OMSetRenderTargets(1,&targets_[4].rtv,nullptr);
        D3D11_VIEWPORT viewport{0,0,float(effectW),float(effectH),0,1};ctx_->RSSetViewports(1,&viewport);
        const float clear[]={0,0,0,1};ctx_->ClearRenderTargetView(targets_[4].rtv,clear);
        zhongliWallpaper_.Draw(t);
        ctx_->OMSetRenderTargets(0,nullptr,nullptr);ctx_->GenerateMips(targets_[4].image.srv);
        ctx_->IASetInputLayout(nullptr);ctx_->VSSetShader(vs_,nullptr,0);ctx_->PSSetShader(ps_,nullptr,0);ctx_->PSSetConstantBuffers(0,1,&cb_);ctx_->PSSetSamplers(0,1,&samp_);ctx_->OMSetBlendState(nullptr,nullptr,0xffffffff);
    }
    for (int pass = v.characterMix+v.yoimiyaMix+v.nahidaMix+v.furinaMix+v.faruzanMix+v.zhongliMix >= .9999f ? 3 : 0; pass < 4; ++pass) {
        const int outIndex = pass % 2;
        ID3D11ShaderResourceView* empty[22]{};
        ctx_->PSSetShaderResources(0, 22, empty);
        ID3D11RenderTargetView* target = pass == 3 ? rtv : targets_[outIndex].rtv;
        ctx_->OMSetRenderTargets(1, &target, nullptr);
        const float tw = pass == 3 ? w : float(effectW);
        const float th = pass == 3 ? h : float(effectH);
        D3D11_VIEWPORT viewport{0, 0, tw, th, 0, 1};
        ctx_->RSSetViewports(1, &viewport);
        float params[16] = {w, h, t, float(pass), v.dim, v.zoom, v.focusX, v.focusY,
                            float(base_.w) / base_.h, v.sat, v.characterMix, v.yoimiyaMix,v.nahidaMix,v.furinaMix,v.faruzanMix,v.zhongliMix};
        D3D11_MAPPED_SUBRESOURCE map{};
        if (FAILED(ctx_->Map(cb_, 0, D3D11_MAP_WRITE_DISCARD, 0, &map))) return;
        memcpy(map.pData, params, sizeof(params)); ctx_->Unmap(cb_, 0);
        ID3D11ShaderResourceView* resources[22] = {
            pass ? targets_[(pass - 1) % 2].image.srv : base_.srv, base_.srv,
            masks_[0].srv, masks_[1].srv, masks_[2].srv, masks_[3].srv, masks_[4].srv, masks_[5].srv,targets_[2].image.srv,overlay_.srv,targets_[3].image.srv,nahida_.srv,nahidaWave_.srv,nahidaShake_.srv,furina_.srv,furinaFlow_.srv,furinaIris_.srv,furinaSway_.srv,wallpaperNoise_.srv,faruzan_.srv,faruzanFlow_.srv,targets_[4].image.srv
        };
        ctx_->PSSetShaderResources(0, 22, resources);
        ctx_->Draw(3, 0);
        if (pass == 2) {
            ctx_->OMSetRenderTargets(0, nullptr, nullptr);
            ctx_->PSSetShaderResources(0, 22, empty);
            ctx_->GenerateMips(targets_[0].image.srv);
        }
    }
    ID3D11ShaderResourceView* empty[22]{};
    ctx_->PSSetShaderResources(0, 22, empty);
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
    if(v.zhongliMix>.001f)for(int i=0;i<95;++i){
        const float depth=.3f+Hash(i,400)*.7f,u=std::fmod(Hash(i,401)+t*(.015f+depth*.018f),1.f);
        const float x=w*(Hash(i,402)+sinf(t*.12f+i)*.012f),y=h*(1.08f-u*1.2f);
        const float a=Smoothstep(0,.2f,u)*(1-Smoothstep(.8f,1,u))*v.zhongliMix*(1-v.dim)*(.14f+depth*.35f)*(.7f+.3f*sinf(t*2+i));
        const float r=(.7f+depth*1.2f)*h/600;Particle(dl,{x,y},r*6,0xFFBF50,a*.26f);
        dl->AddQuadFilled({x,y-r*1.7f},{x+r,y},{x,y+r*1.7f},{x-r,y},IM_COL32(255,221,151,int(a*255)));
    }
    if(v.faruzanMix>.001f)for(int i=0;i<76;++i){
        const float life=3+Hash(i,101)*7,age=std::fmod(t+Hash(i,102)*life,life),u=age/life;
        const float depth=.25f+Hash(i,103)*.75f,r=(.8f+depth*2.1f)*w/1600;
        const ImVec2 p{w*(Hash(i,104)+sinf(t*.55f+i)*.006f+age*.003f),h*(Hash(i,105)+cosf(t*.6f+i)*.008f-age*.002f)};
        const float a=Smoothstep(0,.3f,u)*(1-Smoothstep(.7f,1,u))*v.faruzanMix*(1-v.dim)*(.16f+depth*.24f)*(.7f+.3f*sinf(t*(3+Hash(i,106)*4)+i));
        Particle(dl,p,r*4,0xE8DCA9,a*.25f);dl->AddCircleFilled(p,std::max(.55f,r*.45f),IM_COL32(255,247,198,int(a*255)),8);
    }
    if(v.furinaMix>.001f)for(int i=0;i<22;++i){
        const float life=12+Hash(i,81)*14,u=std::fmod(t+Hash(i,82)*life,life)/life;
        const float depth=.3f+.7f*Hash(i,83),r=(1+depth*3.3f)*w/1600;
        const ImVec2 p{w*(Hash(i,84)+sinf(t*.3f+i)*.012f),h*(1.06f-u*1.18f)};
        const float a=Smoothstep(0,.1f,u)*(1-Smoothstep(.88f,1,u))*v.furinaMix*(1-v.dim)*(.13f+.17f*depth);
        Particle(dl,p,r*3,0x70D4F3,a*.18f);
        dl->AddCircle(p,r,IM_COL32(170,236,252,int(a*255)),20,std::max(.55f,r*.14f));
        dl->AddCircleFilled({p.x-r*.32f,p.y-r*.38f},std::max(.5f,r*.22f),IM_COL32(235,255,255,int(a*255)),8);
    }
    if(v.nahidaMix>.001f){
        for(int i=0;i<72;++i){
            const float life=10.f+Hash(i,61)*15.f,age=std::fmod(t*.8f+Hash(i,62)*life,life),u=age/life;
            const float depth=.25f+.75f*Hash(i,63),x=(1.08f-u*1.2f)*w+sinf(age*.85f+i)*w*.012f;
            const float y=Hash(i,64)*h+age*h*.008f+cosf(age*.7f+i)*h*.012f;
            const float a=Smoothstep(0,.1f,u)*(1-Smoothstep(.88f,1,u))*v.nahidaMix*(1-v.dim)*(.18f+.28f*depth);
            const float r=(1+depth*4)*w/1600;
            Particle(dl,{x,y},r*(i%4?1.8f:4.f),0xF0FFCE,a*.5f);
            if(i%4)dl->AddCircleFilled({x,y},std::max(.6f,r*.38f),IM_COL32(245,255,214,int(a*255)),8);
        }
    }
    if(v.yoimiyaMix>.001f) {
        for(int i=0;i<34;++i){
            const float life=6.f+Hash(i,51)*7.f,age=std::fmod(t+Hash(i,52)*life,life),u=age/life;
            const float x=Hash(i,53)*w+sinf(t*.55f+i)*w*.008f,y=(1.08f-u*1.2f)*h;
            const float a=Smoothstep(0,.1f,u)*(1-Smoothstep(.8f,1,u))*v.yoimiyaMix*(1-v.dim)*(.2f+.3f*Hash(i,54));
            const float z=(.7f+Hash(i,55))*w/1600.f;
            Particle(dl,{x,y},z*7,0xF9874B,a*.4f);
            dl->AddLine({x,y},{x+z*.4f,y+z*(2+Hash(i,56)*3)},IM_COL32(255,220,167,int(a*255)),z);
        }
    }
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
                     a * twinkle * .62f * (1.f - v.dim)*std::max(0.f,1-v.characterMix-v.yoimiyaMix-v.nahidaMix-v.furinaMix-v.faruzanMix-v.zhongliMix));
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
                 a * .38f * (1.f - v.dim)*std::max(0.f,1-v.characterMix-v.yoimiyaMix-v.nahidaMix-v.furinaMix-v.faruzanMix-v.zhongliMix));
    }
    dl->PopClipRect();
}

void Background::Shutdown() {
    valid_ = false;
    base_.Release();
    yae_.Release();overlay_.Release();character_.Shutdown();yoimiya_.Release();yoimiyaCharacter_.Shutdown();
    nahida_.Release();nahidaWave_.Release();nahidaShake_.Release();
    furina_.Release();furinaFlow_.Release();furinaIris_.Release();furinaSway_.Release();wallpaperNoise_.Release();
    faruzan_.Release();faruzanFlow_.Release();
    zhongliWallpaper_.Shutdown();
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
