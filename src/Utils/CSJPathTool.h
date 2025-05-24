#ifndef __CSJPATHTOOL_H__
#define __CSJPATHTOOL_H__

#include <string>

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class CSJPathTool {
public:
    static CSJPathTool* getInstance();

    void setWorkDirectory(fs::path work_directory);
    bool fileExists(std::string &file_path);

    fs::path getExecuteDir();
    fs::path getResourceDir();
    fs::path getModelDir();
    fs::path getTextureDir();
    fs::path getImageDir();
    fs::path getShaderDir();

    std::string getModelFileWithName(std::string& modelFileName);
    std::string getImageWithName(std::string& imageName);
    std::string getShaderFileWithName(std::string& shaderFileName);
    std::string getTextureWithName(std::string& textureFileName);

protected:
    CSJPathTool() = default;
    ~CSJPathTool() = default;

private:
    fs::path m_workPath;
};

#endif // __CSJPATHTOOL_H__