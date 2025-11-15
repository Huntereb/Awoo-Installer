#pragma once

#include <string>

namespace Language {
    void Load();
    std::string LanguageEntry(std::string key);
    std::string GetRandomMsg();
}

inline std::string operator ""_lang (const char* key, size_t size) {
    return Language::LanguageEntry(std::string(key, size));
}