#include "hyprcursor.hpp"
#include "internalSharedTypes.hpp"
#include "internalDefines.hpp"
#include <array>
#include <filesystem>
#include <hyprlang.hpp>
#include <zip.h>
#include <cstring>
#include <algorithm>

#include "Log.hpp"

using namespace Hyprcursor;

// directories for lookup
constexpr const std::array<const char*, 1> systemThemeDirs = {"/usr/share/icons"};
constexpr const std::array<const char*, 2> userThemeDirs   = {"/.local/share/icons", "/.icons"};

//
static std::string themeNameFromEnv() {
    const auto ENV = getenv("HYPRCURSOR_THEME");
    if (!ENV)
        return "";

    return std::string{ENV};
}

static std::string getFirstTheme() {
    // try user directories first

    const auto HOMEENV = getenv("HOME");
    if (!HOMEENV)
        return "";

    const std::string HOME{HOMEENV};

    for (auto& dir : userThemeDirs) {
        const auto FULLPATH = HOME + dir;
        if (!std::filesystem::exists(FULLPATH))
            continue;

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest.hl";

            if (std::filesystem::exists(MANIFESTPATH))
                return themeDir.path().stem().string();
        }
    }

    for (auto& dir : systemThemeDirs) {
        const auto FULLPATH = dir;
        if (!std::filesystem::exists(FULLPATH))
            continue;

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest.hl";

            if (std::filesystem::exists(MANIFESTPATH))
                return themeDir.path().stem().string();
        }
    }

    return "";
}

static std::string getFullPathForThemeName(const std::string& name) {
    const auto HOMEENV = getenv("HOME");
    if (!HOMEENV)
        return "";

    const std::string HOME{HOMEENV};

    for (auto& dir : userThemeDirs) {
        const auto FULLPATH = HOME + dir;
        if (!std::filesystem::exists(FULLPATH))
            continue;

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest.hl";

            if (std::filesystem::exists(MANIFESTPATH))
                return std::filesystem::canonical(themeDir.path()).string();
        }
    }

    for (auto& dir : systemThemeDirs) {
        const auto FULLPATH = dir;
        if (!std::filesystem::exists(FULLPATH))
            continue;

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest.hl";

            if (std::filesystem::exists(MANIFESTPATH))
                return std::filesystem::canonical(themeDir.path()).string();
        }
    }

    return "";
}

CHyprcursorManager::CHyprcursorManager(const char* themeName_) {
    std::string themeName = themeName_ ? themeName_ : "";

    if (themeName.empty()) {
        // try reading from env
        themeName = themeNameFromEnv();
    }

    if (themeName.empty()) {
        // try finding first, in the hierarchy
        themeName = getFirstTheme();
    }

    if (themeName.empty()) {
        // holy shit we're done
        return;
    }

    // initialize theme
    impl               = new CHyprcursorImplementation;
    impl->themeName    = themeName;
    impl->themeFullDir = getFullPathForThemeName(themeName);

    if (impl->themeFullDir.empty())
        return;

    Debug::log(LOG, "Found theme {} at {}\n", impl->themeName, impl->themeFullDir);

    const auto LOADSTATUS = impl->loadTheme();

    if (LOADSTATUS.has_value()) {
        Debug::log(ERR, "Theme failed to load with {}\n", LOADSTATUS.value());
        return;
    }

    finalizedAndValid = true;
}

CHyprcursorManager::~CHyprcursorManager() {
    if (impl)
        delete impl;
}

bool CHyprcursorManager::valid() {
    return finalizedAndValid;
}

cairo_surface_t* CHyprcursorManager::getSurfaceFor(const char* shape_, const SCursorSurfaceInfo& info) {
    std::string REQUESTEDSHAPE = shape_;

    for (auto& shape : impl->theme.shapes) {
        if (REQUESTEDSHAPE != shape.directory && std::find(shape.overrides.begin(), shape.overrides.end(), REQUESTEDSHAPE) == shape.overrides.end())
            continue;

        // matched :)
        for (auto& image : impl->loadedShapes[&shape].images) {
            if (image->side != info.size)
                continue;

            // found pixel-perfect size
            return image->cairoSurface;
        }

        // TODO: resampling
    }
    return nullptr;
}

void CHyprcursorManager::cursorSurfaceDone(cairo_surface_t* surface) {
    ;
    // TODO: when resampling.
}

/*

Implementation

*/

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

SCursorTheme*                 currentTheme;

static Hyprlang::CParseResult parseDefineSize(const char* C, const char* V) {
    Hyprlang::CParseResult result;
    const std::string      VALUE = V;

    if (!VALUE.contains(",")) {
        result.setError("Invalid define_size");
        return result;
    }

    const auto   LHS = removeBeginEndSpacesTabs(VALUE.substr(0, VALUE.find_first_of(",")));
    const auto   RHS = removeBeginEndSpacesTabs(VALUE.substr(VALUE.find_first_of(",") + 1));

    SCursorImage image;
    image.filename = RHS;

    try {
        image.size = std::stoull(LHS);
    } catch (std::exception& e) {
        result.setError(e.what());
        return result;
    }

    currentTheme->shapes.back().images.push_back(image);

    return result;
}

static Hyprlang::CParseResult parseOverride(const char* C, const char* V) {
    Hyprlang::CParseResult result;
    const std::string      VALUE = V;

    currentTheme->shapes.back().overrides.push_back(V);

    return result;
}

/*

PNG reading

*/

static cairo_status_t readPNG(void* data, unsigned char* output, unsigned int len) {
    const auto DATA = (SLoadedCursorImage*)data;

    if (!DATA->data)
        return CAIRO_STATUS_READ_ERROR;

    size_t toRead = len > DATA->dataLen - DATA->readNeedle ? DATA->dataLen - DATA->readNeedle : len;

    std::memcpy(output, DATA->data + DATA->readNeedle, toRead);
    DATA->readNeedle += toRead;

    if (DATA->readNeedle >= DATA->dataLen) {
        delete[] (char*)DATA->data;
        DATA->data = nullptr;
        Debug::log(LOG, "cairo: png read, freeing mem");
    }

    return CAIRO_STATUS_SUCCESS;
}

/*

General

*/

std::optional<std::string> CHyprcursorImplementation::loadTheme() {

    currentTheme = &theme;

    // load manifest
    std::unique_ptr<Hyprlang::CConfig> manifest;
    try {
        // TODO: unify this between util and lib
        manifest = std::make_unique<Hyprlang::CConfig>((themeFullDir + "/manifest.hl").c_str(), Hyprlang::SConfigOptions{});
        manifest->addConfigValue("cursors_directory", Hyprlang::STRING{""});
        manifest->commence();
        manifest->parse();
    } catch (const char* err) {
        Debug::log(ERR, "Failed parsing manifest due to {}", err);
        return std::string{"failed: "} + err;
    }

    const std::string CURSORSSUBDIR = std::any_cast<Hyprlang::STRING>(manifest->getConfigValue("cursors_directory"));
    const std::string CURSORDIR     = themeFullDir + "/" + CURSORSSUBDIR;

    if (CURSORSSUBDIR.empty() || !std::filesystem::exists(CURSORDIR))
        return "loadTheme: cursors_directory missing or empty";

    for (auto& cursor : std::filesystem::directory_iterator(CURSORDIR)) {
        if (!cursor.is_regular_file())
            continue;

        auto& SHAPE       = theme.shapes.emplace_back();
        auto& LOADEDSHAPE = loadedShapes[&SHAPE];

        // extract zip to raw data.
        int         errp = 0;
        zip_t*      zip  = zip_open(cursor.path().string().c_str(), ZIP_RDONLY, &errp);

        zip_file_t* meta_file = zip_fopen(zip, "meta.hl", ZIP_FL_UNCHANGED);
        if (!meta_file)
            return "cursor" + cursor.path().string() + "failed to load meta";

        char* buffer = new char[1024 * 1024]; /* 1MB should be more than enough */

        int   readBytes = zip_fread(meta_file, buffer, 1024 * 1024 - 1);

        zip_fclose(meta_file);

        if (readBytes < 0) {
            delete[] buffer;
            return "cursor" + cursor.path().string() + "failed to read meta";
        }

        buffer[readBytes] = '\0';

        std::unique_ptr<Hyprlang::CConfig> meta;

        try {
            meta = std::make_unique<Hyprlang::CConfig>(buffer, Hyprlang::SConfigOptions{.pathIsStream = true});
            meta->addConfigValue("hotspot_x", Hyprlang::FLOAT{0.F});
            meta->addConfigValue("hotspot_y", Hyprlang::FLOAT{0.F});
            meta->addConfigValue("resize_algorithm", Hyprlang::STRING{"nearest"});
            meta->registerHandler(::parseDefineSize, "define_size", {.allowFlags = false});
            meta->registerHandler(::parseOverride, "define_override", {.allowFlags = false});
            meta->commence();
            meta->parse();
        } catch (const char* err) { return "failed parsing meta: " + std::string{err}; }

        delete[] buffer;

        for (auto& i : SHAPE.images) {
            // load image
            Debug::log(LOG, "Loading {} for shape {}", i.filename, cursor.path().stem().string());
            auto* IMAGE = LOADEDSHAPE.images.emplace_back(std::make_unique<SLoadedCursorImage>()).get();
            IMAGE->side = i.size;

            // read from zip
            zip_file_t* image_file = zip_fopen(zip, i.filename.c_str(), ZIP_FL_UNCHANGED);
            if (!image_file)
                return "cursor" + cursor.path().string() + "failed to load image_file";

            IMAGE->data = new char[1024 * 1024]; /* 1MB should be more than enough, again. This probably should be in the spec. */

            IMAGE->dataLen = zip_fread(image_file, IMAGE->data, 1024 * 1024 - 1);

            zip_fclose(image_file);

            Debug::log(LOG, "Cairo: set up surface read");

            IMAGE->cairoSurface = cairo_image_surface_create_from_png_stream(::readPNG, IMAGE);

            if (const auto STATUS = cairo_surface_status(IMAGE->cairoSurface); STATUS != CAIRO_STATUS_SUCCESS) {
                delete[] (char*)IMAGE->data;
                IMAGE->data = nullptr;
                return "Failed reading cairoSurface, status " + std::to_string((int)STATUS);
            }
        }

        if (SHAPE.images.empty())
            return "meta invalid: no images for shape " + cursor.path().stem().string();

        SHAPE.directory  = cursor.path().stem().string();
        SHAPE.hotspotX   = std::any_cast<float>(meta->getConfigValue("hotspot_x"));
        SHAPE.hotspotY   = std::any_cast<float>(meta->getConfigValue("hotspot_y"));
        SHAPE.resizeAlgo = stringToAlgo(std::any_cast<Hyprlang::STRING>(meta->getConfigValue("resize_algorithm")));

        zip_discard(zip);
    }

    return {};
}