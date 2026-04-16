#include "CSJMediaPlayerWindow.h"

#include <QDebug>

#include <QApplication>
#include <QScreen>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "CSJVideoRendererWidget.h"
#include "CSJPlayerControllerWidget.h"

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

}

CSJMediaPlayerWindow::~CSJMediaPlayerWindow() {
    qDebug() << "CSJMediaPlayerWindow destoryed!";
}

void CSJMediaPlayerWindow::initUI() {
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
    m_playerCtrlWidget = ctrlWidget;
}

void CSJMediaPlayerWindow::show(bool bShow) {
    setVisible(true);
    m_pVideoRenderWidget->setRenderType(ACTIVE_RENDERING);
}

void CSJMediaPlayerWindow::onPlayBtnClicked() {
    if (!m_playController) {
        qDebug() << "Error! Play controller hasn't been created!";
        return ;
    }

    if (m_playStatus == PLAYSTATUS_STOP) {
        qDebug() << "[LOG] Start playing... ";
        m_playController->start();
    } else if (m_playStatus == PLAYSTATUS_PAUSE) {
        qDebug() << "[LOG] Resume playing... ";
        m_playController->resume();
    } else if (m_playStatus == PLAYSTATUS_PLAY) {
        qDebug() << "[LOG] Pause playing... ";
        m_playController->pause();
    }
}

void CSJMediaPlayerWindow::onStopBtnClicked() {
    if (!m_playController) {
        qDebug() << "Error! Play controller hasn't been created!";
        return ;
    }

    qDebug() << "[LOG] Stop playing... ";

    m_playController->stop();
    m_playStatus = PLAYSTATUS_STOP;
}

void CSJMediaPlayerWindow::onFastForwardBtnClicked() {
    if (!m_playController) {
        qDebug() << "Error! Play controller hasn't been created!";
        return ;
    }

    qDebug() << "[LOG] Fast forward... ";
}

void CSJMediaPlayerWindow::onFastBackBtnClicked() {
    if (!m_playController) {
        qDebug() << "Error! Play controller hasn't been created!";
        return ;
    }

    qDebug() << "[LOG] Fast backward... ";
}

void CSJMediaPlayerWindow::onSetImage() {
    if (!m_pVideoRenderWidget) {
        return ;
    }

    m_pVideoRenderWidget->showDefaultImage();
}
