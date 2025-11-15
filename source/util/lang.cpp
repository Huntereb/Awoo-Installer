#include <iostream>
#include <switch.h>
#include <filesystem>
#include <pu/Plutonium>
#include "util/lang.hpp"
#include "util/config.hpp"
#include <sstream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

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
            case SetLanguage_JA:
                languagePath = "romfs:/lang/jp.json";
                break;
            case SetLanguage_FR:
            case SetLanguage_FRCA:
                languagePath = "romfs:/lang/fr.json";
                break;
            case SetLanguage_DE:
                languagePath = "romfs:/lang/de.json";
                break;
            case SetLanguage_IT:
                languagePath = "romfs:/lang/it.json";
                break;
            case SetLanguage_ES:
            case SetLanguage_ES419:
                languagePath = "romfs:/lang/es-419.json";
                break;
            case SetLanguage_ZHCN:
            case SetLanguage_ZHHANS:
                languagePath = "romfs:/lang/zh-CN.json";
                // the default font will miss some chinese character, so use a chinese font (simplified)
                pu::ui::render::SetDefaultFontFromShared(pu::ui::render::SharedFont::ChineseSimplified);
                break;
            case SetLanguage_KO:
                languagePath = "romfs:/lang/ko.json";
                pu::ui::render::SetDefaultFontFromShared(pu::ui::render::SharedFont::Korean);
                break;
            case SetLanguage_NL:
                languagePath = "romfs:/lang/nl.json";
                break;
            case SetLanguage_PT:
            case SetLanguage_PTBR:
                languagePath = "romfs:/lang/pt.json";
                break;
            case SetLanguage_RU:
                languagePath = "romfs:/lang/ru.json";
                break;
            case SetLanguage_ZHTW:
                languagePath = "romfs:/lang/zh-TW.json";
                // the default font will miss some chinese character, so use a chinese font (traditional)
                pu::ui::render::SetDefaultFontFromShared(pu::ui::render::SharedFont::ChineseTraditional);
                break;
            case SetLanguage_ZHHANT:
                languagePath = "romfs:/lang/zh-Hant.json";
                pu::ui::render::SetDefaultFontFromShared(pu::ui::render::SharedFont::ChineseTraditional);
                break;
            case SetLanguage_ENUS:
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

    inline json GetRelativeJson(json j, std::string key) {
        std::istringstream ss(key);
        std::string token;

        while (std::getline(ss, token, '.') && j != nullptr) {
            j = j[token];
        }

        return j;
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