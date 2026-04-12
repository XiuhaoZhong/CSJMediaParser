#include "CSJPathTool.h"

#include <filesystem>

#if defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>

#endif

namespace csjutils {

#if defined(__APPLE__)

static std::string getAppResourcePath() {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (!mainBundle) {
        return "";
    }

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    if (!resourcesURL) {
        return "";
    }

    char path[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) {
        CFRelease(resourcesURL);
        return std::string(path);
    }

    //CFRelease(resourcesURL);
    return "";
}

#endif 

CSJPathTool* CSJPathTool::getInstance() {
    static  CSJPathTool instance;
    return &instance;
}

void CSJPathTool::setWorkDirectory(fs::path work_directory) {
    m_workPath = work_directory;
}

bool CSJPathTool::fileExists(std::string &file_path) {
    if (file_path.size() == 0) {
        return false;
    }

    fs::path tmp(file_path);
    return fs::exists(tmp);
}

bool CSJPathTool::fileExists(std::wstring &file_path) {
    if (file_path.size() == 0) {
        return false;
    }

    fs::path tmp(file_path);
    return fs::exists(tmp);
}

fs::path CSJPathTool::getExecuteDir() {
    return m_workPath;
}

fs::path CSJPathTool::getResourceDir() {
    return getExecuteDir().append("Resources");
}

fs::path CSJPathTool::getModelDir() {
    return getResourceDir().append("models");
}

fs::path CSJPathTool::getTextureDir() {
    return getResourceDir().append("textures");
}

fs::path CSJPathTool::getImageDir() {
#if defined(__APPLE__)
    return getAppResourcePath().append("/Images");
#else
    return getResourceDir().append("images");
#endif
}

fs::path CSJPathTool::getShaderDir() {
#ifdef _WIN32
    return getResourceDir().append("DXShaders");
#else
    return getAppResourcePath().append("/MetalShaders");
#endif
}

fs::path CSJPathTool::getStyleSheetDir() {
    return getResourceDir().append("StyleSheets");
}

std::string CSJPathTool::getModelFileWithName(std::string &model_file_name) {
    return getModelDir().append(model_file_name).string();
}

std::string CSJPathTool::getImageWithName(std::string &image_name) {
    return getImageDir().append(image_name).string();
}

std::string CSJPathTool::getShaderFileWithName(std::string &shader_file_name) {
    return getShaderDir().append(shader_file_name).string();
}

std::string CSJPathTool::getTextureWithName(std::string &texture_file_name) {
    return getTextureDir().append(texture_file_name).string();
}

std::string CSJPathTool::getStyleSheetWithName(std::string &styleSheetsName) {
    return getStyleSheetDir().append(styleSheetsName).string();
}

} // namespace csjutils 