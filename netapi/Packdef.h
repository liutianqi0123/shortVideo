
#ifndef __DEF_PACKDEF__
#define __DEF_PACKDEF__

#include<QHostAddress>
#include<QByteArray>
#include<QDebug>
#include<QFile>
#include<QDateTime>
#include <winsock2.h>
#define BOOL bool
#define DEF_PACK_BASE  (10000)

typedef enum Net_PACK
{
    DEF_PACK_REGISTER_RQ = 10000,
    DEF_PACK_REGISTER_RS,

    DEF_PACK_LOGIN_RQ,
    DEF_PACK_LOGIN_RS,

    DEF_PACK_UPLOAD_RQ,
    DEF_PACK_UPLOAD_RS,
    DEF_PACK_FILEBLOCK_RQ,

    DEF_PACK_DOWNLOAD_RQ,
    DEF_PACK_DOWNLOAD_RS,
    DEF_PACK_CHANGE_VIDEO_RQ,
    DEF_PACK_CHANGE_VIDEO_RS,

    DEF_PACK_ADD_FRIEND_RQ,
    DEF_PACK_ADD_FRIEND_RS,
    DEF_PACK_FRIEND_INFO,
    DEF_PACK_FANS_INFO,
    DEF_PACK_FORCE_OFFLINE,

}Net_PACK;

//注册请求结果
#define userid_is_exist      0
#define register_sucess      1

//登录请求结果
#define userid_no_exist      0
#define password_error       1
#define login_sucess         2
#define user_online          3

//创建房间结果
#define room_is_exist        0
#define create_success       1

//加入房间结果
#define room_no_exist        0
#define join_success         1

//上传请求结果
#define file_is_exist        0
#define file_uploaded        1
#define file_uploadrq_sucess 2
#define file_upload_refuse   3

//上传回复结果
#define fileblock_failed     0
#define fileblock_success    1

//下载请求结果
#define file_downrq_failed   0
#define file_downrq_success  1

//添加好友结果
#define no_this_user    0
#define user_refused    1
#define user_is_offline 2
#define add_success     3




#define _downloadfileblock_fail  0
#define _downloadfileblock_success	1

#define DEF_PACK_COUNT (100)

#define MAX_PATH            (280 )
#define MAX_SIZE            (60  )
#define DEF_HOBBY_COUNT     (8  )
#define MAX_CONTENT_LEN     (4096 )
#define _DEF_PORT (8002)
#define _DEF_RTSP_PORT (8554)


/////////////////////网络//////////////////////////////////////


#define DEF_MAX_BUF	  1024
#define DEF_BUFF_SIZE	  4096
#define MAX_VIDEO_COUNT    200

typedef int PackType;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//登录请求
typedef struct STRU_LOGIN_RQ
{
    STRU_LOGIN_RQ()
    {
        m_nType = DEF_PACK_LOGIN_RQ;
        memset(m_szUser,0,MAX_SIZE);
        memset(m_szPassword,0,MAX_SIZE);
    }

    PackType m_nType;   //包类型
    char     m_szUser[MAX_SIZE] ; //用户ID
    char     m_szPassword[MAX_SIZE];  //密码
}STRU_LOGIN_RQ;


//登录回复
typedef struct STRU_LOGIN_RS
{
    STRU_LOGIN_RS()
    {
        m_nType= DEF_PACK_LOGIN_RS;
    }
    PackType m_nType;   //包类型
    int  m_UserID;
    int  m_lResult ; //注册结果

}STRU_LOGIN_RS;


//注册请求
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ()
    {
        m_nType = DEF_PACK_REGISTER_RQ;
        memset(m_szUser,0,MAX_SIZE);
        memset(m_szPassword,0,MAX_SIZE);
        dance   = 0;
        edu     = 0;
        ennegy  = 0;
        food    = 0;
        funny   = 0;
        music   = 0;
        outside = 0;
        video   = 0;
    }

    PackType m_nType;   //包类型
    char     m_szUser[MAX_SIZE] ; //用户名
    char     m_szPassword[MAX_SIZE];  //密码
    int dance   ;
    int edu     ;
    int ennegy  ;
    int food    ;
    int funny   ;
    int music   ;
    int outside ;
    int video   ;

}STRU_REGISTER_RQ;

//注册回复
typedef struct STRU_REGISTER_RS
{
    STRU_REGISTER_RS()
    {
        m_nType= DEF_PACK_REGISTER_RS;
    }
    PackType m_nType;   //包类型
    int  m_lResult ; //注册结果


}STRU_REGISTER_RS;

//上传文件请求
typedef struct STRU_UPLOAD_RQ
{
    STRU_UPLOAD_RQ()
    {
        m_nType = DEF_PACK_UPLOAD_RQ;
        m_UserId = 0;
        m_nFileId = 0;
        m_nFileSize =0;

        memset(m_szHobby , 0 ,DEF_HOBBY_COUNT);
        memset(m_szFileName , 0 ,MAX_PATH);
        memset(m_szGifName, 0 ,MAX_SIZE);
        memset(m_szFileType , 0 ,MAX_SIZE);
    }
    PackType m_nType; //包类型
    int m_UserId; //用于查数据库, 获取用户名字, 拼接路径
    int m_nFileId; //区分不同文件, 可采用 md5 或 随机数 用户同时只能传一个所以相同概率较低
    int64_t m_nFileSize; //文件大小, 用于文件传输结束
    char m_szHobby[DEF_HOBBY_COUNT];//喜好标签
    char m_szFileName[MAX_PATH]; //文件名, 用于存储文件
    char m_szGifName[MAX_PATH]; //动画名字, 方便直接写入数据库
    char m_szFileType[MAX_SIZE]; //用于区分视频和图片
}STRU_UPLOAD_RQ;

//上传文件请求回复
typedef struct STRU_UPLOAD_RS
{
    STRU_UPLOAD_RS()
    {
        m_nType = DEF_PACK_UPLOAD_RS;
        m_nResult = 0;
    }
    PackType m_nType; //包类型
    int m_nResult;
}STRU_UPLOAD_RS;

//文件块请求
typedef struct STRU_FILEBLOCK_RQ
{
    STRU_FILEBLOCK_RQ()
    {
        m_nType = DEF_PACK_FILEBLOCK_RQ;
        m_nUserId = 0;
        m_nFileId =0;
        m_nBlockLen =0;
        ZeroMemory(m_szFileContent,MAX_CONTENT_LEN);
    }
    PackType m_nType; //包类型
    int m_nUserId; //用户 ID
    int m_nFileId; //文件 id 用于区分文件
    int m_nBlockLen; //文件写入大小
    char m_szFileContent[MAX_CONTENT_LEN];
}STRU_FILEBLOCK_RQ;

//文件信息
struct FileInfo
{
    int fileId;
    int videoId;
    int64_t filePos;
    int64_t fileSize;
    QString filePath;
    QString rtmpUrl;
    QString fileName;
    QFile *pFile;
};

//下载文件请求
typedef struct STRU_DOWNLOAD_RQ
{
    STRU_DOWNLOAD_RQ()
    {
        m_nType = DEF_PACK_DOWNLOAD_RQ;
        m_nUserId = 0;
    }
    PackType m_nType; //包类型
    int m_nUserId; //用户 ID
}STRU_DOWNLOAD_RQ;

//下载文件回复
typedef struct STRU_DOWNLOAD_RS
{
    STRU_DOWNLOAD_RS()
    {
        m_nType = DEF_PACK_DOWNLOAD_RS;
        m_nFileId = 0;
        memset(m_szFileName , 0 ,MAX_PATH);
        memset(m_rtmp , 0 ,MAX_PATH);
    }
    PackType m_nType; //包类型
    int m_nFileId;
    int64_t m_nFileSize;
    int m_nVideoId;
    char m_szFileName[MAX_PATH];
    char m_rtmp[MAX_PATH]; // 播放地址 如//1/103.MP3 用户本地需要转化为 rtmp://服务器 ip/app 名/ + 这个字符串 //本项目 app 名为 vod
}STRU_DOWNLOAD_RS;


//推荐视频切换请求
typedef struct STRU_CHANGE_VIDEO_RQ
{
    STRU_CHANGE_VIDEO_RQ()
    {
        m_nType = DEF_PACK_CHANGE_VIDEO_RQ;
        m_nUserId = 0;
        ChangeType =0;
    }
    PackType m_nType; //包类型
    int m_nUserId; //用户 ID
    //评分状态的更换
    //播放情况 初始为0上一曲-1下一曲1
    int ChangeType;

}STRU_CHANGE_VIDEO_RQ;

//推荐视频切换回复
typedef struct STRU_CHANGE_VIDEO_RS
{
    STRU_CHANGE_VIDEO_RS()
    {
        m_nType = DEF_PACK_CHANGE_VIDEO_RS;
        m_nUserId = 0;
        memset(m_rtmp , 0 ,MAX_PATH);
    }
    PackType m_nType; //包类型
    int m_nUserId; //用户 ID

    char m_rtmp[MAX_PATH]; // 播放地址 如//1/103.MP3 用户本地需要转化为 rtmp://服务器 ip/app 名/ + 这个字符串 //本项目 app 名为 vod
}STRU_CHANGE_VIDEO_RS;

//添加好友请求
typedef struct STRU_ADD_FRIEND_RQ
{
    STRU_ADD_FRIEND_RQ()
    {
        m_nType = DEF_PACK_ADD_FRIEND_RQ;
        m_userID = 0;
        memset(m_szUserName,0,MAX_SIZE);
        memset(m_szAddFriendName,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_userID;
    char m_szUserName[MAX_SIZE];
    char m_szAddFriendName[MAX_SIZE];

}STRU_ADD_FRIEND_RQ;

//添加好友回复
typedef struct STRU_ADD_FRIEND_RS
{
    STRU_ADD_FRIEND_RS()
    {
        m_nType = DEF_PACK_ADD_FRIEND_RS;
        m_userID = 0;
        m_friendID = 0;
        m_result = 0;
        memset(szAddFriendName,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_userID;
    int m_friendID;
    int m_result;
    char szAddFriendName[MAX_SIZE];

}STRU_ADD_FRIEND_RS;

//好友信息
typedef struct STRU_FRIEND_INFO
{
    STRU_FRIEND_INFO()
    {
        m_nType = DEF_PACK_FRIEND_INFO;
        m_userID = 0;
        m_iconID = 0;
        m_state = 0;

        memset(m_szName,0,MAX_SIZE);
        memset(m_feeling,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_userID;
    int m_iconID;
    int m_state;
    char m_szName[MAX_SIZE];
    char m_feeling[MAX_SIZE];

}STRU_FRIEND_INFO;

//粉丝信息
typedef struct STRU_FANS_INFO
{
    STRU_FANS_INFO()
    {
        m_nType = DEF_PACK_FANS_INFO;
        m_userID = 0;
        m_iconID = 0;
        m_state = 0;

        memset(m_szName,0,MAX_SIZE);
        memset(m_feeling,0,MAX_SIZE);
    }
    PackType   m_nType;   //包类型
    int m_userID;
    int m_iconID;
    int m_state;
    char m_szName[MAX_SIZE];
    char m_feeling[MAX_SIZE];

}STRU_FANS_INFO;

//离线请求
typedef struct STRU_FORCE_OFFLINE
{
    STRU_FORCE_OFFLINE()
    {
        m_nType = DEF_PACK_FORCE_OFFLINE;
        m_userID = 0;
    }
    PackType   m_nType;   //包类型
    int m_userID;

}STRU_FORCE_OFFLINE;


#endif
