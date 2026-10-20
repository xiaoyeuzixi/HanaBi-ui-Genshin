#include <windows.h>
#include <shellapi.h>
#include <windowsx.h>
#include <functional>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

#include "assets/fonts.h"
#include "core/display.h"
#include "core/easing.h"
#include "core/time.h"
#include "gfx/background.h"
#include "gfx/window_motion.h"
#include "intro/intro.h"
#include "ui/shell.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

#ifndef HB_ASSET_DIR
#define HB_ASSET_DIR "assets"
#endif

namespace {

ID3D11Device* g_dev = nullptr;
ID3D11DeviceContext* g_ctx = nullptr;
IDXGISwapChain* g_swap = nullptr;
ID3D11RenderTargetView* g_rtv = nullptr;
HWND g_hwnd = nullptr;
int g_w = 0, g_h = 0;
int g_pendingW=0,g_pendingH=0;
float g_uiScale=1.f;
std::function<void()> g_frame;
bool g_rendering=false;
bool g_minimizeRequested=false,g_restoreRequested=false,g_wasMinimized=false;
bool g_windowAnimating=false,g_nativeMinimize=false;

bool CreateDeviceD3D(HWND hwnd, int w, int h) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = (UINT)w;
    sd.BufferDesc.Height = (UINT)h;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 240;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                             levels, 1, D3D11_SDK_VERSION, &sd, &g_swap, &g_dev,
                                             nullptr, &g_ctx)))
        return false;

    ID3D11Texture2D* back = nullptr;
    g_swap->GetBuffer(0, IID_PPV_ARGS(&back));
    g_dev->CreateRenderTargetView(back, nullptr, &g_rtv);
    back->Release();
    g_w = w; g_h = h;

    IDXGIDevice1* dxgiDev = nullptr;
    if (SUCCEEDED(g_dev->QueryInterface(IID_PPV_ARGS(&dxgiDev)))) {
        dxgiDev->SetMaximumFrameLatency(1);
        dxgiDev->Release();
    }
    return true;
}

void CleanupD3D() {
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    if (g_swap) { g_swap->Release(); g_swap = nullptr; }
    if (g_ctx) { g_ctx->Release(); g_ctx = nullptr; }
    if (g_dev) { g_dev->Release(); g_dev = nullptr; }
}

void Resize(int w, int h) {
    if (!g_swap || w <= 0 || h <= 0) return;
    g_ctx->OMSetRenderTargets(0, nullptr, nullptr);
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    const HRESULT resized = g_swap->ResizeBuffers(0, (UINT)w, (UINT)h, DXGI_FORMAT_UNKNOWN, 0);
    ID3D11Texture2D* back = nullptr;
    if (FAILED(g_swap->GetBuffer(0, IID_PPV_ARGS(&back)))) return;
    g_dev->CreateRenderTargetView(back, nullptr, &g_rtv);
    back->Release();
    if (SUCCEEDED(resized)) { g_w = w; g_h = h; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_NCCALCSIZE: if(wParam) return 0; break;
        case WM_NCHITTEST: {
            if(g_windowAnimating)return HTCLIENT;
            POINT p{GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};ScreenToClient(hWnd,&p);
            RECT r{};GetClientRect(hWnd,&r);int edge=std::max(5,int(7*g_uiScale));
            if(!IsZoomed(hWnd)){
                bool l=p.x<edge,rr=p.x>=r.right-edge,t=p.y<edge,b=p.y>=r.bottom-edge;
                if(t&&l)return HTTOPLEFT;if(t&&rr)return HTTOPRIGHT;if(b&&l)return HTBOTTOMLEFT;if(b&&rr)return HTBOTTOMRIGHT;
                if(l)return HTLEFT;if(rr)return HTRIGHT;if(t)return HTTOP;if(b)return HTBOTTOM;
            }
            if(p.y<int(38*g_uiScale)&&p.x<r.right-int(132*g_uiScale))return HTCAPTION;
            return HTCLIENT;
        }
        case WM_GETMINMAXINFO: {
            auto* mm=reinterpret_cast<MINMAXINFO*>(lParam);
            mm->ptMinTrackSize=g_windowAnimating?POINT{1,1}:POINT{784,520};
            MONITORINFO mi{sizeof(mi)};
            if(GetMonitorInfoW(MonitorFromWindow(hWnd,MONITOR_DEFAULTTONEAREST),&mi)){
                mm->ptMaxPosition={mi.rcWork.left-mi.rcMonitor.left,mi.rcWork.top-mi.rcMonitor.top};
                mm->ptMaxSize={mi.rcWork.right-mi.rcWork.left,mi.rcWork.bottom-mi.rcWork.top};
            }
            return 0;
        }
        case WM_ENTERSIZEMOVE:SetTimer(hWnd,1,8,nullptr);return 0;
        case WM_EXITSIZEMOVE:KillTimer(hWnd,1);if(g_frame)g_frame();return 0;
        case WM_TIMER:if(wParam==1&&g_frame)g_frame();return 0;
        case WM_SIZE:
            if(wParam==SIZE_MINIMIZED)g_wasMinimized=true;
            else {
                if(g_wasMinimized){g_restoreRequested=true;g_wasMinimized=false;}
                g_pendingW=LOWORD(lParam);g_pendingH=HIWORD(lParam);
            }
            return 0;
        case WM_ERASEBKGND:return 1;
        case WM_DPICHANGED:{auto* r=reinterpret_cast<RECT*>(lParam);SetWindowPos(hWnd,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);return 0;}
        case WM_SYSCOMMAND:
            if((wParam&0xFFF0)==SC_KEYMENU)return 0;
            if((wParam&0xFFF0)==SC_MINIMIZE&&!g_nativeMinimize&&g_frame){g_minimizeRequested=true;return 0;}
            break;
        case WM_DESTROY:KillTimer(hWnd,1);PostQuitMessage(0);return 0;
    }
    if(ImGui_ImplWin32_WndProcHandler(hWnd,msg,wParam,lParam))return true;
    return DefWindowProcW(hWnd,msg,wParam,lParam);
}

struct Offscreen {
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    ID3D11Texture2D* tex = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11Texture2D* stage = nullptr;
    int w = 0, h = 0;

    ~Offscreen() {
        if (ctx) ctx->ClearState();
        if (stage) stage->Release();
        if (rtv) rtv->Release();
        if (tex) tex->Release();
        if (ctx) ctx->Release();
        if (dev) dev->Release();
    }

    bool Init(int W, int H) {
        w = W; h = H;
        const D3D_FEATURE_LEVEL lv[] = {D3D_FEATURE_LEVEL_11_0};
        if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, lv, 1,
                                     D3D11_SDK_VERSION, &dev, nullptr, &ctx)))
            return false;
        D3D11_TEXTURE2D_DESC d{};
        d.Width = (UINT)W; d.Height = (UINT)H; d.MipLevels = 1; d.ArraySize = 1;
        d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        d.SampleDesc.Count = 1;
        d.Usage = D3D11_USAGE_DEFAULT;
        d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(dev->CreateTexture2D(&d, nullptr, &tex))) return false;
        if (FAILED(dev->CreateRenderTargetView(tex, nullptr, &rtv))) return false;
        d.Usage = D3D11_USAGE_STAGING;
        d.BindFlags = 0;
        d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        if (FAILED(dev->CreateTexture2D(&d, nullptr, &stage))) return false;
        return true;
    }

    bool SaveBmp(const std::wstring& path) {
        ctx->CopyResource(stage, tex);
        D3D11_MAPPED_SUBRESOURCE m{};
        if (FAILED(ctx->Map(stage, 0, D3D11_MAP_READ, 0, &m))) return false;

        const int rowBytes = w * 4;
        const int dataSize = rowBytes * h;
        const int fileSize = 54 + dataSize;
        std::vector<unsigned char> buf(fileSize);
        buf[0] = 'B'; buf[1] = 'M';
        memcpy(&buf[2], &fileSize, 4);
        int off = 54; memcpy(&buf[10], &off, 4);
        int hs = 40; memcpy(&buf[14], &hs, 4);
        memcpy(&buf[18], &w, 4);
        int negH = -h; memcpy(&buf[22], &negH, 4);
        unsigned short planes = 1, bpp = 32;
        memcpy(&buf[26], &planes, 2);
        memcpy(&buf[28], &bpp, 2);
        memcpy(&buf[34], &dataSize, 4);

        const unsigned char* src = (const unsigned char*)m.pData;
        for (int y = 0; y < h; ++y) {
            const unsigned char* row = src + y * m.RowPitch;
            unsigned char* dst = &buf[54 + y * rowBytes];
            for (int x = 0; x < w; ++x) {
                dst[x * 4 + 0] = row[x * 4 + 2];
                dst[x * 4 + 1] = row[x * 4 + 1];
                dst[x * 4 + 2] = row[x * 4 + 0];
                dst[x * 4 + 3] = 255;
            }
        }
        ctx->Unmap(stage, 0);

        HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, nullptr);
        if (f == INVALID_HANDLE_VALUE) return false;
        DWORD written = 0;
        const BOOL ok = WriteFile(f, buf.data(), (DWORD)buf.size(), &written, nullptr);
        CloseHandle(f);
        return ok && written == buf.size();
    }
};

struct NativeWindowAnimation {
    hb::WindowMotion motion;
    hb::WindowSnapshot snapshot;
    RECT originalBounds{};
    WINDOWPLACEMENT placement{sizeof(WINDOWPLACEMENT)};
    LONG_PTR originalStyle=0,originalExStyle=0;
    bool active=false,minimized=false;

    void PrepareWindow() {

        g_windowAnimating=true;
        SetWindowLongPtrW(g_hwnd,GWL_STYLE,originalStyle&~WS_MAXIMIZE);
        SetWindowLongPtrW(g_hwnd,GWL_EXSTYLE,originalExStyle|WS_EX_LAYERED);
    }
    bool BeginMinimize(ID3D11RenderTargetView* source,bool animate) {
        if(active||minimized)return false;
        if(!snapshot.Capture(g_dev,g_ctx,source))return false;
        GetWindowRect(g_hwnd,&originalBounds);GetWindowPlacement(g_hwnd,&placement);
        originalStyle=GetWindowLongPtrW(g_hwnd,GWL_STYLE);
        originalExStyle=GetWindowLongPtrW(g_hwnd,GWL_EXSTYLE);
        PrepareWindow();
        if(!SetLayeredWindowAttributes(g_hwnd,0,255,LWA_ALPHA)){
            SetWindowLongPtrW(g_hwnd,GWL_STYLE,originalStyle);
            SetWindowLongPtrW(g_hwnd,GWL_EXSTYLE,originalExStyle);
            snapshot.image.Release();g_windowAnimating=false;return false;
        }
        motion.BeginMinimize(animate);active=true;
        return true;
    }
    bool BeginRestore(bool animate) {
        if(!minimized||!snapshot.image.Valid())return false;
        minimized=false;active=true;

        GetWindowRect(g_hwnd,&originalBounds);GetWindowPlacement(g_hwnd,&placement);
        originalStyle=GetWindowLongPtrW(g_hwnd,GWL_STYLE);
        PrepareWindow();motion.BeginRestore(animate);
        return true;
    }
    void ApplyBounds() {
        const auto pose=motion.GetPose();
        const int width=originalBounds.right-originalBounds.left,height=originalBounds.bottom-originalBounds.top;
        const int w=std::max(1,int(std::lround(width*pose.width))),h=std::max(1,int(std::lround(height*pose.height)));
        const int x=originalBounds.left+(width-w)/2;
        const int y=originalBounds.top+(height-h)/2+int(std::lround(height*pose.drop));
        RECT current{};GetWindowRect(g_hwnd,&current);
        if(current.left!=x||current.top!=y||current.right-current.left!=w||current.bottom-current.top!=h)
            SetWindowPos(g_hwnd,nullptr,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
    }
    void RestorePlacement() {
        SetWindowLongPtrW(g_hwnd,GWL_STYLE,originalStyle);
        SetWindowPlacement(g_hwnd,&placement);
    }
    void AfterPresent() {
        SetLayeredWindowAttributes(g_hwnd,0,BYTE(std::lround(motion.GetPose().opacity*255.f)),LWA_ALPHA);
        if(motion.phase==hb::WindowMotion::Minimized){

            RestorePlacement();
            g_nativeMinimize=true;ShowWindow(g_hwnd,SW_MINIMIZE);g_nativeMinimize=false;
            active=false;minimized=true;g_windowAnimating=false;
        }else if(motion.phase==hb::WindowMotion::Idle){
            RestorePlacement();SetWindowLongPtrW(g_hwnd,GWL_EXSTYLE,originalExStyle);
            active=false;g_windowAnimating=false;snapshot.image.Release();
        }
    }
    void Shutdown(){snapshot.image.Release();}
};

struct Options {
    bool capture = false, uiOnly = false, wallpaperOnly = false, exercise = false;
    bool introOnly = false;
    bool exerciseSegments = false;
    bool exerciseFonts = false;
    bool exerciseWindow = false;
    bool exerciseNativeWindow = false;
    std::wstring out = L"preview\\ganyu-revision";
    int fps = 60, width = 1600, height = 900, theme = 0, iceStyle = 2, page = 0;
    float duration = 7.5f, start = 0.f, skipAt = -1.f;
    float fontPercent=112.f;
};

struct Experience {
    hb::Background bg;
    hb::Intro intro;
    hb::Shell shell;
    bool shellReady = false;
    bool introOnly = false;
    float time = 0.f, preferredScale = 1.f, backgroundTime = 0.f;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3D11Texture2D* aaTexture = nullptr;
    ID3D11RenderTargetView* aaTarget = nullptr;
    int aaWidth = 0, aaHeight = 0;
    bool usingAA = false;

    void ReleaseAA() {
        if (aaTarget) { aaTarget->Release(); aaTarget = nullptr; }
        if (aaTexture) { aaTexture->Release(); aaTexture = nullptr; }
        aaWidth = aaHeight = 0;
    }

    void PrepareAA(int w, int h) {
        if (w == aaWidth && h == aaHeight) return;
        ReleaseAA();
        aaWidth = w; aaHeight = h;
        UINT quality = 0;
        if (FAILED(device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality)) || !quality) return;
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = w; desc.Height = h; desc.MipLevels = desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 4;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        if (FAILED(device->CreateTexture2D(&desc, nullptr, &aaTexture))) return;
        if (FAILED(device->CreateRenderTargetView(aaTexture, nullptr, &aaTarget))) {
            aaTexture->Release(); aaTexture = nullptr;
        }
    }

    bool Init(ID3D11Device* dev, ID3D11DeviceContext* ctx,
              int w, int h, float scale, const Options& options) {
        preferredScale = scale;
        device = dev; context = ctx;
        introOnly = options.introOnly;
        hb::Fonts& fonts = hb::LoadFonts(scale);
        if (!bg.Init(dev, ctx, "tex/ganyu-bg-4k.jpg")) {
            Shutdown();
            return false;
        }
        intro.Reset(float(w), float(h));
        if (!intro.material.Init(dev, ctx)) {
            Shutdown();
            return false;
        }
        intro.Bind(fonts.opening, fonts.ui, fonts.caps);
        intro.material.SetStyle(options.iceStyle);
        shell.Layout(float(w), float(h), scale);
        shell.ApplyFonts(fonts.ui, fonts.display, fonts.caps);
        shell.targetFigure=hb::LoadTexture(dev,"tex/target-figure.png");
        if(!shell.targetFigure.Valid()){Shutdown();return false;}
        shell.theme = shell.oldTheme = options.theme;
        shell.themeFrom = hb::ThemeAt(options.theme).skin;
        shell.m.themeSel = options.theme;
        shell.m.navSel = options.page;
        shell.m.fontPercent=options.fontPercent;
        if (options.uiOnly) {
            intro.finished = true;
            intro.t = hb::Intro::kDuration;
        }
        return true;
    }

    void Tick(float dt, bool skip) {
        dt = std::clamp(dt, 0.f, .05f);
        time += dt;
        if(!shellReady || shell.m.backgroundMotion) backgroundTime += dt;
        if (skip) intro.Skip();
        intro.Update(dt);
        if ((intro.t >= hb::Intro::kDockStart || intro.finished) && !shellReady && !introOnly) {
            shellReady = true;
            shell.ReplayEnter();
            shell.Begin(0.f);
        } else if (shellReady) {
            shell.Begin(dt);
        }
    }

    ID3D11RenderTargetView* Draw(ID3D11RenderTargetView* rtv, float w, float h, bool wallpaperOnly) {
        if (w != intro.W || h != intro.H) {
            intro.W = w; intro.H = h;
            shell.Layout(w, h, preferredScale);
        }

        usingAA = !wallpaperOnly && (!intro.finished || shell.sceneMix > .001f);
        if (usingAA) PrepareAA(int(w), int(h));
        usingAA = usingAA && aaTarget;
        if (usingAA) rtv = aaTarget;
        hb::Background::View view;
        if(shellReady&&!wallpaperOnly) {
            const float k=hb::Ease(hb::E_PAGE,hb::Clamp01(shell.enterT/1.2f));
            view.zoom=1.f+.22f*k;
            view.focusX=.5f-.105f*k;
            view.focusY=.5f-.015f*k;
            view.dim=(1.f-shell.wallpaperMix.V(.82f))*.65f*k;
        }
        bg.Render(rtv, w, h, backgroundTime, view);
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if(!shellReady || shell.m.particles || wallpaperOnly) bg.DrawParticles(dl, w, h, backgroundTime, view);
        if (!wallpaperOnly) {
            intro.dockCenter=shell.LogoCenter();
            intro.dockRadius=shell.LogoRadius();
            intro.Draw(dl, ImVec2(0, 0));
            if (shellReady) shell.Draw(dl);
            intro.DrawMark(dl, ImVec2(0, 0), shellReady ? shell.sceneMix : 1.f);
        }
        return rtv;
    }

    void Resolve(ID3D11RenderTargetView* destination) {
        if (!usingAA) return;
        ID3D11Resource* target = nullptr;
        destination->GetResource(&target);
        context->OMSetRenderTargets(0, nullptr, nullptr);
        context->ResolveSubresource(target, 0, aaTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
        target->Release();
    }

    void Shutdown() { shell.targetFigure.Release(); ReleaseAA(); intro.Shutdown(); bg.Shutdown(); }
};

void ProbeInput(const Experience& scene, float time, float start) {
    const float t=time-start;const auto& s=scene.shell;
    float x=-100,y=-100;bool down=false;
    auto click=[&](float at,float xx,float yy){if(t>=at&&t<at+.12f){x=xx;y=yy;down=t<at+.05f;}};
    float w=s.W-232,aimLeft=w-std::clamp(w*.37f,270.f,360.f)-18;
    click(0,204+aimLeft-40,167);
    if(t>=.25f&&t<.6f){x=hb::Lerp(224.f,204+aimLeft-24,hb::Clamp01((t-.25f)/.3f));y=331;down=t<.57f;}
    click(.8f,85,231);
    float visLeft=w-std::clamp(w*.44f,310.f,440.f)-18;
    click(1.3f,204+visLeft-39,226);
    click(1.6f,85,285);
    click(2.2f,85,339);
    click(2.8f,260,473);
    click(3.1f,85,393);
    float cw=(w-28)/3.f;
    click(3.65f,204+cw+14+cw*.5f,210);
    click(4.25f,204+2*(cw+14)+cw*.5f,210);
    click(4.85f,204+cw*.5f,210);
    click(5.0f,204+cw+14+cw*.5f,210);
    click(5.15f,204+2*(cw+14)+cw*.5f,210);
    click(5.3f,204+cw*.5f,210);
    click(5.9f,85,177);
    ImGuiIO& io=ImGui::GetIO();
    io.AddMousePosEvent(x*s.scale,y*s.scale);io.AddMouseButtonEvent(0,down);
}

void ProbeSegments(const Experience& scene,float time) {
    float elapsed=time-2.f,x=-100.f,y=-100.f;bool down=false;
    float width=scene.shell.W-232;
    float leftWidth=width-std::clamp(width*.44f,310.f,440.f)-18;
    auto click=[&](float at,int option,float row){
        if(elapsed>=at&&elapsed<at+.12f){
            x=224.f+(leftWidth-40.f)*(float(option)+.5f)/3.f;
            y=row;down=elapsed<at+.05f;
        }
    };
    click(.15f,0,449.f);click(.55f,2,449.f);click(.95f,1,449.f);
    click(1.35f,2,533.f);click(1.75f,1,533.f);click(2.15f,0,533.f);
    auto& io=ImGui::GetIO();io.AddMousePosEvent(x*scene.shell.scale,y*scene.shell.scale);
    io.AddMouseButtonEvent(0,down);
}

void ProbeFonts(const Experience& scene,float time) {
    const auto& shell=scene.shell;
    const float elapsed=time-2.f;
    const float left=(shell.W-250.f)*.54f,trackStart=224.f,trackWidth=left-110.f;
    float x=-100,y=-100;bool down=false;
    if(elapsed>=.15f&&elapsed<.9f) {
        x=trackStart+trackWidth*hb::Clamp01((elapsed-.15f)/.6f);y=591.f;down=elapsed<.85f;
    }
    if(elapsed>=1.15f&&elapsed<1.28f){x=204.f+left-51.f;y=579.f;down=elapsed<1.21f;}
    auto& io=ImGui::GetIO();io.AddMousePosEvent(x*shell.scale,y*shell.scale);io.AddMouseButtonEvent(0,down);
}

void ProbeWindow(const Experience& scene,float time) {
    const bool over=time>=2.f&&time<2.12f;
    auto& io=ImGui::GetIO();
    io.AddMousePosEvent(over?(scene.shell.W-110.f)*scene.shell.scale:-100.f,over?19.f*scene.shell.scale:-100.f);
    io.AddMouseButtonEvent(0,over&&time<2.05f);
}

struct CaptureTiming {
    ID3D11Query* disjoint = nullptr;
    ID3D11Query* first = nullptr;
    ID3D11Query* last = nullptr;
    LARGE_INTEGER frequency{}, started{};
    std::vector<double> cpu, gpu;
    double cpuMs = 0;
    bool ready = false;
    explicit CaptureTiming(ID3D11Device* dev) {
        QueryPerformanceFrequency(&frequency);
        D3D11_QUERY_DESC desc{D3D11_QUERY_TIMESTAMP_DISJOINT, 0};
        if (FAILED(dev->CreateQuery(&desc, &disjoint))) return;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        if (FAILED(dev->CreateQuery(&desc, &first))) return;
        ready = SUCCEEDED(dev->CreateQuery(&desc, &last));
    }
    ~CaptureTiming() {
        if (disjoint) disjoint->Release();
        if (first) first->Release();
        if (last) last->Release();
    }
    void Begin(ID3D11DeviceContext* ctx) {
        QueryPerformanceCounter(&started);
        if (ready) { ctx->Begin(disjoint); ctx->End(first); }
    }
    void End(ID3D11DeviceContext* ctx) {
        if (ready) { ctx->End(last); ctx->End(disjoint); }
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        cpuMs = double(now.QuadPart - started.QuadPart) * 1000.0 / frequency.QuadPart;
    }
    void Collect(ID3D11DeviceContext* ctx) {
        cpu.push_back(cpuMs);
        if (!ready) return;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data{};
        UINT64 a = 0, b = 0;
        if (ctx->GetData(disjoint, &data, sizeof(data), 0) == S_OK && !data.Disjoint &&
            ctx->GetData(first, &a, sizeof(a), 0) == S_OK &&
            ctx->GetData(last, &b, sizeof(b), 0) == S_OK)
            gpu.push_back(double(b - a) * 1000.0 / data.Frequency);
    }
    void Report() {
        auto report = [](const char* label, std::vector<double>& samples) {
            if (samples.empty()) { printf("timing %s unavailable\n", label); return; }
            std::sort(samples.begin(), samples.end());
            printf("timing %s n=%zu median=%.3fms p95=%.3fms max=%.3fms\n", label, samples.size(),
                samples[samples.size()/2], samples[size_t((samples.size()-1)*.95)], samples.back());
        };
        report("CPU-submit", cpu); report("GPU", gpu);
    }
};

int RunRenderClip(const Options& options) {
    const int W = options.width, H = options.height, fps = options.fps;
    std::error_code error;
    std::filesystem::create_directories(options.out, error);
    if (error) return 1;
    Offscreen off;
    if (!off.Init(W, H)) { printf("offscreen init failed\n"); return 1; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = ImVec2((float)W, (float)H);
    ImGui::StyleColorsDark();
    ImGui_ImplDX11_Init(off.dev, off.ctx);

    Experience scene;
    const float scale = std::min({W / hb::kBaseW, H / hb::kBaseH, hb::kMaxScale});
    if (!scene.Init(off.dev, off.ctx, W, H, scale, options)) {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        return 1;
    }
    char selfTest[128];
    bool ok = hb::IntroSelfTest(selfTest, sizeof(selfTest));
    printf("%s\n", selfTest);
    const bool motionOk=hb::WindowMotionSelfTest();
    printf("window motion: 30/60/120/240fps, repeat clicks, disabled animations %s\n",motionOk?"PASS":"FAIL");
    ok=ok&&motionOk;
    const int total = int(std::lround((options.start + options.duration) * fps)) + 1;
    const int first = int(std::lround(options.start * fps));
    const float dt = 1.f / fps;
    const float probeStart = options.uiOnly ? 1.6f : 6.2f;
    bool sawLight = false, sawDark = false,sawSmallFont=false,sawLargeFont=false;
    hb::WindowSnapshot windowSnapshot;
    hb::WindowMotion windowMotion;
    bool capturedWindow=false,sawMinimized=false,returnedWindow=false;
    CaptureTiming timings(off.dev);

    for (int i = 0; i < total && ok; ++i) {
        timings.Begin(off.ctx);
        ImGui_ImplDX11_NewFrame();
        io.DisplaySize = ImVec2((float)W, (float)H);
        io.DeltaTime = dt;
        if (options.exercise) ProbeInput(scene, i * dt, probeStart);
        if (options.exerciseSegments) ProbeSegments(scene, i * dt);
        if (options.exerciseFonts) ProbeFonts(scene, i * dt);
        if (options.exerciseWindow) ProbeWindow(scene, i * dt);
        ImGui::NewFrame();
        const bool motionFrame=options.exerciseWindow&&capturedWindow&&!returnedWindow;
        ID3D11RenderTargetView* drawTarget=off.rtv;
        if(motionFrame){
            if(i*dt>=2.7f&&windowMotion.phase==hb::WindowMotion::Minimized)windowMotion.BeginRestore();
            windowMotion.Update(dt);
            sawMinimized|=windowMotion.phase==hb::WindowMotion::Minimized;
            const auto pose=windowMotion.GetPose();
            const float desktop[]={.07f,.09f,.13f,1.f};off.ctx->ClearRenderTargetView(off.rtv,desktop);
            const float w=W*pose.width,h=H*pose.height,x=(W-w)*.5f,y=(H-h)*.5f+H*pose.drop;
            windowSnapshot.Draw(ImGui::GetBackgroundDrawList(),{x,y},{x+w,y+h},pose.opacity);
            returnedWindow=windowMotion.phase==hb::WindowMotion::Idle;
        }else{
            scene.Tick(i ? dt : 0.f, options.skipAt >= 0.f && i * dt >= options.skipAt);
            drawTarget=scene.Draw(off.rtv, float(W), float(H), options.wallpaperOnly);
        }
        sawLight |= scene.shell.theme == hb::UI_THEME_LIGHT;
        sawDark |= scene.shell.theme == hb::UI_THEME_DARK;
        sawSmallFont |= scene.shell.m.fontPercent==100.f;
        sawLargeFont |= scene.shell.m.fontPercent==125.f;
        ImGui::Render();
        off.ctx->OMSetRenderTargets(1, &drawTarget, nullptr);
        D3D11_VIEWPORT vp{0, 0, (float)W, (float)H, 0, 1};
        off.ctx->RSSetViewports(1, &vp);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if(!motionFrame)scene.Resolve(off.rtv);
        if(options.exerciseWindow&&!capturedWindow&&scene.shell.windowAction==1){
            capturedWindow=windowSnapshot.Capture(off.dev,off.ctx,off.rtv);
            scene.shell.windowAction=0;windowMotion.BeginMinimize();
            ok=ok&&capturedWindow;
        }
        timings.End(off.ctx);

        if (i >= first) {
            wchar_t name[40];
            swprintf_s(name, L"f%04d.bmp", i - first);
            ok = off.SaveBmp((std::filesystem::path(options.out) / name).wstring());
            if (i > first) timings.Collect(off.ctx);
        }
        if (i % 60 == 0) printf("frame %d/%d\n", i, total);
    }
    timings.Report();
    printf("opening antialias: %s\n", scene.aaTarget ? "4x MSAA" : "inactive/unavailable");

    if (options.exercise) {
        const bool probeOk = !scene.shell.m.aimMaster && scene.shell.m.fov > 30.f &&
            sawLight && sawDark && scene.shell.theme == hb::UI_THEME_GANYU &&
            scene.shell.themeT == 1.f && scene.shell.cx.activeId == -1 &&
            !scene.shell.m.boxes && scene.shell.presetsSaved[0] && scene.shell.m.navSel==0;
        printf("input: toggle=%d fov=%.2f light=%d dark=%d final=%d %s\n",
            scene.shell.m.aimMaster, scene.shell.m.fov, sawLight, sawDark,
            scene.shell.theme, probeOk ? "PASS" : "FAIL");
        ok = ok && probeOk;
    }
    if(options.exerciseSegments){
        bool isolated=scene.shell.m.curve==1&&scene.shell.m.reticle==0;
        printf("segments: curve=%d reticle=%d %s\n",scene.shell.m.curve,scene.shell.m.reticle,isolated?"PASS":"FAIL");
        ok=ok&&isolated;
    }
    if(options.exerciseFonts){
        const bool fontsOk=sawSmallFont&&sawLargeFont&&scene.shell.m.fontPercent==112.f&&
            std::abs(scene.shell.cx.textScale-1.12f)<.001f&&scene.shell.cx.activeId==-1;
        printf("font slider: min=%d max=%d reset=%.0f%% %s\n",sawSmallFont,sawLargeFont,scene.shell.m.fontPercent,fontsOk?"PASS":"FAIL");
        ok=ok&&fontsOk;
    }
    if(options.exerciseWindow){
        const bool transitionOk=capturedWindow&&sawMinimized&&returnedWindow&&windowMotion.amount==0.f&&
            scene.shell.m.navSel==options.page&&scene.shell.m.fontPercent==options.fontPercent;
        printf("minimize button: captured=%d hidden=%d restored=%d preserved-page=%d %s\n",capturedWindow,
               sawMinimized,returnedWindow,scene.shell.m.navSel,transitionOk?"PASS":"FAIL");
        ok=ok&&transitionOk;
    }
    const auto mark=scene.intro.Pose({0,0});
    printf("logo: x=%.4f y=%.4f radius=%.4f opacity=%.4f font=%.0f%%\n",mark.center.x,mark.center.y,mark.radius,mark.opacity,scene.shell.m.fontPercent);
    printf("layout: %.1f,%.1f %.1fx%.1f scale=%.3f\n", scene.shell.originX,
        scene.shell.originY, scene.shell.W * scene.shell.scale,
        scene.shell.H * scene.shell.scale, scene.shell.scale);
    windowSnapshot.image.Release();scene.Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
    printf("capture: %s (%d frames)\n", ok ? "PASS" : "FAIL", total - first);
    return ok ? 0 : 1;
}

}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    Options options;
    for (int i = 1; i < argc; ++i) {
        if (wcscmp(argv[i], L"--render-clip") == 0) options.capture = true;
        else if (wcscmp(argv[i], L"--out") == 0 && i + 1 < argc) {
            options.out = argv[++i];
        } else if (wcscmp(argv[i], L"--fps") == 0 && i + 1 < argc) options.fps = _wtoi(argv[++i]);
        else if (wcscmp(argv[i], L"--size") == 0 && i + 2 < argc) {
            options.width = _wtoi(argv[++i]);
            options.height = _wtoi(argv[++i]);
        }
        else if (wcscmp(argv[i], L"--theme") == 0 && i + 1 < argc) options.theme = _wtoi(argv[++i]);
        else if (wcscmp(argv[i], L"--page") == 0 && i + 1 < argc) options.page = _wtoi(argv[++i]);
        else if (wcscmp(argv[i], L"--ice-style") == 0 && i + 1 < argc) options.iceStyle = _wtoi(argv[++i]);
        else if (wcscmp(argv[i], L"--duration") == 0 && i + 1 < argc) options.duration = float(_wtof(argv[++i]));
        else if (wcscmp(argv[i], L"--start") == 0 && i + 1 < argc) options.start = float(_wtof(argv[++i]));
        else if (wcscmp(argv[i], L"--skip-at") == 0 && i + 1 < argc) options.skipAt = float(_wtof(argv[++i]));
        else if (wcscmp(argv[i], L"--ui-only") == 0) options.uiOnly = true;
        else if (wcscmp(argv[i], L"--intro-only") == 0) options.introOnly = true;
        else if (wcscmp(argv[i], L"--wallpaper-only") == 0) options.wallpaperOnly = true;
        else if (wcscmp(argv[i], L"--exercise-ui") == 0) options.exercise = true;
        else if (wcscmp(argv[i], L"--exercise-segments") == 0) options.exerciseSegments = true;
        else if (wcscmp(argv[i], L"--exercise-fonts") == 0) options.exerciseFonts = true;
        else if (wcscmp(argv[i], L"--exercise-window") == 0) options.exerciseWindow = true;
        else if (wcscmp(argv[i], L"--exercise-native-window") == 0) options.exerciseNativeWindow = true;
        else if (wcscmp(argv[i], L"--font-percent") == 0 && i + 1 < argc) options.fontPercent=float(_wtof(argv[++i]));
    }
    if (argv) LocalFree(argv);
    if(options.exerciseNativeWindow)options.uiOnly=true;
    if (options.fps < 20 || options.fps > 240 || options.width < 320 || options.height < 240 ||
        options.width > 7680 || options.height > 4320 || options.theme < 0 || options.theme >= 3 || options.page < 0 || options.page > 4 || options.iceStyle < 0 || options.iceStyle > 2 ||
        !std::isfinite(options.fontPercent) || options.fontPercent<100.f || options.fontPercent>125.f ||
        !std::isfinite(options.duration) || options.duration < 0.f || options.duration > 60.f ||
        !std::isfinite(options.start) || options.start < 0.f || options.start > 60.f) return 2;
    if (options.capture) return RunRenderClip(options);

    ImGui_ImplWin32_EnableDpiAwareness();
    const int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    RECT wa{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
    HDC dc = GetDC(nullptr);
    const float dpi = (float)GetDeviceCaps(dc, LOGPIXELSX) / 96.f;
    ReleaseDC(nullptr, dc);

    const hb::DisplayFit fit = hb::FitToDisplay(sw, sh, wa.right - wa.left, wa.bottom - wa.top, dpi);

    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0, 0, hInst, nullptr, LoadCursor(nullptr,IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH),
                      nullptr, L"HanaBiWnd", nullptr};
    RegisterClassExW(&wc);
    g_hwnd = CreateWindowExW(options.exerciseNativeWindow?WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE:WS_EX_APPWINDOW, L"HanaBiWnd", options.introOnly ? L"HanaBi - Opening Review" : L"HanaBi", WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
                             fit.posX, fit.posY, fit.pxW, fit.pxH, nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hwnd) return 0;
    if(options.exerciseNativeWindow){

        const BOOL cloak=TRUE;DWORD cloaked=0;
        const HRESULT result=DwmSetWindowAttribute(g_hwnd,DWMWA_CLOAK,&cloak,sizeof(cloak));
        DwmGetWindowAttribute(g_hwnd,DWMWA_CLOAKED,&cloaked,sizeof(cloaked));
        if(FAILED(result)||!cloaked){DestroyWindow(g_hwnd);UnregisterClassW(wc.lpszClassName,wc.hInstance);return 3;}
    }
    if (!CreateDeviceD3D(g_hwnd, fit.pxW, fit.pxH)) {
        CleanupD3D();
        DestroyWindow(g_hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_dev, g_ctx);

    Experience scene;
    if (!scene.Init(g_dev, g_ctx, fit.pxW, fit.pxH, fit.uiScale, options)) {
        MessageBoxW(g_hwnd, L"Embedded wallpaper could not be loaded.",
                    L"HanaBi", MB_OK | MB_ICONERROR);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupD3D();
        DestroyWindow(g_hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    LARGE_INTEGER freq, last;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    bool running = true;
    NativeWindowAnimation windowAnimation;
    const LONG_PTR startupExStyle=GetWindowLongPtrW(g_hwnd,GWL_EXSTYLE);
    bool windowShown=false,startupFading=false;
    bool startupHidden=!IsWindowVisible(g_hwnd),startupFrameReady=false;
    float startupElapsed=0.f;
    auto finishStartup=[&] {
        if(!startupFading)return;
        SetLayeredWindowAttributes(g_hwnd,0,255,LWA_ALPHA);
        SetWindowLongPtrW(g_hwnd,GWL_EXSTYLE,startupExStyle);
        startupFading=false;
    };
    int nativeStage=0,nativeCycle=0,nativeMotionFrames=0;
    bool nativeTestOk=true;
    RECT expectedBounds{};DWORD hiddenAt=0,testStarted=GetTickCount();
    g_frame=[&] {
        if(g_rendering||IsIconic(g_hwnd)||!g_rtv)return;
        g_rendering=true;
        if(!windowShown)startupHidden&=!IsWindowVisible(g_hwnd);
        LARGE_INTEGER now;QueryPerformanceCounter(&now);
        float dt=std::clamp(float(now.QuadPart-last.QuadPart)/float(freq.QuadPart),0.f,.05f);last=now;
        if(g_restoreRequested){
            g_restoreRequested=false;
            windowAnimation.BeginRestore(scene.shell.m.animations);
            dt=0.f;
        }
        if(windowAnimation.active){
            if(options.exerciseNativeWindow)++nativeMotionFrames;
            if(g_minimizeRequested){
                g_minimizeRequested=false;
                windowAnimation.motion.BeginMinimize(scene.shell.m.animations);
            }
            windowAnimation.motion.Update(dt);
            windowAnimation.ApplyBounds();
            if(g_pendingW>0&&g_pendingH>0){
                if(g_w!=g_pendingW||g_h!=g_pendingH)Resize(g_pendingW,g_pendingH);
                g_pendingW=g_pendingH=0;
            }
            ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
            const float clear[]={0,0,0,1};g_ctx->ClearRenderTargetView(g_rtv,clear);
            windowAnimation.snapshot.Draw(ImGui::GetBackgroundDrawList(),{0,0},{float(g_w),float(g_h)});
            ImGui::Render();g_ctx->OMSetRenderTargets(1,&g_rtv,nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());g_swap->Present(1,0);
            windowAnimation.AfterPresent();
            g_rendering=false;return;
        }
        if(g_pendingW>0&&g_pendingH>0){
            if(g_w!=g_pendingW||g_h!=g_pendingH)Resize(g_pendingW,g_pendingH);
            g_pendingW=g_pendingH=0;
        }
        ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
        if(options.introOnly&&ImGui::IsKeyPressed(ImGuiKey_Escape))running=false;
        if(options.introOnly&&(ImGui::IsKeyPressed(ImGuiKey_Space)||ImGui::IsKeyPressed(ImGuiKey_R)))scene.intro.Reset(float(g_w),float(g_h));
        scene.Tick(dt,!options.introOnly&&ImGui::IsKeyPressed(ImGuiKey_Escape));
        ID3D11RenderTargetView* target=scene.Draw(g_rtv,float(g_w),float(g_h),options.wallpaperOnly);
        g_uiScale=scene.shell.scale;
        ImGui::Render();g_ctx->OMSetRenderTargets(1,&target,nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());scene.Resolve(g_rtv);
        int action=scene.shell.windowAction;scene.shell.windowAction=0;
        if(action==1)g_minimizeRequested=true;
        if(g_minimizeRequested){
            g_minimizeRequested=false;
            finishStartup();
            if(!windowAnimation.BeginMinimize(g_rtv,scene.shell.m.animations)){
                g_nativeMinimize=true;ShowWindow(g_hwnd,SW_MINIMIZE);g_nativeMinimize=false;
            }
        }
        const HRESULT presented=g_swap->Present(1,0);
        if(!windowShown&&!FAILED(presented)){
            startupFrameReady=true;
            SetWindowLongPtrW(g_hwnd,GWL_EXSTYLE,startupExStyle|WS_EX_LAYERED);
            startupFading=SetLayeredWindowAttributes(g_hwnd,0,0,LWA_ALPHA)!=FALSE;
            if(!startupFading)SetWindowLongPtrW(g_hwnd,GWL_EXSTYLE,startupExStyle);
            windowShown=true;
            ShowWindow(g_hwnd,options.exerciseNativeWindow?SW_SHOWNOACTIVATE:SW_SHOWDEFAULT);
            UpdateWindow(g_hwnd);
            if(startupFading)SetLayeredWindowAttributes(g_hwnd,0,1,LWA_ALPHA);
            QueryPerformanceCounter(&last);
            if(options.exerciseNativeWindow){
                nativeTestOk&=startupHidden&&startupFrameReady;
                printf("native startup: hidden-until-frame=%d first-frame=%d %s\n",startupHidden,startupFrameReady,nativeTestOk?"PASS":"FAIL");
            }
        }else if(startupFading&&presented==S_OK){
            startupElapsed+=dt;
            const float t=std::clamp(startupElapsed/.24f,0.f,1.f);
            const float alpha=t*t*t*(t*(t*6.f-15.f)+10.f);
            SetLayeredWindowAttributes(g_hwnd,0,BYTE(std::max(1,int(std::lround(alpha*255.f)))),LWA_ALPHA);
            if(t>=1.f)finishStartup();
        }
        if(action==2)ShowWindow(g_hwnd,IsZoomed(g_hwnd)?SW_RESTORE:SW_MAXIMIZE);
        if(action==3)PostMessageW(g_hwnd,WM_CLOSE,0,0);
        g_rendering=false;
    };
    if(options.exerciseNativeWindow)SetTimer(g_hwnd,2,20,nullptr);
    while(running){
        MSG msg;
        while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
            TranslateMessage(&msg);DispatchMessageW(&msg);
            if(msg.message==WM_QUIT)running=false;
        }
        if(!running)break;
        if(options.exerciseNativeWindow){
            if(GetTickCount()-testStarted>8000){nativeTestOk=false;running=false;printf("native window: timed out at stage=%d cycle=%d\n",nativeStage,nativeCycle);break;}
            if(nativeStage==0&&scene.shell.enterT>1.f){
                const bool revealed=!startupFading&&GetWindowLongPtrW(g_hwnd,GWL_EXSTYLE)==startupExStyle;
                nativeTestOk&=revealed;
                printf("native startup reveal: complete=%d %s\n",revealed,revealed?"PASS":"FAIL");
                GetWindowRect(g_hwnd,&expectedBounds);nativeMotionFrames=0;
                SendMessageW(g_hwnd,WM_SYSCOMMAND,SC_MINIMIZE,0);nativeStage=1;
            }else if(nativeStage==1&&windowAnimation.minimized){
                BYTE alpha=255;DWORD flags=0;COLORREF color=0;
                const bool transparent=GetLayeredWindowAttributes(g_hwnd,&color,&alpha,&flags)&&alpha==0;
                nativeTestOk&=bool(IsIconic(g_hwnd))&&transparent&&nativeMotionFrames>2;
                hiddenAt=GetTickCount();nativeStage=2;
            }else if(nativeStage==2&&GetTickCount()-hiddenAt>50){
                ShowWindow(g_hwnd,SW_RESTORE);nativeStage=3;
            }else if(nativeStage==3&&!windowAnimation.active&&!windowAnimation.minimized){
                RECT actual{};GetWindowRect(g_hwnd,&actual);
                const bool boundsOk=std::abs(actual.left-expectedBounds.left)<=1&&std::abs(actual.top-expectedBounds.top)<=1&&
                    std::abs(actual.right-expectedBounds.right)<=1&&std::abs(actual.bottom-expectedBounds.bottom)<=1;
                const bool styleOk=bool(IsZoomed(g_hwnd))==(nativeCycle==1)&&
                    (GetWindowLongPtrW(g_hwnd,GWL_EXSTYLE)&WS_EX_LAYERED)==0;
                nativeTestOk&=boundsOk&&styleOk&&scene.shell.m.navSel==options.page&&scene.shell.m.fontPercent==options.fontPercent;
                printf("native %s: bounds=%d style=%d motion-frames=%d page=%d %s\n",nativeCycle?"maximized":"normal",
                       boundsOk,styleOk,nativeMotionFrames,scene.shell.m.navSel,nativeTestOk?"PASS":"FAIL");
                if(nativeCycle==0){ShowWindow(g_hwnd,SW_MAXIMIZE);nativeCycle=1;nativeStage=4;}
                else {running=false;break;}
            }else if(nativeStage==4&&!g_pendingW){
                GetWindowRect(g_hwnd,&expectedBounds);nativeMotionFrames=0;
                SendMessageW(g_hwnd,WM_SYSCOMMAND,SC_MINIMIZE,0);nativeStage=1;
            }
        }
        if(IsIconic(g_hwnd)){WaitMessage();continue;}
        g_frame();
        if(options.exerciseNativeWindow)Sleep(8);
    }
    g_frame={};
    finishStartup();
    if(options.exerciseNativeWindow){KillTimer(g_hwnd,2);printf("native window lifecycle: %s\n",nativeTestOk?"PASS":"FAIL");}

    windowAnimation.Shutdown();
    scene.Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return nativeTestOk?0:1;
}
