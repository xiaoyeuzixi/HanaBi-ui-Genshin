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
float Fbm(float2 p){float v=0,w=.57;[unroll]for(int j=0;j<4;++j){v+=Noise(p)*w;p=p*2.03+float2(3.7,8.1);w*=.47;}return v;}
float Quintic(float a,float b,float t){float u=saturate((t-a)/(b-a));return u*u*u*(u*(u*6-15)+10);}
float4 PS(Pixel i):SV_TARGET{
    bool authored=frame.w<0;
    float t=authored?-frame.w-1:frame.w;
    float2 p=i.local;
    if(frame.z>5.5){
        float gather=Quintic(1.35,3.14,t);
        float flow=Fbm(p*6+float2(t*.55,-t*.8));
        float reveal=smoothstep(flow-.14,flow+.14,gather*1.35);
        reveal*=Quintic(1.25,1.75,t)*i.surface.y;
        if(state.y>.5)return float4(1,1,1,reveal);
        float rim=1-smoothstep(.003,.04,i.n.z);
        float3 n=normalize(float3(i.n.xy*rim,.8));
        float3 behind=scene.Sample(linearClamp,i.p.xy/frame.xy+n.xy*mark.z/frame.xy*.03).rgb;
        float3 color=lerp(behind*.2+float3(.10,.55,.43),float3(.93,1,.975),Quintic(2.0,3.2,t));
        float current=pow(saturate(.5+.5*sin(p.x*8-p.y*13+flow*3-t*5)),14);
        float active=1-smoothstep(3.4,4.5,t);
        color=lerp(color,float3(.40,.94,.73),rim*.25);
        color+=float3(.24,.38,.28)*current*active;
        float edge=exp(-pow((flow-gather*1.35)/.10,2));
        color=lerp(color,float3(.84,1,.93),edge*.8*active);
        float crest=exp(-pow((t-3.24)/.28,2));color=lerp(color,1,crest*.4);
        return float4(saturate(color),reveal*state.x);
    }
    if(frame.z>4.5){
        float reveal=i.surface.y;
        if(state.y>.5)return float4(1,1,1,reveal);
        float rim=1-smoothstep(.003,.048,i.n.z);
        float grain=Fbm(p*8);
        float charge=Quintic(1.55,3.05,t);
        float3 gold=lerp(float3(.55,.27,.055),float3(1,.83,.40),charge);
        float bevel=saturate(dot(i.n.xy,normalize(float2(-.5,-.8)))*.5+.5);
        gold=lerp(gold,lerp(float3(.70,.39,.08),float3(1,.95,.68),bevel),rim*.7);
        float2 stone=float2(p.x+p.y,p.x-p.y);
        float crack=exp(-pow(sin(stone.x*16+floor(stone.y*6)*1.7+grain*.6)/.065,2));
        crack*=smoothstep(.35,.65,grain);
        float resonance=exp(-pow((abs(p.x)+abs(p.y)-frac(t*.30)*2)/.06,2));
        float active=1-smoothstep(3.5,4.6,t);
        gold=lerp(gold,float3(1,.96,.72),crack*(.25+resonance*.7)*active);
        float lock=exp(-pow((i.thickness-.93)/.12,2))*(1-smoothstep(3.05,3.3,t));
        gold=lerp(gold,float3(1,.96,.70),lock*.72);
        float shard=(1-i.thickness)*(.16+i.seed*.27);gold*=1-shard;
        float crest=exp(-pow((t-3.15)/.24,2));gold=lerp(gold,float3(1,.96,.76),crest*.5);
        float sweep=exp(-pow((p.x*.7-p.y*.7-lerp(-1.5,1.5,Quintic(3.45,4.5,t)))/.10,2));
        gold+=float3(.15,.10,.035)*sweep*(1-smoothstep(4.5,4.7,t));
        return float4(saturate(gold),reveal*state.x);
    }
    if(frame.z>3.5){
        float growth=Quintic(.50,2.98,t)*1.09;
        float reveal=(1-smoothstep(growth-.028,growth+.022,i.thickness))*Quintic(.34,.57,t);
        if(state.y>.5)return float4(1,1,1,reveal*i.surface.y);
        float rim=1-smoothstep(.003,.038,i.n.z);
        float charge=Quintic(.75,3.1,t);
        float2 air=p+float2(sin(p.y*4-t*1.6),cos(p.x*3+t*.9))*.045;
        float strands=pow(saturate(.5+.5*sin(abs(air.x)*15+air.y*9-t*4.5)),14);
        float current=pow(saturate(.5+.5*sin(i.thickness*40-t*6.8)),12);
        float alive=1-smoothstep(3.25,4.55,t);
        float3 color=lerp(float3(.18,.72,.53),float3(.93,1,.975),charge);
        color=lerp(color,float3(.36,.91,.70),rim*(.27-.12*charge));
        color=lerp(color,float3(.95,1,.98),(strands*.32+current*.22)*alive);
        float head=exp(-pow((i.thickness-growth)/.04,2))*(1-smoothstep(2.95,3.45,t));
        color=lerp(color,float3(.98,1,.96),head*.9);
        float crest=exp(-pow((t-3.18)/.29,2));color=lerp(color,1,crest*.38);
        return float4(color,reveal*i.surface.y*state.x);
    }
    if(frame.z>2.5){
        float2 screenUV=i.p.xy/frame.xy;
        if(i.surface.x>1.5){
            float rr=dot(p,p);
            float mask=1-smoothstep(.88,1,rr);
            float z=sqrt(saturate(1-rr));
            float rim=pow(1-z,2.2);
            float shine=exp(-dot(p+float2(.32,.36),p+float2(.32,.36))*32);
            float3 behind=scene.Sample(linearClamp,screenUV+p*mark.z/frame.xy*.055).rgb;
            float3 color=lerp(behind*.66+float3(.04,.25,.39),float3(.58,.94,1),rim*.85);
            color+=float3(.76,.94,1)*shine;
            float alpha=mask*(.24+.65*rim+shine*.35)*i.surface.y*state.x;
            if(state.y>.5)return float4(1,1,1,alpha*.32);
            return float4(saturate(color),alpha);
        }
        float growth=authored?1.2:Quintic(.42,3.08,t)*1.08;
        float reveal=(1-smoothstep(growth-.024,growth+.018,i.thickness))*(authored?1:smoothstep(.25,.48,t));
        if(state.y>.5)return float4(1,1,1,reveal*i.surface.y);
        float rim=1-smoothstep(.003,.045,i.n.z);
        float2 normal=i.n.xy*rim;
        normal+=float2(sin(p.y*14-t*2.4),cos(p.x*12+t*1.7))*.14;
        float3 transmitted=scene.Sample(linearClamp,screenUV+normal*mark.z/frame.xy*.045).rgb;
        float charge=Quintic(.85,3.15,t);
        float3 color=lerp(transmitted*.28+float3(.055,.46,.69),float3(.95,.995,1),charge);
        color=lerp(color,float3(.32,.79,.96),rim*(.17-.10*charge));
        float2 water=p+float2(sin(p.y*3-t*.8),cos(p.x*3+t*.65))*.055;
        float caustic=exp(-pow((sin(water.x*11+water.y*4-t*1.4)+sin(water.x*4-water.y*9+t*1.1))/.32,2));
        float flow=pow(saturate(.5+.5*sin(length(water-float2(.03,.29))*17-atan2(water.y-.29,water.x-.03)*2-t*3.2)),12);
        float active=1-smoothstep(3.3,4.45,t);
        color+=float3(.12,.20,.22)*(caustic*.25+flow*.30)*active;
        float head=exp(-pow((i.thickness-growth)/.04,2))*(1-smoothstep(3.0,3.5,t));
        color=lerp(color,float3(.91,1,1),head*.85);
        float crest=exp(-pow((t-3.28)/.30,2));
        color=lerp(color,float3(1,1,1),crest*.4);
        return float4(saturate(color),reveal*i.surface.y*state.x);
    }
    if(frame.z>.5 && i.surface.x>1.5){
        float y=p.y,clock=t*(2.8+i.seed*.7)+i.seed*29;
        float turbulence=Fbm(float2(p.x*3.1+i.seed*17,y*5.4-clock*2.1));
        float curl=(sin(y*7.5-clock*2.3)+.45*sin(y*15-clock*3.2+i.seed*9))*.20*y;
        float x=p.x+curl;
        float width=pow(saturate(1-y),.78)*.68;
        float edge=width-abs(x)+(turbulence-.48)*(.26+.24*y);
        float flame=smoothstep(-.045,.065,edge)*smoothstep(0,.10,y)*(1-smoothstep(.92,1,y));
        float heat=saturate(edge*3.8+.14*(1-y));
        float3 color=lerp(float3(1,.20,.12),float3(1,.56,.26),smoothstep(0,.32,heat));
        color=lerp(color,float3(1,.86,.61),smoothstep(.22,.7,heat));
        color=lerp(color,float3(1,.97,.88),smoothstep(.65,.95,heat));
        float alpha=flame*saturate(i.surface.y)*state.x;
        if(state.y>.5)return float4(1,1,1,alpha*.8);
        return float4(color,alpha);
    }
    float radius=length(p),a=atan2(p.y,p.x);
    float propagation=radius*(1+.17*sin(a*3+radius*4));
    float growth=authored?1.7:smoothstep(.42,2.85,t)*1.3;
    float reveal=1-smoothstep(growth-.12,growth+.10,propagation);
    if(frame.z>1.5){
        propagation=i.thickness;
        growth=authored?1.2:Quintic(.58,2.95,t)*1.09;
        reveal=(1-smoothstep(growth-.025,growth+.022,propagation))*(authored?1:smoothstep(.42,.65,t));
        if(state.y>.5)return float4(1,1,1,reveal*i.surface.y);
        float rim=1-smoothstep(.002,.035,i.n.z);
        float grain=Fbm(p*5+float2(0,t*.12));
        float3 pearl=float3(.92,1,.91)+grain*float3(.05,0,.05);
        float3 jade=float3(.29,.76,.43);
        float alive=1-smoothstep(3.1,4.1,t);
        float vein=pow(saturate(.5+.5*sin(propagation*48-t*5+p.x*3)),16);
        float midrib=pow(saturate(.5+.5*sin(p.x*11+p.y*7+grain*3)),9);
        float3 color=lerp(pearl,jade,rim*.22+midrib*.12*alive);
        color+=float3(.12,.16,.06)*vein*alive;
        float growing=exp(-pow((propagation-growth)/.042,2))*(1-smoothstep(2.85,3.4,t));
        color=lerp(color,float3(.97,1,.82),growing*.92);
        float bloom=exp(-pow((t-3.22)/.31,2));
        color=lerp(color,float3(.98,1,.92),bloom*.65);
        float sweep=exp(-pow((p.x*.55-p.y*.83-lerp(-1.5,1.5,saturate((t-3.4)/1.15)))/.12,2));
        color+=float3(.08,.04,.08)*sweep*(1-smoothstep(4.45,4.7,t));
        return float4(saturate(color),reveal*i.surface.y*state.x);
    }
    if(frame.z>.5) {
        propagation=i.thickness;
        growth=authored?1.2:smoothstep(.30,3.30,t)*1.07;
        float flicker=sin(p.x*24+p.y*19-t*14)*.003+sin(p.x*13-p.y*31+t*11)*.002;
        reveal=(1-smoothstep(growth-.014,growth+.020,propagation+flicker))*(authored?1:smoothstep(.18,.4,t));
    }
    if(state.y>.5)return float4(1,1,1,reveal*i.surface.y);
    float front=step(.25,i.surface.x)*(1-step(.75,i.surface.x));
    float bevel=step(.75,i.surface.x);
    float side=1-step(.25,i.surface.x);
    float3 n=normalize(i.n);
    if(n.z<0)n=-n;
    float grain=Noise(p*float2(8,11));
    if(frame.z>.5) {
        float charge=smoothstep(.75,2.65,t);
        float3 ivory=pow(float3(1,.935,.926),2.2);
        float3 ember=pow(float3(.99,.32,.24),2.2);
        float turbulent=Fbm(p*float2(5,8)+float2(0,t*1.4));
        float tongues=pow(saturate(.5+.5*sin(p.x*18+p.y*9+t*3+turbulent*3)),5);
        float settling=smoothstep(2.85,4.1,t);
        float3 hot=lerp(ember,ivory,charge);
        hot*=1-side*.28;
        hot=lerp(hot,float3(1,.44,.35),bevel*.38);
        hot=lerp(hot,float3(1,.82,.64),tongues*(1-charge)*front*.45);
        float ignition=exp(-pow((propagation-growth)/.022,2))*(1-smoothstep(3.05,3.6,t));
        hot=lerp(hot,float3(1,.94,.74),ignition*.85);
        float moving=Fbm(float2(propagation*18-t*3.2,p.x*3+p.y*6-t*1.4));
        float flow=smoothstep(.38,.69,moving)*(1-settling);
        hot=lerp(hot,float3(1,.47,.24),flow*.32*front);
        float crest=exp(-pow((t-3.16)/.26,2));
        hot=lerp(hot,float3(1,.97,.86),crest*.55);
        return float4(pow(saturate(hot),1/2.2),reveal*i.surface.y*state.x);
    }
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
    if(state.y>4.5){float2 p=mark.xy+(o.uv-.5)*512/mark.w;p.y=state.z+(state.z-p.y)*state.w;o.p=float4(p.x/frame.x*2-1,1-p.y/frame.y*2,0,1);}
    else if(state.y>3.5){float2 p=mark.xy+(o.uv-.5)*512/mark.w;o.p=float4(p.x/frame.x*2-1,1-p.y/frame.y*2,0,1);}
    else o.p=float4(o.uv.x*2-1,1-o.uv.y*2,0,1);
    return o;
}
float4 GlowPS(GlowPixel i):SV_TARGET{
    if(state.y>4.5){
        float depth=(i.p.y-state.z)/mark.z;
        if(depth<0)return 0;
        float2 uv=i.uv;
        float wave=(sin(depth*24-frame.w*2.1)*.0014+sin(depth*49+frame.w*1.4)*.0007)*(1+depth*2);
        uv.x+=wave;
        uv.y+=sin(depth*38-frame.w*1.5)*.0008;
        float mask=scene.Sample(linearClamp,uv).r;
        float band=.88+.12*sin(depth*66-frame.w*1.2);
        float fade=exp(-depth*5.5)*smoothstep(0,.012,depth)*(1-smoothstep(.36,.48,depth));
        float3 water=lerp(float3(.22,.49,.67),float3(.67,.88,.94),saturate(mask));
        return float4(water,mask*fade*band*state.x);
    }
    if(state.y>3.5){
        float clock=frame.w<0?-frame.w-1:frame.w;
        float mask=scene.Sample(linearClamp,i.uv).r;
        float3 purple=lerp(float3(.68,.24,.73),float3(.92,.52,.94),saturate(mask*1.3));
        if(frame.z>5.5){
            float crest=exp(-pow((frame.w-3.24)/.3,2));
            return float4(lerp(float3(.18,.85,.62),float3(.64,1,.82),saturate(mask*1.6)),(1-exp(-mask*3.5))*state.x*(.95+crest*.28));
        }
        if(frame.z>4.5){
            float crest=exp(-pow((frame.w-3.15)/.3,2));
            return float4(lerp(float3(1,.56,.10),float3(1,.84,.43),saturate(mask*1.6)),(1-exp(-mask*3.8))*state.x*(.95+crest*.35));
        }
        if(frame.z>3.5){
            float crest=exp(-pow((frame.w-3.18)/.32,2));
            float3 mint=lerp(float3(.20,.88,.61),float3(.65,1,.83),saturate(mask*1.6));
            return float4(mint,(1-exp(-mask*3.4))*state.x*(.95+crest*.28));
        }
        if(frame.z>2.5){
            float crest=exp(-pow((clock-3.28)/.34,2));
            float3 aqua=lerp(float3(.12,.75,1),float3(.57,.98,1),saturate(mask*1.6));
            return float4(aqua,(1-exp(-mask*3.5))*state.x*(.97+crest*.22));
        }
        if(frame.z>1.5){
            float bloom=exp(-pow((clock-3.22)/.32,2));
            float3 green=lerp(float3(.24,.95,.40),float3(.58,1,.64),saturate(mask*1.7));
            return float4(green,(1-exp(-mask*3.5))*state.x*(.96+bloom*.27));
        }
        if(frame.z>.5){
            float t=clock;
            float crest=exp(-pow((t-3.16)/.30,2));
            float breathe=.94+.06*sin(t*2)+crest*.36;
            float3 coral=lerp(float3(1,.30,.27),float3(1,.57,.53),saturate(mask*1.4));
            return float4(coral,(1-exp(-mask*3.2))*state.x*breathe);
        }
        float ceremony=1-smoothstep(4.2,5.1,clock);
        float breathe=.94+.06*sin(clock*2.1)+ceremony*.18*pow(.5+.5*sin(clock*5.4),4);
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
bool ElectroMaterial::Init(ID3D11Device* dev,ID3D11DeviceContext* ctx,int style){
    device_=dev;context_=ctx;style_=style;ID3DBlob *vs=nullptr,*ps=nullptr,*errors=nullptr;
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
    D3D11_RASTERIZER_DESC raster{};raster.FillMode=D3D11_FILL_SOLID;raster.CullMode=D3D11_CULL_NONE;raster.DepthClipEnable=TRUE;raster.MultisampleEnable=TRUE;raster.ScissorEnable=TRUE;ok&=SUCCEEDED(dev->CreateRasterizerState(&raster,&raster_));
    D3D11_BLEND_DESC blend{};auto& rt=blend.RenderTarget[0];rt.BlendEnable=TRUE;rt.SrcBlend=D3D11_BLEND_SRC_ALPHA;rt.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;rt.BlendOp=rt.BlendOpAlpha=D3D11_BLEND_OP_ADD;rt.SrcBlendAlpha=D3D11_BLEND_ONE;rt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;rt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    ok&=SUCCEEDED(dev->CreateBlendState(&blend,&blend_));D3D11_DEPTH_STENCIL_DESC depth{};depth.DepthFunc=D3D11_COMPARISON_ALWAYS;ok&=SUCCEEDED(dev->CreateDepthStencilState(&depth,&depth_));
    printf("[%s] vector mesh, animated material, Gaussian bloom: %s\n",style_==6?"faruzan":style_==5?"geo":style_==4?"anemo":style_==3?"hydro":style_==2?"dendro":style_==1?"pyro":"electro",ok?"ok":"FAILED");if(!ok)Shutdown();return ok;
}
void ElectroMaterial::Shutdown(){Release(vs_);Release(ps_);Release(glowVs_);Release(glowPs_);for(int j=0;j<2;++j){Release(glowViews_[j]);Release(glowTargets_[j]);Release(glowTextures_[j]);}Release(layout_);Release(vertices_);Release(params_);Release(sampler_);Release(raster_);Release(blend_);Release(depth_);Release(sceneView_);Release(scene_);requests_.clear();width_=height_=0;}
bool ElectroMaterial::Snapshot(ID3D11Texture2D* texture){
    D3D11_TEXTURE2D_DESC d{};texture->GetDesc(&d);
    if(width_!=d.Width||height_!=d.Height||!sceneView_){Release(sceneView_);Release(scene_);width_=d.Width;height_=d.Height;auto copy=d;copy.SampleDesc={1,0};copy.BindFlags=D3D11_BIND_SHADER_RESOURCE;copy.Usage=D3D11_USAGE_DEFAULT;copy.CPUAccessFlags=copy.MiscFlags=0;if(FAILED(device_->CreateTexture2D(&copy,nullptr,&scene_))||FAILED(device_->CreateShaderResourceView(scene_,nullptr,&sceneView_)))return false;}
    if(d.SampleDesc.Count>1)context_->ResolveSubresource(scene_,0,texture,0,d.Format);else context_->CopyResource(scene_,texture);return true;
}
void ElectroMaterial::Queue(ImDrawList* dl,std::vector<CrystalVertex>&& vertices,ImVec2 center,float radius,float time,float alpha,float reflection,float waterY,float motionTime){
    if(!ps_||vertices.empty()||alpha<.001f)return;
    const auto lo=dl->GetClipRectMin(),hi=dl->GetClipRectMax();
    requests_.push_back({this,std::move(vertices),center,radius,time,alpha,{lo.x,lo.y,hi.x,hi.y},reflection,waterY,motionTime});
    dl->AddCallback(Callback,&requests_.back());dl->AddCallback(ImDrawCallback_ResetRenderState,nullptr);
}
void ElectroMaterial::QueueFormation(ImDrawList* dl,std::vector<CrystalVertex>&& vertices,ImVec2 center,float radius,float time,float alpha,float reflection,float waterY){
    Queue(dl,std::move(vertices),center,radius,-1.f-time,alpha,reflection,waterY,time);
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
    const D3D11_RECT full{0,0,512,512};context_->RSSetScissorRects(1,&full);
    ID3D11ShaderResourceView* none=nullptr;context_->PSSetShaderResources(0,1,&none);
    context_->OMSetRenderTargets(1,&glowTargets_[0],nullptr);
    const float clear[]={0,0,0,0};context_->ClearRenderTargetView(glowTargets_[0],clear);
    const float scale=std::min(1.f,180.f/std::max(r.radius,1.f));
    float params[]={512,512,float(style_),r.time,r.center.x,r.center.y,r.radius,scale,1,1,r.center.x-256/scale,r.center.y-256/scale};
    if(SetParams(params))context_->Draw(UINT(r.vertices.size()),0);
    context_->IASetInputLayout(nullptr);context_->VSSetShader(glowVs_,nullptr,0);context_->PSSetShader(glowPs_,nullptr,0);
    if(r.reflection>.001f){
        context_->OMSetRenderTargets(1,&target,depth);context_->RSSetViewports(1,&oldViewport);
        const D3D11_RECT clip{LONG(std::max(0.f,r.clip.x)),LONG(std::max(0.f,r.clip.y)),LONG(std::min(float(width_),r.clip.z)),LONG(std::min(float(height_),r.clip.w))};context_->RSSetScissorRects(1,&clip);
        context_->PSSetShaderResources(0,1,&glowViews_[0]);
        params[0]=float(width_);params[1]=float(height_);params[3]=r.motionTime;params[8]=r.opacity*r.reflection;params[9]=5;params[10]=r.waterY;params[11]=.22f;
        if(SetParams(params))context_->Draw(3,0);
        context_->PSSetShaderResources(0,1,&none);
        context_->RSSetViewports(1,&viewport);context_->RSSetScissorRects(1,&full);
        params[0]=params[1]=512;params[3]=r.time;params[8]=1;
    }
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
    const D3D11_RECT clip{LONG(std::max(0.f,r.clip.x)),LONG(std::max(0.f,r.clip.y)),LONG(std::min(float(width_),r.clip.z)),LONG(std::min(float(height_),r.clip.w))};context_->RSSetScissorRects(1,&clip);
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
    const float params[]={float(width_),float(height_),float(style_),r.time,r.center.x,r.center.y,r.radius,1,r.opacity,0,0,0};
    if(SetParams(params))context_->Draw(UINT(r.vertices.size()),0);
    ID3D11ShaderResourceView* none=nullptr;context_->PSSetShaderResources(0,1,&none);
}
}
