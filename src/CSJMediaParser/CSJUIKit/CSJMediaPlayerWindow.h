#ifndef __CSJMEDIAPLAYERWINDOW_H__
#define __CSJMEDIAPLAYERWINDOW_H__

#define PLAYERWINDOW_WIDTH 1280
#define PLAYERWINDOW_HEIGHT 960

#include <QWidget>

#include "Controllers/CSJPlayerController.h"

class QPushButton;
class CSJVideoRendererWidget;

typedef enum {
    PLAYSTATUS_STOP = 0,
    PLAYSTATUS_PAUSE,
    PLAYSTATUS_PLAY,
} PlayStatus;

class CSJMediaPlayerWindow : public QWidget {
    Q_OBJECT
public:
    CSJMediaPlayerWindow(QWidget *parent = nullptr);
    ~CSJMediaPlayerWindow();

    void initUI();

    void show(bool bShow);

public slots:
    void onPlayBtnClicked();
    void onStopBtnClicked();
    void onFastForwardBtnClicked();
    void onFastBackBtnClicked();

protected:
    void initControllWidget();

    void initPlayController();

private:
    QWidget *m_pVideoThumbnailWiget = nullptr;
    CSJVideoRendererWidget *m_pDXWidget = nullptr;
    QWidget *m_pProgressWidget = nullptr;
    QWidget *m_pMediaControlWidget = nullptr;
    QWidget *m_pAudioWaveWidget = nullptr;

    QPushButton *m_pPlayBtn;
    QPushButton *m_pStopBtn;
    QPushButton *m_pFastForwardBtn;
    QPushButton *m_pFastBackBtn;

    CSJUniqPlayerController m_playController;
    PlayStatus              m_playStatus;
};

#endif
