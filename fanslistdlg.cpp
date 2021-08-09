#include "fanslistdlg.h"
#include "ui_fanslistdlg.h"

#include<QInputDialog>
#include<QRegExp>

FansListDlg::FansListDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FansListDlg)
{
    ui->setupUi(this);
    //connect(this,SIGNAL(SIG_addFriend(QString)),this,SLOT(slot_AddFriendRq(QString)));

    ui->le_search ->installEventFilter(this);
    ui->stk_page->setCurrentIndex(0);

    m_userList = new  IMToolItem("我的关注");
    ui->page1->addItem(m_userList);
}

FansListDlg::~FansListDlg()
{
    delete ui;
}

//关注列表
void FansListDlg::on_tb_attention_clicked()
{
    ui->tb_attention->setChecked(true);
    ui->tb_fans->setChecked(false);
    ui->stk_page->setCurrentIndex(0);
}

//粉丝列表
void FansListDlg::on_tb_fans_clicked()
{
    ui->tb_attention->setChecked(false);
    ui->tb_fans->setChecked(true);
    ui->stk_page->setCurrentIndex(1);
}

//事件过滤器
bool FansListDlg::eventFilter(QObject *watch, QEvent *event)
{
    if( watch == ui->le_search && event->type() == QEvent::MouseButtonPress )
    {
        ui->le_search->setText("");
        QLabel * label = (QLabel*) watch;
    }
    return QObject::eventFilter(watch, event);
}

#include<QMessageBox>
//搜索添加好友
void FansListDlg::on_pb_search_clicked()
{
    if(ui->le_search->text() == "搜索用户备注或名字")
    {
        QMessageBox::about(this , "提示" , "输入用户备注或名字");
        return;
    }

    QString name = ui->le_search->text();
    QRegExp reg("[A-Z0-9a-z]{1,10}");
    if(reg.exactMatch(name) )
        Q_EMIT SIG_addFriend(name);
    else
        QMessageBox::about(this, "提示","用户名非法");
}

void FansListDlg::slot_addItem(UserItem *item)
{
    m_userList->addItem(item);
}

void FansListDlg::on_tb_add_clicked()
{

}
