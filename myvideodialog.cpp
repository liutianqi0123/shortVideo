#include "myvideodialog.h"
#include "ui_myvideodialog.h"
#include"videoplayer.h"
#include<QFileDialog>
#include<QMovie>
#include<QFileInfo>
#include<QDir>
#include<QProcess>
#include<QTime>
#include<QDebug>

myVideoDialog::myVideoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::myVideoDialog)
{
    ui->setupUi(this);

    qRegisterMetaType<Hobby>("Hobby");

    ui->wdg_video1 ->installEventFilter(this);
    ui->wdg_video2 ->installEventFilter(this);
    ui->wdg_video3 ->installEventFilter(this);
    ui->wdg_video4 ->installEventFilter(this);
    ui->wdg_video5 ->installEventFilter(this);
    ui->wdg_video6 ->installEventFilter(this);
    ui->wdg_video7 ->installEventFilter(this);
    ui->wdg_video8 ->installEventFilter(this);
    ui->wdg_video9 ->installEventFilter(this);
    ui->wdg_video10->installEventFilter(this);
}

myVideoDialog::~myVideoDialog()
{
    delete ui;
}
//清空
void myVideoDialog::clear()
{
    ui->le_path->setText("");
    ui->pb_upload->setEnabled(false);
    ui->pgb_upload->setValue( 0 );
}




//点击浏览
void myVideoDialog::on_pb_brower_clicked()
{
    QString path = QFileDialog::getOpenFileName( this, "打开", "./","( *.flv *.mp4)");

    if(path.isEmpty()) return;

    ui->le_path->setText(path);

    //保证有temp文件夹
    QDir dir;
    if(!dir.exists(QDir::currentPath()+"/temp/"))
        dir.mkpath(QDir::currentPath()+"/temp/");

    //显示gif
    QString imgPath = SaveVideoJif( path );

    //加载到控件
    //QMovie * movie = new QMovie(imgpath);

    m_filePath = path;
    m_imgPath  =imgPath;

    ui->pb_upload->setEnabled(true);

}

//更新进度条
void myVideoDialog::slot_updateProcess(qint64 cur, qint64 max)
{
    ui->pgb_upload->setMaximum( max);
    ui->pgb_upload->setValue( cur );
}
//设置控件
#include<QMovie>
#include<QLabel>
void myVideoDialog::slot_setMovie(int id , QString path , QString rtmp)
{
    //load to widget

//  qDebug()<< path;
    QMovie * movie = new QMovie(path);
    movie->setScaledSize( QSize(190,150 ) );
    QString pushbuttonName = QString("wdg_video%1").arg(id);

//    QMyMovieLabel* pb_play =  ui->page_2->findChild<QMyMovieLabel*>( pushbuttonName);
    QLabel* pb_play =  ui->wdg_video->findChild<QLabel*>( pushbuttonName);
    //先清理再加载
    QMovie *lastMovie = pb_play->movie();

    pb_play->setMovie(movie);
    movie->start();
    movie->stop();
//    qDebug()<< rtmp;
//    pb_play->setRtmpUrl( rtmp );

    m_mapButtonNameToRtmpUrl[pushbuttonName] =  rtmp;
    if( lastMovie&&lastMovie->isValid() ){
        delete lastMovie;
    }

}

//上传视频

void myVideoDialog::on_pb_upload_clicked()
{
    this->clear();
    Hobby hy;
    hy.dance    =  1;
    hy.edu      =  1;
    hy.ennegy   =  1;
    hy.food     =  1;
    hy.funny    =  1;
    hy.music    =  1;
    hy.outside  =  1;
    hy.video    =  1;
    emit SIG_UploadFiles(m_filePath,m_imgPath,hy);
}

QString myVideoDialog::SaveVideoJif( QString FilePath )
{

    ////////// 删除上次生成的图片 //////////////////
    QProcess process(0);

    process.start("cmd"  );//开启cmd窗口
    process.waitForStarted();

    QString strcmd = QString("%1/temp/").arg(QDir::currentPath());
            strcmd.replace("/","\\\\");

    QDir cmdDir;

    if( cmdDir.exists(strcmd ) )
    {

        strcmd +=  QString(" \n");

        strcmd = QString("cd /d ")+ strcmd ;

        process.write( strcmd.toLocal8Bit() );

        strcmd = QString("del  /Q  *.jpg \n");

        process.write( strcmd.toLocal8Bit() );
    }

    process.closeWriteChannel();
    process.waitForFinished();

    qDebug()<< QString::fromLocal8Bit( process.readAllStandardOutput() );
    process.close();

    ////////////////////////////
    std::string tmp = FilePath.toStdString();
    char* file_path = (char*)tmp.c_str();

    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    uint8_t *out_buffer;
    static struct SwsContext *img_convert_ctx;
    int videoStream, i, numBytes;
    int ret, got_picture;
    av_register_all();
    //初始化 FFMPEG 调用了这个才能正常适用编码器和解码器
    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0) {
        qDebug() << "can't open the file." ;
        return QString();
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        qDebug() <<"Could't find stream infomation.";
        return QString();
    }
    videoStream = -1;
    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 保存到 videoStream 变量中///这里现在只处理视频流 音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
    }
    ///如果 videoStream 为-1 说明没有找到视频流
    if (videoStream == -1) {
        qDebug() <<"Didn't find a video stream.";
        return QString();
    }
    ///查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        qDebug() <<"Codec not found." ;
        return QString();
    }
    ///打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        qDebug() <<"Could not open codec." ;
        return QString();
    }
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,
                                  pCodecCtx->width,pCodecCtx->height);
    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);
    int y_size = pCodecCtx->width * pCodecCtx->height;
    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet
    av_new_packet(packet, y_size); //分配 packet 的数据
    int index = 0;
    int count = 0;
    while(1)
    {
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            break; //这里认为视频读取完了
        }
        if (packet->stream_index == videoStream) {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
            if (ret < 0) {
                qDebug() <<"decode error." ;
                return QString();
            }
            if (got_picture && pFrame->key_frame == 1 && pFrame->pict_type ==
                    AV_PICTURE_TYPE_I) {
                sws_scale(img_convert_ctx,
                          (uint8_t const * const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                          pFrameRGB->linesize);
                QImage tmpImg((uchar
                               *)out_buffer,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
                QImage image = tmpImg.scaled(640,320,Qt::KeepAspectRatio);

                image.save(QString("./temp/%1.jpg").arg(count,2,10,QChar('0')));
                        //2 占两位 10 十进制 QChar('0') 缺省补'0';
                        count++;
                if(count == 6)
                {
                    av_free_packet(packet);
                    break;
                }
            }
        }
        av_free_packet(packet);
    }
    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);


    QProcess p(0);
    p.start("cmd" );//开启 cmd 窗口
    p.waitForStarted();
    QString curPath = QDir::currentPath();
    curPath.replace("/","\\\\");
    QString strcd = QString("%1/ \n").arg(QCoreApplication::applicationDirPath());
    strcd.replace("/","\\\\");
    strcd = QString("cd /d ")+strcd;
    p.write( strcd.toLocal8Bit() );
    QString imgName = QString("%1\\\\temp\\\\%2.gif")
            .arg(curPath).arg(QTime::currentTime().toString("hhmmsszzz")); //使用时间自定义文件名,避免进程占用,写入文件失败
    QString cmd = QString("ffmpeg -r 2 -i %1\\\\temp\\\\").arg(curPath);
    cmd += "%02d.jpg ";
    cmd += imgName;
    cmd += " \n";
    qDebug() << "cmd "<< cmd;
    p.write( cmd.toLocal8Bit() );
    p.closeWriteChannel();
    p.waitForFinished();
    qDebug()<< QString::fromLocal8Bit( p.readAllStandardOutput() );
    p.close();

    return imgName;
}
//事件过滤器
bool myVideoDialog::eventFilter(QObject *watch, QEvent *event)
{
    if( watch == ui->wdg_video1 || watch == ui->wdg_video2 ||
        watch == ui->wdg_video3 || watch == ui->wdg_video4 ||
        watch == ui->wdg_video5 || watch == ui->wdg_video6 ||
        watch == ui->wdg_video7 || watch == ui->wdg_video8 ||
        watch == ui->wdg_video9 || watch == ui->wdg_video10  )
    {
        QLabel * label = (QLabel*) watch;
        if( event->type() == QEvent::Enter )
        {

            if( label->movie() )
                label->movie()->start();
        }else if( event->type() == QEvent::Leave)
        {

            if( label->movie() )
                label->movie()->stop();
        }else if( event->type() == QEvent::MouseButtonPress )
        {
            QString name = label->objectName();
            Q_EMIT SIG_PlayUrl( m_mapButtonNameToRtmpUrl[name] );
            //Q_EMIT SIG_PlayRecord( );
        }
    }
    return QObject::eventFilter(watch, event);
}


void myVideoDialog::slot_send_UserName(QString name , QString userId)
{
//    m_userName = name;
//    m_userId = userId;
}

//添加关注
void myVideoDialog::on_pb_getFriend_clicked()
{
//    //异常
//    //1,不可以添加自己
//    QString name = ui->label;
//    if(name == m_userName);
//    {
//        QMessageBox::about(this,"提示", "不可以添加自己");
//        return;
//    }


//    std::string tmp = name.toStdString();
//    const char * buf = tmp.c_str();


//    emit SIG_AddFriend(m_userId,m_userName,buf);

}

void myVideoDialog::on_pb_attention_clicked()
{
    emit SIG_openAttentionList();
}
