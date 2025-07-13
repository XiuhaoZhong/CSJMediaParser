#include "mainwindow.h"

#include <iostream>
#include <string>

#include <QApplication>
#include <QScreen>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QFileDialog>

#include "CSJUtils/CSJStringUtils.h"
#include "MpegTool/CSJMpegTool.h"
#include "CSJUIKit/CSJDialog.h"
#include "CSJUIKit/CSJPopupWidget.h"
#include "CSJUIKit/CSJMediaPlayerWindow.h"
#include "CSJUIKit/CSJAccordionWidget.h"
#include "Controllers/CSJMediaDetailModule.h"
#include "Controllers/CSJMediaSPFDataController.h"

#define MAINWINDOW_WIDTH 1280
#define MAINWINDOW_HEIGHT 960
#define MENUBAR_HEIGHT 35

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    setMinimumSize(QSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT));

    initMenuBar();

    initUI();

    //stringTest();
    //ffmpegTestFunc();

    this->move(QApplication::screens().constFirst()->availableGeometry().center()- this->rect().center());

    std::string format_str = csjutils::Format("There are {0} fools in the world", 10);
    std::cout << format_str << std::endl;
}

MainWindow::~MainWindow() {

}

void MainWindow::initMenuBar() {
    QMenuBar *menuBar = this->menuBar();
    this->setMenuBar(menuBar);
    QMenu *fileMenu = menuBar->addMenu("File");
    QMenu *settingsMenu = menuBar->addMenu("Settings");
    QMenu *aboutMenu = menuBar->addMenu("About");

    initFileMenu(fileMenu);
    initSettingsMenu(settingsMenu);
    initAboutMenu(aboutMenu);
}

void MainWindow::initFileMenu(QMenu *menu) {

    QAction *openAction = menu->addAction("Select File");
    openAction->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(onSelectMediaFile()));

    QAction *closeAction = menu->addAction("Close File");
    connect(closeAction, SIGNAL(triggered(bool)), this, SLOT(onCloseMenuClicked()));
}

void MainWindow::initSettingsMenu(QMenu *menu) {
    QAction *colorSettingAction = menu->addAction("Color Settings");
    connect(colorSettingAction, SIGNAL(triggered(bool)), this, SLOT(onSettingsMenuClicked()));
}

void MainWindow::initAboutMenu(QMenu *menu) {
    QAction *pegVerAction = menu->addAction("Version");
    QAction *aboutAction = menu->addAction("About");
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(onAboutMenuClicked()));
    connect(pegVerAction, SIGNAL(triggered()), this, SLOT(onVerActionClicked()));
}

void MainWindow::initUI() {
    m_pCentrelWidget = new QWidget(this);
    this->setCentralWidget(m_pCentrelWidget);
    QPalette pale(m_pCentrelWidget->palette());
    pale.setColor(QPalette::Active, QPalette::Window, QColor(255,255,255));
    m_pCentrelWidget->setAutoFillBackground(true);
    m_pCentrelWidget->setPalette(pale);

    QHBoxLayout *topHonrizontalLayout = new QHBoxLayout(m_pCentrelWidget);

    QVBoxLayout *leftVBoxLayout = new QVBoxLayout();
    QVBoxLayout *rightBoxLayout = new QVBoxLayout();

    topHonrizontalLayout->addLayout(leftVBoxLayout);
    topHonrizontalLayout->addLayout(rightBoxLayout);

    topHonrizontalLayout->setStretchFactor(leftVBoxLayout, 1);
    topHonrizontalLayout->setStretchFactor(rightBoxLayout, 2);

    initLeftLayout(leftVBoxLayout);
    initRightLayout(rightBoxLayout);

    QLabel *tipLabel = new QLabel(m_pMediaInfoWidget);
    tipLabel->setText(QString("Here will show the media information!"));
}

void MainWindow::initLeftLayout(QVBoxLayout *leftLayout) {
    if (!leftLayout) {
        return ;
    }

    m_pBaseOptWiget = new QWidget();
    QPalette pal(m_pBaseOptWiget->palette());
    pal.setColor(QPalette::Active, QPalette::Window, QColor(196,196,196));
    m_pBaseOptWiget->setAutoFillBackground(true);
    m_pBaseOptWiget->setPalette(pal);
    leftLayout->addWidget(m_pBaseOptWiget);

    m_pMediaInfoWidget = new QWidget();
    leftLayout->addWidget(m_pMediaInfoWidget);

    QVBoxLayout *optLayout = new QVBoxLayout(m_pBaseOptWiget);
    optLayout->setSpacing(5);
    optLayout->setAlignment(Qt::AlignTop);
    m_pSelectFileBtn = new QPushButton();
    m_pSelectFileBtn->setText(QString("Select"));
    m_pSelectFileBtn->setFixedWidth(100);
    connect(m_pSelectFileBtn, SIGNAL(pressed()), this, SLOT(onSelectMediaFile()));

    m_pSourceInputEdit = new QLineEdit();
    m_pSourceInputEdit->setPlaceholderText(QString("Choose a file or input a url"));

    m_pOpenFileBtn = new QPushButton();
    m_pOpenFileBtn->setText("Open");
    m_pOpenFileBtn->setFixedWidth(100);
    connect(m_pOpenFileBtn, SIGNAL(pressed()), this, SLOT(onOpenMediaFile()));

    optLayout->addWidget(m_pSelectFileBtn);
    optLayout->addWidget(m_pSourceInputEdit);
    optLayout->addWidget(m_pOpenFileBtn);

    leftLayout->setStretchFactor(m_pBaseOptWiget, 1);
    leftLayout->setStretchFactor(m_pMediaInfoWidget, 2);
}

void MainWindow::initRightLayout(QVBoxLayout *rightLayout) {
    if (!rightLayout) {
        return ;
    }

    CSJMediaDetailModule *module = new CSJMediaDetailModule();
    module->initWidthParentWidget(this);
    rightLayout->addWidget(module->getWidget());
}

void MainWindow::onOpenMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Open File", QMessageBox::Ok);

}

void MainWindow::onCloseMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Close File", QMessageBox::Ok);
}

void MainWindow::onSettingsMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Clicked settings", QMessageBox::Ok);
}

void MainWindow::onAboutMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Clicked about", QMessageBox::Ok);
}

void MainWindow::onVerActionClicked() {
    // QString ffmpegVer = CSJMpegTool::getFFMpegVersion();

    // CSJDialog *dialog = new CSJDialog(ffmpegVer);
    // int res = dialog->exec();
    // if (res == 0) {
    //     return ;
    // }


    //std::shared_ptr<CSJMediaPlayerWindow> playerWindow = std::make_shared<CSJMediaPlayerWindow>();
    CSJMediaPlayerWindow *playerWindow = new CSJMediaPlayerWindow();
    playerWindow->show(true);
}

void MainWindow::onSelectMediaFile() {
    QString selFile = QFileDialog::getOpenFileName(this,
                                                   "Select a media file",
                                                   "",
                                                   "Media Files(*.mp4 *.mp3 *.avi *.aac *.m4a)",
                                                   Q_NULLPTR);

    if (selFile.length() == 0) {
        return ;
    }

    m_selFilePath = selFile;
    m_pSourceInputEdit->setText(m_selFilePath);
}

void MainWindow::onOpenMediaFile() {
    if (m_selFilePath.size() == 0) {
        return ;
    }

    CSJMediaSPFDataController::getInstance()->setMediaUrl(m_selFilePath);
    CSJMediaSPFDataController::getInstance()->startParse();
    m_pOpenFileBtn->setEnabled(false);
}

