#ifndef __CSJGLASSWIDGET_H__
#define __CSJGLASSWIDGET_H__

#include <QWidget>

class QGraphicsBlurEffect;

class CSJGlassWidget : public QWidget {
    Q_OBJECT
public:
    CSJGlassWidget(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QWidget             *m_bgWidget   = nullptr;
    QGraphicsBlurEffect *m_blurEffect = nullptr;

};

#endif // __CSJGLASSWIDGET_H__