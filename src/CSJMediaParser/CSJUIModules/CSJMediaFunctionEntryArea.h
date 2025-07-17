#ifndef __CSJMEDIAFUNCTIONENTRYAREA_H__
#define __CSJMEDIAFUNCTIONENTRYAREA_H__

class QWidget;

/*
 * @brief CSJMediaFunctionEntryArea
 * @details Show the funtions entries.
*/
class CSJMediaFunctionEntryArea {
public:
    CSJMediaFunctionEntryArea();
    ~CSJMediaFunctionEntryArea();

    QWidget *getAreaWidget();

private:
    QWidget *m_pAreaWidget;
};

#endif // __CSJMEDIAFUNCTIONENTRYAREA_H__