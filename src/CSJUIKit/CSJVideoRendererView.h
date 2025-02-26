#ifndef __CSJVIDEORENDERERVIEW_H__
#define __CSJVIDEORENDERERVIEW_H__

#include <QWidget>

#include "renderClient/CSJVideoRenderer.h"

class CSJVideoRendererView : public QWidget {
    Q_OBJECT
public:
    CSJVideoRendererView(QWidget *parent);
    ~CSJVideoRendererView();

    QPaintEngine* paintEngine() const override {
        return NULL;
    }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    CSJSpVideoRenderer m_spVideoRenderer = nullptr;

};

#endif // __CSJVIDEORENDERERVIEW_H__
