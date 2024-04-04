#pragma once

#include <string>
#include <optional>
#include <vector>

/*
    Meta can parse meta.hl and meta.toml
*/
class CMeta {
  public:
    CMeta(const std::string& rawdata_, bool hyprlang_ /* false for toml */, bool dataIsPath = false);

    std::optional<std::string> parse();

    struct SDefinedSize {
        std::string file;
        int         size = 0, delayMs = 200;
    };

    struct {
        std::string               resizeAlgo;
        float                     hotspotX = 0, hotspotY = 0;
        std::vector<std::string>  overrides;
        std::vector<SDefinedSize> definedSizes;
    } parsedData;

  private:
    std::optional<std::string> parseHL();
    std::optional<std::string> parseTOML();

    bool                       dataPath = false;
    bool                       hyprlang = true;

    std::string                rawdata;
};