#ifndef __CSJWIDGET_H__
#define __CSJWIDGET_H__

#include <QWidget>

class CSJWidget : public QWidget {
    Q_OBJECT
public:
    explicit CSJWidget(QWidget *parent = nullptr);

    void setContentWidget(QWidget *contentWidget);

protected:

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    //void paintEvent(QPaintEvent *event) override;

private:
    void createTitleBar();

private:
    QWidget *m_titleBar = nullptr;
    QPoint   m_dragPos;

    QWidget *m_centerWidget = nullptr;

};

#endif // __CSJMAINWINDOW_H__