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

    // QPushButton *imageButton = new QPushButton(m_pVideoThumbnailWiget);
    // imageButton->setText("Show Image");
    // connect(imageButton, &QPushButton::pressed, m_pDXWidget, &CSJVideoRendererWidget::showDefaultImage);

    QVBoxLayout *playerLayout = new QVBoxLayout();
    playerLayout->setSpacing(0);
    mainLayout->addLayout(playerLayout, 18);
    mainLayout->setSpacing(10);

    m_pVideoRenderWidget = new CSJVideoRendererWidget();
    playerLayout->addWidget(m_pVideoRenderWidget);
    //m_pVideoRenderWidget->show();

    CSJPlayerControllerWidget *ctrlWidget = new CSJPlayerControllerWidget(this);
    playerLayout->addWidget(ctrlWidget, 1);

    setWindowTitle("CSJMediaPlayer");
    setStyleSheet("QWidget {background-color: #1A202C;}");

    //connect(ctrlWidget, &CSJPlayerControllerWidget::play, m_pDXWidget, &CSJVideoRendererWidget::showDefaultImage);

    // QVBoxLayout *controllerLayout = new QVBoxLayout();
    // mainLayout->addLayout(controllerLayout, 1);
    // setLayout(mainLayout);
    // mainLayout->activate();

    // QWidget *progressWidget = new QWidget();
    // controllerLayout->addWidget(progressWidget);
    // progressWidget->setFixedHeight(20);
    // progressWidget->setStyleSheet(QString("background-color:#C77A7A"));

    // m_pMediaControlWidget = new QWidget();
    // controllerLayout->addWidget(m_pMediaControlWidget);
    // m_pMediaControlWidget->setFixedHeight(60);
    // m_pMediaControlWidget->setStyleSheet(QString("background-color:#C7ABAB"));
    // initControllWidget();
    // initPlayController();
}

void CSJMediaPlayerWindow::show(bool bShow) {
    // if (bShow == m_pCenterWidget->isVisible()) {
    //     return ;
    // }

    // m_pCenterWidget->setVisible(bShow);
    // if (!bShow) {
    //     // TODO: stop player if the player is playing.
    // }

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
        m_playStatus = PLAYSTATUS_PLAY;

        m_pPlayBtn->setText("Pause");
        m_pFastForwardBtn->setEnabled(true);
        m_pFastBackBtn->setEnabled(true);
    } else if (m_playStatus == PLAYSTATUS_PAUSE) {
        qDebug() << "[LOG] Resume playing... ";

        m_playController->resume();
        m_playStatus = PLAYSTATUS_PLAY;

        m_pPlayBtn->setText("Pause");
        m_pFastForwardBtn->setEnabled(false);
        m_pFastBackBtn->setEnabled(false);
    } else if (m_playStatus == PLAYSTATUS_PLAY) {
        qDebug() << "[LOG] Pause playing... ";

        m_playController->pause();
        m_playStatus = PLAYSTATUS_PAUSE;

        m_pPlayBtn->setText("Resume");
        m_pFastForwardBtn->setEnabled(true);
        m_pFastBackBtn->setEnabled(true);
    }

    m_pStopBtn->setEnabled(true);
}

void CSJMediaPlayerWindow::onStopBtnClicked() {
    if (!m_playController) {
        qDebug() << "Error! Play controller hasn't been created!";
        return ;
    }

    qDebug() << "[LOG] Stop playing... ";

    m_playController->stop();
    m_playStatus = PLAYSTATUS_STOP;

    m_pPlayBtn->setText("Play");
    m_pStopBtn->setEnabled(false);
    m_pFastForwardBtn->setEnabled(false);
    m_pFastBackBtn->setEnabled(false);
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

void CSJMediaPlayerWindow::initControllWidget() {
    m_pPlayBtn = new QPushButton("Play"); // play, pause and resume is the same button, switch by play status.
    m_pStopBtn = new QPushButton("Stop", m_pMediaControlWidget);
    m_pStopBtn->setEnabled(false);
    m_pFastForwardBtn = new QPushButton ("FF");
    m_pFastForwardBtn->setEnabled(false);
    m_pFastBackBtn =  new QPushButton ("FB");
    m_pFastBackBtn->setEnabled(false);

    QHBoxLayout *layout = new QHBoxLayout(m_pMediaControlWidget);
    layout->addWidget(m_pPlayBtn);
    layout->addWidget(m_pStopBtn);
    layout->addWidget(m_pFastForwardBtn);
    layout->addWidget(m_pFastBackBtn);

    connect(m_pPlayBtn, &QPushButton::clicked, this, &CSJMediaPlayerWindow::onPlayBtnClicked);
    connect(m_pStopBtn, &QPushButton::clicked, this, &CSJMediaPlayerWindow::onStopBtnClicked);
    connect(m_pFastForwardBtn, &QPushButton::clicked, this, &CSJMediaPlayerWindow::onFastForwardBtnClicked);
    connect(m_pFastBackBtn, &QPushButton::clicked, this, &CSJMediaPlayerWindow::onFastBackBtnClicked);

    m_pPlayBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50; /* 正常状态背景色 */
            color: white;
            border-radius: 5px;
            padding: 8px;
        }
        QPushButton:disabled {
            background-color: #cccccc; /* 禁用状态背景色 */
            color: #666666;
        }
        QPushButton:pressed {
            background-color: #45a049; /* 按下状态背景色 */
        }
    )");

    m_pStopBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50; /* 正常状态背景色 */
            color: white;
            border-radius: 5px;
            padding: 8px;
        }
        QPushButton:disabled {
            background-color: #cccccc; /* 禁用状态背景色 */
            color: #666666;
        }
        QPushButton:pressed {
            background-color: #45a049; /* 按下状态背景色 */
        }
    )");

    m_pFastBackBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50; /* 正常状态背景色 */
            color: white;
            border-radius: 5px;
            padding: 8px;
        }
        QPushButton:disabled {
            background-color: #cccccc; /* 禁用状态背景色 */
            color: #666666;
        }
        QPushButton:pressed {
            background-color: #45a049; /* 按下状态背景色 */
        }
    )");

    m_pFastForwardBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50; /* 正常状态背景色 */
            color: white;
            border-radius: 5px;
            padding: 8px;
        }
        QPushButton:disabled {
            background-color: #cccccc; /* 禁用状态背景色 */
            color: #666666;
        }
        QPushButton:pressed {
            background-color: #45a049; /* 按下状态背景色 */
        }
    )");
}

void CSJMediaPlayerWindow::initPlayController() {
    if (m_playController) {
        return ;
    }

    m_playController = CSJPlayerController::createPlayerController();
}
