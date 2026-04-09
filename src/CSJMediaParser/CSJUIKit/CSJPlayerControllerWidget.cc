#include "CSJPlayerControllerWidget.h"

#include "CSJUtils/CSJPathTool.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>

using namespace csjutils;

CSJPlayerControllerWidget::CSJPlayerControllerWidget(QWidget *parent) 
    : QWidget(parent) {
    
    setFixedHeight(40);
    setStyleSheet(R"(
        QWidget {
            background-color: #1E293B;
            border-radius: 8px;
        }
        QPushButton {
            background-color: #2563EB;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 4px 12px;
        }
        QPushButton:hover {
            background-color: #3B82F6;
        }
        QSlider::groove:horizontal {
            height: 4px;
            background-color: #334155;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            width: 12px;
            height: 12px;
            background-color: #2563EB;
            border-radius: 6px;
            margin: -4px 0;
        }
    )");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(15);

    QPushButton *playBtn = new QPushButton("Play");
    QPushButton *stopBtn = new QPushButton("Stop");
    QPushButton *imageBtn = new QPushButton("Image Test");

    QSlider *progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);

    QSlider *volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setFixedWidth(80);
    volumeSlider->setValue(80);

    layout->addWidget(playBtn);
    layout->addWidget(stopBtn);
    layout->addWidget(imageBtn);
    layout->addWidget(progressSlider, 1);
    layout->addWidget(volumeSlider);

    m_pPlayBtn = playBtn;
    m_pStopBtn = stopBtn;
    m_pImageBtn = imageBtn;
    m_pProgressSlider = progressSlider;
    m_pVolumeSlider = volumeSlider;

    m_pStopBtn->setEnabled(false);

    connect(m_pPlayBtn, &QPushButton::clicked, this, &CSJPlayerControllerWidget::onPlayBtnClicked);
    connect(m_pStopBtn, &QPushButton::clicked, this, &CSJPlayerControllerWidget::onStopBtnClicked);
    connect(m_pImageBtn, &QPushButton::clicked, this, &CSJPlayerControllerWidget::onImageBtnClicked);
}

void CSJPlayerControllerWidget::onPlayBtnClicked() {
    if (m_pPlayBtn->text() == "Play") {
        m_pPlayBtn->setText("Resume");
        m_pStopBtn->setEnabled(true);
        std::string imageName("cross_street.jpg");
        std::string imagePath = CSJPathTool::getInstance()->getImageWithName(imageName);
        emit play();
    } else if (m_pPlayBtn->text() == "Pause") {
        m_pPlayBtn->setText("Play");
        emit pause();
    } else {

    }
}

void CSJPlayerControllerWidget::onStopBtnClicked() {
    m_pStopBtn->setEnabled(false);
    m_pPlayBtn->setText("Play");
    emit stop();
}

void CSJPlayerControllerWidget::onImageBtnClicked() {
    emit showImage();
}