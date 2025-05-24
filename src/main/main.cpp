#include "mainwindow.h"

#include <QApplication>

#include "Utils/CSJPathTool.h"

int main(int argc, char *argv[]) {

    /* Record the work directory. */
    CSJPathTool *pathTool = CSJPathTool::getInstance();
    pathTool->setWorkDirectory(fs::canonical(fs::path(argv[0]).remove_filename()));

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
