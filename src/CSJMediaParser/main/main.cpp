#include "mainwindow.h"

#include <QApplication>
#include "CSJMainWindow.h"

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"

#include "CSJMediaEngine/CSJMediaEngineInfo.h"

using namespace csjutils;
using csjmediaengine::CSJMediaEngineInfo;

void init_log();
void uninit_log();
void customeMainWindowTheme(QMainWindow &mainWindow);

int main(int argc, char *argv[]) {
    /* Record the work directory. */
    std::string current_path(argv[0]);
    CSJPathTool::setWorkDirectory(fs::canonical(fs::path(current_path).remove_filename()));

    init_log();
    LOG_Info("CSJMediaParser started!");

    CSJMediaEngineInfo engineInfo;
    engineInfo.printEngineInfo();

    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    int exit_code = a.exec();
    LOG_Info("CSJMediaParser quit!");

    uninit_log();
    return exit_code;
}

void init_log() {
#ifdef _WIN32
    std::string log_path("Logs\\media_parser.log");
#else
    std::string log_path("/tmp/Logs/CSJMediaParser/media_parser.log");
#endif

    size_t pos = log_path.find_last_of("/\\");
    if (pos == std::string::npos)
        return;

    std::string dir = log_path.substr(0, pos);
    if (CSJPathTool::createPath(dir)) {
        CSJLog_Init(log_path.c_str());
    }
}

void uninit_log() {
    CSJLog_Uninit();
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
