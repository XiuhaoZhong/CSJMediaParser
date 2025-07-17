#ifndef __CSJMEDIASPACEINFOAREA_H__
#define __CSJMEDIASPACEINFOAREA_H__

class QWidget;

/*
 * @brief CSJMediaSpaceInfoArea
 * @details Show the CSJMediaParser information, including the project icon and some
 *          basic information.
 */
class CSJMediaSpaceInfoArea {
public:
    CSJMediaSpaceInfoArea();
    ~CSJMediaSpaceInfoArea();

    QWidget *getAreaWidget();

private:
    QWidget *m_pAreaWidget;
};

#endif // __CSJMEDIASPACEINFOAREA_H__