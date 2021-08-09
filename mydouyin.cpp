#include "mydouyin.h"
#include "ui_mydouyin.h"
#include "qDebug"
#include<QDir>
#include<QFileDialog>
#include<QMessageBox>

MyDouYin::MyDouYin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyDouYin)
{
    ui->setupUi(this);

    ui->stk_page->setCurrentIndex(2);
}

MyDouYin::~MyDouYin()
{
    delete ui;
}
//void MyDouYin::on_pushButton_2_clicked()
//{
//    filePath = "";
//    //打开文件 弹出对话框 参数:父窗口, 标题, 默认路径, 筛选器
//    QString path = QFileDialog::getOpenFileName(this,"选择要播放的文件夹" , "F:/",
//            "视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);; 所有文件(*.*);;");
//    if(!path.isEmpty())
//    {
//        qDebug()<< path ;
//        QFileInfo info(path);
//        if( info.exists() )
//        {
//            filePath = path;
//            qDebug() << filePath;
//        }
//        else
//        {
//            QMessageBox::information( this, "提示" , "打开文件失败");
//        }
//    }
//}


//点击同城
void MyDouYin::on_tb_town_clicked()
{
    ui->tb_town->setChecked(true);
    ui->tb_recommend->setChecked(false);
    ui->tb_attention->setChecked(false);

    ui->stk_page->setCurrentIndex(0);
}

//点击关注
void MyDouYin::on_tb_attention_clicked()
{
    ui->tb_town->setChecked(false);
    ui->tb_recommend->setChecked(false);
    ui->tb_attention->setChecked(true);

    ui->stk_page->setCurrentIndex(1);
}

//点击推荐
void MyDouYin::on_tb_recommend_clicked()
{
    ui->tb_town->setChecked(false);
    ui->tb_recommend->setChecked(true);
    ui->tb_attention->setChecked(false);

    ui->stk_page->setCurrentIndex(2);
}

//关闭主控件
void MyDouYin::on_pb_close_clicked()
{
    this->close();
}

//最小化
void MyDouYin::on_pb_min_clicked()
{
    this->showMinimized();
}

//下一个视频
void MyDouYin::on_pb_search_clicked()
{
    emit SIG_nextVideo();
}
//上一个视频
void MyDouYin::on_pb_search_2_clicked()
{
    emit SIG_lastVideo();
}
//open broadcast
void MyDouYin::on_pushButton_clicked()
{
    emit SIG_openRecordVideo();
}

//打开我的资料
void MyDouYin::on_tb_me_clicked()
{
    emit SIG_openMyMessage();
}
//打开直播
void MyDouYin::on_tb_broadcast_clicked()
{
    ui->stk_page->setCurrentIndex(3);
    emit SIG_openBroadCast();
}

//直播开始
void MyDouYin::on_tb_start_clicked()
{
    emit SIG_startBroadCast();
}

//直播停止
void MyDouYin::on_tb_end_clicked()
{
    emit SIG_stopBroadCast();
}
