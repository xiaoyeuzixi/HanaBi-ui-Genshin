#include "assets/embedded.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace hb {
namespace {

struct Entry {
    const wchar_t* key;
    const wchar_t* res;
};

const Entry kEntries[] = {
    {L"fonts/subset/LXGWWenKai-Medium.subset.ttf", L"FONT_LXGW_MEDIUM"},
    {L"fonts/subset/LXGWWenKai-Regular.subset.ttf", L"FONT_LXGW_REGULAR"},
    {L"fonts/CormorantGaramond-Variable.ttf", L"FONT_CORMORANT"},
    {L"fonts/Cinzel-Variable.ttf", L"FONT_CINZEL"},
    {L"fonts/SourceSans3-Light.ttf", L"FONT_OPENING"},
    {L"fonts/SourceSans3-LICENSE.md", L"FONT_OPENING_LICENSE"},
    {L"tex/ganyu-bg-4k.jpg", L"TEX_GANYU_BG_4K"},
    {L"tex/ganyu-flow-mask.png", L"TEX_GANYU_FLOW"},
    {L"tex/ganyu-icon.png", L"TEX_GANYU_ICON"},
    {L"tex/ganyu-flow-rg.png", L"TEX_GANYU_FLOW_RG"},
    {L"tex/ganyu-phase.png", L"TEX_GANYU_PHASE"},
    {L"tex/ganyu-shake-opacity.png", L"TEX_GANYU_SHAKE_OPACITY"},
    {L"tex/ganyu-wave-a.png", L"TEX_GANYU_WAVE_A"},
    {L"tex/ganyu-wave-b.png", L"TEX_GANYU_WAVE_B"},
    {L"tex/ganyu-overlay-alpha.png", L"TEX_GANYU_OVERLAY_ALPHA"},
    {L"tex/target-figure.png", L"TEX_TARGET_FIGURE"},
};

const int kCount = (int)(sizeof(kEntries) / sizeof(kEntries[0]));

std::wstring ToWide(const char* s) {
    if (!s || !*s) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (n <= 0) {

        n = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
        if (n <= 0) return std::wstring();
        std::wstring w(n, L'\0');
        MultiByteToWideChar(CP_ACP, 0, s, -1, &w[0], n);
        if (!w.empty() && w.back() == L'\0') w.pop_back();
        return w;
    }
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}

std::wstring Normalize(std::wstring s) {
    for (auto& c : s) if (c == L'\\') c = L'/';
    while (!s.empty() && s.front() == L'/') s.erase(s.begin());
    return s;
}

}

int AssetCount() { return kCount; }

AssetBlob LoadAsset(const char* path) {
    AssetBlob out;
    if (!path || !*path) return out;

    std::wstring want = Normalize(ToWide(path));
    if (want.empty()) return out;

    const Entry* hit = nullptr;
    for (int i = 0; i < kCount; ++i) {
        if (want == kEntries[i].key) { hit = &kEntries[i]; break; }
    }
    if (!hit) {
        for (int i = 0; i < kCount; ++i) {
            std::wstring k = kEntries[i].key;
            if (want.size() > k.size() &&
                want.compare(want.size() - k.size(), k.size(), k) == 0) {
                hit = &kEntries[i];
                break;
            }
        }
    }
    if (!hit) return out;

    HMODULE mod = GetModuleHandleW(nullptr);
    HRSRC res = FindResourceW(mod, hit->res, MAKEINTRESOURCEW(10));
    if (!res) {
        printf("[emb] resource missing: %ls\n", hit->key);
        return out;
    }
    HGLOBAL glob = LoadResource(mod, res);
    if (!glob) return out;
    const void* ptr = LockResource(glob);
    DWORD sz = SizeofResource(mod, res);
    if (!ptr || !sz) return out;

    out.data = ptr;
    out.size = (size_t)sz;
    return out;
}

}
