#ifndef __CSJDIALOG_H__
#define __CSJDIALOG_H__

#include <QDialog>

#include <QString>

typedef enum {
    CSJDIALOGTYPE_INFO = 0,
    CSJDIALOGTYPE_WARNNING,
} CSJDIALOGTYPE;

class QHBoxLayout;
class QPushBottun;
class CSJDialog : public QDialog {
    Q_OBJECT
public:
    CSJDialog(QString &content, QWidget *parent = nullptr, CSJDIALOGTYPE type = CSJDIALOGTYPE_INFO);
    ~CSJDialog();

    void open() override;
    void done(int) override;
    void accept() override;
    void reject() override;

protected:
    void initUI();
    QHBoxLayout *createBtnLayout();

private:
    QString m_content;

    QPushButton *m_pOkBtn       = nullptr;
    QPushButton *m_pCancelBtn   = nullptr;

    QHBoxLayout *m_pBtnLayout   = nullptr;
};

#endif // __CSJDIALOG_H__
