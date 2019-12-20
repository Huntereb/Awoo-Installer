#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

namespace Language {
    void Load();
    std::string LanguageEntry(std::string key);
    std::string GetRandomMsg();
    inline json GetRelativeJson(json j, std::string key) {
        std::istringstream ss(key);
        std::string token;

        while (std::getline(ss, token, '.') && j != nullptr) {
            j = j[token];
        }

        return j;
    }
}

inline std::string operator ""_lang (const char* key, size_t size) {
    return Language::LanguageEntry(std::string(key, size));
}