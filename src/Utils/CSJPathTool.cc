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

fs::path CSJPathTool::getExecuteDir() {
    return m_workPath;
}

fs::path CSJPathTool::getResourceDir() {
    return getExecuteDir().append("resource");
}

fs::path CSJPathTool::getModelDir() {
    return getResourceDir().append("models");
}

fs::path CSJPathTool::getTextureDir() {
    return getResourceDir().append("textures");
}

fs::path CSJPathTool::getImageDir() {
    return getResourceDir().append("images");
}

fs::path CSJPathTool::getShaderDir() {
    return getResourceDir().append("shaders");
}

std::string CSJPathTool::getModelFileWithName(std::string &model_file_name) {
    return getModelDir().string().append(model_file_name);
}

std::string CSJPathTool::getImageWithName(std::string &image_name) {
    return getImageDir().string().append(image_name);
}

std::string CSJPathTool::getShaderFileWithName(std::string &shader_file_name) {
    return getShaderDir().string().append(shader_file_name);
}

std::string CSJPathTool::getTextureWithName(std::string &texture_file_name) {
    return getTextureDir().string().append(texture_file_name);
}
