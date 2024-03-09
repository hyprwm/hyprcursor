#include <iostream>
#include <zip.h>
#include <optional>
#include <filesystem>
#include <array>
#include <format>
#include <hyprlang.hpp>
#include "internalSharedTypes.hpp"

enum eOperation {
    OPERATION_CREATE  = 0,
    OPERATION_EXTRACT = 1,
};

eResizeAlgo explicitResizeAlgo = RESIZE_INVALID;

struct XCursorConfigEntry {
    int         size = 0, hotspotX = 0, hotspotY = 0, delay = 0;
    std::string image;
};

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

static bool promptForDeletion(const std::string& path) {
    std::cout << "About to delete (recursively) " << path << ", are you sure? [Y/n]\n";
    std::string result;
    std::cin >> result;

    if (result != "Y" && result != "Y\n" && result != "y\n" && result != "y") {
        std::cout << "Abort.\n";
        exit(1);
        return false;
    }

    std::filesystem::remove_all(path);

    return true;
}

std::unique_ptr<SCursorTheme> currentTheme;

static Hyprlang::CParseResult parseDefineSize(const char* C, const char* V) {
    Hyprlang::CParseResult result;
    const std::string      VALUE = V;

    if (!VALUE.contains(",")) {
        result.setError("Invalid define_size");
        return result;
    }

    auto         LHS   = removeBeginEndSpacesTabs(VALUE.substr(0, VALUE.find_first_of(",")));
    auto         RHS   = removeBeginEndSpacesTabs(VALUE.substr(VALUE.find_first_of(",") + 1));
    auto         DELAY = 0;

    SCursorImage image;

    if (RHS.contains(",")) {
        const auto LL = removeBeginEndSpacesTabs(RHS.substr(0, RHS.find(",")));
        const auto RR = removeBeginEndSpacesTabs(RHS.substr(RHS.find(",") + 1));

        try {
            image.delay = std::stoull(RR);
        } catch (std::exception& e) {
            result.setError(e.what());
            return result;
        }

        RHS = LL;
    }

    image.filename = RHS;

    try {
        image.size = std::stoull(LHS);
    } catch (std::exception& e) {
        result.setError(e.what());
        return result;
    }

    currentTheme->shapes.back()->images.push_back(image);

    return result;
}

static Hyprlang::CParseResult parseOverride(const char* C, const char* V) {
    Hyprlang::CParseResult result;
    const std::string      VALUE = V;

    currentTheme->shapes.back()->overrides.push_back(V);

    return result;
}

static std::optional<std::string> createCursorThemeFromPath(const std::string& path, const std::string& out_ = {}) {
    if (!std::filesystem::exists(path))
        return "input path does not exist";

    std::string out = out_.empty() ? path.substr(0, path.find_last_of('/') + 1) + "theme/" : out_;

    const auto  MANIFESTPATH = path + "/manifest.hl";
    if (!std::filesystem::exists(MANIFESTPATH))
        return "manifest.hl is missing";

    std::unique_ptr<Hyprlang::CConfig> manifest;
    try {
        manifest = std::make_unique<Hyprlang::CConfig>(MANIFESTPATH.c_str(), Hyprlang::SConfigOptions{});
        manifest->addConfigValue("cursors_directory", Hyprlang::STRING{""});
        manifest->addConfigValue("name", Hyprlang::STRING{""});
        manifest->addConfigValue("description", Hyprlang::STRING{""});
        manifest->addConfigValue("version", Hyprlang::STRING{""});
        manifest->commence();
        manifest->parse();
    } catch (const char* err) { return "failed parsing manifest: " + std::string{err}; }

    const std::string CURSORSSUBDIR = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("cursors_directory"));
    const std::string CURSORDIR     = path + "/" + CURSORSSUBDIR;

    if (CURSORSSUBDIR.empty() || !std::filesystem::exists(CURSORDIR))
        return "manifest: cursors_directory missing or empty";

    // iterate over the directory and record all cursors

    currentTheme = std::make_unique<SCursorTheme>();
    for (auto& dir : std::filesystem::directory_iterator(CURSORDIR)) {
        const auto METAPATH = dir.path().string() + "/meta.hl";

        auto&      SHAPE = currentTheme->shapes.emplace_back(std::make_unique<SCursorShape>());

        //
        std::unique_ptr<Hyprlang::CConfig> meta;

        try {
            meta = std::make_unique<Hyprlang::CConfig>(METAPATH.c_str(), Hyprlang::SConfigOptions{});
            meta->addConfigValue("hotspot_x", Hyprlang::FLOAT{0.F});
            meta->addConfigValue("hotspot_y", Hyprlang::FLOAT{0.F});
            meta->addConfigValue("resize_algorithm", Hyprlang::STRING{"nearest"});
            meta->registerHandler(::parseDefineSize, "define_size", {.allowFlags = false});
            meta->registerHandler(::parseOverride, "define_override", {.allowFlags = false});
            meta->commence();
            meta->parse();
        } catch (const char* err) { return "failed parsing meta (" + METAPATH + "): " + std::string{err}; }

        // check if we have at least one image.
        for (auto& i : SHAPE->images) {

            if (SHAPE->shapeType == SHAPE_INVALID) {
                if (i.filename.ends_with(".svg"))
                    SHAPE->shapeType = SHAPE_SVG;
                else if (i.filename.ends_with(".png"))
                    SHAPE->shapeType = SHAPE_PNG;
                else {
                    std::cout << "WARNING: image " << i.filename << " has no known extension, assuming png.\n";
                    SHAPE->shapeType = SHAPE_PNG;
                }
            } else {
                if (SHAPE->shapeType == SHAPE_SVG && !i.filename.ends_with(".svg"))
                    return "meta invalid: cannot add .png files to an svg shape";
                else if (SHAPE->shapeType == SHAPE_PNG && i.filename.ends_with(".svg"))
                    return "meta invalid: cannot add .svg files to a png shape";
            }

            if (!std::filesystem::exists(dir.path().string() + "/" + i.filename))
                return "meta invalid: image " + i.filename + " does not exist";
            break;
        }

        if (SHAPE->images.empty())
            return "meta invalid: no images for shape " + dir.path().stem().string();

        SHAPE->directory  = dir.path().stem().string();
        SHAPE->hotspotX   = std::any_cast<float>(meta->getConfigValue("hotspot_x"));
        SHAPE->hotspotY   = std::any_cast<float>(meta->getConfigValue("hotspot_y"));
        SHAPE->resizeAlgo = stringToAlgo(std::any_cast<Hyprlang::STRING>(meta->getConfigValue("resize_algorithm")));

        std::cout << "Shape " << SHAPE->directory << ": \n\toverrides: " << SHAPE->overrides.size() << "\n\tsizes: " << SHAPE->images.size() << "\n";
    }

    // create output fs structure
    if (!std::filesystem::exists(out))
        std::filesystem::create_directory(out);
    else {
        // clear the entire thing, avoid melting themes together
        promptForDeletion(out);
        std::filesystem::create_directory(out);
    }

    // manifest is copied
    std::filesystem::copy(MANIFESTPATH, out + "/manifest.hl");

    // create subdir for cursors
    std::filesystem::create_directory(out + "/" + CURSORSSUBDIR);

    // create zips (.hlc) for each
    for (auto& shape : currentTheme->shapes) {
        const auto CURRENTCURSORSDIR = path + "/" + CURSORSSUBDIR + "/" + shape->directory;
        const auto OUTPUTFILE        = out + "/" + CURSORSSUBDIR + "/" + shape->directory + ".hlc";
        int        errp              = 0;
        zip_t*     zip               = zip_open(OUTPUTFILE.c_str(), ZIP_CREATE | ZIP_EXCL, &errp);

        if (!zip) {
            zip_error_t ziperror;
            zip_error_init_with_code(&ziperror, errp);
            return "Failed to open " + OUTPUTFILE + " for writing: " + zip_error_strerror(&ziperror);
        }

        // add meta.hl
        zip_source_t* meta = zip_source_file(zip, (CURRENTCURSORSDIR + "/meta.hl").c_str(), 0, 0);
        if (!meta)
            return "(1) failed to add meta " + (CURRENTCURSORSDIR + "/meta.hl") + " to hlc";
        if (zip_file_add(zip, "meta.hl", meta, ZIP_FL_ENC_UTF_8) < 0)
            return "(2) failed to add meta " + (CURRENTCURSORSDIR + "/meta.hl") + " to hlc";

        meta = nullptr;

        // add each cursor png
        for (auto& i : shape->images) {
            zip_source_t* image = zip_source_file(zip, (CURRENTCURSORSDIR + "/" + i.filename).c_str(), 0, 0);
            if (!image)
                return "(1) failed to add image " + (CURRENTCURSORSDIR + "/" + i.filename) + " to hlc";
            if (zip_file_add(zip, (i.filename).c_str(), image, ZIP_FL_ENC_UTF_8) < 0)
                return "(2) failed to add image " + i.filename + " to hlc";

            std::cout << "Added image " << i.filename << " to shape " << shape->directory << "\n";
        }

        // close zip and write
        if (zip_close(zip) < 0) {
            zip_error_t ziperror;
            zip_error_init_with_code(&ziperror, errp);
            return "Failed to write " + OUTPUTFILE + ": " + zip_error_strerror(&ziperror);
        }

        std::cout << "Written " << OUTPUTFILE << "\n";
    }

    // done!
    std::cout << "Done, written " << currentTheme->shapes.size() << " shapes.\n";

    return {};
}

static std::string spawnSync(const std::string& cmd) {
    std::array<char, 128>                          buffer;
    std::string                                    result;
    const std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        return "";

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

static std::optional<std::string> extractXTheme(const std::string& xpath, const std::string& out_) {

    if (!std::filesystem::exists(xpath) || !std::filesystem::exists(xpath + "/cursors"))
        return "input path does not exist or is not an xcursor theme";

    std::string out = out_.empty() ? xpath.substr(0, xpath.find_last_of('/') + 1) + "extracted/" : out_;

    // create output fs structure
    if (!std::filesystem::exists(out))
        std::filesystem::create_directory(out);
    else {
        // clear the entire thing, avoid melting themes together
        promptForDeletion(out);
        std::filesystem::create_directory(out);
    }

    // write a boring manifest
    std::ofstream manifest(out + "/manifest.hl", std::ios::trunc);
    if (!manifest.good())
        return "failed writing manifest";

    manifest << "name = Extracted Theme\ndescription = Automatically extracted with hyprcursor-util\nversion = 0.1\ncursors_directory = hyprcursors\n";

    manifest.close();

    // make a cursors dir

    std::filesystem::create_directory(out + "/hyprcursors/");

    // create a temp extract dir
    std::filesystem::create_directory("/tmp/hyprcursor-util/");

    // write all cursors
    for (auto& xcursor : std::filesystem::directory_iterator(xpath + "/cursors/")) {
        // ignore symlinks, we'll write them to the meta.hl file.
        if (!xcursor.is_regular_file() || xcursor.is_symlink())
            continue;

        const auto CURSORDIR = out + "/hyprcursors/" + xcursor.path().stem().string();
        std::filesystem::create_directory(CURSORDIR);

        std::cout << "Found xcursor " << xcursor.path().stem().string() << "\n";

        // decompile xcursor
        const auto OUT = spawnSync(std::format("rm -f /tmp/hyprcursor-util/* && cd /tmp/hyprcursor-util && xcur2png {} -d /tmp/hyprcursor-util 2>&1",
                                               std::filesystem::canonical(xcursor.path()).string()));

        // read the config
        std::vector<XCursorConfigEntry> entries;
        std::ifstream                   xconfig("/tmp/hyprcursor-util/" + xcursor.path().stem().string() + ".conf");
        if (!xconfig.good())
            return "Failed reading xconfig for " + xcursor.path().string();

        std::string line = "";

        while (std::getline(xconfig, line)) {
            if (line.starts_with("#"))
                continue;

            auto& ENTRY = entries.emplace_back();

            // extract
            try {
                std::string curval = line.substr(0, line.find_first_of('\t'));
                ENTRY.size         = std::stoi(curval);
                line               = line.substr(line.find_first_of('\t') + 1);

                curval         = line.substr(0, line.find_first_of('\t'));
                ENTRY.hotspotX = std::stoi(curval);
                line           = line.substr(line.find_first_of('\t') + 1);

                curval         = line.substr(0, line.find_first_of('\t'));
                ENTRY.hotspotY = std::stoi(curval);
                line           = line.substr(line.find_first_of('\t') + 1);

                curval      = line.substr(0, line.find_first_of('\t'));
                ENTRY.image = curval;
                line        = line.substr(line.find_first_of('\t') + 1);

                curval      = line.substr(0, line.find_first_of('\t'));
                ENTRY.delay = std::stoi(curval);
            } catch (std::exception& e) { return "Failed reading xconfig " + xcursor.path().string() + " because of " + e.what(); }

            std::cout << "Extracted " << xcursor.path().stem().string() << " at size " << ENTRY.size << "\n";
        }

        if (entries.empty())
            return "Empty xcursor " + xcursor.path().string();

        // copy pngs
        for (auto& extracted : std::filesystem::directory_iterator("/tmp/hyprcursor-util")) {
            if (extracted.path().string().ends_with(".conf"))
                continue;

            std::filesystem::copy(extracted, CURSORDIR + "/");
        }

        // write a meta.hl
        std::string metaString = std::format("resize_algorithm = {}\n", explicitResizeAlgo == RESIZE_INVALID ? "none" : algoToString(explicitResizeAlgo));

        // find hotspot from first entry
        metaString +=
            std::format("hotspot_x = {:.2f}\nhotspot_y = {:.2f}\n\n", (float)entries[0].hotspotX / (float)entries[0].size, (float)entries[0].hotspotY / (float)entries[0].size);

        // define all sizes
        for (auto& entry : entries) {
            const auto ENTRYSTEM = entry.image.substr(entry.image.find_last_of('/') + 1);

            metaString += std::format("define_size = {}, {}, {}\n", entry.size, ENTRYSTEM, entry.delay);
        }

        metaString += "\n";

        // define overrides, scan for symlinks

        for (auto& xcursor2 : std::filesystem::directory_iterator(xpath + "/cursors/")) {
            if (!xcursor2.is_symlink())
                continue;

            if (std::filesystem::canonical(xcursor2) != std::filesystem::canonical(xcursor))
                continue;

            // this sym points to us
            metaString += std::format("define_override = {}\n", xcursor2.path().stem().string());
        }

        // meta done, write
        std::ofstream meta(CURSORDIR + "/meta.hl", std::ios::trunc);
        meta << metaString;
        meta.close();
    }

    std::filesystem::remove_all("/tmp/hyprcursor-util/");

    return {};
}

int main(int argc, char** argv, char** envp) {

    if (argc < 2) {
        std::cerr << "Not enough args.\n";
        return 1;
    }

    eOperation  op   = OPERATION_CREATE;
    std::string path = "", out = "";

    for (size_t i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (i == 1) {
            // mode
            if (arg == "--create" || arg == "-c") {
                op = OPERATION_CREATE;

                if (argc < 3) {
                    std::cerr << "Missing path for create.\n";
                    return 1;
                }

                path = argv[++i];
            } else if (arg == "--extract" || arg == "-x") {
                op = OPERATION_EXTRACT;

                if (argc < 3) {
                    std::cerr << "Missing path for extract.\n";
                    return 1;
                }

                path = argv[++i];
            } else {
                std::cerr << "Invalid mode.\n";
                return 1;
            }
            continue;
        }

        if (arg == "-o" || arg == "--output") {
            out = argv[++i];
            continue;
        } else if (arg == "--resize") {
            explicitResizeAlgo = stringToAlgo(argv[++i]);
            continue;
        } else {
            std::cerr << "Unknown arg: " << arg << "\n";
            return 1;
        }
    }

    if (path.ends_with("/"))
        path.pop_back();

    switch (op) {
        case OPERATION_CREATE: {
            const auto RET = createCursorThemeFromPath(path, out);
            if (RET.has_value()) {
                std::cerr << "Failed: " << RET.value() << "\n";
                return 1;
            }
            break;
        }
        case OPERATION_EXTRACT: {
            const auto RET = extractXTheme(path, out);
            if (RET.has_value()) {
                std::cerr << "Failed: " << RET.value() << "\n";
                return 1;
            }
            break;
        }
        default: std::cerr << "Invalid mode.\n"; return 1;
    }

    return 0;
}