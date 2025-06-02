#include "mainwindow.h"

#include <QApplication>

#include "Utils/CSJPathTool.h"
#include "Utils/CSJLogger.h"

int main(int argc, char *argv[]) {

    /* Record the work directory. */
    CSJPathTool *pathTool = CSJPathTool::getInstance();
    pathTool->setWorkDirectory(fs::canonical(fs::path(argv[0]).remove_filename()));

    CSJLogger* logger = CSJLogger::getLoggerInst();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser started!\0");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    int exit_code = a.exec();
    logger->log(CSJLogger::LogLevel::INFO_LOG, "CSJMediaParser quit!\0");

    return exit_code;
}
