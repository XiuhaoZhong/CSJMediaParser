#include "CSJPathTool.h"

#include <filesystem>

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
    return getExecuteDir().append(L"resource");
}

fs::path CSJPathTool::getModelDir() {
    return getResourceDir().append(L"models");
}

fs::path CSJPathTool::getTextureDir() {
    return getResourceDir().append(L"textures");
}

fs::path CSJPathTool::getImageDir() {
    return getResourceDir().append(L"images");
}

fs::path CSJPathTool::getShaderDir() {
#ifdef _WIN32
    return getResourceDir().append(L"DXShaders");
#else
    return getResourceDir().append(L"MetalShaders");
#endif
}

std::wstring CSJPathTool::getModelFileWithName(std::wstring &model_file_name) {
    return getModelDir().append(model_file_name);
}

std::wstring CSJPathTool::getImageWithName(std::wstring &image_name) {
    return getImageDir().append(image_name);
}

std::wstring CSJPathTool::getShaderFileWithName(std::wstring &shader_file_name) {
    return getShaderDir().append(shader_file_name);
}

std::wstring CSJPathTool::getTextureWithName(std::wstring &texture_file_name) {
    return getTextureDir().append(texture_file_name);
}
