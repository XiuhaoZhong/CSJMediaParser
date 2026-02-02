#ifndef __CSJMAINWINDOW_H__
#define __CSJMAINWINDOW_H__

#include <QMainWindow>

class QLineEdit;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;
class CSJVideoRendererWidget;

class CSJMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CSJMainWindow(QWidget *parent = nullptr);

protected:
    void createTitleBar();

    void initWidgets();

    void assembleLayout();

    void fillContentWidget(QWidget *contentWidget);

    void initHeaderLayout(QVBoxLayout *leftLayout);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void quitApp();

protected slots:
    void onOpenMenuClicked();
    void onCloseMenuClicked();
    void onSettingsMenuClicked();
    void onAboutMenuClicked();
    void onVerActionClicked();

    void onSelectMediaFile();
    void onOpenMediaFile();

    void onQuitClicked();

private:
    QPoint       m_dragPos;

    QWidget     *m_titleBar;
    QWidget     *m_pCentralContainer = nullptr;
    QWidget     *m_pBaseOptWiget = nullptr;
    QWidget     *m_pMediaInfoWidget = nullptr;
    QPushButton *m_pSelectFileBtn = nullptr;
    QLineEdit   *m_pSourceInputEdit = nullptr;
    QPushButton *m_pOpenFileBtn = nullptr;

    QString      m_selFilePath;

};

#endif // __CSJMAINWINDOW_H__