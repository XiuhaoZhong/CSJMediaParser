#include "CSJMediaDetailModule.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>

#include "CSJUIKit/CSJAccordionWidget.h"
#include "CSJMediaSPFDataController.h"

CSJMediaDetailModule::CSJMediaDetailModule() {

}

CSJMediaDetailModule::~CSJMediaDetailModule() {

}

void CSJMediaDetailModule::initControlUI(QHBoxLayout *layout) {
    if (!layout) {
        return ;
    }

    QButtonGroup *buttonGroup = new QButtonGroup(m_pWidget);
    //layout->addWidget(buttonGroup);
    QRadioButton *gopReadBtn = new QRadioButton("读取一个GOP");
    QRadioButton *numReadBtn = new QRadioButton("读取指定帧数");

    layout->addWidget(gopReadBtn);
    layout->addWidget(numReadBtn);

    buttonGroup->addButton(gopReadBtn, 0);
    buttonGroup->addButton(numReadBtn, 1);

    layout->addStretch();
    QPushButton *readBtn = new QPushButton(m_pWidget);
    readBtn->setText("读取数据");
    layout->addWidget(readBtn);
}

void CSJMediaDetailModule::initStreamUI(QVBoxLayout *layout) {
    QGroupBox *streamBox = new QGroupBox();
    streamBox->setTitle("Stream Infomations");

    CSJAccordionWidget *trackInfoWidget = new CSJAccordionWidget(streamBox);
    QVBoxLayout *streamBoxLayout = new QVBoxLayout(streamBox);
    streamBoxLayout->addWidget(trackInfoWidget);
    layout->addWidget(streamBox);
    m_pTrackInfoWidget = trackInfoWidget;
}

void CSJMediaDetailModule::initDetailDataUI(QHBoxLayout *layout) {
    QVBoxLayout *pktInfoLayout = new QVBoxLayout();
    QVBoxLayout *frmInfoLayout = new QVBoxLayout();

    layout->addLayout(pktInfoLayout, 1);
    layout->addLayout(frmInfoLayout, 1);

    QGroupBox *pktBox = new QGroupBox();
    pktBox->setTitle("Packet infomations");
    QVBoxLayout *ptkBoxLayout = new QVBoxLayout(pktBox);

    CSJAccordionWidget *pktInfoWidget = new CSJAccordionWidget(pktBox);
    pktInfoLayout->addWidget(pktBox);
    ptkBoxLayout->addWidget(pktInfoWidget);
    pktInfoWidget->showTestPage();
    m_pPacketInfoWidget = pktInfoWidget;

    QGroupBox *frmBox = new QGroupBox();
    frmBox->setTitle("Frame informations");
    QVBoxLayout *frmBoxLayout = new QVBoxLayout(frmBox);

    CSJAccordionWidget *frmInfoWidget = new CSJAccordionWidget(frmBox);
    frmBoxLayout->addWidget(frmInfoWidget);
    frmInfoLayout->addWidget(frmBox);
    frmInfoWidget->showTestPage();
    m_pFrameInfoWidget = frmInfoWidget;
}

void CSJMediaDetailModule::initWidthParentWidget(QWidget *parent) {
    m_pWidget = new QWidget(parent);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_pWidget);

    QVBoxLayout *trackLayout = new QVBoxLayout();
    QHBoxLayout *controllLayout = new QHBoxLayout();
    QHBoxLayout *detailLayout = new QHBoxLayout();

    mainLayout->addLayout(trackLayout, 3);
    mainLayout->addLayout(controllLayout, 1);
    mainLayout->addLayout(detailLayout, 8);

    initStreamUI(trackLayout);
    initControlUI(controllLayout);
    initDetailDataUI(detailLayout);

    CSJMediaSPFDataController::getInstance()->setMediaDataWidget(m_pTrackInfoWidget,
                                                                 m_pPacketInfoWidget,
                                                                 m_pFrameInfoWidget);
}
