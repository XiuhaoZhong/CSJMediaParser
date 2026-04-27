#ifndef __CSJMEDIADETAILMODULE_H__
#define __CSJMEDIADETAILMODULE_H__

#include <QObject>
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class CSJAccordionWidget;
class CSJMediaDetailModule : public QObject {
    Q_OBJECT
public:
    CSJMediaDetailModule();
    ~CSJMediaDetailModule();

    void initWidthParentWidget(QWidget *parent);

    QWidget* getWidget() {
        return m_pWidget;
    }

protected:
    void initControlUI(QHBoxLayout *layout);
    void initStreamUI(QVBoxLayout *layout);
    void initDetailDataUI(QHBoxLayout *layout);

private:
    QWidget *m_pWidget;

    CSJAccordionWidget *m_pTrackInfoWidget;
    CSJAccordionWidget *m_pPacketInfoWidget;
    CSJAccordionWidget *m_pFrameInfoWidget;

};

#endif // __CSJMEDIADETAILMODULE_H__
