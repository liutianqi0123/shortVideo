 #ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include<QThread>
#include <QObject>
#include<QDebug>
#include<QImage>
#include"packetqueue.h"
#include"Packdef.h"

extern "C"
{
 #include "libavcodec/avcodec.h"
 #include "libavformat/avformat.h"
 #include "libavutil/pixfmt.h"
 #include "libswscale/swscale.h"
 #include "libavdevice/avdevice.h"
 #include"libswresample/swresample.h"
 #include"libavutil/time.h"
 #include "SDL.h"
}

enum PlayerState
{
    Playing = 0,
    Pause,
    Stop
};


class VideoPlayer;

typedef struct VideoState {
    AVFormatContext *pFormatCtx;//相当于视频”文件指针”
    ///////////////音频///////////////////////////////////
    AVStream *audio_st; //音频流
    PacketQueue *audioq;//音频缓冲队列
    AVCodecContext *pAudioCodecCtx ;//音频解码器信息指针
    int audioStream;//视频音频流索引
    double audio_clock; ///<pts of last decoded frame 音频时钟
    SDL_AudioDeviceID audioID; //音频 ID
    AVFrame out_frame; //设置参数，供音频解码后的 swr_alloc_set_opts 使用。
    /////////////////////////////////////////////////////
    ///////////////视频///////////////////////////////////
    AVStream *video_st; //视频流
    PacketQueue *videoq;//视频队列
    AVCodecContext *pCodecCtx ;//音频解码器信息指针
    int videoStream;//视频音频流索引
    double video_clock; ///<pts of last decoded frame 视频时钟
    SDL_Thread *video_tid; //视频线程 id
    /////////////////////////////////////////////////////
    //////////播放控制//////////////////////////////////////
    bool isPause;//暂停标志
    int quit;
    bool readFinished; //读线程文件读取完毕
    bool readThreadFinished; //读取线程是否结束
    bool videoThreadFinished; // 视频线程是否结束
    //////////////////////////////////////////////////////
    ///  //// 跳转相关的变量
    int             seek_req; //跳转标志 -- 读线程
    int64_t         seek_pos; //跳转的位置 -- 微秒
    int             seek_flag_audio;//跳转标志 -- 用于音频线程中
    int             seek_flag_video;//跳转标志 -- 用于视频线程中
    double          seek_time; //跳转的时间(秒)  值和seek_pos是一样的
    //////////////////////////////////////////////////////
    int64_t start_time; //单位 微秒
    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
    }
    VideoPlayer* m_player;//用于调用函数
} VideoState;


class VideoPlayer : public QThread
{
    Q_OBJECT
public:
    explicit VideoPlayer();

signals:
    void SIG_GetOneImage(QImage image);

public slots:
    void SendGetOneImage(QImage &img);
    void slot_InitLocalFiles(QString m_fileName);
public:
    void run();//线程函数 当你调用start的时候他就会跑到->> run()
    QString m_fileName;
    VideoState m_videoState;
    QString m_videoList[MAX_VIDEO_COUNT]; //缓存歌曲路径
    quint32 m_videoCount;
    quint32 m_videoCountMax;
    PlayerState m_PlayerState;

    void stop(bool isWait);//是否阻塞等待
    void setFileName(const QString &fileName);
    void playUrl(const QString &url);
};

#endif // VIDEOPLAYER_H
