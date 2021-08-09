#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include"mydouyin.h"
#include"qmytcpclient.h"
#include"Packdef.h"
#include"logindialog.h"
#include"myvideodialog.h"
#include"videoplayer.h"
#include"broadcast.h"
#include"screenrecorder.h"
#include"fanslistdlg.h"
#include"useritem.h"
#include<qmap>
#include<QThread>



class CKernel;
typedef void (CKernel::*PFUN)(char *buf,int nlen);

class UploadWorker:public QObject
{
    Q_OBJECT
public slots:
    void slot_UploadVideo(QString filePath, QString imgPath, Hobby hy);
};

static QString m_serverIP;

class CKernel : public QObject
{
    Q_OBJECT
private:
    explicit CKernel(QObject *parent = 0);
    ~CKernel(){}

public:
    static CKernel * m_Kernel;
public slots:
    static CKernel *GetInstance();
    void DestroyInstance();
    void setNetPackMap();
    void InitServerIP();
    void setQss();


    void slot_ReadyData(char *buf, int nlen);
    void slot_dealRegisterRs(char *buf, int nlen);
    void slot_dealLoginRs(char *buf, int len);
    void slot_fileBlockRq(char *buf, int len);
    void slot_downloadRs(char *buf, int len);
    void slot_uploadRs(char *buf, int len);
    void slot_dealAddFriendRs(char *buf, int len);
    void slot_dealFriendInfo(char *buf, int len);

    void slot_RegisterCommit(QString name, QString password, Hobby hy);
    void slot_LoginCommit(QString name, QString password);
    void UploadFile(QString filePath, Hobby hy, QString gifName = "");

    void slot_UploadVideo(QString filePath, QString imgPath, Hobby hy);
    void slot_PlayUrl(QString url);

    void slot_PlayVideoUrl();
    void slot_nextVideo();
    void slot_lastVideo();

    void slot_openMessage(); 
    void slot_openBroadCast();
    void slot_startBroadCast();
    void slot_stopBroadCast();
    void slot_openRecordVideo();

    void slot_changeVideoRs(char *buf, int len);

signals:
    void SIG_setMovie(int id , QString path , QString rtmp);
    void SIG_equipInit(QString,QString);
    void SIG_addItem(UserItem *);
    void SIG_PlayVideoUrl();

private:
    QMyTcpClient * m_tcpClient;
    QMyTcpClient * m_tcpRtsp;
    PFUN m_NetPackFunMap[DEF_PACK_COUNT];
    LoginDialog * m_login;
    MyDouYin * m_douyin;
    VideoPlayer * m_Player;
    broadcast * m_pBroadCast;
    myVideoDialog *m_myDialog;
    FansListDlg * m_fanlist;
    ScreenRecorder *m_screenRecorder;

    int m_iconID;
    int m_userID;
    QString m_UserName;
    QString m_feeling;


    QThread * m_uploadThread;
    UploadWorker * m_uploadWorker;

    QMap<int , FileInfo *> m_mapVideoIDToFileInfo;
    QMap<int , UserItem *> m_mapIDToUserItem;
public:
    QString m_savePath;
    QString m_Video;
    QString m_Audio;
    QStringList videoDevList;
    QStringList audioDevList;

    void initDev();
    void setVideoDevice(QString str);
    void setAudioDevice(QString str);

};

#endif // CKERNEL_H
