#ifndef __CSJMEDIAPLAYERWINDOW_H__
#define __CSJMEDIAPLAYERWINDOW_H__

#include <QWidget>

#include "Controllers/CSJPlayerController.h"

class CSJWidget;
class QPushButton;
class CSJVideoRendererWidget;
class CSJPlayerControllerWidget;

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

    void onSetImage();

private:
    CSJVideoRendererWidget    *m_pVideoRenderWidget = nullptr;
    CSJUniqPlayerController    m_playController;              /* Player function controller. */
    CSJPlayerControllerWidget *m_playerCtrlWidget = nullptr;  /* Player controller UI. */
    PlayStatus                 m_playStatus;
};

#endif
