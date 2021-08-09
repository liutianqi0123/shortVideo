#include "videoplayer.h"
#include<packetqueue.h>
#include <qdir.h>
#include<qdebug.h>

VideoPlayer::VideoPlayer()
{
    m_videoCount =2;
    m_videoCountMax =0 ;
    m_videoState.readThreadFinished = true;


    //m_fileName=m_videoList[m_videoCount];


}

#define MAX_AUDIO_SIZE (1024*16*25*10)//音频阈值
#define MAX_VIDEO_SIZE (1024*255*25*2)//视频阈值
//当队列里面的数据超过某个大小的时候 就暂停读取 防止一下子就把视频读完了，导致的空间分配不足

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 //1 second of 48khz 32bit audio
#define SDL_AUDIO_BUFFER_SIZE 1024 //S

#define OUT_SAMPLE_RATE 44100

//AVFrame wanted_frame; //解码时需要使用的格式 采样率 声道数的参数
//PacketQueue audio_queue;//音频队列
//int quit = 0;//退出标志
//quint32 m_videoCount = 0;
//quint32 m_videoCountMax = 0;

//回调函数
void audio_callback(void *userdata, Uint8 *stream, int len);
//解码函数
int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size);
//找 auto_stream
int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int*audio_stream);

//时间补偿函数
double synchronize_video(VideoState*is,AVFrame*src_frame,double pts);


//视频解码线程函数
int video_thread(void *arg)
{
    VideoState *is = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;
    int ret, got_picture, numBytes;
    double video_pts = 0; //当前视频的 pts
    double audio_pts = 0; //音频 pts
    ///解码视频相关
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb; //解码后的 rgb 数据
    struct SwsContext *img_convert_ctx; //用于解码后的视频格式转换
    AVCodecContext *pCodecCtx = is->pCodecCtx; //视频解码器
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    ///这里我们改成了 将解码后的 YUV 数据转换成 RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,
                                  pCodecCtx->width,pCodecCtx->height);
    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);
    while(1)
    {
        if(is->quit)break;
        if(is->isPause == true)
        {
            SDL_Delay(10);
            continue;
        }
        if (packet_queue_get(is->videoq, packet, 0) <= 0)
        {
            if(is->readFinished && is->audioq->nb_packets == 0)//播放结束了
            {
                break;
            }else{
                SDL_Delay(10);  //只是队列里暂时没有数据而已
                continue;
            }
        }
       //延时
        while(1)
        {
            if(is->quit)break;
            if(is->audioq->size == 0)break;
            video_pts = is->video_clock;
            audio_pts = is->audio_clock;
            if (video_pts <= audio_pts) break;
            SDL_Delay(5);
        }
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (packet->dts == AV_NOPTS_VALUE && pFrame->opaque&& *(uint64_t*)
                pFrame->opaque != AV_NOPTS_VALUE)
        {
            video_pts = *(uint64_t *) pFrame->opaque;
        }
        else if (packet->dts != AV_NOPTS_VALUE)
        {
            video_pts = packet->dts;
        }
        else
        {
            video_pts = 0;
        }
        video_pts *= 1000000 *av_q2d(is->video_st->time_base);
        video_pts = synchronize_video(is, pFrame, video_pts);
        if (got_picture) {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //把这个 RGB 数据 用 QImage 加载
            QImage tmpImg((uchar*)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
            is->m_player->SendGetOneImage(image); //调用激发信号的函数
        }
        av_free_packet(packet);
    }

    if(!is->quit)
    {
        is->quit = true;
    }
    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);

    is->videoThreadFinished = true; return 0;
}

void VideoPlayer::SendGetOneImage(QImage &img)
{
    emit SIG_GetOneImage(img);
}

//时间补偿函数
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {
    double frame_delay;
    if (pts != 0) {
        /* if we have pts, set video clock to it */
        is->video_clock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        pts = is->video_clock;
    } /* update the video clock */
    frame_delay = av_q2d(is->video_st->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;
    return pts;
}

//初始化本地视频文件
void VideoPlayer::slot_InitLocalFiles(QString m_fileName)
{
    QDir qd(m_fileName);

    //加载目录下所有文件，可以添加过滤
    QFileInfoList subFileList = qd.entryInfoList(QDir::Files | QDir::CaseSensitive);//过滤条件为只限文件并区分大小写


    int j=0;
    //遍历并输出指定类型的文件名
    for (int i = 0;i < subFileList.size(); i++)
    {
        QString suffix = subFileList[i].suffix();//获取后缀名
        if (strcmp(suffix.toStdString().c_str(),"mp4") == 0)//如果后缀为"txt"
        {
            m_videoList[j]=subFileList[i].filePath();
            j++;
        }
    }
    m_videoCountMax = j-1;
    j=0;
}


//读取文件的线程
void VideoPlayer::run()
{
    qDebug()<< "VideoPlayer"<<QThread::currentThread()->currentThreadId();
    qDebug()<<__func__;

    //添加音频需要的变量
    int audioStream = -1;//音频解码器需要的流的索引
    AVCodecContext *pAudioCodecCtx = NULL;//音频解码器信息指针
    AVCodec *pAudioCodec = NULL; //音频解码器
    //SDL
    SDL_AudioSpec wanted_spec; //SDL 音频设置
    SDL_AudioSpec spec ; //SDL 音频设置

    //视频所需的变量
    AVCodecContext *pCodecCtx;//视频的解码信息指针
    AVCodec        *pCodec;//解码器
    AVFrame *pFrame, *pFrameRGB;//存解码后的数据
    AVPacket *packet;//读取解码前的包
    static struct SwsContext *img_convert_ctx;//YUV转RGB的结构
    uint8_t * out_buffer;//存储转化为RGB格式数据的缓冲区
    int numBytes;//帧数据大小
    
    // 解码获取图片
    int videoStream = -1;//视频解码器需要的流的索引
    //1.初始化 FFMPEG 调用了这个才能正常适用编码器和解码器 注册所用函数
    av_register_all();
    //SDL 初始化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Couldn't init SDL:%s\n", SDL_GetError());
        return;
    }
    memset(&m_videoState,0,sizeof(VideoState));
    //2.需要分配一个 AVFormatContext，FFMPEG 所有的操作都要通过这个 AVFormatContext 来进行可以理解为视频文件指针
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //m_fileName = m_videoList[m_videoCount];
    //中文兼容
    std::string path = m_fileName.toStdString();
    const char* file_path = path.c_str();
    //qDebug()<< m_fileName;

    //打开视频文件
    //3. 打开视频文件
    if( avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0 )
    {
        qDebug()<<"can't open file";
        return;
    }
    //3.1 获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug()<<"Could't find stream infomation.";
        return;
    }
    //查找音频视频流索引
    if (find_stream_index(pFormatCtx, &videoStream, &audioStream) == -1)
    {
        printf("Couldn't find stream index\n");
        return;
    }

    m_videoState.pFormatCtx = pFormatCtx;
    m_videoState.videoStream = videoStream;
    m_videoState.audioStream = audioStream;
    m_videoState.m_player = this;

    if(audioStream != -1)
    {
        //5.找到对应的音频解码器
        pAudioCodecCtx = pFormatCtx->streams[audioStream]->codec;
        pAudioCodec = avcodec_find_decoder(pAudioCodecCtx ->codec_id);
        if (!pAudioCodec)
        {
            printf( "Couldn't find decoder\n");
            return;
        }//打开音频解码器
        avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);

        m_videoState.audio_st=pFormatCtx->streams[audioStream];
        m_videoState.pAudioCodecCtx = pAudioCodecCtx;
        SDL_LockAudio();
        //6.设置音频信息, 用来打开音频设备。
        wanted_spec.freq = pAudioCodecCtx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = pAudioCodecCtx->channels; //通道数
        wanted_spec.silence = 0; //设置静音值
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //读取第一帧后调整
        wanted_spec.callback = audio_callback;//回调函数
        wanted_spec.userdata = /*pAudioCodecCtx;*/&m_videoState;//回调函数参数

        //7.打开音频设备
        m_videoState.audioID = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0,0),0,&wanted_spec,&spec,0);
        if( m_videoState.audioID < 0 ) //第二次打开 audio 会返回-1
        {
            printf( "Couldn't open Audio: %s\n", SDL_GetError());
            return;
        }
        //设置参数，供解码时候用, swr_alloc_set_opts 的 in 部分参数
        m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
        m_videoState.out_frame.sample_rate = pAudioCodecCtx->sample_rate;
        m_videoState.out_frame.channel_layout = av_get_default_channel_layout(pAudioCodecCtx->channels);
        m_videoState.out_frame.channels =pAudioCodecCtx->channels;
        //初始化队列
        m_videoState.audioq = new PacketQueue;
        packet_queue_init(m_videoState.audioq);

        SDL_UnlockAudio();
        // SDL 播放声音 0 播放
        SDL_PauseAudioDevice(m_videoState.audioID,0);
    }

    if(videoStream != -1)
    {
        //5.查找解码器
        pCodecCtx = pFormatCtx->streams[videoStream]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL) {
            printf("Codec not found.");
            return;
        }
        //打开解码器
        if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            printf("Could not open codec.");
            return;
        }
        //视频流
        m_videoState.video_st = pFormatCtx->streams[videoStream];
        m_videoState.pCodecCtx = pCodecCtx;
        //视频同步队列
        m_videoState.videoq = new PacketQueue;
        packet_queue_init(m_videoState.videoq);

        //创建视频线程
        m_videoState.video_tid = SDL_CreateThread(video_thread,"video_thread",&m_videoState);

    }
    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet

    int DelayCount =0;
    //8.循环读取视频帧, 转换为 RGB 格式, 抛出信号去控件显示
    while(1)
    {
        if(m_videoState.quit)break;
        if(m_videoState.isPause)
        {
            SDL_Delay(10);
            continue;
        }

        //这里做了个限制 当队列里面的数据超过某个大小的时候 就暂停读取 防止一下子就把视频读完了，导致的空间分配不足
        /* 这里 audioq.size 是指队列中的所有数据包带的音频数据的总量或者视频数据总量，并不是包的数量 */
        //这个值可以稍微写大一些
        if( m_videoState.audioStream != -1 && m_videoState.audioq->size >MAX_AUDIO_SIZE ) {
            SDL_Delay(10);
            continue;
        }
        if ( m_videoState.videoStream != -1 &&m_videoState.videoq->size >MAX_VIDEO_SIZE) {
            SDL_Delay(10);
            continue;
        }
        //可以看出 av_read_frame 读取的是一帧视频，并存入一个 AVPacket 的结构中
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            DelayCount++;
            if(DelayCount >=300)
            {
                m_videoState.readFinished = true;
                DelayCount=0;
            }
            if(m_videoState.quit) break;

            SDL_Delay(10);
            continue;
        }
        DelayCount=0;
        //生成图片
        if (packet->stream_index == videoStream)
        {
            packet_queue_put(m_videoState.videoq, packet);
        }else if(packet->stream_index == audioStream)
        {
            packet_queue_put(m_videoState.audioq, packet);
        }else
        {
            av_free_packet(packet);
        }
    }
    while(!m_videoState.quit)
    {
        SDL_Delay(100);
    }
    //回收队列
    if(m_videoState.videoStream != -1)
    {
        packet_queue_flush(m_videoState.videoq);
    }

    if(m_videoState.audioStream != -1)
    {
        packet_queue_flush(m_videoState.audioq);
    }

    while(m_videoState.videoStream!=-1 && !m_videoState.videoThreadFinished)
    {
        SDL_Delay(10);
    }
    //9.回收数据
    if(videoStream !=-1)
    {
        avcodec_close(pAudioCodecCtx);
    }
    if(audioStream !=-1)
    {
        avcodec_close(pCodecCtx);
    }
    avformat_close_input(&pFormatCtx);

    //读取文件线程退出
    m_videoState.readThreadFinished = true;
}

void VideoPlayer::stop(bool isWait)//阻塞标志
{
    m_videoState .quit = 1;
    if( isWait ) //阻塞标志
    {
        while(!m_videoState.readThreadFinished )//等待读取线程退出
        {
            SDL_Delay(10);
        }
    }
    //关闭 SDL 音频设备
    if (m_videoState.audioID != 0)
    {
        SDL_LockAudio();
        SDL_PauseAudioDevice(m_videoState.audioID,1);//停止播放,即停止音频回调函数
        SDL_UnlockAudio();
        m_videoState.audioID = 0;
    }
    //m_playerState = PlayerState::Stop;
    //Q_EMIT SIG_PlayerStateChanged(PlayerState::Stop);
}

void VideoPlayer::setFileName(const QString &fileName)
{
    if(fileName == NULL) return;
//    if(m_videoCount < m_videoCountMax)
//        m_videoCount++;
//    else
//        m_videoCount = (m_videoCount+1)%m_videoCountMax+1;
    //if( m_playerState != PlayerState::Stop  )   return;
    m_fileName = fileName;
    //m_playerState = PlayerState::Playing;
    this->start();
}

void VideoPlayer::playUrl(const QString &url)
{
    m_fileName = url;
    this->start();
}

//13.回调函数中将从队列中取数据, 解码后填充到播放缓冲区.
void audio_callback(void *userdata, Uint8 *stream, int len)
{
    VideoState *is = (VideoState *) userdata;


    int len1, audio_data_size;
    static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;
    /* len 是由 SDL 传入的 SDL 缓冲区的大小，如果这个缓冲未满，我们就一直往里填充数据 */
    /* audio_buf_index 和 audio_buf_size 标示我们自己用来放置解码出来的数据的缓冲区，*/
    /* 这些数据待 copy 到 SDL 缓冲区， 当 audio_buf_index >= audio_buf_size 的时候意味着我*/
    /* 们的缓冲为空，没有数据可供 copy，这时候需要调用 audio_decode_frame 来解码出更
/* 多的桢数据 */
    while (len > 0)
    {
        if (audio_buf_index >= audio_buf_size) {
            audio_data_size = audio_decode_frame(is,audio_buf,sizeof(audio_buf));
            /* audio_data_size < 0 表示没能解码出数据，我们默认播放静音 */
            if (audio_data_size < 0) {
                /* silence */
                audio_buf_size = 1024;
                /* 清零，静音 */
                memset(audio_buf, 0, audio_buf_size);
            } else {
                audio_buf_size = audio_data_size;
            }
            audio_buf_index = 0;
        }
        /* 查看 stream 可用空间，决定一次 copy 多少数据，剩下的下次继续 copy */
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }
        //SDL_MixAudio 并不能用
        memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
        len -= len1; stream += len1;
        audio_buf_index += len1;
    }
}
//对于音频来说，一个 packet 里面，可能含有多帧(frame)数据。
//解码音频帧函数
int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size)
{
    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    int len1, data_size;
    int sampleSize = 0;
    AVCodecContext *aCodecCtx = is->pAudioCodecCtx;
    AVFrame *audioFrame = av_frame_alloc();
    PacketQueue *audioq = is->audioq;
    AVFrame wanted_frame = is->out_frame;

    if(!is || !aCodecCtx || !audioq)return 0;
    static struct SwrContext *swr_ctx = NULL;
    int convert_len;
    int n = 0;
    for(;;)
    {
        if(is->quit) return -1;
        //if(!audio) return -1;
        if(packet_queue_get(audioq, &pkt, 0) <= 0) //一定注意
        {
            if(is->readFinished && is->audioq->nb_packets == 0)
            is->quit = true;
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
        while(audio_pkt_size > 0)
        {
            if( is->quit ) return -1;
            if(is->isPause == true)
            {
                SDL_Delay(10);
                continue;
            }
            int got_picture;
            memset(audioFrame, 0, sizeof(AVFrame));
            int ret =avcodec_decode_audio4( aCodecCtx, audioFrame, &got_picture, &pkt);
            if( ret < 0 ) {
                printf("Error in decoding audio frame.\n");
                exit(0);
            }

            //计算时钟
            //一帧一个声道读取数据字节数是 nb_samples , channels 为声道数 2 表示 16 位 2 个字节
            //data_size = audioFrame->nb_samples * wanted_frame.channels * 2;
            switch( is->out_frame.format )
            {
            case AV_SAMPLE_FMT_U8:
                data_size = audioFrame->nb_samples * wanted_frame.channels * 1;
                break;
            case AV_SAMPLE_FMT_S16:
                data_size = audioFrame->nb_samples * wanted_frame.channels * 2;
                break;
            default:
                data_size = audioFrame->nb_samples * wanted_frame.channels * 2;
                break;
            }
            //计算时钟 时钟每次要加一帧数据的时间 = 一帧数据的大小/一秒钟采样sampel_rate多次对应的字节数
            //时钟+= 一帧数据的时间 = 一帧数据的大小/一秒数据的大小
            //sampleSize 表示一帧(大小 nb_samples)audioFrame 音频数据对应的字节数.-->一帧数据的大小
            sampleSize = av_samples_get_buffer_size(NULL, is->pAudioCodecCtx->channels,
                                                    audioFrame->nb_samples,
                                                    is->pAudioCodecCtx->sample_fmt, 1);
            //n 表示每次采样的字节数
            n = av_get_bytes_per_sample(is->pAudioCodecCtx->sample_fmt)*is->pAudioCodecCtx->channels;
            //时钟每次要加一帧数据的时间= 一帧数据的大小/一秒钟采样 sample_rate 多次对应的字节数.
            is->audio_clock += (double)sampleSize*1000000 /(double) (n* is->pAudioCodecCtx->sample_rate);




            if( got_picture )
            {
                if (swr_ctx != NULL)
                {
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }
                swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout,
                                             (AVSampleFormat)wanted_frame.format,wanted_frame.sample_rate,
                                             audioFrame->channel_layout,(AVSampleFormat)audioFrame->format,
                                             audioFrame->sample_rate, 0, NULL);
                //初始化
                if (swr_ctx == NULL || swr_init(swr_ctx) < 0)
                {
                    printf("swr_init error\n");
                    break;
                }
                convert_len = swr_convert(swr_ctx, &audio_buf,
                                          AVCODEC_MAX_AUDIO_FRAME_SIZE,
                                          (const uint8_t **)audioFrame->data,
                                          audioFrame->nb_samples);
            }
            audio_pkt_size -= ret;
            if (audioFrame->nb_samples <= 0)
            {
                continue;
            }
            av_free_packet(&pkt);
            return data_size ;
        }
        av_free_packet(&pkt);
    }
}
//查找数据流函数
int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int *audio_stream)
{
    assert(video_stream != NULL || audio_stream != NULL);
    int i = 0;
    int audio_index = -1;
    int video_index = -1;
    for (i = 0; i < pformat_ctx->nb_streams; i++)
    {
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_index = i;
        }
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_index = i;
        }
    }
    //注意以下两个判断有可能返回-1.
    if (video_stream == NULL)
    {
        *audio_stream = audio_index;
        return *audio_stream;
    }
    if (audio_stream == NULL)
    {
        *video_stream = video_index;
        return *video_stream;
    }
    *video_stream = video_index;
    *audio_stream = audio_index;
    return 0;
}

