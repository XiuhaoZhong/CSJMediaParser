#include "mainwindow.h"

#include <QApplication>
#include "CSJMainWindow.h"

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"

#include "CSJMediaEngine/CSJMediaEngineInfo.h"

using namespace csjutils;
using csjmediaengine::CSJMediaEngineInfo;

void customeMainWindowTheme(QMainWindow &mainWindow);

int main(int argc, char *argv[]) {
    /* Record the work directory. */
    CSJPathTool *pathTool = CSJPathTool::getInstance();
    std::string current_path(argv[0]);
    pathTool->setWorkDirectory(fs::canonical(fs::path(current_path).remove_filename()));

    CSJLogger* logger = CSJLogger::getLoggerInst();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser started!\0");

    CSJMediaEngineInfo engineInfo;
    engineInfo.printEngineInfo();

    QApplication a(argc, argv);
    CSJMainWindow w;

    w.show();

    int exit_code = a.exec();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser quit!\0");

    return exit_code;
}

void customeMainWindowTheme(QMainWindow &mainWindow) {
    mainWindow.setObjectName("CustomeMainWindow");

    mainWindow.setStyleSheet(R"(
        /* ------------- 菜单栏 QMenuBar 主题颜色 ------------- */
        QMenuBar {
            background-color: #4E94CE; /* 菜单栏背景色（主题主色调） */
            color: white; /* 菜单栏文字色 */
            font-size: 14px;
            height: 36px;
        }
        
        QMenuBar::item {
            padding: 0 20px; /* 菜单项内边距 */
        }
        
        QMenuBar::item:selected {
            background-color: #2E64AA; /* 菜单项选中背景色（主题辅助色调） */
        }
        
        QMenu {
            background-color: #FFFFFF; /* 下拉菜单背景色 */
            border: 1px solid #E0E0E0;
            color: #333333;
        }
        
        QMenu::item:selected {
            background-color: #F0F5FF;
            color: #4E94CE;
        }
    )");
}
