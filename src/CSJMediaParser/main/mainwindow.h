#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLineEdit;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;
class CSJVideoRendererWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

    void initMenuBar();
    void initFileMenu(QMenu *menu);
    void initSettingsMenu(QMenu *menu);
    void initAboutMenu(QMenu *menu);

    void initUI();

    void initLeftLayout(QVBoxLayout *leftLayout);
    void initRightLayout(QVBoxLayout *rightLayout);

    void stringTest();

protected slots:
    void onOpenMenuClicked();
    void onCloseMenuClicked();
    void onSettingsMenuClicked();
    void onAboutMenuClicked();
    void onVerActionClicked();

    void onSelectMediaFile();
    void onOpenMediaFile();

private:
    QWidget     *m_pCentrelWidget = nullptr;
    QWidget     *m_pBaseOptWiget = nullptr;
    QWidget     *m_pMediaInfoWidget = nullptr;
    QPushButton *m_pSelectFileBtn = nullptr;
    QLineEdit   *m_pSourceInputEdit = nullptr;
    QPushButton *m_pOpenFileBtn = nullptr;

    QMenuBar    *m_pMenu = nullptr;

    QString      m_selFilePath;
};
#endif // MAINWINDOW_H
