#include "assets/fonts.h"

#include "assets/embedded.h"

#include <cstdio>
#include <string>

namespace hb {
namespace {

std::string P(const char* rel) { return std::string(rel); }

const char* kSubsetMedium = "fonts/subset/LXGWWenKai-Medium.subset.ttf";
const char* kSubsetRegular = "fonts/subset/LXGWWenKai-Regular.subset.ttf";
const char* kCormorant = "fonts/CormorantGaramond-Variable.ttf";
const char* kCinzel = "fonts/Cinzel-Variable.ttf";
const char* kOpening = "fonts/SourceSans3-Light.ttf";

static ImVector<ImWchar> MakeRangeFromFont() {
    ImVector<ImWchar> r;
    r.push_back(0x20);  r.push_back(0x7E);
    r.push_back(0x3000); r.push_back(0x30FF);
    r.push_back(0x4E00); r.push_back(0x9FFF);
    r.push_back(0xFF00); r.push_back(0xFFEF);
    r.push_back(0);
    return r;
}

void* FontBytes(const char* rel, int* outSize) {
    AssetBlob b = LoadAsset(P(rel).c_str());
    if (!b.Valid()) {
        printf("[font] embedded miss: %s\n", rel);
        *outSize = 0;
        return nullptr;
    }
    *outSize = (int)b.size;
    return const_cast<void*>(b.data);
}

Fonts g_fonts;

}

Fonts& LoadFonts(float scale) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;
    cfg.PixelSnapH = false;
    cfg.FontDataOwnedByAtlas = false;
    static ImVector<ImWchar> ranges = MakeRangeFromFont();

    int szMed = 0, szReg = 0, szCor = 0, szCin = 0;
    void* med = FontBytes(kSubsetMedium, &szMed);
    void* reg = FontBytes(kSubsetRegular, &szReg);
    void* cor = FontBytes(kCormorant, &szCor);
    void* cin = FontBytes(kCinzel, &szCin);
    printf("[font] embedded: lxgw-med %d, lxgw-reg %d, cormorant %d, cinzel %d bytes\n",
           szMed, szReg, szCor, szCin);

    if (med && szMed > 0) {
        g_fonts.ui = io.Fonts->AddFontFromMemoryTTF(med, szMed, 14.5f * scale, &cfg, ranges.Data);
    }
    if (reg && szReg > 0) {
        g_fonts.uiSm = io.Fonts->AddFontFromMemoryTTF(reg, szReg, 12.5f * scale, &cfg, ranges.Data);
        g_fonts.uiXs = io.Fonts->AddFontFromMemoryTTF(reg, szReg, 10.0f * scale, &cfg, ranges.Data);
    }
    if (med && szMed > 0) {
        g_fonts.uiLg = io.Fonts->AddFontFromMemoryTTF(med, szMed, 20.0f * scale, &cfg, ranges.Data);
        g_fonts.uiXl = io.Fonts->AddFontFromMemoryTTF(med, szMed, 29.0f * scale, &cfg, ranges.Data);
    }

    ImFontConfig fd = cfg;
    fd.GlyphMinAdvanceX = 0.f;
    if (cor && szCor > 0) {
        g_fonts.display = io.Fonts->AddFontFromMemoryTTF(cor, szCor, 54.0f * scale, &fd);
        g_fonts.displaySmall = io.Fonts->AddFontFromMemoryTTF(cor, szCor, 29.0f * scale, &fd);
    }
    if (cin && szCin > 0) {
        g_fonts.caps = io.Fonts->AddFontFromMemoryTTF(cin, szCin, 10.0f * scale, &fd);
    }
    int szOpening = 0;
    if (void* opening = FontBytes(kOpening, &szOpening)) {
        g_fonts.opening = io.Fonts->AddFontFromMemoryTTF(opening, szOpening, 18.f, &fd);
    }

    if (!g_fonts.ui) g_fonts.ui = io.Fonts->AddFontDefault();
    if (!g_fonts.uiSm) g_fonts.uiSm = g_fonts.ui;
    if (!g_fonts.uiXs) g_fonts.uiXs = g_fonts.ui;
    if (!g_fonts.uiLg) g_fonts.uiLg = g_fonts.ui;
    if (!g_fonts.uiXl) g_fonts.uiXl = g_fonts.ui;
    if (!g_fonts.display) g_fonts.display = g_fonts.ui;
    if (!g_fonts.displaySmall) g_fonts.displaySmall = g_fonts.ui;
    if (!g_fonts.caps) g_fonts.caps = g_fonts.ui;
    if (!g_fonts.opening) g_fonts.opening = io.Fonts->AddFontDefault();

    io.FontDefault = g_fonts.ui;
    return g_fonts;
}

void ReloadFonts(float scale) {
    LoadFonts(scale);
}

}
