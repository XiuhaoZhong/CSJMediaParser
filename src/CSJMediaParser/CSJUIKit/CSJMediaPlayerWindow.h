#ifndef __CSJMEDIAPLAYERWINDOW_H__
#define __CSJMEDIAPLAYERWINDOW_H__

#include <QWidget>

#include "Controllers/CSJPlayerController.h"

class CSJWidget;
class QPushButton;
class CSJVideoRendererWidget;
class CSJPlayerControllerWidget;
class CSJPlayerController;

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

    // before the widget closed.
    void closeEvent(QCloseEvent *event) override;

    void initUI();

    void show(bool bShow);

signals:
    void aFrameRendered();

public slots:
    void onPlayBtnClicked();
    void onPauseBtnClicked();
    void onResumeBtnClicked();
    void onStopBtnClicked();
    void onFastForwardBtnClicked();
    void onFastBackBtnClicked();

    // slots for rendering states
    void onAFrameRendered();

    void onSetImage();

protected:
    void onWidgetClose();

    void setupDelegate();

private:
    CSJVideoRendererWidget       *m_pVideoRenderWidget = nullptr;
    CSJPlayerControllerSharedPtr  m_playController;              /* Player function controller. */
    CSJPlayerControllerWidget    *m_playerCtrlWidget = nullptr;  /* Player controller UI. */
    PlayStatus                    m_playStatus;

    int                           m_iRenderCount = 0;

};

#endif
