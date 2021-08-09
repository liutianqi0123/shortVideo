#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H



#include "TCPNet.h"
#include "Mysql.h"
#include <map>
#include"RedisTools.h"
#include"recommendation.h"

class TcpKernel;
typedef void (TcpKernel::*PFUN)(int,char*,int nlen);

typedef struct
{
    PackType m_type;
    PFUN m_pfun;
} ProtocolMap;



class TcpKernel:public IKernel
{
public:
    int Open();
    void Close();
    void DealData(int,char*,int);

    //注册
    void RegisterRq(int,char*,int);
    //登录
    void LoginRq(int,char*,int);

    //上传文件
    void UploadRq(int,char*,int);
    //上传文件块
    void UpLoadBlockRq(int,char*,int);
    //下载
    void DownLoadBlockRq(int,char*,int);
    //获取文件列表
    void GetFileList(list<FileInfo *> &filelist, int userid);

    void AddFriendRq(int clientfd, char *szbuf, int nlen);
    //void AudioFrameRq(int,char* ,int);

    //切换视频
    void ChangeVideoRq(int clientfd, char *szbuf, int nlen);
public:
    RedisTool* redis;
    string m_UserName;
    string m_CommendName;
    int index;

private:
    CMysql * m_sql;
    TcpNet * m_tcp;
    Recommendation recommend;

    map<int , int> m_mapIDToUserFd;
    map<int ,FileInfo*>m_mapFileIDToFileInfo;
    map<int , UserInfo*>m_mapIDToUserInfo;
    UserInfo* getUserInfoFromSql(int id);
    void SendUserList(int id);
    void SendMsgToOnlineClient(int id, char *szbuf, int nlen);
};






////读取上传的视频流信息
//void UpLoadVideoInfoRq(int,char*);
//void UpLoadVideoStreamRq(int,char*);
//void GetVideoRq(int,char*);
//char* GetVideoPath(char*);
//void QuitRq(int,char*);
//void PraiseVideoRq(int,char*);
//void GetAuthorInfoRq(int,char*);
#endif
