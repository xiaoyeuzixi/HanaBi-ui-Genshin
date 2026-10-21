#include "gfx/layered_wallpaper.h"
#include "assets/embedded.h"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace hb {
namespace {
template<class T>void Release(T*& p){if(p){p->Release();p=nullptr;}}
const char* shader=R"(
cbuffer Params:register(b0){float4 frame;float4 rect;float4 ripple;float4 spin;float4 wave[8];};
Texture2D image:register(t0);Texture2D masks:register(t1);Texture2D noise:register(t2);Texture2D normals:register(t3);
SamplerState borderSample:register(s0);SamplerState repeatSample:register(s1);SamplerState clampSample:register(s2);
struct P{float4 p:SV_POSITION;float2 uv:TEXCOORD0;};
P VS(uint id:SV_VertexID){P o;o.uv=float2((id<<1)&2,id&2);float2 p=(rect.xy+o.uv*rect.zw)/float2(2559,1428);o.p=float4(p.x*2-1,1-p.y*2,0,1);return o;}
float2 Rotate(float2 p,float angle){float c=cos(angle),s=sin(angle);return float2(c*p.x-s*p.y,s*p.x+c*p.y);}
float4 PS(P i):SV_TARGET{
    if(any(i.uv<0)||any(i.uv>1))discard;
    float2 q=i.uv;float t=frame.z;
    if(abs(ripple.w)>.000001){
        float2 a=(q+t*ripple.x)*ripple.y,b=(q*1.333-t*ripple.x)*ripple.y;
        a.y*=ripple.z;b.y*=ripple.z;
        float3 n1=normals.Sample(repeatSample,a).xyz*2-1,n2=normals.Sample(repeatSample,b).xyz*2-1;
        float3 n=normalize(float3(n1.xy+n2.xy,n1.z));
        float mask=ripple.w<0?masks.Sample(borderSample,q).r:1;
        q+=n.xy*abs(ripple.w)*mask;
    }
    if(abs(spin.w)>.0001){
        float aspect=rect.z/rect.w;float2 p=(q-spin.xy)*float2(aspect,1);
        float k=1-smoothstep(spin.z-.002,spin.z+.002,length(p));
        q=lerp(q,Rotate(p,t*spin.w)/float2(aspect,1)+spin.xy,k);
    }
    [unroll]for(int j=3;j>=0;--j){
        float4 a=wave[j*2],b=wave[j*2+1];
        if(b.w>.5){
            float2 rotated=Rotate(q,b.z);
            float phase=(noise.Sample(repeatSample,q*a.w).g*6.2831853+rotated.x*10+rotated.y*5)*a.z;
            float4 sn=sin(phase+t*a.x*float4(1,-.16161616,.0083333,-.00019841));
            float4 cs=sin(.4+phase+t*a.x*float4(-.5,.041666666,-.0013888889,.000024801587));
            float mask=masks.Sample(borderSample,q)[j];
            q+=b.xy*float2(dot(sn,1),dot(cs,1))*a.y*mask;
        }
    }
    if(ripple.w>0)return image.Sample(clampSample,q);
    return image.Sample(borderSample,q);
}
)";
}
bool LayeredWallpaper::Init(ID3D11Device* dev,ID3D11DeviceContext* ctx,const char* path){
    context_=ctx;const auto blob=LoadAsset(path);if(!blob.Valid())return false;
    const auto* p=static_cast<const uint8_t*>(blob.data);size_t left=blob.size;
    auto bytes=[&](size_t n)->const uint8_t*{if(n>left)return nullptr;const auto* b=p;p+=n;left-=n;return b;};
    auto read=[&](void* value,size_t n){const auto* b=bytes(n);if(!b)return false;memcpy(value,b,n);return true;};
    const auto* magic=bytes(8);UINT count=0,sizes[2]{};
    if(!magic||memcmp(magic,"HBW0001",8)||!read(&count,4)||!read(sizes,8)||count>64||!count)return false;
    for(int j=0;j<2;++j){const auto* b=bytes(sizes[j]);if(!b){Shutdown();return false;}(j?normal_:noise_)=LoadTexture(dev,b,sizes[j]);}
    for(UINT j=0;j<count;++j){
        UINT header[3]{};layers_.emplace_back();auto& layer=layers_.back();
        if(!read(header,12)||!read(layer.params.data(),sizeof(layer.params))){Shutdown();return false;}
        for(float f:layer.params)if(!std::isfinite(f)){Shutdown();return false;}
        const auto* a=bytes(header[1]);const auto* b=bytes(header[2]);
        if(!a||!b){Shutdown();return false;}
        layer.image=LoadTexture(dev,a,header[1]);layer.mask=LoadTexture(dev,b,header[2]);
        if(!layer.image.Valid()||!layer.mask.Valid()){Shutdown();return false;}
    }
    ID3DBlob *code=nullptr,*errors=nullptr;
    auto compile=[&](const char* entry,const char* profile){const auto hr=D3DCompile(shader,strlen(shader),"layered-wallpaper",nullptr,nullptr,entry,profile,D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&code,&errors);if(errors){printf("[wallpaper] %s\n",static_cast<const char*>(errors->GetBufferPointer()));Release(errors);}return SUCCEEDED(hr);};
    if(!compile("VS","vs_5_0")){Shutdown();return false;}
    bool ok=SUCCEEDED(dev->CreateVertexShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,&vs_));Release(code);
    if(!compile("PS","ps_5_0")){Shutdown();return false;}
    ok&=SUCCEEDED(dev->CreatePixelShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,&ps_));Release(code);
    D3D11_BUFFER_DESC cb{};cb.ByteWidth=192;cb.Usage=D3D11_USAGE_DYNAMIC;cb.BindFlags=D3D11_BIND_CONSTANT_BUFFER;cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    ok&=SUCCEEDED(dev->CreateBuffer(&cb,nullptr,&params_));
    D3D11_SAMPLER_DESC sampler{};sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sampler.MaxLOD=D3D11_FLOAT32_MAX;
    sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_BORDER;ok&=SUCCEEDED(dev->CreateSamplerState(&sampler,&border_));
    sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;ok&=SUCCEEDED(dev->CreateSamplerState(&sampler,&repeat_));
    sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;ok&=SUCCEEDED(dev->CreateSamplerState(&sampler,&clamp_));
    D3D11_BLEND_DESC blend{};auto& rt=blend.RenderTarget[0];rt.BlendEnable=TRUE;rt.SrcBlend=D3D11_BLEND_SRC_ALPHA;rt.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;rt.BlendOp=rt.BlendOpAlpha=D3D11_BLEND_OP_ADD;rt.SrcBlendAlpha=D3D11_BLEND_ONE;rt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;rt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    ok&=SUCCEEDED(dev->CreateBlendState(&blend,&blend_));ok&=noise_.Valid()&&normal_.Valid();
    printf("[wallpaper] %s: %zu native layers, masked sway, spin, ripple %s\n",path,layers_.size(),ok?"PASS":"FAIL");
    if(!ok)Shutdown();return ok;
}
void LayeredWallpaper::Draw(float t){
    if(!ps_)return;
    context_->IASetInputLayout(nullptr);context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vs_,nullptr,0);context_->PSSetShader(ps_,nullptr,0);context_->VSSetConstantBuffers(0,1,&params_);context_->PSSetConstantBuffers(0,1,&params_);
    ID3D11SamplerState* samplers[]={border_,repeat_,clamp_};context_->PSSetSamplers(0,3,samplers);context_->OMSetBlendState(blend_,nullptr,0xffffffff);
    for(const auto& layer:layers_){
        float values[48]={2559,1428,t,0};memcpy(values+4,layer.params.data(),sizeof(layer.params));
        D3D11_MAPPED_SUBRESOURCE map{};if(FAILED(context_->Map(params_,0,D3D11_MAP_WRITE_DISCARD,0,&map)))continue;
        memcpy(map.pData,values,sizeof(values));context_->Unmap(params_,0);
        ID3D11ShaderResourceView* resources[]={layer.image.srv,layer.mask.srv,noise_.srv,normal_.srv};context_->PSSetShaderResources(0,4,resources);context_->Draw(3,0);
    }
    ID3D11ShaderResourceView* none[4]{};context_->PSSetShaderResources(0,4,none);
}
void LayeredWallpaper::Shutdown(){for(auto& layer:layers_){layer.image.Release();layer.mask.Release();}layers_.clear();noise_.Release();normal_.Release();Release(vs_);Release(ps_);Release(params_);Release(border_);Release(repeat_);Release(clamp_);Release(blend_);}
}
