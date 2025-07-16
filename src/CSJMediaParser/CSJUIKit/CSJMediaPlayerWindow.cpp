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

CSJMediaPlayerWindow::CSJMediaPlayerWindow(QWidget *parent)
    : QWidget(parent)
    , m_playStatus(PLAYSTATUS_STOP) {
    initUI();

    setAttribute(Qt::WA_DeleteOnClose, true);

    // Shows the center widget at the center of the screen.
    QPoint screenCenter = QApplication::screens().constFirst()->availableGeometry().center();
    QRect curRect = this->geometry();
    setGeometry(QRect(screenCenter.rx() - curRect.width() / 2,
                                       screenCenter.ry() - curRect.height() / 2,
                                       curRect.width(),
                                       curRect.height()));

}

CSJMediaPlayerWindow::~CSJMediaPlayerWindow() {
    qDebug() << "CSJMediaPlayerWindow destoryed!";
}

void CSJMediaPlayerWindow::initUI() {
    setFixedSize(QSize(PLAYERWINDOW_WIDTH, PLAYERWINDOW_HEIGHT));

    QVBoxLayout *mianLayout = new QVBoxLayout(this);

    m_pVideoThumbnailWiget = new QWidget();
    mianLayout->addWidget(m_pVideoThumbnailWiget);

    QPushButton *imageButton = new QPushButton(m_pVideoThumbnailWiget);
    imageButton->setText("Show Image");

    QVBoxLayout *playerLayout = new QVBoxLayout();
    playerLayout->setSpacing(0);
    mianLayout->addLayout(playerLayout);

    mianLayout->setStretchFactor(m_pVideoThumbnailWiget, 1);
    mianLayout->setStretchFactor(playerLayout, 6);

    m_pDXWidget = new CSJVideoRendererWidget(this);
    playerLayout->addWidget(m_pDXWidget);

    connect(imageButton, SIGNAL(pressed()), m_pDXWidget, SLOT(showDefaultImage()));

    QWidget *progressWidget = new QWidget();
    playerLayout->addWidget(progressWidget);
    progressWidget->setFixedHeight(20);
    progressWidget->setStyleSheet(QString("background-color:#C77A7A"));

    m_pMediaControlWidget = new QWidget();
    playerLayout->addWidget(m_pMediaControlWidget);
    m_pMediaControlWidget->setFixedHeight(60);
    m_pMediaControlWidget->setStyleSheet(QString("background-color:#C7ABAB"));
    initControllWidget();
    initPlayController();

    m_pAudioWaveWidget = new QWidget();
    playerLayout->addWidget(m_pAudioWaveWidget);
    m_pAudioWaveWidget->setFixedHeight(100);
    m_pAudioWaveWidget->setStyleSheet(QString("background-color:#C4C4C4"));
}

void CSJMediaPlayerWindow::show(bool bShow) {
    // if (bShow == m_pCenterWidget->isVisible()) {
    //     return ;
    // }

    // m_pCenterWidget->setVisible(bShow);
    // if (!bShow) {
    //     // TODO: stop player if the player is playing.
    // }

    setVisible(bShow);
    m_pDXWidget->setRenderType(ACTIVE_RENDERING);
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
