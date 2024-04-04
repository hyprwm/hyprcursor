#pragma once

#include <string>
#include <optional>

/*
    Manifest can parse manifest.hl and manifest.toml
*/
class CManifest {
  public:
    /*
        path_ is the path to a manifest WITHOUT the extension.
        CManifest will attempt all parsable extensions (.hl, .toml)
    */
    CManifest(const std::string& path_);

    std::optional<std::string> parse();
    std::string                getPath();

    struct {
        std::string name, description, version, cursorsDirectory, author;
    } parsedData;

  private:
    enum eParser {
        PARSER_HYPRLANG = 0,
        PARSER_TOML
    };

    std::optional<std::string> parseHL();
    std::optional<std::string> parseTOML();

    eParser                    selectedParser = PARSER_HYPRLANG;

    std::string                path;
};