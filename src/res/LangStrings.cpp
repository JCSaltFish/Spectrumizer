/**
 * @file LangStrings.cpp
 * @brief Implementation of language string resource retrieval functions.
 */

#include "res/LangStrings.h"

#ifdef _WIN32
#include <windows.h>

std::string LangStrings::get(Lang lang) {
    LPCSTR name = nullptr;

    switch (lang) {
    case Lang::EN_US:
        name = "LANG_EN_US_JSON";
        break;
    case Lang::ZH_CN:
        name = "LANG_ZH_CN_JSON";
        break;
    default:
        return {};
    }

    HMODULE hExe = GetModuleHandle(nullptr);
    if (!hExe)
        return {};
    HRSRC res = FindResource(hExe, name, RT_RCDATA);
    if (!res)
        return {};
    HGLOBAL mem = LoadResource(hExe, res);
    if (!mem)
        return {};
    DWORD size = SizeofResource(hExe, res);
    if (size == 0)
        return {};
    const void* data = LockResource(mem);
    if (!data)
        return {};

    return std::string(static_cast<const char*>(data), size);
}
#else
extern const unsigned char _binary_en_US_json_start[];
extern const unsigned char _binary_en_US_json_end[];

extern const unsigned char _binary_zh_CN_json_start[];
extern const unsigned char _binary_zh_CN_json_end[];

std::string LangStrings::get(Lang lang) {
    switch (lang) {
    case Lang::EN_US:
        return std::string(
            reinterpret_cast<const char*>(_binary_en_US_json_start),
            reinterpret_cast<const char*>(_binary_en_US_json_end) -
            reinterpret_cast<const char*>(_binary_en_US_json_start)
        );
    case Lang::ZH_CN:
        return std::string(
            reinterpret_cast<const char*>(_binary_zh_CN_json_start),
            reinterpret_cast<const char*>(_binary_zh_CN_json_end) -
            reinterpret_cast<const char*>(_binary_zh_CN_json_start)
        );
    default:
        return "";
    }
}

#endif
