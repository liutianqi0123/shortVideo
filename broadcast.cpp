#include "broadcast.h"
#include "ui_broadcast.h"
#include<qdesktopwidget.h>
#include"videoplayer.h"
#include<QFileDialog>
#include<QMessageBox>


static broadcast * pThis = NULL;
broadcast::broadcast(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::broadcast)
{
    ui->setupUi(this);

    this->setWindowTitle( "录制&上传" );

    pThis = this;

    m_screenRecorder = NULL;
}

broadcast::~broadcast()
{
    delete ui;
}

Ui::broadcast *broadcast::getUi() const
{
    return ui;
}

//获取设备名

void broadcast::slot_equipInit(QString audioName,QString videoName)
{
    m_Video = videoName;
    m_Audio = audioName ;
}

//关闭视频
void broadcast::on_tb_close_clicked()
{
    //停止录像
    if( m_screenRecorder )
    m_screenRecorder->stopRecord();

    m_savePath="";
    QImage img;
    img.fill( Qt::black);

    ui->wdg_broadcast->slot_setImage( img );//结束录制, 发送黑色图片, 清空显示

}

//路径设置
void broadcast::on_tb_people_clicked()
{
        QString tmpPath = QTime::currentTime().toString("hhmmsszzz");
        tmpPath += ".flv";
        tmpPath += "F:/"+ tmpPath;
        QString path = QFileDialog::getSaveFileName( this ,"保存",tmpPath ,"视频文件(*.flv);;");
        if( path.remove(" ").isEmpty() )
        {
            QMessageBox::about(this ,"提示","重新设置");
            return;
        }
        //ui->le_broadcast->setText( "" );
        m_savePath = path.replace("/" , "\\\\");

        //ui->le_savePath->setText( m_savePath );
}
//打开视频
void broadcast::on_tb_open_clicked()
{
        //设置 录制
        if(m_screenRecorder && m_screenRecorder->m_saveVideoFileThread &&
                m_screenRecorder->m_saveVideoFileThread->isRunning() )
        {
            QMessageBox::critical(this , "提示","正在处理录制视频,稍后再来");
            return;
        }
        QString audioDevName = m_Audio;
        QString CamDevName =   m_Video;
        if( m_savePath.remove(" ").isEmpty())
        {
            QMessageBox::critical(this,"提示"," 先设置保存文件的名字! ");
            return;
        }
        if (audioDevName.isEmpty()||CamDevName.isEmpty())
        {
            QMessageBox::critical(this,"提示","出错了,音频或视频设备未就绪，程序无法运行！");
            return;
        }
        if (m_screenRecorder){
            delete m_screenRecorder;  m_screenRecorder = NULL;
        }
        m_screenRecorder = new ScreenRecorder;

        connect(m_screenRecorder , SIGNAL(SIG_GetOneImage(QImage)) ,
                ui->wdg_broadcast, SLOT(slot_setImage(QImage)) );

        std::string str = m_savePath.toStdString();
        char* buf =(char*)str.c_str();
        m_screenRecorder->setFileName(buf);

        m_screenRecorder->setVideoFrameRate(DEFAULT_FRAME_RATE);//设置帧率 默认 15 , 可以开
        //放设置添加帧率
//        if (ui->cb_desk->isChecked()) //看是摄像头还是桌面
//        {
//            int res = m_screenRecorder->init("desktop",true,false,audioDevName,true);
//            if ( res == SUCCEED )
//            {
//                m_screenRecorder->startRecord();
//            }
//            else
//            {
//                if( res == VideoOpenFailed )
//                    QMessageBox::critical(this,"提示","出错了,初始化录屏设备失败！");
//                else
//                    QMessageBox::critical(this,"提示","出错了,初始化音频设备失败！");
//                return;
//            }
//        }
//        else
//        {
            int res = m_screenRecorder->init(CamDevName,false,true,audioDevName,true);
            if ( res == SUCCEED)
            {
                m_screenRecorder->startRecord();
            }
            else
            {
                if( res == VideoOpenFailed )
                    QMessageBox::critical(this,"提示","出错了,初始化视频设备失败！");
                else
                    QMessageBox::critical(this,"提示","出错了,初始化音频设备失败！");
                return;
            }
// }
}

void broadcast::on_pb_close_clicked()
{
    this->close();
}
