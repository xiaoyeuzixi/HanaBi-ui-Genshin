#include "intro/crystal_material.h"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>

namespace hb {
namespace {
constexpr UINT kMaxVertices = 24000;
template<class T> void Release(T*& object) { if(object) { object->Release(); object=nullptr; } }
const char* shader = R"(
cbuffer Params : register(b0) {
    float4 frame;
    float4 effect;
    float4 lighting;
};
struct Vertex {
    float2 p : POSITION;
    float3 n : NORMAL;
    float2 local : TEXCOORD0;
    float2 surface : TEXCOORD1;
    float thickness : TEXCOORD2;
    float seed : TEXCOORD3;
};
struct Pixel {
    float4 p : SV_POSITION;
    float3 n : NORMAL;
    float2 local : TEXCOORD0;
    float2 surface : TEXCOORD1;
    float thickness : TEXCOORD2;
    nointerpolation float seed : TEXCOORD3;
};
Pixel VS(Vertex v) {
    Pixel o;
    o.p=float4(v.p.x/frame.x*2-1,1-v.p.y/frame.y*2,0,1);
    o.n=v.n; o.local=v.local; o.surface=v.surface; o.thickness=v.thickness;
    o.seed=v.seed;
    return o;
}
Texture2D scene : register(t0);
SamplerState linearClamp : register(s0);
float3 Linear(float3 c) { return pow(max(c,0),2.2); }
float3 Environment(float3 r) {

    float3 light=float3(.012,.038,.095);
    float key=exp(-pow((dot(r,normalize(float3(.87,.19,.45)))-.80)/.055,2));
    key*=smoothstep(-.45,.1,r.y)*(.80+.20*cos(r.z*3));
    float rim=exp(-pow((dot(r,normalize(float3(-.72,-.35,.60)))-.87)/.025,2));
    return light+float3(.38,.82,1.2)*key+float3(1.65,2.0,2.3)*rim;
}
float Noise(float2 p) {
    float2 cell=floor(p),f=frac(p);f=f*f*(3-2*f);
    float4 h=frac(sin(float4(dot(cell,float2(127.1,311.7)),
                            dot(cell+float2(1,0),float2(127.1,311.7)),
                            dot(cell+float2(0,1),float2(127.1,311.7)),
                            dot(cell+1,float2(127.1,311.7))))*43758.5453);
    return lerp(lerp(h.x,h.y,f.x),lerp(h.z,h.w,f.x),f.y);
}
float2 IceCell(float2 p) {
    return frac(sin(float2(dot(p,float2(127.1,311.7)),
                           dot(p,float2(269.5,183.3))))*43758.5453);
}
float VeinDistance(float2 p) {
    float2 cell=floor(p),f=frac(p);
    float nearest=8,second=8;
    [unroll] for(int y=-1;y<=1;++y) [unroll] for(int x=-1;x<=1;++x) {
        float2 q=float2(x,y);
        float2 d=q+IceCell(cell+q)-f;
        float distance=dot(d,d);
        if(distance<nearest) { second=nearest; nearest=distance; }
        else second=min(second,distance);
    }
    return sqrt(second)-sqrt(nearest);
}
float Segment(float2 p,float2 a,float2 b) {
    float2 d=b-a;
    return length(p-a-d*saturate(dot(p-a,d)/dot(d,d)));
}
float CutShadow(float distance,float aa) {

    return smoothstep(-aa,aa,distance)*exp(-max(distance,0)/.014);
}
float CutHighlight(float distance,float aa) {
    return (1-smoothstep(0,aa,abs(distance)))*min(1,.0025/aa);
}
float4 PS(Pixel i) : SV_TARGET {
    float2 uv=i.p.xy/frame.xy;
    float2 mark=(i.p.xy-effect.yz)/frame.z;
    float3 n=normalize(i.n);
    if(n.z<0)n=-n;
    float side=1-smoothstep(.20,.40,i.surface.x);
    float bevel=smoothstep(.75,.90,i.surface.x);
    float nv=saturate(n.z);
    float fresnel=.028+(1-.028)*pow(1-nv,5);
    float thickness=max(.012,i.thickness);
    float front=(1-side)*(1-bevel);

    float2 iceUV=i.local+float2(.13,.28)+i.seed*float2(2.17,1.43);
    float cloud=.60*Noise(iceUV*float2(11,7))+.28*Noise(iceUV*37)+.12*Noise(iceUV*96);
    float fineFrost=.65*Noise(iceUV*float2(31,92))+.35*Noise(iceUV*155);
    float frost=smoothstep(.55,.76,fineFrost)*smoothstep(.38,.68,cloud);
    float vein=VeinDistance(iceUV*10);
    float veinAA=max(fwidth(vein),.012);
    float veinMask=smoothstep(.62,.84,Noise(iceUV*7+3.7));
    float internalCrack=(1-smoothstep(.014,.014+veinAA,vein))*veinMask;
    float deepVein=VeinDistance((iceUV+float2(.026,-.033))*10);
    float buriedCrack=(1-smoothstep(.012,.012+max(fwidth(deepVein),.012),deepVein))*veinMask;
    float2 microSlope=float2(Noise(iceUV*21+float2(.12,0))-Noise(iceUV*21-float2(.12,0)),
                             Noise(iceUV*21+float2(0,.12))-Noise(iceUV*21-float2(0,.12)));
    float3 opticalNormal=normalize(n+float3(microSlope*.12*front,0));
    float3 transmitted=refract(float3(0,0,-1),opticalNormal,1/1.31);
    float2 offset=transmitted.xy/max(abs(transmitted.z),.20)*thickness*frame.z/frame.xy;
    float3 through;
    through.r=scene.Sample(linearClamp,uv+offset*1.018).r;
    through.g=scene.Sample(linearClamp,uv+offset).g;
    through.b=scene.Sample(linearClamp,uv+offset*.982).b;

    float opticalDepth=thickness*(1.25+cloud*.55);
    float clearStyle=1-step(.5,lighting.z);
    float frostStyle=step(.5,lighting.z)*(1-step(1.5,lighting.z));
    float blueStyle=step(1.5,lighting.z);
    float3 absorption=float3(2.6,.55,.15)*clearStyle+float3(2.0,.45,.22)*frostStyle+float3(6.8,1.2,.14)*blueStyle;
    float3 absorb=exp(-absorption*opticalDepth);
    float3 transmission=Linear(through)*absorb;
    float3 reflected=Environment(reflect(float3(0,0,-1),n));
    float reflectWeight=saturate(fresnel+.025+bevel*.10+side*.035);
    float3 color=lerp(transmission,reflected,reflectWeight);
    float top=saturate(.50-mark.y*.52);
    float3 deepColor=float3(.012,.085,.16)*clearStyle+float3(.12,.25,.34)*frostStyle+float3(.006,.085,.34)*blueStyle;
    float3 lightColor=float3(.075,.28,.36)*clearStyle+float3(.40,.66,.73)*frostStyle+float3(.035,.34,.76)*blueStyle;
    float3 body=lerp(deepColor,lightColor,top);
    float facetLight=saturate(dot(n,normalize(float3(-.65,-.72,.60))));
    float scatter=1-exp(-opticalDepth*7);

    float planeLight=smoothstep(-.18,.30,-n.x*.85-n.y*.52);
    float volumeLight=(.12+.50*scatter)*(.35+.65*planeLight);
    volumeLight*=.55*clearStyle+1.15*frostStyle+1.20*blueStyle;
    color+=body*volumeLight*(1-reflectWeight);
    float softReflection=pow(saturate(dot(n,normalize(float3(-.28,-.18,1)))),16);
    float panelDistance=i.local.x+.15*(i.local.y+.58);
    float panel=exp(-pow((panelDistance-.040)/.052,2));
    float panelEnd=smoothstep(-1.10,-.82,i.local.y)*(1-smoothstep(-.20,.02,i.local.y));
    float frontReflection=(.02+.12*panel*panelEnd)*softReflection;
    color+=float3(.48,.73,.86)*frontReflection*front*(.45*clearStyle+.55*frostStyle+.40*blueStyle);

    float3 reflectedRay=reflect(float3(0,0,-1),n);
    float softbox=exp(-pow((reflectedRay.x+reflectedRay.y*.28-.23)/.095,2));
    float coolbox=exp(-pow((reflectedRay.x-reflectedRay.y*.35+.31)/.16,2));
    float darkBand=exp(-pow((reflectedRay.x+reflectedRay.y*.18-.02)/.11,2));
    color*=1-darkBand*front*(.35*clearStyle+.10*frostStyle+.30*blueStyle);
    color+=float3(.78,1.02,1.14)*softbox*front*(.36*clearStyle+.12*frostStyle+.27*blueStyle);
    color+=float3(.05,.32,.57)*coolbox*front*(.30*clearStyle+.10*frostStyle+.38*blueStyle);
    float milk=.50*Noise(iceUV*float2(13,52))+.50*Noise(iceUV*75);
    float snow=smoothstep(.41,.74,milk)*smoothstep(.35,.68,cloud);
    color+=float3(.55,.82,.92)*snow*front*(.04*clearStyle+.48*frostStyle+.09*blueStyle);

    color*=1-buriedCrack*front*.08;
    color+=float3(.055,.34,.61)*buriedCrack*front*.06;
    color+=float3(.52,.83,1.05)*internalCrack*front*(.025+.045*facetLight);
    color+=float3(.48,.73,.82)*frost*(front*.35+bevel*.65)*(.25+.30*scatter)*(.25*clearStyle+1.1*frostStyle+.40*blueStyle);

    float3 cuts=float3(i.local.x,(.70*i.local.x+i.local.y+.60)/1.2206556,
                                 (-.60*i.local.x+i.local.y+.26)/1.1661904);
    float3 cutAA=max(fwidth(cuts),.0008);
    float shadow=max(CutShadow(cuts.x,cutAA.x),
                     max(CutShadow(cuts.y,cutAA.y)*.82,CutShadow(-cuts.z,cutAA.z)*.60));
    color*=1-float3(.20,.13,.06)*shadow*front;
    float ridge=max(CutHighlight(cuts.x,cutAA.x),
                    max(CutHighlight(cuts.y,cutAA.y)*.72,CutHighlight(cuts.z,cutAA.z)*.45));
    float ridgeLight=saturate(dot(n,normalize(float3(-.65,-.72,.60))));
    color+=float3(.48,.83,1.0)*ridge*pow(ridgeLight,3)*front*.06;
    float3 halfKey=normalize(normalize(float3(-.65,-.28,1.0))+float3(0,0,1));
    float spec=pow(saturate(dot(n,halfKey)),125);
    float sweep=exp(-pow((dot(mark,float2(.72,-.69))
                       -lerp(-1.3,1.3,saturate((frame.w-3.65)/1.0)))/.016,2));
    sweep*=smoothstep(3.65,3.95,frame.w)*(1-smoothstep(4.35,4.65,frame.w));
    color+=float3(1,1,1)*(spec*.75+sweep*.14)*(bevel+ridge*front*.30);

    float causticBand=dot(mark,float2(.66,-.75));
    float caustic=exp(-pow((causticBand-lerp(-1.3,1.3,saturate((frame.w-3.48)/1.15)))/.10,2));
    caustic*=smoothstep(3.48,3.66,frame.w)*(1-smoothstep(4.40,4.65,frame.w));
    color+=float3(.10,.48,.85)*caustic*front*(.25+internalCrack*.80+frost*.45);

    float lockAge=saturate((frame.w-3.52)/.48);
    float lockEnvelope=smoothstep(3.52,3.62,frame.w)*(1-smoothstep(3.80,4.06,frame.w));
    float lockWave=exp(-pow((length(mark)-lerp(.02,1.28,lockAge))/.12,2))*lockEnvelope;
    color+=float3(.24,.90,1.5)*lockWave*(front*.48+bevel*.70);

    float edgeLight=pow(saturate(dot(n,normalize(float3(-.48,-.64,.42)))),7);
    color+=lerp(float3(.36,.70,.84),float3(.87,.97,1),edgeLight)*edgeLight*bevel*.72;

    float3 lampDirection=normalize(float3((lighting.xy-i.p.xy)/frame.y,.42));
    float warmEdge=pow(saturate(dot(n,lampDirection)),4)*smoothstep(-.15,.9,mark.x);
    float3 lampColor=Linear(scene.Sample(linearClamp,lighting.xy/frame.xy).rgb);
    color+=lampColor*float3(.18,.095,.035)*warmEdge*bevel;
    float lowerEdge=pow(saturate(dot(n,normalize(float3(.05,.85,.35)))),4);
    color+=float3(.014,.105,.075)*lowerEdge*smoothstep(0,.85,mark.y)*bevel;

    float grain=pow(saturate(Noise(i.local*155)),7);
    float aa=max(length(fwidth(i.local)),.001);
    float fracture=min(Segment(i.local,float2(-.035,-.90),float2(-.104,-.76)),
                       Segment(i.local,float2(-.104,-.76),float2(-.15,-.73)));
    float crack=(1-smoothstep(.0006,.0006+aa,fracture))*.0012/aa;
    color+=float3(.65,.87,.96)*(grain*bevel*.20+crack*.12*(1-side));

    float tipDistance=length(i.local-float2(0,-1.10))*frame.z;
    float tipFacing=pow(saturate(dot(normalize(mark+float2(0,.00001)),normalize(float2(-.45,-.89)))),4);

    float glintScale=smoothstep(20,80,frame.z);
    float tipGlint=(1-smoothstep(lerp(.22,1.6,glintScale),lerp(.65,3.2,glintScale),tipDistance))*tipFacing;
    color+=float3(8,8,8)*tipGlint;
    float retreat=.08+(1+i.local.y)*.50+(cloud-.5)*.12;
    float fade=1-smoothstep(retreat,retreat+.40,effect.x);
    float sublimation=exp(-pow((effect.x-retreat-.14)/.055,2));
    sublimation*=smoothstep(.01,.12,effect.x)*(1-smoothstep(.85,1,effect.x));
    color+=float3(.12,.60,.95)*sublimation*(.20+internalCrack*.40);
    color=color/(1+color*.25);
    float opacity=lerp(.72,.82,saturate((thickness-.04)/.12));
    opacity=lerp(opacity,.84,bevel);
    opacity=lerp(opacity,.80,side);
    opacity=lerp(opacity,.98,tipGlint);
    return float4(pow(saturate(color),1/2.2),i.surface.y*fade*opacity);
}
)";
}
bool CrystalMaterial::Init(ID3D11Device* device, ID3D11DeviceContext* context) {
    device_=device; context_=context;
    ID3DBlob *vs=nullptr,*ps=nullptr,*errors=nullptr;
    auto compile=[&](const char* entry,const char* profile,ID3DBlob** result) {
        HRESULT hr=D3DCompile(shader,strlen(shader),"cryo-glass",nullptr,nullptr,entry,profile,
                              D3DCOMPILE_OPTIMIZATION_LEVEL3,0,result,&errors);
        if(errors) { printf("[cryo] %s\n",(const char*)errors->GetBufferPointer()); Release(errors); }
        return SUCCEEDED(hr);
    };
    if(!compile("VS","vs_5_0",&vs)||!compile("PS","ps_5_0",&ps)) {
        Release(vs); Release(ps); Shutdown(); return false;
    }
    bool ok=SUCCEEDED(device_->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vs_));
    ok&=SUCCEEDED(device_->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&ps_));
    const D3D11_INPUT_ELEMENT_DESC elements[]={
        {"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,8,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",1,DXGI_FORMAT_R32G32_FLOAT,0,28,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",2,DXGI_FORMAT_R32_FLOAT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",3,DXGI_FORMAT_R32_FLOAT,0,40,D3D11_INPUT_PER_VERTEX_DATA,0}
    };
    ok&=SUCCEEDED(device_->CreateInputLayout(elements,6,vs->GetBufferPointer(),vs->GetBufferSize(),&layout_));
    Release(vs); Release(ps);
    D3D11_BUFFER_DESC buffer{};
    buffer.ByteWidth=kMaxVertices*sizeof(CrystalVertex); buffer.Usage=D3D11_USAGE_DYNAMIC;
    buffer.BindFlags=D3D11_BIND_VERTEX_BUFFER; buffer.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    ok&=SUCCEEDED(device_->CreateBuffer(&buffer,nullptr,&vertices_));
    buffer.ByteWidth=48; buffer.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
    ok&=SUCCEEDED(device_->CreateBuffer(&buffer,nullptr,&params_));
    D3D11_SAMPLER_DESC sampler{};
    sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler.MaxLOD=D3D11_FLOAT32_MAX;
    ok&=SUCCEEDED(device_->CreateSamplerState(&sampler,&sampler_));
    D3D11_RASTERIZER_DESC raster{};
    raster.FillMode=D3D11_FILL_SOLID; raster.CullMode=D3D11_CULL_NONE;
    raster.DepthClipEnable=TRUE; raster.MultisampleEnable=TRUE;
    ok&=SUCCEEDED(device_->CreateRasterizerState(&raster,&raster_));
    D3D11_BLEND_DESC blend{};
    auto& rt=blend.RenderTarget[0]; rt.BlendEnable=TRUE;
    rt.SrcBlend=D3D11_BLEND_SRC_ALPHA; rt.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOp=rt.BlendOpAlpha=D3D11_BLEND_OP_ADD;
    rt.SrcBlendAlpha=D3D11_BLEND_ONE; rt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
    rt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    ok&=SUCCEEDED(device_->CreateBlendState(&blend,&blend_));
    D3D11_DEPTH_STENCIL_DESC depth{}; depth.DepthFunc=D3D11_COMPARISON_ALWAYS;
    ok&=SUCCEEDED(device_->CreateDepthStencilState(&depth,&depth_));
    printf("[cryo] refractive material: %s\n",ok?"ok":"FAILED");
    if(!ok)Shutdown();
    return ok;
}
void CrystalMaterial::Shutdown() {
    Release(vs_);Release(ps_);Release(layout_);Release(vertices_);Release(params_);
    Release(sampler_);Release(raster_);Release(blend_);Release(depth_);
    Release(sceneView_);Release(scene_);width_=height_=0;pending_.clear();
    device_=nullptr;context_=nullptr;
}
bool CrystalMaterial::Snapshot(ID3D11Texture2D* target) {
    D3D11_TEXTURE2D_DESC desc{}; target->GetDesc(&desc);
    if(width_!=desc.Width||height_!=desc.Height||!sceneView_) {
        Release(sceneView_);Release(scene_);width_=desc.Width;height_=desc.Height;
        auto copy=desc;copy.SampleDesc={1,0};copy.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        copy.Usage=D3D11_USAGE_DEFAULT;copy.CPUAccessFlags=copy.MiscFlags=0;
        if(FAILED(device_->CreateTexture2D(&copy,nullptr,&scene_)))return false;
        if(FAILED(device_->CreateShaderResourceView(scene_,nullptr,&sceneView_)))return false;
    }
    if(desc.SampleDesc.Count>1)context_->ResolveSubresource(scene_,0,target,0,desc.Format);
    else context_->CopyResource(scene_,target);
    return true;
}
void CrystalMaterial::Queue(ImDrawList* list,std::vector<CrystalVertex>&& vertices,
                            ImVec2 center,float radius,float time,float dissolve,ImVec2 lantern) {
    pending_=std::move(vertices);center_=center;radius_=radius;time_=time;dissolve_=dissolve;
    lantern_=lantern;
    if(!ps_||pending_.empty())return;
    list->AddCallback(Callback,this);
    list->AddCallback(ImDrawCallback_ResetRenderState,nullptr);
}
void CrystalMaterial::Callback(const ImDrawList*,const ImDrawCmd* cmd) {
    static_cast<CrystalMaterial*>(cmd->UserCallbackData)->Render();
}
void CrystalMaterial::Render() {
    if(pending_.size()>kMaxVertices)return;
    ID3D11RenderTargetView* target=nullptr; ID3D11DepthStencilView* depth=nullptr;
    context_->OMGetRenderTargets(1,&target,&depth);
    if(!target){Release(depth);return;}
    ID3D11Resource* resource=nullptr; ID3D11Texture2D* texture=nullptr;
    target->GetResource(&resource);resource->QueryInterface(IID_PPV_ARGS(&texture));Release(resource);
    context_->OMSetRenderTargets(0,nullptr,nullptr);
    const bool ready=texture&&Snapshot(texture);
    context_->OMSetRenderTargets(1,&target,depth);Release(target);Release(depth);Release(texture);
    if(!ready)return;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(context_->Map(vertices_,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return;
    memcpy(mapped.pData,pending_.data(),pending_.size()*sizeof(CrystalVertex));context_->Unmap(vertices_,0);
    const float params[]={float(width_),float(height_),radius_,time_,dissolve_,center_.x,center_.y,0,
                          lantern_.x,lantern_.y,float(style_),0};
    if(FAILED(context_->Map(params_,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return;
    memcpy(mapped.pData,params,sizeof(params));context_->Unmap(params_,0);
    UINT stride=sizeof(CrystalVertex),offset=0;
    context_->IASetInputLayout(layout_);context_->IASetVertexBuffers(0,1,&vertices_,&stride,&offset);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vs_,nullptr,0);context_->VSSetConstantBuffers(0,1,&params_);
    context_->PSSetShader(ps_,nullptr,0);context_->PSSetConstantBuffers(0,1,&params_);
    context_->PSSetShaderResources(0,1,&sceneView_);context_->PSSetSamplers(0,1,&sampler_);
    context_->RSSetState(raster_);context_->OMSetDepthStencilState(depth_,0);
    const float factors[4]={0,0,0,0};context_->OMSetBlendState(blend_,factors,0xffffffff);
    context_->Draw(UINT(pending_.size()),0);
    ID3D11ShaderResourceView* none=nullptr;context_->PSSetShaderResources(0,1,&none);
}
}
