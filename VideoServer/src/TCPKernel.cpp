#include<TCPKernel.h>
#include "packdef.h"
#include<stdio.h>

using namespace std;

static const ProtocolMap m_ProtocolMapEntries[] =
{
    {DEF_PACK_REGISTER_RQ , &TcpKernel::RegisterRq},
    {DEF_PACK_LOGIN_RQ , &TcpKernel::LoginRq},
    {DEF_PACK_UPLOAD_RQ,&TcpKernel::UploadRq},
    {DEF_PACK_FILEBLOCK_RQ,&TcpKernel::UpLoadBlockRq},
    {DEF_PACK_DOWNLOAD_RQ,&TcpKernel::DownLoadBlockRq},
    {DEF_PACK_CHANGE_VIDEO_RQ,&TcpKernel::ChangeVideoRq},
    {DEF_PACK_ADD_FRIEND_RQ,&TcpKernel::AddFriendRq},
    {0,0}
};
#define RootPath   "/home/colin/video/"

int TcpKernel::Open()
{
    m_sql = new CMysql;
    m_tcp = new TcpNet(this);
    m_tcp->SetpThis(m_tcp);
    pthread_mutex_init(&m_tcp->alock,NULL);
    pthread_mutex_init(&m_tcp->rlock,NULL);
    if(  !m_sql->ConnectMysql("localhost","root","colin123","douyinServer")  )
    {
        printf("Conncet Mysql Failed...\n");
        return FALSE;
    }
    else
    {
        printf("MySql Connect Success...\n");
    }
    if( !m_tcp->InitNetWork()  )
    {
        printf("InitNetWork Failed...\n");
        return FALSE;
    }
    else
    {
        printf("Init Net Success...\n");
    }

    this->redis = new RedisTool;

    return TRUE;
}

void TcpKernel::Close()
{
//    for(auto ite = m_mapNameToFD.begin();ite!=m_mapNameToFD.end();ite++)
//    {
//    }
    for( auto ite = m_mapIDToUserInfo.begin();ite!=m_mapIDToUserInfo.end();++ite)
       {
           delete ite ->second;
       }
    m_mapIDToUserInfo.clear();
    m_mapIDToUserFd.clear();
    m_sql->DisConnect();
    m_tcp->UnInitNetWork();
    delete redis;
}


void TcpKernel::DealData(int clientfd,char *szbuf,int nlen)
{
    PackType *pType = (PackType*)szbuf;
    int i = 0;
    while(1)
    {
        if(*pType == m_ProtocolMapEntries[i].m_type)
        {
            auto fun= m_ProtocolMapEntries[i].m_pfun;
            (this->*fun)(clientfd,szbuf,nlen);
        }
        else if(m_ProtocolMapEntries[i].m_type == 0 &&
                m_ProtocolMapEntries[i].m_pfun == 0)
            return;
        ++i;
    }
    return;
}

//??????
void TcpKernel::RegisterRq(int clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d RegisterRq\n", clientfd);

    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;

    m_UserName = rq->m_szUser;
    char sqlBuf[_DEF_SQLIEN]="";
    sprintf(sqlBuf,"select name from t_UserData where name  = '%s';",rq->m_szUser);
    list<string> reslist;
    bool res = m_sql->SelectMysql(sqlBuf,1,reslist);
    //?????? ???name;
    if(!res)
    {
        cout << "SelectMysql error:" << sqlBuf << endl;
        return ;
    }

    if(reslist.size()>0)
    {//??? ?????????
        rs.m_lResult = userid_is_exist;
    }else
    {//?????? ?????? ??????
        char sqlBuf[_DEF_SQLIEN]="";
        sprintf(sqlBuf,"insert into t_UserData (name , password , food , funny , ennegy , dance ,"
                              " music , video , outside , edu ) values ('%s','%s',%d,%d,%d,%d,%d,%d,%d,%d);",
                       rq->m_szUser,rq->m_szPassword,
                       rq->food,rq->funny,rq->ennegy,rq->dance,rq->music,rq->video,rq->outside,rq->edu);
        m_sql->UpdataMysql(sqlBuf);
        //???id
        sprintf(sqlBuf,"select id from t_UserData where name = '%s';",rq->m_szUser);
        list<string>resID;
        m_sql->SelectMysql(sqlBuf,1,resID);
        int id = 0;
        if(resID.size()>0)
        {
            id = atoi(resID.front().c_str());
        }
        //??????????????????
        sprintf(sqlBuf,"insert into t_userInfo (id,name,icon,feeling) values (%d,'%s',%d,'%s');",
                id,rq->m_szUser,0,"");
        m_sql->UpdataMysql(sqlBuf);
        //?????????????????? ?????????????????????
        char path[MAX_PATH] = "";
        sprintf(path , "%sflv/%s/",RootPath , rq->m_szUser);//home/colin/video/flv/username
        printf("%s\n",path);

        umask(0);
        mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO);
        //S_IRWXU 00700????????????????????????????????????????????????????????????????????????
        //S_IRWXG 00070????????????????????????????????????????????????????????????????????????
        //S_IRWXO 00007??????????????????????????????????????????????????????????????????

        //???????????????????????????hash
        redis->SetHashValue(rq->m_szUser,"beautifulgirls","50");
        redis->SetHashValue(rq->m_szUser,"film","50");
        redis->SetHashValue(rq->m_szUser,"sports","50");
        redis->SetHashValue(rq->m_szUser,"basketball","50");
        redis->SetHashValue(rq->m_szUser,"football","50");
        redis->SetHashValue(rq->m_szUser,"fun","50");
        redis->SetHashValue(rq->m_szUser,"dinner","50");
        redis->SetHashValue(rq->m_szUser,"travel","50");
        redis->SetHashValue(rq->m_szUser,"view","50");
        redis->SetHashValue(rq->m_szUser,"fitness","50");



        redis->SetHashValue("beautifulgirls",rq->m_szUser,"0");
        redis->SetHashValue("film",rq->m_szUser,"0");
        redis->SetHashValue("sports",rq->m_szUser,"0");
        redis->SetHashValue("basketball",rq->m_szUser,"0");
        redis->SetHashValue("football",rq->m_szUser,"0");
        redis->SetHashValue("fun",rq->m_szUser,"0");
        redis->SetHashValue("dinner",rq->m_szUser,"0");
        redis->SetHashValue("travel",rq->m_szUser,"0");
        redis->SetHashValue("view",rq->m_szUser,"0");
        redis->SetHashValue("fitness",rq->m_szUser,"0");

        rs.m_lResult = register_sucess;
    }


    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}
//??????
void TcpKernel::LoginRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d LoginRq\n", clientfd);

    STRU_LOGIN_RQ * rq = (STRU_LOGIN_RQ *)szbuf;
    STRU_LOGIN_RS rs;

    //??????????????????????????????
    m_UserName = rq->m_szUser;
    char sqlBuf[_DEF_SQLIEN]="";
    sprintf(sqlBuf,"select password,id from t_UserData where name = '%s';",rq->m_szUser);
    list<string> reslist;
    bool res = m_sql->SelectMysql(sqlBuf,2,reslist);
    if(!res)
    {
        cout << "SelectMysql error:" << sqlBuf << endl;
        return ;
    }
    if(reslist.size()>0)
    {
        if(strcmp(reslist.front().c_str(),rq->m_szPassword) == 0)
        {
           rs.m_lResult = login_sucess;
           reslist.pop_front();
           rs.m_UserID = atoi(reslist.front().c_str());
           //??????socket???id
           this->m_mapIDToUserFd[rs.m_UserID]  = clientfd;

           if(m_mapIDToUserInfo.find(rs.m_UserID) == m_mapIDToUserInfo.end())
           {//????????? ??????
               UserInfo *info = new UserInfo;
               info->m_fd = clientfd;
               info->m_id = rs.m_UserID;
               info->m_state = 1;
               strcpy(info->m_userName,rq->m_szUser);
               m_mapIDToUserInfo[rs.m_UserID] = info;
           }
           else
           {//?????? ??????????????????
               STRU_FORCE_OFFLINE off;
               off.m_UserID = rs.m_UserID;
               UserInfo * info = m_mapIDToUserInfo[rs.m_UserID];
               //???info
               info->m_fd = clientfd;
               //???map
               m_mapIDToUserInfo[rs.m_UserID] = info;

           }
           //????????????
           getUserInfoFromSql( rs.m_UserID);
           SendUserList(rs.m_UserID);
           //?????????????????????
           //?????????????????????????????????????????????????????????
           //????????????id?????????????????????
           vector<string> str = recommend.GetUserTag(m_UserName);
           //?????????????????????????????????
           vector<int> users = recommend.GetSameUser(str);
           //??????????????????topX?????????
           vector<double> dou = recommend.GetSimilarUsers(users,m_UserName);
           //???????????????????????????????????????
           vector<string> samevector = recommend.sortSimilarUser(dou);
           //?????????????????????????????????????????????
           vector<string> userSortId = recommend.getVideoID(samevector);
           //???????????????????????????????????????????????????????????????????????????????????????????????????????????????zset???
           //??????
           //?????????????????????????????????????????????????????????
           string strVideo = userSortId[0] + "Video";
           m_CommendName = userSortId[0];
           //??????zset????????????zset<???????????????>,zset<?????????>.
           //???????????????name
           vector<string> VideoName = redis->HashKeysContent(strVideo);
           vector<string> VideoScore = redis->HashValsContent(strVideo);
           string strCommendList = m_UserName + "CommendList";
           //?????????????????????????????????top1??????????????????zset???
           for(int i=0;i<(int)VideoName.size();i++)
           {
               redis->ZaddZsetValue(strCommendList,VideoScore[i],VideoName[i]);
           }

        }else
        {
            rs.m_lResult = password_error;
        }
    }else
    {
         rs.m_lResult = userid_no_exist;
    }
    m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs) );
}
//??????????????????????????????
UserInfo* TcpKernel::getUserInfoFromSql(int id)
{
    if(m_mapIDToUserInfo.find(id) == m_mapIDToUserInfo.end())return nullptr;
    UserInfo*info = m_mapIDToUserInfo[id];
    //?????????????????? ??????info ?????????
    char sqlBuf[_DEF_SQLIEN] = "";
    sprintf(sqlBuf,"select name,icon,feeling from t_userInfo where id = %d;",id);
    list<string> res;
    m_sql->SelectMysql(sqlBuf,3,res);
    if(res.size()>0)
    {
        strcpy(info->m_userName,res.front().c_str());
        res.pop_front();
        info->m_iconID = atoi(res.front().c_str());
        res.pop_front();
        strcpy(info->m_feeling,res.front().c_str());
        res.pop_front();
    }

    return info;
}
//???????????? ??? ???id =id ??????????????????????????????????????????????????????????????????????????????
void TcpKernel::SendUserList(int id)
{
    //???????????????
    if(m_mapIDToUserInfo.find(id) == m_mapIDToUserInfo.end())return ;
    //???????????????  loginer ??????????????????loginrq
    UserInfo *loginer = m_mapIDToUserInfo[id];
    STRU_FRIEND_INFO loginRq;
    loginRq.m_state = 1;
    loginRq.m_iconID = loginer->m_iconID;
    strcpy(loginRq.m_szName , loginer->m_userName);
    strcpy(loginRq.m_feeling , loginer->m_feeling);
    loginRq.m_userID = loginer->m_id;

    char sqlbuf[_DEF_SQLIEN] = "";
    sprintf(sqlbuf,"select idB from t_friend where idA = %d;",id);
    list<string>resID;
    m_sql->SelectMysql(sqlbuf,1,resID);

    if(resID.size() == 0)return;
    //?????????????????????  friender ????????????friendrq
    //?????????????????????????????? ??? ???????????????????????????????????????
    for(auto ite = resID.begin();ite != resID.end();++ite)
    {
        int friendid = atoi(ite->c_str());

        STRU_FRIEND_INFO friendRq;
        friendRq.m_userID = friendid;
        if(m_mapIDToUserInfo.find(friendid) != m_mapIDToUserInfo.end())
        {//?????? ????????????????????????
            friendRq.m_state = 1;
            //???????????????????????????
            UserInfo *friender = m_mapIDToUserInfo[friendid];
            friendRq.m_iconID = friender->m_iconID;
            strcpy(friendRq.m_szName,friender->m_userName);
            strcpy(friendRq.m_feeling,friender->m_feeling);
            //??????????????????????????????
            m_tcp->SendData(friender->m_fd,(char*)&loginRq,sizeof(loginRq));
        }
        else
        {//????????????????????????????????????
            friendRq.m_state = 0;
            //????????????????????? ??????info ?????????
            char sqlBuf[_DEF_SQLIEN] = "";
            sprintf(sqlBuf,"select name, icon , feeling from t_userInfo where id = %d",friendid);
            list<string> res;
            m_sql->SelectMysql( sqlBuf , 3, res);
            if( res.size() > 0 )
            {
                strcpy(friendRq.m_szName, res.front().c_str());
                res.pop_front();
                friendRq.m_iconID = atoi(res.front().c_str());
                res.pop_front();
                strcpy(friendRq.m_feeling, res.front().c_str());
                res.pop_front();
            }


        }
        //??????????????????????????????
        m_tcp->SendData(loginer->m_fd,(char*)&friendRq,sizeof(friendRq));
    }
}
//??????????????????
void TcpKernel::UploadRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d UploadRq\n", clientfd);
    STRU_UPLOAD_RQ *rq = (STRU_UPLOAD_RQ *)szbuf;
    FileInfo *info = new FileInfo;

    info->m_nFileID = rq->m_nFileId;
    info->m_nPos = 0;
    info->m_nFileSize = rq->m_nFileSize;
    info->m_nUserId = rq->m_UserId;
    memcpy(info->m_Hobby,rq->m_szHobby,DEF_HOBBY_COUNT);
    memcpy(info->m_szFileName,rq->m_szFileName,MAX_PATH);
    memcpy(info->m_szFileType,rq->m_szFileType,MAX_PATH);


    //??????????????????
    char szsql[_DEF_SQLIEN];
    bzero(szsql,sizeof(szsql));

    list<string>reslist;
    //????????????????????????
    sprintf(szsql,"select name from t_UserData where id = %d;",rq->m_UserId);
    printf("%s\n",szsql);
    if(m_sql->SelectMysql(szsql,1,reslist) == FALSE)
    {
        err_str("SelectMysql Failed:",-1);
        delete info;
        return;
    }
    if(reslist.size() <= 0)
    {
        delete info;
        return;
    }

    strcpy(info->m_UserName , reslist.front().c_str());
    sprintf(info->m_szRtmp,"//%s/%s",info->m_UserName,info->m_szFileName);
    printf("%s\n",info->m_szRtmp);
    sprintf(info->m_szFilePath,"%sflv/%s/%s",RootPath,info->m_UserName ,rq->m_szFileName);

    if(strcmp(rq->m_szFileType,"gif") != 0)
    {
        strcpy(info->m_szGifName,rq->m_szGifName);
        sprintf( info->m_szGifPath ,"%sflv/%s/%s",RootPath,info->m_UserName,rq->m_szGifName);
    }
    info->m_VideoID =0;
    info->pFile = fopen(info->m_szFilePath,"w");
    if(info->pFile)
    {
        m_mapFileIDToFileInfo[info->m_nFileID] = info;
    }else
    {
        delete info;
    }

    char strVideo[30] = {0};
    sprintf(strVideo,"%d%s",atoi(info->m_UserName),"Video");

    if(strcmp(rq->m_szFileType,"gif") != 0)
    {
         redis->SetHashValue(strVideo,rq->m_szFileName,"10");
    }


}
//???????????????
void TcpKernel::UpLoadBlockRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d UpLoadBlockRq\n", clientfd);
    STRU_FILEBLOCK_RQ *rq = (STRU_FILEBLOCK_RQ *)szbuf;
    if( m_mapFileIDToFileInfo .find ( rq->m_nFileId ) == m_mapFileIDToFileInfo.end() ) return;

    FileInfo* info  = m_mapFileIDToFileInfo [ rq->m_nFileId ];

    int64_t res = fwrite( rq->m_szFileContent , 1 , rq->m_nBlockLen , info->pFile );
    info->m_nPos += res;


    if(  rq->m_nBlockLen < MAX_CONTENT_LEN || info->m_nPos >= info->m_nFileSize  )
    {
        //?????????
        fclose( info->pFile);
        //?????? ??????gif ????????????   ????????????
        if(  strcmp( info ->m_szFileType ,"gif") != 0)
        {
            //xiebiao
            //create table t_VideoInfo( videoid bigint unsigned  AUTO_INCREMENT primary key ,
            //                          userId bigint unsigned , videoName nvarchar (300),
            //                          picName nvarchar (300), videoPath nvarchar (300) ,
            //                          picPath nvarchar (300) ,rtmp nvarchar (300) , food int, funny int ,ennegy int ,dance int ,
            //                          music int,  video int,  outside int, edu int , hotdegree int);

            char sqlstr[_DEF_SQLIEN] = "";
            sprintf( sqlstr , "insert into t_VideoInfo ( userId , videoName, picName ,videoPath ,picPath ,rtmp ,food ,funny  ,ennegy  ,dance , music ,  video ,  outside , edu  , hotdegree) values (%d , '%s' ,'%s', '%s' , '%s' , '%s' , %d ,%d , %d ,%d , %d , %d , %d ,%d , %d);"
                                 ,rq->m_nUserId , info->m_szFileName , info->m_szGifName, info->m_szFilePath , info->m_szGifPath , info->m_szRtmp
                                 , info->m_Hobby[0] ,info->m_Hobby[1],info->m_Hobby[2],info->m_Hobby[3]
                                ,info->m_Hobby[4],info->m_Hobby[5],info->m_Hobby[6],info->m_Hobby[7] , 0 );

            printf("%s\n" , sqlstr);
            if( !m_sql->UpdataMysql(sqlstr ) )
            {
                cout<< "UpdataMysql error;"<< sqlstr <<endl;
            }
            //??????
            STRU_UPLOAD_RS rs;
            rs.m_nResult = 1;
            m_tcp->SendData( clientfd , (char*)&rs , sizeof(rs));
        }

        m_mapFileIDToFileInfo.erase( rq->m_nFileId );
        delete info;
        info = NULL;
    }

}
//????????????
void TcpKernel::DownLoadBlockRq(int clientfd ,char* szbuf,int nlen)
{
    printf("clientfd:%d DownLoadBlockRq\n", clientfd);

    //????????????????????? ????????? ??????????????????rtmp??????

    STRU_DOWNLOAD_RQ * rq = (STRU_DOWNLOAD_RQ*)szbuf;
    list<FileInfo*> fileList;
    GetFileList(fileList,rq->m_nUserId);
    //??????list ???????????? ???????????????

    while(fileList.size() > 0)
    {
        FileInfo *info = fileList.front();
        fileList.pop_front();

        STRU_DOWNLOAD_RS rs;
        strcpy(rs.m_rtmp,info->m_szRtmp);
        rs.m_nFileId = info->m_nFileID;
        rs.m_nVideoId = info->m_VideoID;
        rs.m_nFileSize = info->m_nFileSize;
        strcpy(rs.m_szFileName,info->m_szFileName);

        m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

        info->pFile = fopen(info->m_szFilePath,"r");
        if(info->pFile)
        {
            while(1)
            {
                STRU_FILEBLOCK_RQ blockrq;
                int64_t res = fread(blockrq.m_szFileContent,1,MAX_CONTENT_LEN,info->pFile);
                blockrq.m_nBlockLen = res;

                info->m_nPos += res;
                blockrq.m_nFileId = info->m_VideoID;
                blockrq.m_nUserId = rq->m_nUserId;
                m_tcp->SendData(clientfd,(char*)&blockrq,sizeof(blockrq));
                if(info->m_nPos>= info->m_nFileSize)
                {
                    fclose(info->pFile);
                    delete info;
                    info = NULL;
                    break;
                }
            }
        }
    }

}

void TcpKernel::GetFileList(list<FileInfo*>&filelist , int userid)
{
    //1.?????? userid ????????????????????????in???t_UserRecv-->videoid????????????????????? ???videoInfo???videoid??????t_UserRecv????????????
    char sqlstr[1024] = "";
    sprintf(sqlstr,"select count(videoId) from t_VideoInfo where t_VideoInfo.videoId not in ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = %d );",userid);
    int nCount = 0;
    list<string>resList;
    if(!m_sql->SelectMysql(sqlstr,1,resList))
    {
        printf("SelectMysql error:%s\n",sqlstr);
        return;
    }
    if(resList.size() == 0)return;

    nCount = atoi(resList.front().c_str());
    //2.?????? == 0 ??????userid ?????? t_UserRecv ???
    if(nCount == 0)
    {
        sprintf(sqlstr,"delete from t_UserRecv where t_UserRecv.userId = %d;",userid);
        if(!m_sql->UpdataMysql(sqlstr))
        {
            printf("UpdataMysql error : %s\n",sqlstr);
            return;
        }
    }
    //???????????????????????????????????????

    //3.?????? ???????????????????????? ?????????10  order by hotdegree limit 0-10 ????????? ??????--??? filelist

//    select videoId ,picName , picPath , rtmp , hotdegree from t_VideoInfo where t_VideoInfo.videoId not in
//    ( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = 5 ) order by hotdegree desc limit
//    0,10;
    resList.clear();
    sprintf(sqlstr,"select videoId ,picName , picPath , rtmp from t_VideoInfo where t_VideoInfo.videoId not in"
                   "( select t_UserRecv.videoId from t_UserRecv where t_UserRecv.userId = %d ) order by hotdegree desc limit "
                   "0,10;",userid);
    if(!m_sql->SelectMysql(sqlstr,4,resList))
    {
        printf("SelectMysql error : %s\n",sqlstr);
        return;
    }
    nCount = 1;
    int nSize = resList.size()/4;
    for(int i =0; i<nSize;++i)
    {
        FileInfo * info = new FileInfo;
        //info ??????
        info->m_nPos=0;
        info->m_VideoID = atoi(resList.front().c_str());
        resList.pop_front();

        strcpy(info->m_szFileName , resList.front().c_str());
        resList.pop_front();

        strcpy(info->m_szFilePath , resList.front().c_str());
        resList.pop_front();

        strcpy(info->m_szRtmp , resList.front().c_str());
        resList.pop_front();

        info->m_nFileID = nCount ++;

        info->pFile = fopen(info->m_szFilePath,"r");
        fseek(info->pFile , 0 , SEEK_END);
        info->m_nFileSize = ftell(info->pFile);//??????4G
        fseek(info->pFile , 0 , SEEK_SET);
        fclose(info->pFile);
        info->pFile = NULL;
        //list??????
        filelist.push_back(info);
        //4.????????????????????????t_UserRecv???
        sprintf(sqlstr,"insert into t_UserRecv values(%d ,%d);",userid,info->m_VideoID);
        if(!m_sql->UpdataMysql(sqlstr))
        {
            printf("UpdataMysql error : %s\n",sqlstr);
            return;
        }
    }

}

//????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
 //???10????????????????????????2??????????????????????????????

void TcpKernel::ChangeVideoRq(int clientfd,char* szbuf,int nlen)
{

    printf("clientfd:%d ChangeVideoRq\n", clientfd);

    STRU_CHANGE_VIDEO_RQ * rq = (STRU_CHANGE_VIDEO_RQ *)szbuf;
    STRU_CHANGE_VIDEO_RS rs;

    rs.m_nUserId = rq->m_nUserId;
    //??????????????????????????????????????? 1.?????????????????? 2.??????????????????
    string strCommendList = m_UserName + "CommendList";
    vector<string> VideoNames = redis->ZRangeZSetKeys(strCommendList,0,-1);
    reverse(VideoNames.begin(),VideoNames.end());

    if(rq->ChangeType == 0)
    {
        index = 0;
    }
    else if(rq->ChangeType == 1)
    {
        if(index < (int)VideoNames.size()-1)
        index++;
        else
            index = 0;
    }
    else if(rq->ChangeType == -1)
    {
        if(index == 0)
            index =0;
        else
        index--;
    }
    //???redis??????????????????url
    sprintf(rs.m_rtmp,"//%s/%s",m_CommendName.c_str(),VideoNames[index].c_str());

    //??????????????????
    m_tcp->SendData(clientfd,(char*)& rs,sizeof(rs));
}

void TcpKernel::AddFriendRq(int clientfd,char* szbuf,int nlen)
{
    printf("clientfd:%d AddFriendRq\n", clientfd);

    STRU_ADD_FRIEND_RQ * rq = (STRU_ADD_FRIEND_RQ *)szbuf;
    STRU_ADD_FRIEND_RS rs;

    rs.m_userID = rq->m_userID;
    strcpy(rs.szAddFriendName , rq->m_szAddFriendName);

    //??????????????????id ?????????????????????
        char szsql[_DEF_SQLIEN]="";
        sprintf(szsql,"select id from t_UserData where name = '%s';",rq->m_szAddFriendName);
        list<string> resID;
        m_sql->SelectMysql(szsql,1,resID);
        if(resID.size()>0)
        {
            int id = atoi(resID.front().c_str());
            //SendMsgToOnlineClient( rq->m_userID , szbuf , nlen);
            rs.m_result = add_success;
            char sqlbuf[_DEF_SQLIEN]="";

            sprintf(sqlbuf, "insert into t_friend(idA , idB) values( %d , %d);", rq->m_userID, id);
            m_sql->UpdataMysql(sqlbuf);

            sprintf(sqlbuf, "insert into t_fans(idA , idB) values( %d , %d);", id, rq->m_userID);
            m_sql->UpdataMysql(sqlbuf);

            getUserInfoFromSql(rq->m_userID);
            SendUserList(rq->m_userID);

        }
        else
        {
            rs.m_result = no_this_user;
        }
    m_tcp->SendData(clientfd,(char*)&rs,sizeof(rs));

}


//????????????????????????
void TcpKernel::SendMsgToOnlineClient(int id,char* szbuf,int nlen)
{
    if(m_mapIDToUserInfo.find(id) != m_mapIDToUserInfo.end())
    {
        m_tcp->SendData(m_mapIDToUserInfo[id]->m_fd,szbuf,nlen);
    }
}
