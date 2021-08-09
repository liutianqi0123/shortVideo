#include "ckernel.h"
#include"ui_mydouyin.h"
#include<QApplication>
#include<QMessageBox>
#include<QCryptographicHash>
#include<thread>
#include<QDebug>
#include<QDir>


#define MD5_KEY 1234

static QByteArray GetMD5(QString val)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString key = QString("%1_%2").arg(val).arg(MD5_KEY);
    hash.addData(key.toLocal8Bit());
    QByteArray bt = hash.result();

    return bt.toHex();  //ad1234F...32位 MD5
}

static CKernel * pThis = NULL;
CKernel *CKernel::m_Kernel = NULL;
CKernel::CKernel(QObject *parent) : QObject(parent)
  ,m_iconID(0),m_userID(0)
{
    qDebug()<<__func__;
    qDebug()<< "main:" <<QThread::currentThread();
    m_Kernel = this;
    pThis = this;

    qsrand( QTime(0,0,0).msecsTo( QTime::currentTime() ) );
    qRegisterMetaType<Hobby>("Hobby");

    std::string strIP = DEF_SERVER_IP;
    m_serverIP = QString::fromStdString( strIP );

    InitServerIP();
    setNetPackMap();
    setQss();
    initDev();

    connect(this,SIGNAL(SIG_PlayVideoUrl()),this,SLOT(slot_PlayVideoUrl()));
    m_douyin = new MyDouYin;

    connect(m_douyin ,SIGNAL(SIG_nextVideo()) , this, SLOT(slot_nextVideo()));
    connect(m_douyin ,SIGNAL(SIG_lastVideo()) , this, SLOT(slot_lastVideo()));
    connect(m_douyin ,SIGNAL(SIG_openMyMessage()) , this, SLOT(slot_openMessage()));
    connect(m_douyin ,SIGNAL(SIG_openBroadCast()) , this, SLOT(slot_openBroadCast()));
    connect(m_douyin ,SIGNAL(SIG_startBroadCast()) , this, SLOT(slot_startBroadCast()));
    connect(m_douyin ,SIGNAL(SIG_stopBroadCast()) , this, SLOT(slot_stopBroadCast()));
    connect(m_douyin ,SIGNAL(SIG_openRecordVideo()) , this, SLOT(slot_openRecordVideo()));

    m_tcpClient = new QMyTcpClient;
    connect( m_tcpClient , SIGNAL(SIG_ReadyData(char*,int)) ,
             this, SLOT(slot_ReadyData(char*,int)) );
    m_tcpClient->setIpAndPort( (char*) m_serverIP.toStdString().c_str() );

    m_login = new LoginDialog;
    connect(m_login , SIGNAL(SIG_loginCommit(QString,QString)) ,
            this , SLOT( slot_LoginCommit(QString,QString))  );
    connect(m_login , SIGNAL(SIG_registerCommit(QString,QString,Hobby)) ,
            this , SLOT( slot_RegisterCommit(QString,QString,Hobby))  );

    //m_login->show();
    m_douyin->show();


    m_myDialog = new myVideoDialog;
    m_uploadThread = new QThread;
    m_uploadWorker = new UploadWorker;

    connect(m_myDialog,SIGNAL(SIG_UploadFiles(QString,QString,Hobby)),m_uploadWorker,SLOT(slot_UploadVideo(QString,QString,Hobby)));
    m_uploadWorker->moveToThread(m_uploadThread);
    m_uploadThread->start();

    //connect(this,SIGNAL(SIG_updateProcess(qint64,qint64)),m_myDialog,SLOT(slot_updateProcess(qint64,qint64)));
    connect(this,SIGNAL(SIG_setMovie(int,QString,QString)),m_myDialog,SLOT(slot_setMovie(int,QString,QString)));
    connect(m_myDialog,SIGNAL(SIG_PlayUrl(QString)),this,SLOT(slot_PlayUrl(QString)));
    //connect(this,SIGNAL(SIG_send_UserName(QString,QString)),m_myDialog,SLOT(slot_send_UserName(QString,QString)));
    //connect(m_myDialog,SIGNAL(SIG_AddFriend(int,QString,QString)),this,SLOT(slot_AddFriend(int,QString,QString)));
    //connect(m_myDialog,SIGNAL(SIG_openAttentionList()),this,SLOT(slot_openAttentionList()));


    m_Player = new VideoPlayer;
    m_Player->slot_InitLocalFiles("C:/Users/77513/Desktop/音视频/素材");
    connect(m_Player,SIGNAL(SIG_GetOneImage(QImage)),m_douyin->ui->wdg_video,SLOT(slot_setImage(QImage)));


    m_pBroadCast = new broadcast;
    connect(this,SIGNAL(SIG_equipInit(QString,QString)),m_pBroadCast,SLOT(slot_equipInit(QString,QString)));

//    m_fanlist = new FansListDlg;
//    //connect(m_fanlist,SIGNAL(SIG_addFriend(QString)),this,SLOT(slot_AddFriend(QString)));
//    connect(this,SIGNAL(SIG_addItem(UserItem*)),m_fanlist,SLOT(slot_addItem(UserItem *)));

}
//单例
CKernel *CKernel::GetInstance()
{
    static CKernel kernel;
    return &kernel;
}


void CKernel::DestroyInstance()
{
    qDebug()<<__func__;
//    if( m_pVideoRead )
//    {
//        m_pVideoRead->slot_closeVideo();
//        delete m_pVideoRead ; m_pVideoRead = NULL;
//    }
    if( m_tcpClient )
    {
        delete m_tcpClient; m_tcpClient = NULL;
    }
    if( m_tcpRtsp)
    {
        delete m_tcpRtsp; m_tcpRtsp = NULL;
    }
    if(m_login )
    {
        delete m_login; m_login = NULL;
    }

    if(m_Player)
    {
        m_Player->m_videoState.quit = 1;
        m_Player->quit();
        if(m_Player->isRunning())
        {
            m_Player->terminate();
            m_Player->wait();
        }
        delete m_Player;
    }
    if(m_uploadWorker)
    {
        delete m_uploadWorker;m_uploadWorker = NULL;
    }

    if(m_uploadThread)
    {
        m_uploadThread->quit();
        m_uploadThread->wait(100);
        if(m_uploadThread->isRunning())
        {
            m_uploadThread->terminate();
            m_uploadThread->wait(100);
        }
        delete m_uploadThread;m_uploadThread = NULL;
    }

    if(m_pBroadCast)
    {
        delete m_pBroadCast;
    }

    if(m_myDialog)
    {
        delete m_myDialog;
        m_myDialog=NULL;
    }
//    for( auto ite = m_mapIDToChatdlg.begin() ;ite !=  m_mapIDToChatdlg.end() ;++ite )
//    {
//        ChatDialog * chat = *ite;
//        chat->close();
//        delete *ite;
//        *ite = NULL;
//    }
//    m_mapIDToChatdlg.clear();

//    if( m_pAudioRead )
//    {
//        m_pAudioRead->PauseAudio();
//        delete m_pAudioRead; m_pAudioRead = NULL;
//    }
//    for( auto ite = m_mapIDToAudioWrite.begin() ;ite !=  m_mapIDToAudioWrite.end() ;++ite )
//    {
//        delete *ite;
//        *ite = NULL;
//    }
//    m_mapIDToAudioWrite.clear();

//    if( m_weChatDlg )
//    {
//        m_weChatDlg->close();
//        delete m_weChatDlg; m_weChatDlg = NULL;
//    }
}

#include<QSettings>
#include<QCoreApplication>
#include<QFileInfo>

//加载配置文件 QSetting
void CKernel::InitServerIP()
{
    //获取路径 exe路径 拼接成配置文件路径
    QString path = QCoreApplication::applicationDirPath() + "/config.ini";
    QFileInfo info(path);

    if(info.exists())
    {//如果存在 读取配置 ip
        QSettings setting(path , QSettings::IniFormat , NULL);//相当于打开配置文件 存在读取不存在创建
        setting.beginGroup("net");
        QVariant var = setting.value("ip");
        QString ip = var.toString();
        if(!ip.isEmpty()) m_serverIP = ip;

        setting.endGroup();

        qDebug()<< m_serverIP;

    }else{
        //如果不存在创建 并写默认
        QSettings setting (path,QSettings::IniFormat , NULL);
        setting.beginGroup("net");
        setting.setValue("ip",m_serverIP);//将默认写入文件
        setting.endGroup();
    }
}

void CKernel::setQss()
{
    QString path = QString("%1/%2")
            .arg(QCoreApplication::applicationDirPath())
            .arg("douyin.css");
    qDebug()<<path;
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }

}

#define NetPackFunMap(a) m_NetPackFunMap[ a - DEF_PACK_BASE]
//协议映射表

void CKernel::setNetPackMap()
{
    memset(m_NetPackFunMap , 0 ,sizeof(PFUN)*DEF_PACK_COUNT);
    NetPackFunMap(DEF_PACK_LOGIN_RS       ) = &CKernel::slot_dealLoginRs;
    NetPackFunMap(DEF_PACK_REGISTER_RS    ) = &CKernel::slot_dealRegisterRs;
    NetPackFunMap(DEF_PACK_UPLOAD_RS      ) = &CKernel::slot_uploadRs;
    NetPackFunMap(DEF_PACK_DOWNLOAD_RS    ) = &CKernel::slot_downloadRs;
    NetPackFunMap(DEF_PACK_FILEBLOCK_RQ   ) = &CKernel::slot_fileBlockRq;
    NetPackFunMap(DEF_PACK_CHANGE_VIDEO_RS) = &CKernel::slot_changeVideoRs;
    NetPackFunMap(DEF_PACK_ADD_FRIEND_RS  ) = &CKernel::slot_dealAddFriendRs;
    NetPackFunMap(DEF_PACK_FRIEND_INFO    ) = &CKernel::slot_dealFriendInfo;
}

//网络包处理
void CKernel::slot_ReadyData(char *buf, int nlen)
{
    int type = *(int*) buf;
    if( type >= DEF_PACK_BASE && type < DEF_PACK_BASE + DEF_PACK_COUNT ) {
        PFUN p = m_NetPackFunMap[ type - DEF_PACK_BASE ];
        if( p != NULL )
        {
            (this->*p)( buf , nlen );
        }
    }
    delete[] buf;
}

//初始化播放
void CKernel::slot_PlayVideoUrl()
{
    STRU_CHANGE_VIDEO_RQ rq;
    rq.m_nUserId = m_userID;
    rq.ChangeType =0;
    m_tcpClient->SendData((char*)&rq , sizeof(rq));
}
//下一曲播放
void CKernel::slot_nextVideo()
{
    m_Player->stop(true);

//    if(m_Player->m_videoCount<=m_Player->m_videoCountMax)
//        m_Player->m_videoCount++;
//    else
//        m_Player->m_videoCount=0;

//    m_Player->setFileName(m_Player->m_videoList[m_Player->m_videoCount]);

    STRU_CHANGE_VIDEO_RQ rq;
    rq.m_nUserId = m_userID;
    rq.ChangeType =1;
    m_tcpClient->SendData((char*)&rq , sizeof(rq));
}

//上一曲播放
void CKernel::slot_lastVideo()
{
    m_Player->stop(true);

    STRU_CHANGE_VIDEO_RQ rq;
    rq.m_nUserId = m_userID;
    rq.ChangeType =-1;
    m_tcpClient->SendData((char*)&rq , sizeof(rq));

//    if(m_Player->m_videoCount<=m_Player->m_videoCountMax)
//        m_Player->m_videoCount--;
//    else
//        m_Player->m_videoCount=m_Player->m_videoCountMax;

//    m_Player->setFileName(m_Player->m_videoList[m_Player->m_videoCount]);
}
//打开我的资料
void CKernel::slot_openMessage()
{
    m_Player->stop(true);
    m_myDialog->show();

    STRU_DOWNLOAD_RQ rq;
    rq.m_nUserId = m_userID;

    m_tcpClient->SendData((char*)&rq,sizeof(rq));
}
//打开直播页面
void CKernel::slot_openBroadCast()
{
    // videotest   conf  videotest appName --> rtmp路径

    //直播  conf   tst1  --> rtmp   tst2  ... tst3  ...  10000
    // mysql  t_live    id --> appName    id=1   appName  tst1
//    id  appName
//    10    tst1
//    12    tst2
//    0    tst3    id 10  --> rtmp地址    rtmp://服务器ip/appName
    //ui->le_savePath->setText( "" );
    QString rtmpUrl = QString("rtp://%1:8554/live").arg(m_serverIP);
    qDebug()<<rtmpUrl<<endl;
    m_savePath = rtmpUrl;
    //ui->le_broadcast->setText( m_savePath );

//    //进行tcp连接
//    m_tcpRtsp = new QMyTcpClient;
//    //m_tcpRtsp->setIpAndPort((char*) m_serverIP.toStdString().c_str(),_DEF_RTSP_PORT);
//    if(m_tcpRtsp->InitNetWork((char*) m_serverIP.toStdString().c_str(),_DEF_RTSP_PORT))
//        {
//        qDebug()<< "TcpRtsp InitNet success ...\n";
//    }


}

//开始直播 进行推流
void CKernel::slot_startBroadCast()
{
    //设置 录制
    if(m_screenRecorder && m_screenRecorder->m_saveVideoFileThread &&m_screenRecorder->m_saveVideoFileThread->isRunning() )
    {
        QMessageBox::critical(m_douyin , "提示","正在处理录制视频,稍后再来");
        return;
    }
    QString audioDevName = audioDevList[0];
    QString CamDevName = videoDevList[0];
    if( m_savePath.remove(" ").isEmpty())
    {
        QMessageBox::critical(m_douyin,"提示"," 先设置保存文件的名字! ");
        return;
    }
    if (audioDevName.isEmpty()||CamDevName.isEmpty())
    {
        QMessageBox::critical(m_douyin,"提示","出错了,音频或视频设备未就绪，程序无法运行！");
        return;
    }
    if (m_screenRecorder){
        delete m_screenRecorder;  m_screenRecorder = NULL;
    }
    m_screenRecorder = new ScreenRecorder;

    connect(m_screenRecorder , SIGNAL(SIG_GetOneImage(QImage)) ,
            m_douyin->ui->wdg_broadcast , SLOT(slot_setImage(QImage)) );

    std::string str = m_savePath.toStdString();
    char* buf =(char*)str.c_str();
    m_screenRecorder->setFileName(buf);

    m_screenRecorder->setVideoFrameRate(DEFAULT_FRAME_RATE);//设置帧率 默认 15 , 可以开
    //放设置添加帧率
//    if (ui->cb_desk->isChecked()) //看是摄像头还是桌面
//    {
//        if (m_screenRecorder->init("desktop",true,false,audioDevName,true) == SUCCEED)
//        {
//            m_screenRecorder->startRecord();
//        }
//        else
//        {
//            QMessageBox::critical(this,"提示","出错了,初始化录屏设备失败！");
//            return;
//        }
//    }
//    else
//    {
        if (m_screenRecorder->init(CamDevName,false,true,audioDevName,true) == SUCCEED)
        {
            m_screenRecorder->startRecord();
        }
        else
        {
            QMessageBox::critical(m_douyin,"提示","出错了,初始化音频设备失败！");
            return;
        }
}
//停止直播
void CKernel::slot_stopBroadCast()
{
    //停止录像
    if( m_screenRecorder )
    m_screenRecorder->stopRecord();

    m_savePath="";
//    QImage img;
//    img.fill( Qt::black);

//    ui->wdg_broadcast ->slot_setImage( img );//结束录制, 发送黑色图片, 清空显示
}

//打开录制视频wedget
void CKernel::slot_openRecordVideo()
{
    m_Player->m_videoState.isPause = true;
    //this->hide();
    m_pBroadCast->show();

    Q_EMIT SIG_equipInit(m_Audio,m_Video);
}

static void FFMPEG_CallBack(void* ptr,int level,const char* fmt,va_list vl);

//初始化设备
void CKernel::initDev()
{
    if(!videoDevList.isEmpty())
        videoDevList.clear();
    if(!audioDevList.isEmpty())
        audioDevList.clear();

    av_register_all();
    avformat_network_init();
    avdevice_register_all();
    AVFormatContext * pFormatCtx = avformat_alloc_context();
    av_log_set_callback(&FFMPEG_CallBack);

    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
    iformat = av_find_input_format("vfwcap");
    avformat_open_input(&pFormatCtx,"list",iformat,NULL);
    //rct = qApp->desktop()->screenGeometry(); //获取用户的桌面大小, 用于录制屏幕的尺寸.
}
static bool isDShowVideoBegin = false;
static bool isDShowAudioBegin = false;
static bool isVFWVideoBegin = false;
static void FFMPEG_CallBack(void* ptr,int level,const char* fmt,va_list vl)
{
    //可以根据 level 的级别选择要打印的内容
    if(level <= AV_LOG_INFO)
    {
        char buffer[1024];
        vsprintf(buffer,fmt,vl);
        std::string strbuf = buffer;
        QString str = QString::fromStdString( strbuf );

        if(str.contains("DirectShow video devices"))
        {
            isDShowVideoBegin = true;
            isDShowAudioBegin = false;
            return;
        }
        if(str.contains("DirectShow audio devices"))
        {
            isDShowVideoBegin = false;
            isDShowAudioBegin = true;
            return;
        }
        if(str.contains("Driver 0"))
        {
            isDShowVideoBegin = false;
            isDShowAudioBegin = false;
            isVFWVideoBegin = true;
            return;
        }

        int index = str.indexOf("\"");
        str = str.remove(0,index);
        str = str.remove("\"");
        str = str.remove("\n");
        str = str.remove("\r");
        if( str.left(1) == " ")
            str = str.mid(1);
        if(str.isEmpty() || str.contains("%s"))
            return;
        if(isDShowVideoBegin)
        {
            if( "screen-capture-recorder" != str &&
                    !str.contains("dev") && !str.contains("Virtual"))
            {
                pThis->setVideoDevice(str);
            }
        }else if(isDShowAudioBegin)
        {
            if( "virtual-audio-capturer" != str &&
                    !str.contains("dev"))
            {
                pThis->setAudioDevice(str);
            }
        }else if(isVFWVideoBegin)
        {
            if(pThis->videoDevList.size() == 0 && str.contains("Microsoft WDM Image Capture"))
            {
                pThis->setVideoDevice(str);
                return;
            }
        }

    }

}

void CKernel::setVideoDevice(QString str)
{
    qDebug()<<"视频设备名:"<<str;
    videoDevList.append(str);
    m_Video = videoDevList[videoDevList.size()-1];
}

void CKernel::setAudioDevice(QString str)
{
    qDebug()<<"音频设备名:"<<str;
    audioDevList.append(str);
    m_Audio = audioDevList[audioDevList.size()-1];
}



//播放我的视频
//播放url
void CKernel::slot_PlayUrl(QString url)
{
    m_Player->stop( true ); //如果播放 你要先关闭

    //m_playType = FromOnline;

    m_Player ->playUrl( url );  //url
    //ui->lb_videoName->setText( "" );

    //slot_PlayerStateChanged(PlayerState::Playing);
}
////打开关注粉丝列表
//void CKernel::slot_openAttentionList()
//{
//    return NULL;
//}
//处理服务器登录回复
void CKernel::slot_dealLoginRs(char* buf,int len)
{
    STRU_LOGIN_RS *rs = (STRU_LOGIN_RS *)buf;
    switch(rs->m_lResult)
    {
    case userid_no_exist:
        QMessageBox::information(this->m_login,"提示","用户不存在,登录失败");
        break;
    case login_sucess:
        this->m_login->hide();
        m_douyin->show();
        m_userID  = rs->m_UserID;
       //m_Player->setFileName(m_Player->m_videoList[m_Player->m_videoCount]);
       //m_Player->start();
        emit SIG_PlayVideoUrl();

        break;
    case password_error:
        QMessageBox::information(this->m_login,"提示","密码错误,登录失败");
        break;
    default:break;
    }

}

//注册回复
void CKernel::slot_dealRegisterRs(char *buf, int nlen)
{
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS *)buf;
    switch( rs->m_lResult )
    {
    case userid_is_exist:
        QMessageBox::about( this->m_login , "提示" , "注册失败, 用户已存在");
        break;
    case register_sucess:
        QMessageBox::about( this->m_login , "提示" , "注册成功");
        break;
    default:break;
    }
}


//提交登录
void CKernel::slot_LoginCommit(QString name, QString password)
{
    STRU_LOGIN_RQ rq;
    std::string strTmp = name.toStdString();
    char * buf = (char *)strTmp.c_str();
    strcpy_s(rq.m_szUser,buf);


    QByteArray bt = GetMD5(password);
    memcpy(rq.m_szPassword,bt.data(),bt.size());

    if(m_tcpClient->SendData((char *)&rq,sizeof(rq)) < 0)
    {
        QMessageBox::about(m_douyin,"提示","网络故障");
    }

}

//提交注册
void CKernel::slot_RegisterCommit(QString name, QString password, Hobby hy)
{
    STRU_REGISTER_RQ rq;
    std::string strTmp = name.toStdString();
    char * buf = (char *)strTmp.c_str();
    strcpy_s(rq.m_szUser,buf);


    QByteArray bt = GetMD5(password);
    memcpy(rq.m_szPassword,bt.data(),bt.size());

    rq.dance   = hy.dance   ;
    rq.edu     = hy.edu     ;
    rq.ennegy  = hy.ennegy  ;
    rq.food    = hy.food    ;
    rq.funny   = hy.funny   ;
    rq.music   = hy.music   ;
    rq.outside = hy.outside ;
    rq.video   = hy.video   ;

    if(m_tcpClient->SendData((char *)&rq,sizeof(rq)) < 0)
    {
        QMessageBox::about(m_douyin,"提示","网络故障");
    }

}

//上传视频相应
void CKernel::slot_UploadVideo(QString filePath, QString imgPath, Hobby hy)
{
    //上传
    qDebug() << "上传" ;
    UploadFile(imgPath,hy);
    UploadFile(filePath,hy,imgPath);

}
#include<QFileInfo>
//上传文件
void CKernel::UploadFile(QString filePath, Hobby hy, QString gifName)
{
    QFileInfo info(filePath);
    //兼容中文
    QString FileName = info.fileName();
    std::string strName = FileName.toStdString();
    const char* filename  = strName.c_str();


    STRU_UPLOAD_RQ rq;
    rq.m_nFileId = qrand()%10000;
    qDebug() << rq.m_nFileId;
    rq.m_nFileSize = info.size();
    strcpy_s(rq.m_szFileName,MAX_PATH,filename);

    QByteArray bt = filePath.right(filePath.length() - filePath.lastIndexOf('.') -1).toLatin1();
    memcpy(rq.m_szFileType,bt.data(),bt.length());

    if(!gifName.isEmpty())
    {
        QFileInfo info(gifName);
        strcpy_s(rq.m_szGifName,MAX_PATH , info.fileName().toLocal8Bit().data());
    }

    memcpy(rq.m_szHobby,&hy,sizeof(hy));

    rq.m_UserId = m_userID;

    m_tcpClient->SendData((char*)&rq,sizeof(rq));

    //发送文件块

    FileInfo fi;
    fi.fileId =rq.m_nFileId;
    fi.fileName = rq.m_szFileName;
    fi.filePath = filePath;
    fi.filePos = 0;
    fi.fileSize = rq.m_nFileSize;
    fi.pFile = new QFile(filePath);

    if(fi.pFile->open(QIODevice::ReadOnly))
    {
        while(1)
        {
            STRU_FILEBLOCK_RQ blockrq;
            int64_t res = fi.pFile->read(blockrq.m_szFileContent,MAX_CONTENT_LEN);
            fi.filePos+=res;
            blockrq.m_nBlockLen = res;
            blockrq.m_nFileId = rq.m_nFileId;
            blockrq.m_nUserId = m_userID;

            m_tcpClient->SendData((char *)&blockrq,sizeof(blockrq));

            //emit SIG_updateProcess(fi.filePos,fi.fileSize);

            if(fi.filePos >= fi.fileSize)
            {
                fi.pFile->close();
                delete fi.pFile;
                fi.pFile = NULL;
                break;
            }

        }

    }
}


//工作者上传文件
void UploadWorker::slot_UploadVideo(QString filePath, QString imgPath, Hobby hy)
{
    qDebug()<< "worker:" <<QThread::currentThread();
    CKernel::m_Kernel->slot_UploadVideo(filePath,imgPath,hy);
}


//上传文件回复
void CKernel::slot_uploadRs(char *buf, int len)
{
    STRU_UPLOAD_RS *rs = (STRU_UPLOAD_RS*) buf;
    switch(rs->m_nResult)
    {
    case 1:
        QMessageBox::about(m_login,"提示","上传成功");
        break;
    }
}

//下载回复
void CKernel::slot_downloadRs(char *buf, int len)
{
    STRU_DOWNLOAD_RS *rs = (STRU_DOWNLOAD_RS*) buf;

    //文件头

    //给FileInfo 赋值

    FileInfo * info = new FileInfo;
    //videoid 作为文件的标识  fileid用来区分不同控件
    info->videoId = rs->m_nVideoId;
    info->fileId = rs->m_nFileId;
    info->fileName = rs->m_szFileName;

    QDir dir;
    qDebug()<<QDir::currentPath()+"/temp"<<endl;

    if(!dir.exists(QDir::currentPath()+"/temp/"))
    {
        dir.mkpath(QDir::currentPath()+"/temp/");
    }

    info->filePath = QString("./temp/%1").arg(rs->m_szFileName);
    info->filePos =0;
    info->fileSize = rs->m_nFileSize;
    info->rtmpUrl = QString("rtmp://%1:1935/vod%2").arg(m_serverIP).arg(rs->m_rtmp);//    //1/104.mp4
    qDebug()<<"rtmp--" << info->rtmpUrl;
    //rtmp://192.168.0.10/vod//1/104.mp4
    info->pFile = new QFile(info->filePath);

    if(info->pFile->open(QIODevice::WriteOnly))
    {
        m_mapVideoIDToFileInfo[info->videoId] = info;
    }
    else
    {
        delete info;
    }

}

//处理下载文件块请求
void CKernel::slot_fileBlockRq(char *buf, int len)
{
    STRU_FILEBLOCK_RQ *rq = (STRU_FILEBLOCK_RQ *)buf;
    auto ite = m_mapVideoIDToFileInfo.find(rq->m_nFileId);
    if(ite == m_mapVideoIDToFileInfo.end())return;

    FileInfo * info = m_mapVideoIDToFileInfo[rq->m_nFileId];

    int64_t res = info->pFile->write(rq->m_szFileContent,rq->m_nBlockLen);
    info->filePos+=res;

    if(info->filePos >= info->fileSize)
    {
        //关闭文件
        info->pFile->close();
        //删除该节点
        m_mapVideoIDToFileInfo.erase(ite);
        //设置控件
        Q_EMIT SIG_setMovie(info->fileId,info->filePath,info->rtmpUrl);
//        //info->fileId -->控件号码
//        QString pbNum = QString("wdg_video%1").arg(info->fileId);
//        QMyMovieLabel * pb_play =  ui->wdg_video->findChild<QMyMovieLabel*>(pbNum);

//        QMovie *LastMovie = pb_play->movie();
//        if(LastMovie && LastMovie->isValid())
//        {
//            delete LastMovie;
//        }
//        QMovie * movie = new QMovie(info->filePath);
//        pb_play->setMovie(movie);

//        pb_play->setRtmpUrl(info->rtmpUrl);
        //回收info
        delete info;
        info = NULL;
    }
}

//处理切换视频回复
void CKernel::slot_changeVideoRs(char* buf, int len)
{
    STRU_CHANGE_VIDEO_RS *rs = (STRU_CHANGE_VIDEO_RS*) buf;

    QString rtmpUrl = QString("rtmp://%1:1935/vod%2").arg(m_serverIP).arg(rs->m_rtmp);//    //1/104.mp4
    qDebug()<<"rtmp--" << rtmpUrl;

    m_Player ->playUrl( rtmpUrl );

}

//处理关注好友回复
void CKernel::slot_dealAddFriendRs(char* buf, int len)
{
    STRU_FRIEND_INFO * info = (STRU_FRIEND_INFO*)buf;
    //查找map
    if(m_mapIDToUserItem.find(info->m_userID) != m_mapIDToUserItem.end())
    {//有 更新
        UserItem * item = m_mapIDToUserItem[info->m_userID];
        item->setInfo(info->m_userID,info->m_szName,info->m_state/*,QString(":/tx/%1.png").arg(info->m_iconID) ,info->m_feeling*/);


    }
    else
    {//没有 创建并且添加到控件
        UserItem * item = new UserItem;
        item->setInfo(info->m_userID,info->m_szName,info->m_state/*,QString(":/tx/%1.png").arg(info->m_iconID) ,info->m_feeling*/);
        //connect(item,SIGNAL(SIG_UserItemClicked()),this,SLOT(slot_UserItemCliecked()));
        m_mapIDToUserItem[info->m_userID] = item;

        //m_userList->addItem(item);
        emit SIG_addItem(item);
    }
}

//处理粉丝好友回复
void CKernel::slot_dealFriendInfo(char* buf, int len)
{
    STRU_FANS_INFO *info = (STRU_FANS_INFO*)buf;
}
