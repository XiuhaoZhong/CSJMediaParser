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
    : QWidget(parent) {
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
    setFixedSize(QSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT));

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
    m_pDXWidget->setRenderType(RENDER_WITH_TIMER);
}
