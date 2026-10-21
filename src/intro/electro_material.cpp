#include "intro/electro_material.h"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace hb {
namespace {
template<class T>void Release(T*& p){if(p){p->Release();p=nullptr;}}
constexpr UINT kMaxVertices=16000;
const char* shader=R"(
cbuffer Params:register(b0){float4 frame;float4 mark;float4 state;};
Texture2D scene:register(t0);
SamplerState linearClamp:register(s0);
struct Vertex{float2 p:POSITION;float3 n:NORMAL;float2 local:TEXCOORD0;float2 surface:TEXCOORD1;float thickness:TEXCOORD2;float seed:TEXCOORD3;};
struct Pixel{float4 p:SV_POSITION;float3 n:NORMAL;float2 local:TEXCOORD0;float2 surface:TEXCOORD1;float thickness:TEXCOORD2;nointerpolation float seed:TEXCOORD3;};
Pixel VS(Vertex v){Pixel o;float2 p=(v.p-state.zw)*mark.w;o.p=float4(p.x/frame.x*2-1,1-p.y/frame.y*2,0,1);o.n=v.n;o.local=v.local;o.surface=v.surface;o.thickness=v.thickness;o.seed=v.seed;return o;}
float Hash(float2 p){return frac(sin(dot(p,float2(127.1,311.7)))*43758.5453);}
float Noise(float2 p){float2 i=floor(p),f=frac(p);f=f*f*(3-2*f);return lerp(lerp(Hash(i),Hash(i+float2(1,0)),f.x),lerp(Hash(i+float2(0,1)),Hash(i+1),f.x),f.y);}
float4 PS(Pixel i):SV_TARGET{
    float t=frame.w;
    float2 p=i.local;
    float radius=length(p),a=atan2(p.y,p.x);
    float propagation=radius*(1+.17*sin(a*3+radius*4));
    float growth=smoothstep(.42,2.85,t)*1.3;
    float reveal=1-smoothstep(growth-.12,growth+.10,propagation);
    if(state.y>.5)return float4(1,1,1,reveal);
    float front=step(.25,i.surface.x)*(1-step(.75,i.surface.x));
    float bevel=step(.75,i.surface.x);
    float side=1-step(.25,i.surface.x);
    float3 n=normalize(i.n);
    if(n.z<0)n=-n;
    float grain=Noise(p*float2(8,11));
    float3 reference=pow(float3(.981,.901,.981),2.2);
    float3 color=reference*(.99+.012*grain+.006*sin(p.x*2-p.y*3));
    float charged=smoothstep(1.75,3.15,t);
    float3 violet=pow(float3(.52,.20,.67),2.2);
    color=lerp(violet,color,charged);
    float2 uv=i.p.xy/frame.xy;
    float2 bend=n.xy*mark.z/frame.xy*.09;
    float3 transmitted=pow(max(scene.Sample(linearClamp,uv+bend).rgb,.001),2.2);
    color=lerp(color,color*(.94+transmitted*.06),.06*front);
    float3 light=normalize(float3(-.45,-.6,.85));
    float specular=pow(saturate(dot(n,light)),12);
    color*=1-side*.24;
    color=lerp(color,pow(float3(.78,.45,.88),2.2),bevel*.30);
    color+=float3(.14,.07,.16)*specular*(front*.16+bevel*.65);
    float warp=Noise(p*15+float2(t*.04,0))-.5;
    float path=sin(a*3-radius*7+sin(radius*19+t*.8)*.13+warp*.65);
    float filament=exp(-pow(path/.090,2))*smoothstep(.035,.12,radius);
    float branch=exp(-pow(sin(a*6+radius*11+warp*1.2)/.038,2));
    branch*=smoothstep(.26,.42,radius)*(1-smoothstep(.65,.9,radius));
    float pulse=pow(saturate(.5+.5*sin(radius*16-t*4.2+a*.35)),10);
    float awake=smoothstep(.28,.85,t);
    float veins=saturate(filament*.66+branch*.24)*awake*front;
    color=lerp(color,pow(float3(.72,.36,.84),2.2),veins*(.38+.45*pulse));
    float head=exp(-pow((radius-frac(t*.30+a*.06)*1.35)/.040,2));
    color+=float3(.60,.34,.70)*veins*head;
    color+=float3(.92,.40,1.05)*veins*(1-charged)*(.8+pulse*2.0);
    float tip=exp(-pow((propagation-growth)/.09,2))*(1-smoothstep(2.7,3.2,t));
    color+=float3(.85,.40,1.0)*tip;
    float sweep=exp(-pow((dot(p,float2(.73,-.68))-lerp(-1.3,1.3,saturate((t-3.45)/1.15)))/.10,2));
    sweep*=smoothstep(3.4,3.7,t)*(1-smoothstep(4.3,4.65,t));
    color+=float3(.09,.05,.11)*sweep;
    float opacity=reveal*i.surface.y*state.x;
    return float4(pow(saturate(color),1/2.2),opacity);
}
struct GlowPixel{float4 p:SV_POSITION;float2 uv:TEXCOORD0;};
GlowPixel GlowVS(uint id:SV_VertexID){
    GlowPixel o;o.uv=float2((id<<1)&2,id&2);
    if(state.y>3.5){float2 p=mark.xy+(o.uv-.5)*512/mark.w;o.p=float4(p.x/frame.x*2-1,1-p.y/frame.y*2,0,1);}
    else o.p=float4(o.uv.x*2-1,1-o.uv.y*2,0,1);
    return o;
}
float4 GlowPS(GlowPixel i):SV_TARGET{
    if(state.y>3.5){
        float mask=scene.Sample(linearClamp,i.uv).r;
        float3 purple=lerp(float3(.68,.24,.73),float3(.92,.52,.94),saturate(mask*1.3));
        float ceremony=1-smoothstep(4.2,5.1,frame.w);
        float breathe=.94+.06*sin(frame.w*2.1)+ceremony*.18*pow(.5+.5*sin(frame.w*5.4),4);
        return float4(purple,(1-exp(-mask*3.2))*state.x*breathe);
    }
    float2 direction=state.y<2.5?float2(1,0):float2(0,1);
    float sigma=max(.8,mark.z*.052*mark.w);
    float result=0,weight=0;
    [unroll]for(int j=-6;j<=6;++j){float x=j*.5,w=exp(-x*x*.5);result+=scene.Sample(linearClamp,i.uv+direction*x*sigma/512).r*w;weight+=w;}
    return result/weight;
}
)";
}
bool ElectroMaterial::Init(ID3D11Device* dev,ID3D11DeviceContext* ctx){
    device_=dev;context_=ctx;ID3DBlob *vs=nullptr,*ps=nullptr,*errors=nullptr;
    auto compile=[&](const char* entry,const char* profile,ID3DBlob** code){HRESULT hr=D3DCompile(shader,strlen(shader),"electro-crystal",nullptr,nullptr,entry,profile,D3DCOMPILE_OPTIMIZATION_LEVEL3,0,code,&errors);if(errors){printf("[electro] %s\n",(const char*)errors->GetBufferPointer());Release(errors);}return SUCCEEDED(hr);};
    if(!compile("VS","vs_5_0",&vs)||!compile("PS","ps_5_0",&ps)){Release(vs);Release(ps);Shutdown();return false;}
    bool ok=SUCCEEDED(dev->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vs_));
    ok&=SUCCEEDED(dev->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&ps_));
    const D3D11_INPUT_ELEMENT_DESC elements[]={
        {"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,8,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",1,DXGI_FORMAT_R32G32_FLOAT,0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",2,DXGI_FORMAT_R32_FLOAT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",3,DXGI_FORMAT_R32_FLOAT,0,40,D3D11_INPUT_PER_VERTEX_DATA,0}};
    ok&=SUCCEEDED(dev->CreateInputLayout(elements,6,vs->GetBufferPointer(),vs->GetBufferSize(),&layout_));Release(vs);Release(ps);
    if(!compile("GlowVS","vs_5_0",&vs)||!compile("GlowPS","ps_5_0",&ps)){Release(vs);Release(ps);Shutdown();return false;}
    ok&=SUCCEEDED(dev->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&glowVs_));
    ok&=SUCCEEDED(dev->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&glowPs_));Release(vs);Release(ps);
    D3D11_TEXTURE2D_DESC glow{};glow.Width=glow.Height=512;glow.MipLevels=glow.ArraySize=1;glow.Format=DXGI_FORMAT_R8G8B8A8_UNORM;glow.SampleDesc.Count=1;glow.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
    for(int j=0;j<2;++j){
        ok&=SUCCEEDED(dev->CreateTexture2D(&glow,nullptr,&glowTextures_[j]));
        if(glowTextures_[j]){ok&=SUCCEEDED(dev->CreateRenderTargetView(glowTextures_[j],nullptr,&glowTargets_[j]));ok&=SUCCEEDED(dev->CreateShaderResourceView(glowTextures_[j],nullptr,&glowViews_[j]));}
    }
    D3D11_BUFFER_DESC buffer{};buffer.ByteWidth=kMaxVertices*sizeof(CrystalVertex);buffer.Usage=D3D11_USAGE_DYNAMIC;buffer.BindFlags=D3D11_BIND_VERTEX_BUFFER;buffer.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    ok&=SUCCEEDED(dev->CreateBuffer(&buffer,nullptr,&vertices_));buffer.ByteWidth=48;buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;ok&=SUCCEEDED(dev->CreateBuffer(&buffer,nullptr,&params_));
    D3D11_SAMPLER_DESC sampler{};sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;sampler.MaxLOD=D3D11_FLOAT32_MAX;ok&=SUCCEEDED(dev->CreateSamplerState(&sampler,&sampler_));
    D3D11_RASTERIZER_DESC raster{};raster.FillMode=D3D11_FILL_SOLID;raster.CullMode=D3D11_CULL_NONE;raster.DepthClipEnable=TRUE;raster.MultisampleEnable=TRUE;ok&=SUCCEEDED(dev->CreateRasterizerState(&raster,&raster_));
    D3D11_BLEND_DESC blend{};auto& rt=blend.RenderTarget[0];rt.BlendEnable=TRUE;rt.SrcBlend=D3D11_BLEND_SRC_ALPHA;rt.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;rt.BlendOp=rt.BlendOpAlpha=D3D11_BLEND_OP_ADD;rt.SrcBlendAlpha=D3D11_BLEND_ONE;rt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;rt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    ok&=SUCCEEDED(dev->CreateBlendState(&blend,&blend_));D3D11_DEPTH_STENCIL_DESC depth{};depth.DepthFunc=D3D11_COMPARISON_ALWAYS;ok&=SUCCEEDED(dev->CreateDepthStencilState(&depth,&depth_));
    printf("[electro] vector mesh, purple current, Gaussian bloom: %s\n",ok?"ok":"FAILED");if(!ok)Shutdown();return ok;
}
void ElectroMaterial::Shutdown(){Release(vs_);Release(ps_);Release(glowVs_);Release(glowPs_);for(int j=0;j<2;++j){Release(glowViews_[j]);Release(glowTargets_[j]);Release(glowTextures_[j]);}Release(layout_);Release(vertices_);Release(params_);Release(sampler_);Release(raster_);Release(blend_);Release(depth_);Release(sceneView_);Release(scene_);requests_.clear();width_=height_=0;}
bool ElectroMaterial::Snapshot(ID3D11Texture2D* texture){
    D3D11_TEXTURE2D_DESC d{};texture->GetDesc(&d);
    if(width_!=d.Width||height_!=d.Height||!sceneView_){Release(sceneView_);Release(scene_);width_=d.Width;height_=d.Height;auto copy=d;copy.SampleDesc={1,0};copy.BindFlags=D3D11_BIND_SHADER_RESOURCE;copy.Usage=D3D11_USAGE_DEFAULT;copy.CPUAccessFlags=copy.MiscFlags=0;if(FAILED(device_->CreateTexture2D(&copy,nullptr,&scene_))||FAILED(device_->CreateShaderResourceView(scene_,nullptr,&sceneView_)))return false;}
    if(d.SampleDesc.Count>1)context_->ResolveSubresource(scene_,0,texture,0,d.Format);else context_->CopyResource(scene_,texture);return true;
}
void ElectroMaterial::Queue(ImDrawList* dl,std::vector<CrystalVertex>&& vertices,ImVec2 center,float radius,float time,float alpha){
    if(!ps_||vertices.empty()||alpha<.001f)return;
    requests_.push_back({this,std::move(vertices),center,radius,time,alpha});
    dl->AddCallback(Callback,&requests_.back());dl->AddCallback(ImDrawCallback_ResetRenderState,nullptr);
}
void ElectroMaterial::Callback(const ImDrawList*,const ImDrawCmd* cmd){const auto& request=*static_cast<const Request*>(cmd->UserCallbackData);request.owner->Render(request);}
bool ElectroMaterial::SetParams(const float* values){
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(context_->Map(params_,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return false;
    memcpy(mapped.pData,values,48);context_->Unmap(params_,0);return true;
}
void ElectroMaterial::DrawGlow(const Request& r,ID3D11RenderTargetView* target,ID3D11DepthStencilView* depth){
    D3D11_VIEWPORT oldViewport{};UINT count=1;context_->RSGetViewports(&count,&oldViewport);
    D3D11_VIEWPORT viewport{0,0,512,512,0,1};context_->RSSetViewports(1,&viewport);
    ID3D11ShaderResourceView* none=nullptr;context_->PSSetShaderResources(0,1,&none);
    context_->OMSetRenderTargets(1,&glowTargets_[0],nullptr);
    const float clear[]={0,0,0,0};context_->ClearRenderTargetView(glowTargets_[0],clear);
    const float scale=std::min(1.f,180.f/std::max(r.radius,1.f));
    float params[]={512,512,r.radius,r.time,r.center.x,r.center.y,r.radius,scale,1,1,r.center.x-256/scale,r.center.y-256/scale};
    if(SetParams(params))context_->Draw(UINT(r.vertices.size()),0);
    context_->IASetInputLayout(nullptr);context_->VSSetShader(glowVs_,nullptr,0);context_->PSSetShader(glowPs_,nullptr,0);
    context_->OMSetBlendState(nullptr,nullptr,0xffffffff);
    params[10]=params[11]=0;
    for(int pass=0;pass<2;++pass){
        context_->PSSetShaderResources(0,1,&none);
        context_->OMSetRenderTargets(1,&glowTargets_[1-pass],nullptr);
        context_->PSSetShaderResources(0,1,&glowViews_[pass]);
        params[9]=float(2+pass);
        if(SetParams(params))context_->Draw(3,0);
    }
    context_->PSSetShaderResources(0,1,&none);
    context_->OMSetRenderTargets(1,&target,depth);context_->RSSetViewports(1,&oldViewport);
    params[0]=float(width_);params[1]=float(height_);params[8]=r.opacity;params[9]=4;
    context_->OMSetBlendState(blend_,nullptr,0xffffffff);
    context_->PSSetShaderResources(0,1,&glowViews_[0]);
    if(SetParams(params))context_->Draw(3,0);
    context_->PSSetShaderResources(0,1,&none);
}
void ElectroMaterial::Render(const Request& r){
    if(r.vertices.size()>kMaxVertices)return;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(context_->Map(vertices_,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return;
    memcpy(mapped.pData,r.vertices.data(),r.vertices.size()*sizeof(CrystalVertex));context_->Unmap(vertices_,0);
    ID3D11RenderTargetView* target=nullptr;ID3D11DepthStencilView* depth=nullptr;context_->OMGetRenderTargets(1,&target,&depth);
    if(!target){Release(depth);return;}
    ID3D11Resource* resource=nullptr;ID3D11Texture2D* texture=nullptr;target->GetResource(&resource);resource->QueryInterface(IID_PPV_ARGS(&texture));Release(resource);
    context_->OMSetRenderTargets(0,nullptr,nullptr);bool ready=texture&&Snapshot(texture);context_->OMSetRenderTargets(1,&target,depth);Release(texture);
    if(!ready){Release(target);Release(depth);return;}
    UINT stride=sizeof(CrystalVertex),offset=0;
    context_->IASetInputLayout(layout_);context_->IASetVertexBuffers(0,1,&vertices_,&stride,&offset);context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vs_,nullptr,0);context_->VSSetConstantBuffers(0,1,&params_);context_->PSSetShader(ps_,nullptr,0);context_->PSSetConstantBuffers(0,1,&params_);
    context_->PSSetSamplers(0,1,&sampler_);context_->RSSetState(raster_);context_->OMSetDepthStencilState(depth_,0);context_->OMSetBlendState(blend_,nullptr,0xffffffff);
    DrawGlow(r,target,depth);Release(target);Release(depth);
    context_->IASetInputLayout(layout_);context_->VSSetShader(vs_,nullptr,0);context_->PSSetShader(ps_,nullptr,0);
    context_->PSSetShaderResources(0,1,&sceneView_);
    const float params[]={float(width_),float(height_),r.radius,r.time,r.center.x,r.center.y,r.radius,1,r.opacity,0,0,0};
    if(SetParams(params))context_->Draw(UINT(r.vertices.size()),0);
    ID3D11ShaderResourceView* none=nullptr;context_->PSSetShaderResources(0,1,&none);
}
}
