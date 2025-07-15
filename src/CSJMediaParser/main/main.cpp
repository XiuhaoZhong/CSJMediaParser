#include "mainwindow.h"

#include <QApplication>

#include "CSJUtils/CSJPathTool.h"
#include "CSJUtils/CSJLogger.h"

#include "CSJMediaEngine/CSJMediaEngineInfo.h"

using namespace csjutils;
using csjmediaengine::CSJMediaEngineInfo;

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
    MainWindow w;
    w.show();

    int exit_code = a.exec();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser quit!\0");

    return exit_code;
}
