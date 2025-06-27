#include "mainwindow.h"

#include <QApplication>

#include "Utils/CSJPathTool.h"
#include "Utils/CSJLogger.h"
#include "Utils/CSJStringUtils.h"

#include "CSJMediaEngine/CSJMediaEngineInfo.h"

int main(int argc, char *argv[]) {

    /* Record the work directory. */
    CSJPathTool *pathTool = CSJPathTool::getInstance();
    std::wstring current_path(csjutils::CSJStringUtil::char2wstring(argv[0]));
    pathTool->setWorkDirectory(fs::canonical(fs::path(current_path).remove_filename()));

    CSJLogger* logger = CSJLogger::getLoggerInst();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser started!\0");

    CSJMediaEngine::CSJMediaEngineInfo engineInfo;
    engineInfo.printEngineInfo();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    int exit_code = a.exec();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser quit!\0");

    return exit_code;
}
