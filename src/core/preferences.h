#pragma once
#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <random>
#include <string>
#include <vector>
#include "ui/theme.h"

namespace hb {
struct Preferences {
    int current=0, mode=0, fixed=0;
    float font=112.f, wallpaper=.82f;
    bool motion=true, particles=true, animations=true;
    int accent=0;
    bool operator==(const Preferences&) const = default;
};
inline int ThemeForKey(const wchar_t* key) {
    for(int i=0;i<kThemeCount;++i) {
        std::wstring w; for(const char* p=ThemeAt(i).key;*p;++p)w+=wchar_t(*p);
        if(w==key)return i;
    }
    return kGanyuTheme;
}
inline std::wstring PreferencePath() {
    PWSTR folder=nullptr;
    if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&folder)))return {};
    std::filesystem::path path(folder);CoTaskMemFree(folder);
    path/=L"HanaBi";
    std::error_code ec;std::filesystem::create_directories(path,ec);
    if(ec)return {};
    return (path/L"settings.ini").wstring();
}
inline Preferences LoadPreferences(const std::wstring& path) {
    Preferences p;if(path.empty())return p;
    auto key=[&](const wchar_t* name){wchar_t value[64]{};GetPrivateProfileStringW(L"appearance",name,L"ganyu",value,64,path.c_str());return ThemeForKey(value);};
    auto number=[&](const wchar_t* name,int fallback){return int(GetPrivateProfileIntW(L"appearance",name,fallback,path.c_str()));};
    p.current=key(L"current");p.fixed=key(L"fixed");p.mode=std::clamp(number(L"startup",0),0,2);
    p.font=float(std::clamp(number(L"font",112),100,125));
    p.wallpaper=std::clamp(number(L"wallpaper",820),200,1000)/1000.f;
    p.motion=number(L"motion",1)!=0;p.particles=number(L"particles",1)!=0;p.animations=number(L"animations",1)!=0;
    p.accent=std::clamp(number(L"accent",0),0,3);
    return p;
}
inline bool SavePreferences(const std::wstring& path,const Preferences& p) {
    if(path.empty())return false;
    std::string data="[appearance]\r\nversion=1\r\ncurrent=";
    data+=ThemeAt(p.current).key;data+="\r\nfixed=";data+=ThemeAt(p.fixed).key;
    data+="\r\nstartup="+std::to_string(p.mode)+"\r\nfont="+std::to_string(int(std::round(p.font)));
    data+="\r\nwallpaper="+std::to_string(int(std::round(p.wallpaper*1000)));
    data+="\r\nmotion="+std::to_string(p.motion)+"\r\nparticles="+std::to_string(p.particles)+"\r\nanimations="+std::to_string(p.animations)+"\r\n";
    data+="accent="+std::to_string(p.accent)+"\r\n";
    const auto temp=path+L".tmp";
    HANDLE h=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(h==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;bool ok=WriteFile(h,data.data(),DWORD(data.size()),&written,nullptr)&&written==data.size();
    CloseHandle(h);
    if(ok)ok=MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok)DeleteFileW(temp.c_str());
    return ok;
}
inline int ResolveStartupTheme(const Preferences& p,std::mt19937& rng) {
    if(p.mode==0)return p.current;
    if(p.mode==1)return p.fixed;
    std::vector<int> choices;
    for(int i=0;i<kThemeCount;++i)if(IsCharacterTheme(i))choices.push_back(i);
    if(choices.empty())return kGanyuTheme;
    return choices[std::uniform_int_distribution<size_t>(0,choices.size()-1)(rng)];
}
inline bool PreferencesSelfTest() {
    wchar_t temp[MAX_PATH]{};if(!GetTempPathW(MAX_PATH,temp))return false;
    const auto folder=std::filesystem::path(temp)/(L"HanaBi-settings-test-"+std::to_wstring(GetCurrentProcessId()));
    std::error_code ec;std::filesystem::create_directories(folder,ec);if(ec)return false;
    const auto path=(folder/L"主题.ini").wstring();
    bool ok=LoadPreferences(path)==Preferences{};
    Preferences p;p.current=kYaeTheme;p.fixed=kDarkTheme;p.mode=1;p.font=125;p.wallpaper=.5f;p.motion=false;p.particles=false;p.animations=false;p.accent=3;
    ok=SavePreferences(path,p)&&ok;ok=(LoadPreferences(path)==p)&&ok;
    std::mt19937 rng(73519);
    ok=(ResolveStartupTheme(p,rng)==kDarkTheme)&&ok;
    p.mode=0;ok=(ResolveStartupTheme(p,rng)==kYaeTheme)&&ok;
    p.current=kYoimiyaTheme;p.fixed=kYoimiyaTheme;
    ok=SavePreferences(path,p)&&LoadPreferences(path)==p&&ResolveStartupTheme(p,rng)==kYoimiyaTheme&&ok;
    p.current=kNahidaTheme;p.fixed=kNahidaTheme;p.mode=1;
    ok=SavePreferences(path,p)&&LoadPreferences(path)==p&&ResolveStartupTheme(p,rng)==kNahidaTheme&&ok;
    p.current=kFurinaTheme;p.fixed=kFurinaTheme;
    ok=SavePreferences(path,p)&&LoadPreferences(path)==p&&ResolveStartupTheme(p,rng)==kFurinaTheme&&ok;
    p.current=kFaruzanTheme;p.fixed=kFaruzanTheme;
    ok=SavePreferences(path,p)&&LoadPreferences(path)==p&&ResolveStartupTheme(p,rng)==kFaruzanTheme&&ok;
    p.current=kZhongliTheme;p.fixed=kZhongliTheme;
    ok=SavePreferences(path,p)&&LoadPreferences(path)==p&&ResolveStartupTheme(p,rng)==kZhongliTheme&&ok;
    p.mode=2;unsigned seen=0,expected=0;
    for(int theme=0;theme<kThemeCount;++theme)if(IsCharacterTheme(theme))expected|=1u<<theme;
    for(int i=0;i<2048;++i){int theme=ResolveStartupTheme(p,rng);ok=IsCharacterTheme(theme)&&ok;seen|=1u<<theme;}
    ok=ok&&seen==expected;
    ok=SavePreferences(path,p)&&ok;ok=LoadPreferences(path)==p&&ok;
    WritePrivateProfileStringW(L"appearance",L"current",L"future-missing-character",path.c_str());
    WritePrivateProfileStringW(L"appearance",L"startup",L"999",path.c_str());
    WritePrivateProfileStringW(L"appearance",L"font",L"999",path.c_str());
    const auto invalid=LoadPreferences(path);
    ok=ok&&invalid.current==kGanyuTheme&&invalid.mode==2&&invalid.font==125;
    DeleteFileW(path.c_str());std::filesystem::remove(folder,ec);
    printf("appearance settings: Unicode path, atomic round-trip, restart policy, invalid data, 2048 character-only random draws %s\n",ok?"PASS":"FAIL");
    return ok;
}
}
