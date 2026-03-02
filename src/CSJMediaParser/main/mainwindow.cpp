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
#include <QGraphicsBlurEffect>

#include "CSJUtils/CSJStringUtils.h"
#include "CSJUIKit/CSJDialog.h"
#include "CSJUIKit/CSJPopupWidget.h"
#include "CSJUIKit/CSJMediaPlayerWindow.h"
#include "CSJUIKit/CSJAccordionWidget.h"
#include "CSJUIModules/CSJGlassWidget.h"
#include "Controllers/CSJMediaDetailModule.h"
#include "Controllers/CSJMediaSPFDataController.h"

#define MAINWINDOW_WIDTH 600
#define MAINWINDOW_HEIGHT 420
#define MENUBAR_HEIGHT 35

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    setFixedSize(QSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT));
    
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
    m_pCentrelWidget->setObjectName("centerWidget");
    m_pCentrelWidget->setStyleSheet(R"(
        QWidget#centerWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                           stop:0 #a1e9df, stop:1 #029eb9);
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(m_pCentrelWidget);

    QHBoxLayout *headerBoxLayout = new QHBoxLayout();
    mainLayout->addLayout(headerBoxLayout, 1);
    initLogoArea(headerBoxLayout);

    QHBoxLayout *optionsLayout = new QHBoxLayout();
    mainLayout->addLayout(optionsLayout, 1);

    QWidget *playerWidget = new CSJGlassWidget();
    QPushButton *playerEntranceBtn = new QPushButton("Media Player");
    QVBoxLayout *playerEntranceLayout = new QVBoxLayout(playerWidget);
    playerEntranceLayout->setAlignment(Qt::AlignCenter);
    playerEntranceLayout->setContentsMargins(5, 5, 5 , 5);
    playerEntranceLayout->addWidget(playerEntranceBtn, 1);
    connect(playerEntranceBtn, &QPushButton::clicked, this, &MainWindow::onPlayerEntanceClicked);

    QWidget *parserWidget = new QWidget();
    parserWidget->setObjectName("parserWidget");
    parserWidget->setStyleSheet(R"(
        QWidget#parserWidget {
            border-radius: 20px;
            background-color: rgb(255, 128, 255);
        }
    )");
    QPushButton *mediaParserEntranceBtn = new QPushButton("Media Parser");
    QVBoxLayout *mediaParserEntranceLayout = new QVBoxLayout(parserWidget);
    mediaParserEntranceLayout->setAlignment(Qt::AlignCenter);
    mediaParserEntranceLayout->setContentsMargins(5, 5, 5 , 5);
    mediaParserEntranceLayout->addWidget(mediaParserEntranceBtn, 1);
    connect(mediaParserEntranceBtn, &QPushButton::clicked, this, &MainWindow::onParserEntranceClicked);

    QWidget *formatWidget = new QWidget();
    formatWidget->setObjectName("formatWidget");
    formatWidget->setStyleSheet(R"(
        QWidget#formatWidget {
            border-radius: 20px;
            background-color: #FFFF7F;
        }
    )");
    QPushButton *formatFactoryEntranceBtn = new QPushButton("Format Factory");
    QVBoxLayout *formatFactoryEntranceLayout = new QVBoxLayout(formatWidget);
    formatFactoryEntranceLayout->setAlignment(Qt::AlignCenter);
    formatFactoryEntranceLayout->setContentsMargins(5, 5, 5 , 5);
    formatFactoryEntranceLayout->addWidget(formatFactoryEntranceBtn, 1);
    connect(formatFactoryEntranceBtn, &QPushButton::clicked, this, &MainWindow::onParserEntranceClicked);

    optionsLayout->addWidget(playerWidget, 1);
    optionsLayout->addWidget(parserWidget, 1);
    optionsLayout->addWidget(formatWidget, 1);
}

void MainWindow::initLogoArea(QHBoxLayout *headerLayout) {
    if (!headerLayout) {
        return ;
    }

    QWidget *logoWidget = new QWidget();
    logoWidget->setStyleSheet(R"(
        QWidget {
            background-color: #807FFFFF;
            border-radius: 20px;
        }
    )");

    headerLayout->addWidget(logoWidget, 1);

    QHBoxLayout *layout = new QHBoxLayout(logoWidget);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *logoLabel = new QLabel("Welcome to CSJ Media Space!");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet(R"(
        QLabel {
            font-family: "Georgia";
            font-size: 32px;
            font-weight: bold;
            font-style: italic;
            background-color: transparent;
        }    
    )");

    layout->addWidget(logoLabel, 1);
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
    QString ffmpegVer("Current ffmpeg version: customer compiled!");

    CSJDialog *dialog = new CSJDialog(ffmpegVer);
    int res = dialog->exec();
    if (res == 0) {
        return ;
    }
}

void MainWindow::onPlayerEntanceClicked() {
    CSJMediaPlayerWindow *playerWindow = new CSJMediaPlayerWindow();
    playerWindow->show(true);
}

void MainWindow::onParserEntranceClicked() {

}

void MainWindow::onFormatFactoryEntranceClicked() {

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

