#ifndef __CSJPOPUPWIDGET_H__
#define __CSJPOPUPWIDGET_H__

class QWidget;

class CSJPopupWidget {
public:
    CSJPopupWidget();
    ~CSJPopupWidget();

    void show();

private:
    QWidget *m_pWidget;
};

#endif // __CSJPOPUPWIDGET_H__
