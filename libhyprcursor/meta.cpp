#include "meta.hpp"

#include <hyprlang.hpp>
#include <toml++/toml.hpp>
#include <filesystem>
#include <regex>

#include "VarList.hpp"

CMeta* currentMeta = nullptr;

CMeta::CMeta(const std::string& rawdata_, bool hyprlang_ /* false for toml */, bool dataIsPath) : rawdata(rawdata_), hyprlang(hyprlang_), dataPath(dataIsPath) {
    if (!dataIsPath)
        return;

    rawdata = "";

    try {
        if (std::filesystem::exists(rawdata_ + ".hl")) {
            rawdata  = rawdata_ + ".hl";
            hyprlang = true;
            return;
        }

        if (std::filesystem::exists(rawdata_ + ".toml")) {
            rawdata  = rawdata_ + ".toml";
            hyprlang = false;
            return;
        }
    } catch (...) {}
}

std::optional<std::string> CMeta::parse() {
    if (rawdata.empty())
        return "Invalid meta (missing?)";

    std::optional<std::string> res;

    currentMeta = this;

    if (hyprlang)
        res = parseHL();
    else
        res = parseTOML();

    currentMeta = nullptr;

    return res;
}

static std::string removeBeginEndSpacesTabs(std::string str) {
    if (str.empty())
        return str;

    int countBefore = 0;
    while (str[countBefore] == ' ' || str[countBefore] == '\t') {
        countBefore++;
    }

    int countAfter = 0;
    while ((int)str.length() - countAfter - 1 >= 0 && (str[str.length() - countAfter - 1] == ' ' || str[str.length() - 1 - countAfter] == '\t')) {
        countAfter++;
    }

    str = str.substr(countBefore, str.length() - countBefore - countAfter);

    return str;
}

static Hyprlang::CParseResult parseDefineSize(const char* C, const char* V) {
    Hyprlang::CParseResult result;
    const std::string      VALUE = V;

    if (!VALUE.contains(",")) {
        result.setError("Invalid define_size");
        return result;
    }

    auto                LHS   = removeBeginEndSpacesTabs(VALUE.substr(0, VALUE.find_first_of(",")));
    auto                RHS   = removeBeginEndSpacesTabs(VALUE.substr(VALUE.find_first_of(",") + 1));
    auto                DELAY = 0;

    CMeta::SDefinedSize size;

    if (RHS.contains(",")) {
        const auto LL = removeBeginEndSpacesTabs(RHS.substr(0, RHS.find(",")));
        const auto RR = removeBeginEndSpacesTabs(RHS.substr(RHS.find(",") + 1));

        try {
            size.delayMs = std::stoull(RR);
        } catch (std::exception& e) {
            result.setError(e.what());
            return result;
        }

        RHS = LL;
    }

    if (!std::regex_match(RHS, std::regex("^[A-Za-z0-9_\\-\\.]+$"))) {
        result.setError("Invalid cursor file name, characters must be within [A-Za-z0-9_\\-\\.] (if this seems like a mistake, check for invisible characters)");
        return result;
    }

    size.file = RHS;

    if (!size.file.ends_with(".svg")) {
        try {
            size.size = std::stoull(LHS);
        } catch (std::exception& e) {
            result.setError(e.what());
            return result;
        }
    } else
        size.size = 0;

    currentMeta->parsedData.definedSizes.push_back(size);

    return result;
}

static Hyprlang::CParseResult parseOverride(const char* C, const char* V) {
    Hyprlang::CParseResult result;
    const std::string      VALUE = V;

    currentMeta->parsedData.overrides.push_back(VALUE);

    return result;
}

std::optional<std::string> CMeta::parseHL() {
    std::unique_ptr<Hyprlang::CConfig> meta;

    try {
        meta = std::make_unique<Hyprlang::CConfig>(rawdata.c_str(), Hyprlang::SConfigOptions{.pathIsStream = !dataPath});
        meta->addConfigValue("hotspot_x", Hyprlang::FLOAT{0.F});
        meta->addConfigValue("hotspot_y", Hyprlang::FLOAT{0.F});
        meta->addConfigValue("resize_algorithm", Hyprlang::STRING{"nearest"});
        meta->registerHandler(::parseDefineSize, "define_size", {.allowFlags = false});
        meta->registerHandler(::parseOverride, "define_override", {.allowFlags = false});
        meta->commence();
        meta->parse();
    } catch (const char* err) { return "failed parsing meta: " + std::string{err}; }

    parsedData.hotspotX   = std::any_cast<Hyprlang::FLOAT>(meta->getConfigValue("hotspot_x"));
    parsedData.hotspotY   = std::any_cast<Hyprlang::FLOAT>(meta->getConfigValue("hotspot_y"));
    parsedData.resizeAlgo = std::any_cast<Hyprlang::STRING>(meta->getConfigValue("resize_algorithm"));

    return {};
}

std::optional<std::string> CMeta::parseTOML() {
    try {
        auto MANIFEST = dataPath ? toml::parse_file(rawdata) : toml::parse(rawdata);

        parsedData.hotspotX = MANIFEST["General"]["hotspot_x"].value_or(0.f);
        parsedData.hotspotY = MANIFEST["General"]["hotspot_y"].value_or(0.f);

        const std::string OVERRIDES = MANIFEST["General"]["define_override"].value_or("");
        const std::string SIZES     = MANIFEST["General"]["define_size"].value_or("");

        //
        CVarList OVERRIDESLIST(OVERRIDES, 0, ';', true);
        for (auto& o : OVERRIDESLIST) {
            const auto RESULT = ::parseOverride("define_override", o.c_str());
            if (RESULT.error)
                throw;
        }

        CVarList SIZESLIST(SIZES, 0, ';', true);
        for (auto& s : SIZESLIST) {
            const auto RESULT = ::parseDefineSize("define_size", s.c_str());
            if (RESULT.error)
                throw;
        }

        parsedData.resizeAlgo = MANIFEST["General"]["resize_algorithm"].value_or("");
    } catch (std::exception& e) { return std::string{"Failed parsing toml: "} + e.what(); }

    return {};
}
