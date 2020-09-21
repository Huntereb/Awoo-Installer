#include <iostream>
#include <switch.h>
#include <filesystem>
#include <pu/Plutonium>
#include "util/lang.hpp"
#include "util/config.hpp"

namespace Language {
    json lang;

    void Load() {
        std::ifstream ifs;
        std::string languagePath;
        int langInt = inst::config::languageSetting;
        if (langInt == 99) {
            SetLanguage ourLang;
            u64 lcode = 0;
            setInitialize();
            setGetSystemLanguage(&lcode);
            setMakeLanguage(lcode, &ourLang);
            setExit();
            langInt = (int)ourLang;
        } 
        switch (langInt) {
            case 0:
                languagePath = "romfs:/lang/jp.json";
                break;
            case 2:
            case 13:
                languagePath = "romfs:/lang/fr.json";
                break;
            case 3:
                languagePath = "romfs:/lang/de.json";
                break;
            case 4:
                languagePath = "romfs:/lang/it.json";
                break;
            case 5:
            case 14:
                languagePath = "romfs:/lang/es-419.json";
                break;
            case 6:
            case 15:
                languagePath = "romfs:/lang/zh-CN.json";
                // the default font will miss some chinese character, so use a chinese font (simplified)
                pu::ui::render::SetDefaultFontFromShared(pu::ui::render::SharedFont::ChineseSimplified);
                break;
            case 7:
                languagePath = "romfs:/lang/ko.json";
                break;
            case 8:
                languagePath = "romfs:/lang/nl.json";
                break;
            case 9:
                languagePath = "romfs:/lang/pt.json";
                break;
            case 10:
                languagePath = "romfs:/lang/ru.json";
                break;
            case 11:
                languagePath = "romfs:/lang/zh-TW.json";
                // the default font will miss some chinese character, so use a chinese font (traditional)
                pu::ui::render::SetDefaultFontFromShared(pu::ui::render::SharedFont::ChineseTraditional);
                break;
            default:
                languagePath = "romfs:/lang/en.json";
        }
        if (std::filesystem::exists(languagePath)) ifs = std::ifstream(languagePath);
        else ifs = std::ifstream("romfs:/lang/en.json");
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