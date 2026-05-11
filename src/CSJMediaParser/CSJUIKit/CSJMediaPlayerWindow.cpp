#include "CSJMediaPlayerWindow.h"

#include <QApplication>
#include <QScreen>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "CSJUtils/CSJLogger.h"
#include "CSJUtils/CSJPathTool.h"

#include "Global/global_constant.h"

#include "CSJVideoRendererWidget.h"
#include "CSJPlayerControllerWidget.h"

using namespace csjutils;

static int player_window_width = 1280;
static int player_window_height = 760;

CSJMediaPlayerWindow::CSJMediaPlayerWindow(QWidget *parent)
    : QWidget(parent)
    , m_playStatus(PLAYSTATUS_STOP) {

    setAttribute(Qt::WA_DeleteOnClose, true);
    setMinimumSize(640, 380);

    initUI();

    // Shows the center widget at the center of the screen.
    QPoint screenCenter = QApplication::screens().constFirst()->availableGeometry().center();
    QRect curRect = geometry();
    setGeometry(QRect(screenCenter.rx() - curRect.width() / 2,
                        screenCenter.ry() - curRect.height() / 2,
                        curRect.width(),
                        curRect.height()));

    m_playController = CSJPlayerController::createPlayerController();
}

CSJMediaPlayerWindow::~CSJMediaPlayerWindow() {
    LOG_Info("CSJMediaPlayerWindow destoryed!");
}

void CSJMediaPlayerWindow::closeEvent(QCloseEvent *event) {
    LOG_Info("Player window is going to be closed");

    onWidgetClose();

    QWidget::closeEvent(event);
}

void CSJMediaPlayerWindow::initUI()
{
    resize(QSize(player_window_width, player_window_height));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QVBoxLayout *playerLayout = new QVBoxLayout();
    playerLayout->setSpacing(0);
    mainLayout->addLayout(playerLayout, 18);
    mainLayout->setSpacing(10);

    m_pVideoRenderWidget = new CSJVideoRendererWidget();
    playerLayout->addWidget(m_pVideoRenderWidget);

    CSJPlayerControllerWidget *ctrlWidget = new CSJPlayerControllerWidget(this);
    playerLayout->addWidget(ctrlWidget, 1);

    setWindowTitle("CSJMediaPlayer");
    setStyleSheet("QWidget {background-color: #1A202C;}");

    connect(ctrlWidget, &CSJPlayerControllerWidget::showImage, this, &CSJMediaPlayerWindow::onSetImage);
    connect(ctrlWidget, &CSJPlayerControllerWidget::play, this, &CSJMediaPlayerWindow::onPlayBtnClicked);
    connect(ctrlWidget, &CSJPlayerControllerWidget::pause, this, &CSJMediaPlayerWindow::onPauseBtnClicked);
    connect(ctrlWidget, &CSJPlayerControllerWidget::resume, this, &CSJMediaPlayerWindow::onResumeBtnClicked);
    connect(ctrlWidget, &CSJPlayerControllerWidget::stop, this, &CSJMediaPlayerWindow::onStopBtnClicked);
    m_playerCtrlWidget = ctrlWidget;
}

void CSJMediaPlayerWindow::show(bool bShow) {
    setVisible(true);
    m_pVideoRenderWidget->setRenderType(ACTIVE_RENDERING);
}

void CSJMediaPlayerWindow::onPlayBtnClicked() {
    LOG_Info("Start playing...");
    if (!m_playController) {
        LOG_Warn("Error! Play controller hasn't been created!");
        return ;
    }

    std::string file_path = CSJPathTool::getResVideoFileWithName(g_test_mp4_file);
    m_playController->setPlayFile(file_path);

    if (m_playStatus == PLAYSTATUS_STOP) {
        LOG_Info(" Start playing... ");
        m_playController->start();
    } else if (m_playStatus == PLAYSTATUS_PAUSE) {
        LOG_Info(" Resume playing... ");
        m_playController->resume();
    } else if (m_playStatus == PLAYSTATUS_PLAY) {
        LOG_Info(" Pause playing... ");
        m_playController->pause();
    }
}

void CSJMediaPlayerWindow::onPauseBtnClicked() {
    LOG_Info("Pause playing ...");
    if (!m_playController) {
        LOG_Warn("Error! Play controller hasn't been created!");
        return ;
    }

    if (m_playController->isPlaying()) {
        LOG_Info("player pause!");
        m_playController->pause();
    }
}

void CSJMediaPlayerWindow::onResumeBtnClicked() {
    LOG_Info("Resume playing ...");
    if (!m_playController) {
        LOG_Warn("Error! Play controller hasn't been created!");
        return ;
    }

    if (m_playController->isPausing()) {
        LOG_Info("player resume!");
        m_playController->resume();
    }
}

void CSJMediaPlayerWindow::onStopBtnClicked() {
    LOG_Info("Stop playing...");
    if (!m_playController) {
        LOG_Warn("Error! Play controller hasn't been created!");
        return ;
    }

    if (!m_playController->isStopping()) {
        LOG_Info("player stop!");

        m_playController->stop();
        m_playStatus = PLAYSTATUS_STOP;
    }
}

void CSJMediaPlayerWindow::onFastForwardBtnClicked() {
    if (!m_playController) {
        LOG_Warn("Error! Play controller hasn't been created!");
        return ;
    }

    LOG_Info("Fast forward... ");
}

void CSJMediaPlayerWindow::onFastBackBtnClicked() {
    if (!m_playController) {
        LOG_Warn("Error! Play controller hasn't been created!");
        return ;
    }

    LOG_Info("Fast backward... ");
}

void CSJMediaPlayerWindow::onSetImage() {
    if (!m_pVideoRenderWidget) {
        return ;
    }

    m_pVideoRenderWidget->showDefaultImage();
}

void CSJMediaPlayerWindow::onWidgetClose() {
    if (m_playController) {
        m_playController->stop();
    }
}
