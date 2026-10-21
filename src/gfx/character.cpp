#include "gfx/character.h"
#include "assets/embedded.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace hb {
namespace {
template<class T>void Release(T*& value){if(value){value->Release();value=nullptr;}}
struct Reader {
    const uint8_t* p;size_t remaining;
    const void* Bytes(size_t count){if(count>remaining)return nullptr;const void* result=p;p+=count;remaining-=count;return result;}
    template<class T>bool Read(T& value){const void* data=Bytes(sizeof(T));if(!data)return false;memcpy(&value,data,sizeof(T));return true;}
};
constexpr UINT maxVertices=4096;
const char* shader=R"(
cbuffer Params:register(b0){float4 frame;float4 wave;float4 size;};
Texture2D image:register(t0);Texture2D masks:register(t1);SamplerState sampleBorder:register(s0);
struct V{float2 p:POSITION;float2 uv:TEXCOORD0;};
struct P{float4 p:SV_POSITION;float2 uv:TEXCOORD0;};
P VS(V i){P o;o.p=float4(i.p.x/3840*2-1,1-i.p.y/2160*2,0,1);o.uv=i.uv;return o;}
float Hash(float2 p){return frac(sin(dot(p,float2(127.1,311.7)))*43758.5453);}
float Noise(float2 p){float2 i=floor(p),f=frac(p);f=f*f*(3-2*f);return lerp(lerp(Hash(i),Hash(i+float2(1,0)),f.x),lerp(Hash(i+float2(0,1)),Hash(i+1),f.x),f.y);}
float4 PS(P i):SV_TARGET{
    float t=frame.z;float4 mask=masks.Sample(sampleBorder,i.uv);
    float2 direction=float2(-sin(wave.x),cos(wave.x));
    float phase=t*wave.z+dot(i.uv,direction)*wave.y;
    float2 uv=i.uv+sin(phase)*float2(direction.y,-direction.x)*wave.w*wave.w*mask.r;
    if(frame.w>.5&&frame.w<1.5){
        float clock=t*.75,cell=floor(clock),k=smoothstep(.8,1,-cos(frac(clock)*3.14159265)*.5+.5);
        float2 start=sin(1.9*cell)+sin(2.5*cell+float2(1,2));
        float2 end=sin(1.9*(cell+1))+sin(2.5*(cell+1)+float2(1,2));
        float2 gaze=(lerp(start,end,k)+float2(sin(clock),cos(clock))*.5)*float2(2.5,.5)*.001;
        uv+=gaze*mask.g;
        uv+=float2(sin(uv.y*80+t*.15),cos(uv.x*80-t*.15))*.00035*mask.b;
    }
    float4 color=image.Sample(sampleBorder,uv);
    if(frame.w>.5&&frame.w<1.5)color.rgb*=1+sin(t)*.10*mask.b;
    if(frame.w>1.5){float flame=Noise(uv*float2(10,20)+float2(-t*.05,t*.35));color.rgb+=float3(.19,.045,.095)*smoothstep(.3,.75,flame)*mask.a;}
    return color;
}
)";
}
bool Character::Init(ID3D11Device* dev,ID3D11DeviceContext* context){
    context_=context;auto asset=LoadAsset("tex/yae-character.hbc");
    if(!asset.Valid())return false;
    Reader reader{static_cast<const uint8_t*>(asset.data),asset.size};const auto* magic=reader.Bytes(8);UINT count=0;
    if(!magic||memcmp(magic,"HBC0001",8)||!reader.Read(count)||!reader.Read(period_)||!reader.Read(frameCount_)||count>64||frameCount_<2||frameCount_>2048||!std::isfinite(period_)||period_<=0)return false;
    for(UINT i=0;i<count;++i){
        layers_.emplace_back();auto& layer=layers_.back();UINT vertexCount=0,pngSize=0,maskSize=0;
        if(!reader.Read(layer.id)||!reader.Read(vertexCount)||!reader.Read(layer.indexCount)||!reader.Read(pngSize)||!reader.Read(maskSize)||!reader.Read(layer.kind)||!reader.Read(layer.wave)||vertexCount>maxVertices||!vertexCount||layer.indexCount>65535){Shutdown();return false;}
        const auto* png=reader.Bytes(pngSize);const auto* mask=reader.Bytes(maskSize);
        layer.image=LoadTexture(dev,png,pngSize);layer.mask=LoadTexture(dev,mask,maskSize);
        const auto* base=reader.Bytes(vertexCount*sizeof(Vertex));const auto* indices=reader.Bytes(layer.indexCount*sizeof(uint16_t));
        const size_t poseSize=size_t(frameCount_)*vertexCount*2;const auto* poses=reader.Bytes(poseSize*sizeof(int16_t));
        if(!layer.image.Valid()||!layer.mask.Valid()||!base||!indices||!poses){Shutdown();return false;}
        layer.base.resize(vertexCount);memcpy(layer.base.data(),base,vertexCount*sizeof(Vertex));layer.poses.resize(poseSize);memcpy(layer.poses.data(),poses,poseSize*sizeof(int16_t));
        for(UINT j=0;j<layer.indexCount;++j){uint16_t index=0;memcpy(&index,static_cast<const uint8_t*>(indices)+j*2,2);if(index>=vertexCount){Shutdown();return false;}}
        D3D11_BUFFER_DESC buffer{};buffer.ByteWidth=layer.indexCount*sizeof(uint16_t);buffer.Usage=D3D11_USAGE_IMMUTABLE;buffer.BindFlags=D3D11_BIND_INDEX_BUFFER;D3D11_SUBRESOURCE_DATA init{indices};
        if(FAILED(dev->CreateBuffer(&buffer,&init,&layer.indices))){Shutdown();return false;}
    }
    ID3DBlob *vs=nullptr,*ps=nullptr,*errors=nullptr;
    auto compile=[&](const char* entry,const char* profile,ID3DBlob** result){auto hr=D3DCompile(shader,strlen(shader),"character",nullptr,nullptr,entry,profile,D3DCOMPILE_OPTIMIZATION_LEVEL3,0,result,&errors);if(errors){printf("[character] %s\n",static_cast<const char*>(errors->GetBufferPointer()));Release(errors);}return SUCCEEDED(hr);};
    if(!compile("VS","vs_5_0",&vs)||!compile("PS","ps_5_0",&ps)){Release(vs);Release(ps);Shutdown();return false;}
    bool ok=SUCCEEDED(dev->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vs_));ok&=SUCCEEDED(dev->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&ps_));
    D3D11_INPUT_ELEMENT_DESC elements[]={{"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,D3D11_INPUT_PER_VERTEX_DATA,0}};
    ok&=SUCCEEDED(dev->CreateInputLayout(elements,2,vs->GetBufferPointer(),vs->GetBufferSize(),&layout_));Release(vs);Release(ps);
    D3D11_BUFFER_DESC buffer{};buffer.ByteWidth=maxVertices*sizeof(Vertex);buffer.Usage=D3D11_USAGE_DYNAMIC;buffer.BindFlags=D3D11_BIND_VERTEX_BUFFER;buffer.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;ok&=SUCCEEDED(dev->CreateBuffer(&buffer,nullptr,&vertices_));buffer.ByteWidth=48;buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;ok&=SUCCEEDED(dev->CreateBuffer(&buffer,nullptr,&params_));
    D3D11_SAMPLER_DESC sampler{};sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_BORDER;sampler.MaxLOD=D3D11_FLOAT32_MAX;ok&=SUCCEEDED(dev->CreateSamplerState(&sampler,&sampler_));
    D3D11_BLEND_DESC blend{};auto& rt=blend.RenderTarget[0];rt.BlendEnable=TRUE;rt.SrcBlend=D3D11_BLEND_SRC_ALPHA;rt.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;rt.BlendOp=rt.BlendOpAlpha=D3D11_BLEND_OP_ADD;rt.SrcBlendAlpha=D3D11_BLEND_ONE;rt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;rt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;ok&=SUCCEEDED(dev->CreateBlendState(&blend,&blend_));
    printf("[character] source puppet: %zu layers, %u poses, %.1fs loop %s\n",layers_.size(),frameCount_,period_,ok?"PASS":"FAIL");if(!ok)Shutdown();return ok;
}
void Character::Draw(float time){
    if(!ps_)return;
    const float at=std::fmod(std::max(time,0.f),period_)/period_*(frameCount_-1),k=at-floorf(at);const size_t a=size_t(at),b=std::min(a+1,size_t(frameCount_-1));
    const UINT stride=sizeof(Vertex),offset=0;context_->IASetInputLayout(layout_);context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);context_->IASetVertexBuffers(0,1,&vertices_,&stride,&offset);
    context_->VSSetShader(vs_,nullptr,0);context_->PSSetShader(ps_,nullptr,0);context_->PSSetConstantBuffers(0,1,&params_);context_->PSSetSamplers(0,1,&sampler_);context_->OMSetBlendState(blend_,nullptr,0xffffffff);
    for(const auto& layer:layers_){
        D3D11_MAPPED_SUBRESOURCE mapped{};if(FAILED(context_->Map(vertices_,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))continue;
        auto* vertices=static_cast<Vertex*>(mapped.pData);const size_t n=layer.base.size(),startA=a*n*2,startB=b*n*2;
        for(size_t j=0;j<n;++j){vertices[j]=layer.base[j];vertices[j].x+=(layer.poses[startA+j*2]*(1-k)+layer.poses[startB+j*2]*k)/32;vertices[j].y+=(layer.poses[startA+j*2+1]*(1-k)+layer.poses[startB+j*2+1]*k)/32;}
        context_->Unmap(vertices_,0);
        const float params[]={3840,2160,time,float(layer.kind),layer.wave[0],layer.wave[1],layer.wave[2],layer.wave[3],float(layer.image.w),float(layer.image.h),0,0};
        if(FAILED(context_->Map(params_,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))continue;memcpy(mapped.pData,params,sizeof(params));context_->Unmap(params_,0);
        ID3D11ShaderResourceView* textures[]={layer.image.srv,layer.mask.srv};context_->PSSetShaderResources(0,2,textures);context_->IASetIndexBuffer(layer.indices,DXGI_FORMAT_R16_UINT,0);context_->DrawIndexed(layer.indexCount,0,0);
    }
    ID3D11ShaderResourceView* none[2]{};context_->PSSetShaderResources(0,2,none);
}
void Character::Shutdown(){for(auto& layer:layers_){layer.image.Release();layer.mask.Release();Release(layer.indices);}layers_.clear();Release(vs_);Release(ps_);Release(layout_);Release(vertices_);Release(params_);Release(sampler_);Release(blend_);}
}
