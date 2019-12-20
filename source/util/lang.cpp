#include "util/lang.hpp"

#include <iostream>

namespace Language {
    json lang;

    void Load() {
        std::ifstream ifs("romfs:/lang/en.json");
        if (!ifs.good()) {
            std::cout << "[FAILED TO LOAD LANGUAGE FILE]" << std::endl;
            return;
        }
        lang = json::parse(ifs);
        ifs.close();
    }

    std::string LanguageEntry(std::string key) {
        json j = GetRelativeJson(lang, key);
        if (j == nullptr) {
            return "didn't find: " + key;
        }
        return j.get<std::string>();
    }

    std::string GetRandomMsg() {
        json j = Language::GetRelativeJson(lang, "inst.finished");
        srand(time(NULL));
        return(j[rand() % j.size()]);
    }
}