#include "hyprcursor/hyprcursor.hpp"
#include "internalSharedTypes.hpp"
#include "internalDefines.hpp"
#include <array>
#include <filesystem>
#include <zip.h>
#include <cstring>
#include <algorithm>
#include <librsvg/rsvg.h>

#include "manifest.hpp"
#include "meta.hpp"
#include "Log.hpp"

using namespace Hyprcursor;

// directories for lookup
constexpr const std::array<const char*, 1> systemThemeDirs = {"/usr/share/icons"};
constexpr const std::array<const char*, 2> userThemeDirs   = {"/.local/share/icons", "/.icons"};

//
static std::string themeNameFromEnv(PHYPRCURSORLOGFUNC logfn) {
    const auto ENV = getenv("HYPRCURSOR_THEME");
    if (!ENV) {
        Debug::log(HC_LOG_INFO, logfn, "themeNameFromEnv: env unset");
        return "";
    }

    return std::string{ENV};
}

static bool pathAccessible(const std::string& path) {
    try {
        if (!std::filesystem::exists(path))
            return false;

    } catch (std::exception& e) { return false; }

    return true;
}

static bool themeAccessible(const std::string& path) {
    return pathAccessible(path + "/manifest.hl") || pathAccessible(path + "manifest.toml");
}

static std::string getFirstTheme(PHYPRCURSORLOGFUNC logfn) {
    // try user directories first

    const auto HOMEENV = getenv("HOME");
    if (!HOMEENV)
        return "";

    const std::string HOME{HOMEENV};

    for (auto& dir : userThemeDirs) {
        const auto FULLPATH = HOME + dir;
        if (!pathAccessible(FULLPATH)) {
            Debug::log(HC_LOG_TRACE, logfn, "Skipping path {} because it's inaccessible.", FULLPATH);
            continue;
        }

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            if (!themeAccessible(themeDir.path().string())) {
                Debug::log(HC_LOG_TRACE, logfn, "Skipping theme {} because it's inaccessible.", themeDir.path().string());
                continue;
            }

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest.";

            if (std::filesystem::exists(MANIFESTPATH + "hl") || std::filesystem::exists(MANIFESTPATH + "toml")) {
                Debug::log(HC_LOG_INFO, logfn, "getFirstTheme: found {}", themeDir.path().string());
                return themeDir.path().stem().string();
            }
        }
    }

    for (auto& dir : systemThemeDirs) {
        const auto FULLPATH = dir;
        if (!pathAccessible(FULLPATH)) {
            Debug::log(HC_LOG_TRACE, logfn, "Skipping path {} because it's inaccessible.", FULLPATH);
            continue;
        }

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            if (!themeAccessible(themeDir.path().string())) {
                Debug::log(HC_LOG_TRACE, logfn, "Skipping theme {} because it's inaccessible.", themeDir.path().string());
                continue;
            }

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest.";

            if (std::filesystem::exists(MANIFESTPATH + "hl") || std::filesystem::exists(MANIFESTPATH + "toml")) {
                Debug::log(HC_LOG_INFO, logfn, "getFirstTheme: found {}", themeDir.path().string());
                return themeDir.path().stem().string();
            }
        }
    }

    return "";
}

static std::string getFullPathForThemeName(const std::string& name, PHYPRCURSORLOGFUNC logfn) {
    const auto HOMEENV = getenv("HOME");
    if (!HOMEENV)
        return "";

    const std::string HOME{HOMEENV};

    for (auto& dir : userThemeDirs) {
        const auto FULLPATH = HOME + dir;
        if (!pathAccessible(FULLPATH)) {
            Debug::log(HC_LOG_TRACE, logfn, "Skipping path {} because it's inaccessible.", FULLPATH);
            continue;
        }

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            if (!themeAccessible(themeDir.path().string())) {
                Debug::log(HC_LOG_TRACE, logfn, "Skipping theme {} because it's inaccessible.", themeDir.path().string());
                continue;
            }

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest";

            if (name.empty()) {
                if (std::filesystem::exists(MANIFESTPATH + ".hl") || std::filesystem::exists(MANIFESTPATH + ".toml")) {
                    Debug::log(HC_LOG_INFO, logfn, "getFullPathForThemeName: found {}", themeDir.path().string());
                    return std::filesystem::canonical(themeDir.path()).string();
                }
                continue;
            }

            if (!std::filesystem::exists(MANIFESTPATH))
                continue;

            CManifest manifest{MANIFESTPATH};
            if (!manifest.parse().has_value())
                continue;

            const std::string NAME = manifest.parsedData.name;

            if (NAME != name && name != themeDir.path().stem().string())
                continue;

            Debug::log(HC_LOG_INFO, logfn, "getFullPathForThemeName: found {}", themeDir.path().string());
            return std::filesystem::canonical(themeDir.path()).string();
        }
    }

    for (auto& dir : systemThemeDirs) {
        const auto FULLPATH = dir;
        if (!pathAccessible(FULLPATH)) {
            Debug::log(HC_LOG_TRACE, logfn, "Skipping path {} because it's inaccessible.", FULLPATH);
            continue;
        }

        // loop over dirs and see if any has a manifest.hl
        for (auto& themeDir : std::filesystem::directory_iterator(FULLPATH)) {
            if (!themeDir.is_directory())
                continue;

            if (!themeAccessible(themeDir.path().string())) {
                Debug::log(HC_LOG_TRACE, logfn, "Skipping theme {} because it's inaccessible.", themeDir.path().string());
                continue;
            }

            const auto MANIFESTPATH = themeDir.path().string() + "/manifest";

            if (std::filesystem::exists(MANIFESTPATH + ".hl") || std::filesystem::exists(MANIFESTPATH + ".toml"))
                continue;

            CManifest manifest{MANIFESTPATH};
            if (!manifest.parse().has_value())
                continue;

            const std::string NAME = manifest.parsedData.name;

            if (NAME != name && name != themeDir.path().stem().string())
                continue;

            Debug::log(HC_LOG_INFO, logfn, "getFullPathForThemeName: found {}", themeDir.path().string());
            return std::filesystem::canonical(themeDir.path()).string();
        }
    }

    if (!name.empty()) { // try without name
        Debug::log(HC_LOG_INFO, logfn, "getFullPathForThemeName: failed, trying without name of {}", name);
        return getFullPathForThemeName("", logfn);
    }

    return "";
}

CHyprcursorManager::CHyprcursorManager(const char* themeName_) {
    init(themeName_);
}

CHyprcursorManager::CHyprcursorManager(const char* themeName_, PHYPRCURSORLOGFUNC fn) {
    logFn = fn;
    init(themeName_);
}

void CHyprcursorManager::init(const char* themeName_) {
    std::string themeName = themeName_ ? themeName_ : "";

    if (themeName.empty()) {
        // try reading from env
        Debug::log(HC_LOG_INFO, logFn, "CHyprcursorManager: attempting to find theme from env");
        themeName = themeNameFromEnv(logFn);
    }

    if (themeName.empty()) {
        // try finding first, in the hierarchy
        Debug::log(HC_LOG_INFO, logFn, "CHyprcursorManager: attempting to find any theme");
        themeName = getFirstTheme(logFn);
    }

    if (themeName.empty()) {
        // holy shit we're done
        Debug::log(HC_LOG_INFO, logFn, "CHyprcursorManager: no themes matched");
        return;
    }

    // initialize theme
    impl               = new CHyprcursorImplementation(this, logFn);
    impl->themeName    = themeName;
    impl->themeFullDir = getFullPathForThemeName(themeName, logFn);

    if (impl->themeFullDir.empty())
        return;

    Debug::log(HC_LOG_INFO, logFn, "Found theme {} at {}\n", impl->themeName, impl->themeFullDir);

    const auto LOADSTATUS = impl->loadTheme();

    if (LOADSTATUS.has_value()) {
        Debug::log(HC_LOG_ERR, logFn, "Theme failed to load with {}\n", LOADSTATUS.value());
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

SCursorImageData** CHyprcursorManager::getShapesC(int& outSize, const char* shape_, const SCursorStyleInfo& info) {
    if (!shape_) {
        Debug::log(HC_LOG_ERR, logFn, "getShapesC: shape of nullptr is invalid");
        return nullptr;
    }

    std::string                      REQUESTEDSHAPE = shape_;

    std::vector<SLoadedCursorImage*> resultingImages;
    float                            hotX = 0, hotY = 0;

    for (auto& shape : impl->theme.shapes) {
        if (REQUESTEDSHAPE != shape->directory && std::find(shape->overrides.begin(), shape->overrides.end(), REQUESTEDSHAPE) == shape->overrides.end())
            continue;

        hotX = shape->hotspotX;
        hotY = shape->hotspotY;

        // matched :)
        bool foundAny = false;
        for (auto& image : impl->loadedShapes[shape.get()].images) {
            if (image->side != info.size)
                continue;

            // found size
            resultingImages.push_back(image.get());
            foundAny = true;
        }

        if (foundAny || shape->shapeType == SHAPE_SVG /* something broke, this shouldn't happen with svg */)
            break;

        // if we get here, means loadThemeStyle wasn't called most likely. If resize algo is specified, this is an error.
        if (shape->resizeAlgo != RESIZE_NONE) {
            Debug::log(HC_LOG_ERR, logFn, "getSurfaceFor didn't match a size?");
            return nullptr;
        }

        // find nearest
        int leader = 13371337;
        for (auto& image : impl->loadedShapes[shape.get()].images) {
            if (std::abs((int)(image->side - info.size)) > leader)
                continue;

            leader = image->side;
        }

        if (leader == 13371337) { // ???
            Debug::log(HC_LOG_ERR, logFn, "getSurfaceFor didn't match any nearest size?");
            return nullptr;
        }

        // we found nearest size
        for (auto& image : impl->loadedShapes[shape.get()].images) {
            if (image->side != leader)
                continue;

            // found size
            resultingImages.push_back(image.get());
            foundAny = true;
        }

        if (foundAny)
            break;

        Debug::log(HC_LOG_ERR, logFn, "getSurfaceFor didn't match any nearest size (2)?");
        return nullptr;
    }

    // alloc and return what we need
    SCursorImageData** data = (SCursorImageData**)malloc(sizeof(SCursorImageData*) * resultingImages.size());
    for (size_t i = 0; i < resultingImages.size(); ++i) {
        data[i]           = (SCursorImageData*)malloc(sizeof(SCursorImageData));
        data[i]->delay    = resultingImages[i]->delay;
        data[i]->size     = resultingImages[i]->side;
        data[i]->surface  = resultingImages[i]->cairoSurface;
        data[i]->hotspotX = hotX * data[i]->size;
        data[i]->hotspotY = hotY * data[i]->size;
    }

    outSize = resultingImages.size();

    Debug::log(HC_LOG_INFO, logFn, "getShapesC: found {} images for {}", outSize, shape_);

    return data;
}

bool CHyprcursorManager::loadThemeStyle(const SCursorStyleInfo& info) {
    Debug::log(HC_LOG_INFO, logFn, "loadThemeStyle: loading for size {}", info.size);

    for (auto& shape : impl->theme.shapes) {
        if (shape->resizeAlgo == RESIZE_NONE && shape->shapeType != SHAPE_SVG) {
            // don't resample NONE style cursors
            Debug::log(HC_LOG_TRACE, logFn, "loadThemeStyle: ignoring {}", shape->directory);
            continue;
        }

        bool sizeFound = false;

        if (shape->shapeType == SHAPE_PNG) {
            for (auto& image : impl->loadedShapes[shape.get()].images) {
                if (image->side != info.size)
                    continue;

                sizeFound = true;
                break;
            }

            // size wasn't found, let's resample.
            if (sizeFound)
                continue;

            SLoadedCursorImage* leader    = nullptr;
            int                 leaderVal = 1000000;
            for (auto& image : impl->loadedShapes[shape.get()].images) {
                if (image->side < info.size)
                    continue;

                if (image->side > leaderVal)
                    continue;

                leaderVal = image->side;
                leader    = image.get();
            }

            if (!leader) {
                for (auto& image : impl->loadedShapes[shape.get()].images) {
                    if (std::abs((int)(image->side - info.size)) > leaderVal)
                        continue;

                    leaderVal = image->side;
                    leader    = image.get();
                }
            }

            if (!leader) {
                Debug::log(HC_LOG_ERR, logFn, "Resampling failed to find a candidate???");
                return false;
            }

            const auto FRAMES = impl->getFramesFor(shape.get(), leader->side);

            Debug::log(HC_LOG_TRACE, logFn, "loadThemeStyle: png shape {} has {} frames", shape->directory, FRAMES.size());

            for (auto& f : FRAMES) {
                auto& newImage           = impl->loadedShapes[shape.get()].images.emplace_back(std::make_unique<SLoadedCursorImage>());
                newImage->artificial     = true;
                newImage->side           = info.size;
                newImage->artificialData = new char[info.size * info.size * 4];
                newImage->cairoSurface   = cairo_image_surface_create_for_data((unsigned char*)newImage->artificialData, CAIRO_FORMAT_ARGB32, info.size, info.size, info.size * 4);
                newImage->delay          = f->delay;

                const auto PCAIRO = cairo_create(newImage->cairoSurface);

                cairo_set_antialias(PCAIRO, shape->resizeAlgo == RESIZE_BILINEAR ? CAIRO_ANTIALIAS_GOOD : CAIRO_ANTIALIAS_NONE);

                cairo_save(PCAIRO);
                cairo_set_operator(PCAIRO, CAIRO_OPERATOR_CLEAR);
                cairo_paint(PCAIRO);
                cairo_restore(PCAIRO);

                const auto PTN = cairo_pattern_create_for_surface(f->cairoSurface);
                cairo_pattern_set_extend(PTN, CAIRO_EXTEND_NONE);
                const float scale = info.size / (float)f->side;
                cairo_scale(PCAIRO, scale, scale);
                cairo_pattern_set_filter(PTN, shape->resizeAlgo == RESIZE_BILINEAR ? CAIRO_FILTER_GOOD : CAIRO_FILTER_NEAREST);
                cairo_set_source(PCAIRO, PTN);

                cairo_rectangle(PCAIRO, 0, 0, info.size, info.size);

                cairo_fill(PCAIRO);
                cairo_surface_flush(newImage->cairoSurface);

                cairo_pattern_destroy(PTN);
                cairo_destroy(PCAIRO);
            }
        } else if (shape->shapeType == SHAPE_SVG) {
            const auto FRAMES = impl->getFramesFor(shape.get(), 0);

            Debug::log(HC_LOG_TRACE, logFn, "loadThemeStyle: svg shape {} has {} frames", shape->directory, FRAMES.size());

            for (auto& f : FRAMES) {
                auto& newImage           = impl->loadedShapes[shape.get()].images.emplace_back(std::make_unique<SLoadedCursorImage>());
                newImage->artificial     = true;
                newImage->side           = info.size;
                newImage->artificialData = new char[info.size * info.size * 4];
                newImage->cairoSurface   = cairo_image_surface_create_for_data((unsigned char*)newImage->artificialData, CAIRO_FORMAT_ARGB32, info.size, info.size, info.size * 4);
                newImage->delay          = f->delay;

                const auto PCAIRO = cairo_create(newImage->cairoSurface);

                cairo_save(PCAIRO);
                cairo_set_operator(PCAIRO, CAIRO_OPERATOR_CLEAR);
                cairo_paint(PCAIRO);
                cairo_restore(PCAIRO);

                GError*     error  = nullptr;
                RsvgHandle* handle = rsvg_handle_new_from_data((unsigned char*)f->data, f->dataLen, &error);

                if (!handle) {
                    Debug::log(HC_LOG_ERR, logFn, "Failed reading svg: {}", error->message);
                    return false;
                }

                RsvgRectangle rect = {0, 0, (double)info.size, (double)info.size};

                if (!rsvg_handle_render_document(handle, PCAIRO, &rect, &error)) {
                    Debug::log(HC_LOG_ERR, logFn, "Failed rendering svg: {}", error->message);
                    return false;
                }

                // done
                cairo_surface_flush(newImage->cairoSurface);
                cairo_destroy(PCAIRO);
            }
        } else {
            Debug::log(HC_LOG_ERR, logFn, "Invalid shapetype in loadThemeStyle");
            return false;
        }
    }

    return true;
}

void CHyprcursorManager::cursorSurfaceStyleDone(const SCursorStyleInfo& info) {
    for (auto& shape : impl->theme.shapes) {
        if (shape->resizeAlgo == RESIZE_NONE && shape->shapeType != SHAPE_SVG)
            continue;

        std::erase_if(impl->loadedShapes[shape.get()].images, [info, &shape](const auto& e) {
            const bool isSVG        = shape->shapeType == SHAPE_SVG;
            const bool isArtificial = e->artificial;

            // clean artificial rasters made for this
            if (isArtificial && e->side == info.size)
                return true;

            // clean invalid non-svg rasters
            if (!isSVG && e->side == 0)
                return true;

            return false;
        });
    }
}

void CHyprcursorManager::registerLoggingFunction(PHYPRCURSORLOGFUNC fn) {
    logFn = fn;
}

/*

PNG reading

*/

static cairo_status_t readPNG(void* data, unsigned char* output, unsigned int len) {
    const auto DATA = (SLoadedCursorImage*)data;

    if (!DATA->data)
        return CAIRO_STATUS_READ_ERROR;

    size_t toRead = len > DATA->dataLen - DATA->readNeedle ? DATA->dataLen - DATA->readNeedle : len;

    std::memcpy(output, (uint8_t*)DATA->data + DATA->readNeedle, toRead);
    DATA->readNeedle += toRead;

    if (DATA->readNeedle >= DATA->dataLen) {
        delete[] (char*)DATA->data;
        DATA->data = nullptr;
    }

    return CAIRO_STATUS_SUCCESS;
}

/*

General

*/

std::optional<std::string> CHyprcursorImplementation::loadTheme() {

    if (!themeAccessible(themeFullDir))
        return "Theme inaccessible";

    // load manifest
    CManifest  manifest(themeFullDir + "/manifest");
    const auto PARSERESULT = manifest.parse();

    if (PARSERESULT.has_value())
        return "couldn't parse manifest: " + *PARSERESULT;

    const std::string CURSORSSUBDIR = manifest.parsedData.cursorsDirectory;
    const std::string CURSORDIR     = themeFullDir + "/" + CURSORSSUBDIR;

    if (CURSORSSUBDIR.empty() || !std::filesystem::exists(CURSORDIR))
        return "loadTheme: cursors_directory missing or empty";

    for (auto& cursor : std::filesystem::directory_iterator(CURSORDIR)) {
        if (!cursor.is_regular_file()) {
            Debug::log(HC_LOG_TRACE, logFn, "loadTheme: skipping {}", cursor.path().string());
            continue;
        }

        auto& SHAPE       = theme.shapes.emplace_back(std::make_unique<SCursorShape>());
        auto& LOADEDSHAPE = loadedShapes[SHAPE.get()];

        // extract zip to raw data.
        int         errp = 0;
        zip_t*      zip  = zip_open(cursor.path().string().c_str(), ZIP_RDONLY, &errp);

        zip_file_t* meta_file = zip_fopen(zip, "meta.hl", ZIP_FL_UNCHANGED);
        bool        metaIsHL  = true;
        if (!meta_file) {
            meta_file = zip_fopen(zip, "meta.toml", ZIP_FL_UNCHANGED);
            metaIsHL  = false;
            if (!meta_file)
                return "cursor" + cursor.path().string() + "failed to load meta";
        }

        char* buffer = new char[1024 * 1024]; /* 1MB should be more than enough */

        int   readBytes = zip_fread(meta_file, buffer, 1024 * 1024 - 1);

        zip_fclose(meta_file);

        if (readBytes < 0) {
            delete[] buffer;
            return "cursor" + cursor.path().string() + "failed to read meta";
        }

        buffer[readBytes] = '\0';

        CMeta meta{buffer, metaIsHL};

        delete[] buffer;

        const auto METAPARSERESULT = meta.parse();
        if (METAPARSERESULT.has_value())
            return "cursor" + cursor.path().string() + "failed to parse meta: " + *METAPARSERESULT;

        for (auto& i : meta.parsedData.definedSizes) {
            SHAPE->images.push_back(SCursorImage{i.file, i.size, i.delayMs});
        }

        for (auto& i : SHAPE->images) {
            if (SHAPE->shapeType == SHAPE_INVALID) {
                if (i.filename.ends_with(".svg"))
                    SHAPE->shapeType = SHAPE_SVG;
                else if (i.filename.ends_with(".png"))
                    SHAPE->shapeType = SHAPE_PNG;
                else {
                    Debug::log(HC_LOG_WARN, logFn, "WARNING: image {} has no known extension, assuming png.", i.filename);
                    SHAPE->shapeType = SHAPE_PNG;
                }
            } else {
                if (SHAPE->shapeType == SHAPE_SVG && !i.filename.ends_with(".svg"))
                    return "meta invalid: cannot add .png files to an svg shape";
                else if (SHAPE->shapeType == SHAPE_PNG && i.filename.ends_with(".svg"))
                    return "meta invalid: cannot add .svg files to a png shape";
            }

            // load image
            Debug::log(HC_LOG_TRACE, logFn, "Loading {} for shape {}", i.filename, cursor.path().stem().string());
            auto* IMAGE  = LOADEDSHAPE.images.emplace_back(std::make_unique<SLoadedCursorImage>()).get();
            IMAGE->side  = SHAPE->shapeType == SHAPE_SVG ? 0 : i.size;
            IMAGE->delay = i.delay;
            IMAGE->isSVG = SHAPE->shapeType == SHAPE_SVG;

            // read from zip
            zip_file_t* image_file = zip_fopen(zip, i.filename.c_str(), ZIP_FL_UNCHANGED);
            if (!image_file)
                return "cursor" + cursor.path().string() + "failed to load image_file";

            IMAGE->data = new char[1024 * 1024]; /* 1MB should be more than enough, again. This probably should be in the spec. */

            IMAGE->dataLen = zip_fread(image_file, IMAGE->data, 1024 * 1024 - 1);

            zip_fclose(image_file);

            Debug::log(HC_LOG_TRACE, logFn, "Cairo: set up surface read");

            if (SHAPE->shapeType == SHAPE_PNG) {

                IMAGE->cairoSurface = cairo_image_surface_create_from_png_stream(::readPNG, IMAGE);

                if (const auto STATUS = cairo_surface_status(IMAGE->cairoSurface); STATUS != CAIRO_STATUS_SUCCESS) {
                    delete[] (char*)IMAGE->data;
                    IMAGE->data = nullptr;
                    return "Failed reading cairoSurface, status " + std::to_string((int)STATUS);
                }
            } else {
                Debug::log(HC_LOG_TRACE, logFn, "Skipping cairo load for a svg surface");
            }
        }

        if (SHAPE->images.empty())
            return "meta invalid: no images for shape " + cursor.path().stem().string();

        SHAPE->directory  = cursor.path().stem().string();
        SHAPE->hotspotX   = meta.parsedData.hotspotX;
        SHAPE->hotspotY   = meta.parsedData.hotspotY;
        SHAPE->resizeAlgo = stringToAlgo(meta.parsedData.resizeAlgo);

        zip_discard(zip);
    }

    return {};
}

std::vector<SLoadedCursorImage*> CHyprcursorImplementation::getFramesFor(SCursorShape* shape, int size) {
    std::vector<SLoadedCursorImage*> frames;

    for (auto& image : loadedShapes[shape].images) {
        if (!image->isSVG && image->side != size)
            continue;

        if (image->artificial)
            continue;

        frames.push_back(image.get());
    }

    return frames;
}
