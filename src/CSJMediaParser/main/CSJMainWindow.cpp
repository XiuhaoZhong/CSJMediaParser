#include "CSJMainWindow.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QStyle>
#include <QPainter>
#include <QLinearGradient>
#include <QPainterPath>
#include <QMenuBar>
#include <QToolBar>
#include <QScreen>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QFileDialog>

#include <qDebug>

#include "CSJUtils/CSJStringUtils.h"
#include "CSJUIKit/CSJDialog.h"
#include "CSJUIKit/CSJPopupWidget.h"
#include "CSJUIKit/CSJMediaPlayerWindow.h"
#include "CSJUIKit/CSJAccordionWidget.h"
#include "Controllers/CSJMediaDetailModule.h"
#include "Controllers/CSJMediaSPFDataController.h"

CSJMainWindow::CSJMainWindow(QWidget *parent) 
    : QMainWindow(parent) {

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setFixedSize(800, 600);

    setObjectName("CSJMainWindow");
    setStyleSheet(R"(
        QWidget#CSJMainWindow {
            corner-radius: 5px;
            background-color: #4E94CE;
        }    
    )");

    createTitleBar();

    initWidgets();

    assembleLayout();
}

void CSJMainWindow::createTitleBar() {
    m_titleBar = new QWidget();
    m_titleBar->setFixedHeight(30); // 标题栏高度
    m_titleBar->setObjectName("titleBar");

    // 核心：设置标题栏主题样式（背景色、文字色、按钮样式，可自定义浅色/深色主题）
    // 浅色主题：background-color: #F8F9FA; color: #333333;
    // 深色主题：background-color: #2C3E50; color: #FFFFFF;
    m_titleBar->setStyleSheet(R"(
        QWidget#titleBar {
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
        }

        QWidget {
            /* 标题栏背景色（主题主色调，与整体窗口主题统一） */
            background-color: #4E94CE;
        }

        QLabel {
            /* 标题文字色（主题辅助色） */
            color: white;
            font-size: 16px;
            font-weight: 500;
        }

        QWidget#BtnWidget {
            border-top-right-radius: 5px;
        }   

        QPushButton#CloseBtn {
            border-top-right-radius: 5px;
        }

        QPushButton {
            background-color: transparent; /* 按钮透明背景，贴合标题栏主题 */
            color: white; /* 按钮图标/文字色 */
            border: none;
            width: 48px;
            height: 48px;
        }
        QPushButton:hover {
            /* 按钮悬浮色（主题高亮色，增强交互体验） */
            background-color: rgba(255, 255, 255, 0.2);
        }
        /* 关闭按钮单独设置悬浮色，提升警示性 */
        QPushButton#CloseBtn:hover {
            background-color: #E74C3C;
        } 
    )");

    // 标题栏布局：左（窗口标题）、中（预留，可添加搜索框等）、右（窗口控制按钮）
    QHBoxLayout *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(16, 0, 0, 0);
    titleLayout->setSpacing(0);

    // 窗口标题
    QLabel *titleLabel = new QLabel("CSJMediaParser");
    titleLayout->addWidget(titleLabel);

    // 拉伸填充，让按钮右对齐
    titleLayout->addStretch();

    // 窗口控制按钮容器
    QWidget *btnWidget = new QWidget();
    btnWidget->setObjectName("BtnWidget");
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(0);

    // 最小化按钮
    QPushButton *minBtn = new QPushButton();
    minBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    connect(minBtn, &QPushButton::clicked, this, &QMainWindow::showMinimized);

    // 最大化/还原按钮
    QPushButton *maxBtn = new QPushButton();
    maxBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    connect(maxBtn, &QPushButton::clicked, this, [=]() {
        isMaximized() ? showNormal() : showMaximized();
        maxBtn->setIcon(style()->standardIcon(isMaximized() ? QStyle::SP_TitleBarNormalButton : QStyle::SP_TitleBarMaxButton));
    });

    // 关闭按钮（设置对象名，匹配QSS单独样式）
    QPushButton *closeBtn = new QPushButton();
    closeBtn->setObjectName("CloseBtn");
    closeBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(closeBtn, &QPushButton::clicked, this, &CSJMainWindow::onQuitClicked);

    // 添加按钮到布局
    btnLayout->addWidget(minBtn);
    btnLayout->addWidget(maxBtn);
    btnLayout->addWidget(closeBtn);
    titleLayout->addWidget(btnWidget);
}

void CSJMainWindow::initWidgets() {
    // 菜单栏（设置与标题栏协调的主题颜色）
    QMenuBar *menuBar = this->menuBar();
    menuBar->setStyleSheet(R"(
        QMenuBar {
            background-color: #2E64AA;
            color: yellow;
            font-size: 14px;
            height: 36px;
        }
        QMenuBar::item {
            padding: 0 20px;
        }
        QMenuBar::item:selected {
            background-color: #1E4E8E;
        }
        QMenu {
            background-color: white;
            color: #333;
            border: 1px solid #E0E0E0;
        }
        QMenu::item:selected {
            background-color: #F0F5FF;
            color: #4E94CE;
        }
    )");

    QMenu *fileMenu = menuBar->addMenu("File(&F)");
    QAction *openAction = fileMenu->addAction("Open(&O)");
    QAction *closeAction = fileMenu->addAction("Close");
    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction("Quit(&Q)");

    openAction->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(openAction, &QAction::triggered, this, &CSJMainWindow::onSelectMediaFile);
    connect(closeAction, &QAction::triggered, this, &CSJMainWindow::onCloseMenuClicked);

    connect(quitAction, &QAction::triggered, this, &CSJMainWindow::onQuitClicked);

    QMenu *settingsMenu = menuBar->addMenu("Settings");
    QAction *colorSettingAction = settingsMenu->addAction("Color Settings");
    connect(colorSettingAction, &QAction::triggered, this, &CSJMainWindow::onSettingsMenuClicked);

    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *pegVerAction = helpMenu->addAction("Version");
    QAction *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &CSJMainWindow::onAboutMenuClicked);
    connect(pegVerAction, &QAction::triggered, this, &CSJMainWindow::onVerActionClicked);
}

void CSJMainWindow::assembleLayout() {
    // 创建主容器控件，承载标题栏和QMainWindow的中心部件
    QWidget *mainContainer = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 添加自定义标题栏
    mainLayout->addWidget(m_titleBar);

    // 添加QMainWindow的原有中央部件容器（保留菜单栏、工具栏、中心部件）
    QWidget *centralContainer = new QWidget();
    centralContainer->setObjectName("centralContainer");
    centralContainer->setStyleSheet(R"(
        QWidget#centralContainer {
            background-color:  #FFFFFA;
        }
    )");
    QVBoxLayout *centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    // 添加菜单栏（若需菜单栏在标题栏下方，直接添加）
    centralLayout->addWidget(this->menuBar());
    QWidget *centralWidget = new QWidget();
    centralWidget->setStyleSheet("background-color: #F8F9FA;");
    centralLayout->addWidget(centralWidget, 1);

    mainLayout->addWidget(centralContainer, 1);

    fillContentWidget(centralWidget);

    // 设置QMainWindow的中心部件为主容器
    this->setCentralWidget(mainContainer);
}

void CSJMainWindow::fillContentWidget(QWidget *contentWidget) {
    QVBoxLayout *containerLayout = new QVBoxLayout(contentWidget);

    QVBoxLayout *headerBoxLayout = new QVBoxLayout();
    containerLayout->addLayout(headerBoxLayout, 1);
    initHeaderLayout(headerBoxLayout);

    QHBoxLayout *optionsLayout = new QHBoxLayout();
    containerLayout->addLayout(optionsLayout, 1);

    QWidget *playerWidget = new QWidget();
    playerWidget->setObjectName("playerWidget");
    playerWidget->setStyleSheet(R"(
        QWidget#playerWidget {
            border-radius: 20px;
            background-color: #7FFFFF;
        }
    )");

    QWidget *parserWidget = new QWidget();
    parserWidget->setObjectName("parserWidget");
    parserWidget->setStyleSheet(R"(
        QWidget#parserWidget {
            border-radius: 20px;
            background-color: #FF7FFF;
        }
    )");

    QWidget *formatWidget = new QWidget();
    formatWidget->setObjectName("formatWidget");
    formatWidget->setStyleSheet(R"(
        QWidget#formatWidget {
            border-radius: 20px;
            background-color: #FFFF7F;
        }
    )");

    optionsLayout->addWidget(playerWidget, 1);
    optionsLayout->addWidget(parserWidget, 1);
    optionsLayout->addWidget(formatWidget, 1);
}

void CSJMainWindow::initHeaderLayout(QVBoxLayout *leftLayout) {
    if (!leftLayout) {
        return ;
    }

    m_pBaseOptWiget = new QWidget();
    QPalette pal(m_pBaseOptWiget->palette());
    pal.setColor(QPalette::Window, QColor(196,196,196));
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

void CSJMainWindow::mousePressEvent(QMouseEvent *event) {
     if (event->y() <= m_titleBar->height() && event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void CSJMainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() & Qt::LeftButton) {
        if (!m_dragPos.isNull()) {
            m_dragPos = QPoint();
        }
    }
}

void CSJMainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !m_dragPos.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

void CSJMainWindow::quitApp() {
    close();
}

void CSJMainWindow::onOpenMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Open File", QMessageBox::Ok);

}

void CSJMainWindow::onCloseMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Close File", QMessageBox::Ok);
}

void CSJMainWindow::onSettingsMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Clicked settings", QMessageBox::Ok);
}

void CSJMainWindow::onAboutMenuClicked() {
    QMessageBox::information(nullptr, "Tips", "Clicked about", QMessageBox::Ok);
}

void CSJMainWindow::onVerActionClicked() {
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

void CSJMainWindow::onSelectMediaFile() {
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

void CSJMainWindow::onOpenMediaFile() {
    if (m_selFilePath.size() == 0) {
        return ;
    }

    CSJMediaSPFDataController::getInstance()->setMediaUrl(m_selFilePath);
    CSJMediaSPFDataController::getInstance()->startParse();
    m_pOpenFileBtn->setEnabled(false);
}

void CSJMainWindow::onQuitClicked() {
    quitApp();
}

