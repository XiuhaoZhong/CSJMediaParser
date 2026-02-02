#ifndef __CSJPATHTOOL_H__
#define __CSJPATHTOOL_H__

#include <string>

#include <filesystem>
#include <memory>

#include "CSJUtils_Export.h"

namespace fs = std::filesystem;

namespace csjutils {

class CSJUTILS_API CSJPathTool {
public:
    static CSJPathTool* getInstance();

    void setWorkDirectory(fs::path work_directory);
    static bool fileExists(std::string &file_path);
    static bool fileExists(std::wstring &file_path);

    fs::path getExecuteDir();
    fs::path getResourceDir();
    fs::path getModelDir();
    fs::path getTextureDir();
    fs::path getImageDir();
    fs::path getShaderDir();
    fs::path getStyleSheetDir();

    std::string getModelFileWithName(std::string& modelFileName);
    std::string getImageWithName(std::string& imageName);
    std::string getShaderFileWithName(std::string& shaderFileName);
    std::string getTextureWithName(std::string& textureFileName);
    std::string getStyleSheetWithName(std::string& styleSheetsName);

protected:
    CSJPathTool() = default;
    ~CSJPathTool() = default;

private:
    fs::path m_workPath;
};

} // namespace csjutils

#endif // __CSJPATHTOOL_H__