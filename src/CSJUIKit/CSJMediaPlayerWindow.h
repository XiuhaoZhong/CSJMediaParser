#ifndef __CSJMEDIAPLAYERWINDOW_H__
#define __CSJMEDIAPLAYERWINDOW_H__

#define MAINWINDOW_WIDTH 1280
#define MAINWINDOW_HEIGHT 960

#include <QWidget>

class CSJVideoRendererWidget;

class CSJMediaPlayerWindow : public QWidget {
    Q_OBJECT
public:
    CSJMediaPlayerWindow(QWidget *parent = nullptr);
    ~CSJMediaPlayerWindow();

    void initUI();

    void show(bool bShow);

private:
    QWidget *m_pVideoThumbnailWiget = nullptr;
    CSJVideoRendererWidget *m_pDXWidget = nullptr;
    QWidget *m_pProgressWidget = nullptr;
    QWidget *m_pMediaControlWidget = nullptr;
    QWidget *m_pAudioWaveWidget = nullptr;

};

#endif
