#ifndef FANSLISTDLG_H
#define FANSLISTDLG_H

#include <QDialog>
#include"IMToolBox.h"
#include"useritem.h"
//#include<customwidget.h>

namespace Ui {
class FansListDlg;
}

class FansListDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FansListDlg(QWidget *parent = 0);
    ~FansListDlg();

public slots:
    void slot_addItem(UserItem *item);
signals:
    void SIG_addFriend(QString);

protected:
    bool eventFilter(QObject *watch, QEvent *event);
private slots:

    void on_tb_attention_clicked();

    void on_tb_fans_clicked();

    //void on_lb_search_linkActivated(const QString &link);

    void on_pb_search_clicked();

    //void slot_AddFriendRq(QString name);

    void on_tb_add_clicked();

private:
    Ui::FansListDlg *ui;
    IMToolItem * m_userList;
};

#endif // FANSLISTDLG_H
