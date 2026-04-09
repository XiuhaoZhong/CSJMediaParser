#ifndef __CSJPLAYERCONTROLLERWIDGET_H__
#define __CSJPLAYERCONTROLLERWIDGET_H__

#include <QWidget>

class QPushButton;
class QSlider;

class CSJPlayerControllerWidget : public QWidget {
    Q_OBJECT
public:
    CSJPlayerControllerWidget(QWidget *parent = nullptr);

signals:
    void play();
    void pause();
    void resume();
    void stop();

    void showImage();

public slots:
    void onPlayBtnClicked();
    void onStopBtnClicked();
    void onImageBtnClicked();

private:
    QPushButton *m_pPlayBtn;
    QPushButton *m_pStopBtn;
    QPushButton *m_pImageBtn;
    QSlider     *m_pVolumeSlider;
    QSlider     *m_pProgressSlider;
};

#endif 