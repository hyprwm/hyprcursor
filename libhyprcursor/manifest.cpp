#include "manifest.hpp"

#include <toml++/toml.hpp>
#include <hyprlang.hpp>

#include <filesystem>

CManifest::CManifest(const std::string& path_) {
    try {
        if (std::filesystem::exists(path_ + ".hl")) {
            path           = path_ + ".hl";
            selectedParser = PARSER_HYPRLANG;
            return;
        }

        if (std::filesystem::exists(path_ + ".toml")) {
            path           = path_ + ".toml";
            selectedParser = PARSER_TOML;
            return;
        }
    } catch (...) { ; }
}

std::optional<std::string> CManifest::parse() {
    if (path.empty())
        return "Failed to find an appropriate manifest.";

    if (selectedParser == PARSER_HYPRLANG)
        return parseHL();
    if (selectedParser == PARSER_TOML)
        return parseTOML();

    return "No parser available for " + path;
}

std::optional<std::string> CManifest::parseHL() {
    std::unique_ptr<Hyprlang::CConfig> manifest;
    try {
        // TODO: unify this between util and lib
        manifest = std::make_unique<Hyprlang::CConfig>(path.c_str(), Hyprlang::SConfigOptions{.throwAllErrors = true});
        manifest->addConfigValue("cursors_directory", Hyprlang::STRING{""});
        manifest->addConfigValue("name", Hyprlang::STRING{""});
        manifest->addConfigValue("description", Hyprlang::STRING{""});
        manifest->addConfigValue("version", Hyprlang::STRING{""});
        manifest->addConfigValue("author", Hyprlang::STRING{""});
        manifest->commence();
        manifest->parse();
    } catch (const char* err) { return std::string{"failed: "} + err; }

    parsedData.cursorsDirectory = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("cursors_directory"));
    parsedData.name             = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("name"));
    parsedData.description      = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("description"));
    parsedData.version          = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("version"));
    parsedData.author           = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("author"));

    return {};
}

std::optional<std::string> CManifest::parseTOML() {
    try {
        auto MANIFEST = toml::parse_file(path);

        parsedData.cursorsDirectory = MANIFEST["General"]["cursors_directory"].value_or("");
        parsedData.name             = MANIFEST["General"]["name"].value_or("");
        parsedData.description      = MANIFEST["General"]["description"].value_or("");
        parsedData.version          = MANIFEST["General"]["version"].value_or("");
        parsedData.author           = MANIFEST["General"]["author"].value_or("");
    } catch (...) { return "Failed parsing toml"; }

    return {};
}

std::string CManifest::getPath() {
    return path;
}