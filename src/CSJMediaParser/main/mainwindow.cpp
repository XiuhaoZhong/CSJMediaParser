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
#include "CSJUIKit/CSJDialog.h"
#include "CSJUIKit/CSJPopupWidget.h"
#include "CSJUIKit/CSJMediaPlayerWindow.h"
#include "CSJUIKit/CSJAccordionWidget.h"
#include "Controllers/CSJMediaDetailModule.h"
#include "Controllers/CSJMediaSPFDataController.h"

#define MAINWINDOW_WIDTH 800//640
#define MAINWINDOW_HEIGHT 560//480
#define MENUBAR_HEIGHT 35

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    setMinimumSize(QSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT));

    initMenuBar();

    initUI();

    this->move(QApplication::screens().constFirst()->availableGeometry().center()- this->rect().center());
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
    connect(openAction, &QAction::triggered, this, &MainWindow::onSelectMediaFile);

    QAction *closeAction = menu->addAction("Close File");
    connect(closeAction, &QAction::triggered, this, &MainWindow::onCloseMenuClicked);
}

void MainWindow::initSettingsMenu(QMenu *menu) {
    QAction *colorSettingAction = menu->addAction("Color Settings");
    connect(colorSettingAction, &QAction::triggered, this, &MainWindow::onSettingsMenuClicked);
}

void MainWindow::initAboutMenu(QMenu *menu) {
    QAction *pegVerAction = menu->addAction("Version");
    QAction *aboutAction = menu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutMenuClicked);
    connect(pegVerAction, &QAction::triggered, this, &MainWindow::onVerActionClicked);
}

void MainWindow::initUI() {

    this->setAutoFillBackground(true);

    m_pCentrelWidget = new QWidget(this);
    this->setCentralWidget(m_pCentrelWidget);
    this->setObjectName("centerWidget");
    m_pCentrelWidget->setStyleSheet(R"(
        QWidget#centerWidget {
            background-color: #adccadff;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(m_pCentrelWidget);

    QVBoxLayout *headerBoxLayout = new QVBoxLayout();
    mainLayout->addLayout(headerBoxLayout, 1);
    initHeaderLayout(headerBoxLayout);

    QHBoxLayout *optionsLayout = new QHBoxLayout();
    mainLayout->addLayout(optionsLayout, 1);

    QWidget *playerWidget = new QWidget();
    playerWidget->setStyleSheet(R"(
        QWidget {
            border-radius: 20px;
            background-color: #7FFFFF;
        }
    )");

    QWidget *parserWidget = new QWidget();
    parserWidget->setStyleSheet(R"(
        QWidget {
            border-radius: 20px;
            background-color: #FF7FFF;
        }
    )");

    QWidget *formatWidget = new QWidget();
    formatWidget->setStyleSheet(R"(
        QWidget {
            border-radius: 20px;
            background-color: #FFFF7F;
        }
    )");

    optionsLayout->addWidget(playerWidget, 1);
    optionsLayout->addWidget(parserWidget, 1);
    optionsLayout->addWidget(formatWidget, 1);
}

void MainWindow::initHeaderLayout(QVBoxLayout *headerLayout) {
    if (!headerLayout) {
        return ;
    }

    m_pBaseOptWiget = new QWidget();
    QPalette pal(m_pBaseOptWiget->palette());
    pal.setColor(QPalette::Window, QColor(196,196,196));
    m_pBaseOptWiget->setAutoFillBackground(true);
    m_pBaseOptWiget->setPalette(pal);
    headerLayout->addWidget(m_pBaseOptWiget);

    QVBoxLayout *optLayout = new QVBoxLayout(m_pBaseOptWiget);
    optLayout->setSpacing(5);
    optLayout->setAlignment(Qt::AlignTop);
    m_pSelectFileBtn = new QPushButton();
    m_pSelectFileBtn->setText(QString("Select"));
    m_pSelectFileBtn->setFixedWidth(100);
    connect(m_pSelectFileBtn, &QPushButton::pressed, this, &MainWindow::onSelectMediaFile);

    m_pSourceInputEdit = new QLineEdit();
    m_pSourceInputEdit->setPlaceholderText(QString("Choose a file or input a url"));

    m_pOpenFileBtn = new QPushButton();
    m_pOpenFileBtn->setText("Open");
    m_pOpenFileBtn->setFixedWidth(100);
    connect(m_pOpenFileBtn, &QPushButton::pressed, this, &MainWindow::onOpenMediaFile);

    optLayout->addWidget(m_pSelectFileBtn);
    optLayout->addWidget(m_pSourceInputEdit);
    optLayout->addWidget(m_pOpenFileBtn);
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

