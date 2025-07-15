#include "CSJMediaPlayerBase.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/eval.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/fifo.h"
#include "libavutil/parseutils.h"
#include "libavutil/time.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
#include "libavutil/mem.h"

#include "libavdevice/avdevice.h"
#include "libswresample/swresample.h"
//#include "libavcodec/avfft.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
}
#endif

using UniqueThread = std::unique_ptr<std::thread>;
using StdCondVar = std::condition_variable;
using StdMutex = std::mutex;

namespace csjutils {
class CSJLogger;
}

using csjutils::CSJLogger;

namespace csjmediaengine {

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum audio buffer size, in sample. */
#define AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks. */
#define AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Stop size of volume control in dB. */
#define VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV synv threshold. */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold. */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duraion is longer than this, it will not be duplicated to compensate AV sync. */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error. */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync. */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness. */
#define EXTERNAL_CLOCK_SPEED_MIN 0.900
#define EXTERNAL_CLOCK_SPEED_MAX 1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average. */
#define AUDIO_DIFF_AVG_NB 20

/* polls for possible required screen refresh at leatst this often, should be less than 1/fps.*/
#define REFRESH_RATE 0.01

/* the size must be big enough to compensate the hardware audio buffersize size */
/* We assume that a decoded and resampledd frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

static unsigned sws_flags = SWS_BICUBIC;

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE    16
#define SAMPLE_QUEUE_SIZE        9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, \
                            FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct MyAVPacketList {
    AVPacket *pkt;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue {
    AVFifo     *pkt_list;
    int         nb_packets;
    int         size;
    int64_t     duration;
    int         abort_request;
    int         serial;

    StdMutex   *mutex;
    StdCondVar *cond;
} PacketQueue;

typedef struct AudioParams {
    int                 freq;
    AVChannelLayout     ch_layout;
    enum AVSampleFormat fmt;
    int                 frame_size;
    int                 bytes_per_sec;
} AudioParams;

typedef struct Clock {
    double  pts;            // clock base.
    double  pts_drift;      // clock base minus time at which we updated the clock.
    double  last_updated;
    double  speed;
    int     serial;         // clock is based on a packet with this serial.
    int     paused;
    int    *queue_serial;   // pointer to the current packet queue, used for obsolete clock detection.
} Clock;

/**
 * Common struct for handling all types of decoded data and allocated render buffers.
 */
typedef struct Frame {
    AVFrame     *frame;
    AVSubtitle   sub;
    int          serial;
    double       pts;       // presentation timestamp for the frame.
    double       duration;  // estimated duration of the frame.
    int64_t      pos;       // byte position of the frame in the input file.
    int          width;
    int          height;
    int          format;
    int          uploaded;
    int          flip_v;
    AVRational   sar;
} Frame;

typedef struct FrameQueue {
    Frame       queue[FRAME_QUEUE_SIZE];
    int         rindex;
    int         windex;
    int         size;
    int         max_size;
    int         keep_last;
    int         rindex_shown;

    StdMutex   *mutex;
    StdCondVar *cond;

    PacketQueue *pktq;
} FrameQueue;

enum {
    AV_SYNC_AUDIO_MASTER, // default choice.
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, // synchronize to an external clock.
};

typedef struct Decoder {
    AVPacket        *pkt;
    PacketQueue     *queue;
    AVCodecContext  *avctx;
    int              pkt_serial;
    int              finished;
    int              packet_pending;
    int64_t          start_pts;
    AVRational       start_pts_tb;
    int64_t          next_pts;
    AVRational       next_pts_tb;
    UniqueThread     decoder_thr;
    StdCondVar      *empty_queue_cond;
} Decoder;

typedef enum ShowMode {
    SHOW_MODE_NONE = -1,
    SHOW_MODE_VIDEO,
    SHOW_MODE_WAVES,
    SHOW_MODE_RDFT,
    SHOW_MODE_NB
} CSJShowMode;

class CSJFFPlayerKernel : public CSJMediaPlayerBase {
public:
    CSJFFPlayerKernel();

    ~CSJFFPlayerKernel();

    void setPlayFile(std::string& filePath) override;
    bool initPlayer() override;

    int getDuration() override;

    void play() override;
    void pause() override;
    void resume() override;
    void stop() override;
    void seek(double timeStamp) override;

    bool isPlaying() override;
    bool isPause() override;
    bool isStop() override;

    int getAbortRequest() const {
        return m_abortRequest;
    }

protected:
    int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);
    int packet_queue_put(PacketQueue *q, AVPacket *pkt);
    int packet_queue_put_nullpacket(PacketQueue *q, AVPacket *pkt, int stream_index);
    int packet_queue_init(PacketQueue *q);
    void packet_queue_flush(PacketQueue *q);
    void packet_queue_destory(PacketQueue *q);
    void packet_queue_abort(PacketQueue *q);
    void packet_queue_start(PacketQueue *q);
    void decoder_destroy(Decoder *d);
    void frame_queue_unref_item(Frame *vp);
    int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);
    int decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, std::condition_variable *empty_queue_cond);

    // Display the video content, include audio FFT and video frame.
    void display_video();

    int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub);
    int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last);
    void frame_queue_destory(FrameQueue *f);
    void frame_queue_signal(FrameQueue *f);

    Frame* frame_queue_peek(FrameQueue *f);
    Frame* frame_queue_peek_next(FrameQueue *f);
    Frame* frame_queue_peek_last(FrameQueue *f);
    Frame* frame_queue_peek_writable(FrameQueue *f);
    Frame* frame_queue_peek_readable(FrameQueue *f);
    void   frame_queue_push(FrameQueue *f);
    void   frame_queue_next(FrameQueue *f);
    int    frame_queue_nb_remaining(FrameQueue *f);
    int64_t frame_queue_last_pos(FrameQueue *f);
    void   decoder_abort(Decoder *d, FrameQueue *fq);

    double get_clock(Clock *c);
    void   set_clock_at(Clock *c, double pts, int serial, double time);
    void   set_clock(Clock *c, double pts, int serial);
    void   set_clock_speed(Clock *c, double speed);
    void   init_clock(Clock *c, int *queue_serial);
    void   sync_clock_to_slave(Clock *c, Clock *slave);
    int    get_master_sync_type();
    double get_master_clock();
    void   check_external_clock_speed();
    void   stream_toggle_pause();

    void   resetPlayState();

    void toggle_mute();
    /*
     * 了解一下 SDL中如何设置音量!!!
     */
    void update_volume(int sign, double step);

    void stream_seek(int64_t pos, int64_t rel, int by_bytes);

    void step_to_next_frame();
    double compute_target_delay(double delay);
    double vp_duration(Frame *vp, Frame *nextvp);

    void update_video_pts(double pts, int64_t pos, int serial);
    void video_refresh(double *remaining_time);

    int queue_picture(AVFrame *src_frame, double pts, double duration,
                      int64_t pos, int serial);
    int get_video_frame(AVFrame *frame);

    int audio_decode_task();

    int video_decode_task();

    int subtitle_decode_task();

    void update_sample_display(short *samples, int sample_size);

    /*
     * return the wanted number of samples to get better sync if sync_type
     * is video or external master clock.
     */
    int synchronize_audio(int nb_samples);

    /* @brief
     * Decode one audio frame and return its uncompressed size.
     *
     * The processed audio frame is decoded, converted if required, and
     * stored in m_audioBuf, with size in bytes given by the return
     * value.
     */
    int audio_decode_frame();

    void sdl_audio_callback(uint8_t *stream, int len);

    /**
     * @brief audio_open, 在sdl中打开音频设备，留着作为参考
     * @param wanted_channel_layout Audio channel layout that desired.
     * @param wanted_sample_rate    Audio sample rate that desired.
     * @param audio_hw_params       Audio params.
     * @return
     */
    int audio_open(AVChannelLayout *wanted_channel_layout,
                   int wanted_sample_rate,
                   struct AudioParams *audio_hw_params);

    /**
     * @brief stream_component_open
     * @param stream_index
     * @return
     */
    int stream_component_open(int stream_index);

    //int decode_interrupt_cb(void *ctx);

    int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue);

    int is_realtime(AVFormatContext *s);

    /**
     * @brief read packet from file or network.
     * @return
     */
    int read_thread();

    bool stream_open();

    AVDictionary** setup_find_stream_info_opts(AVFormatContext *s,
                                                AVDictionary *codec_opts);

    void stream_component_close(int stream_index);
    void stream_close();
    void do_exit();

    /*
     * The following three functions are testing functions for the thread model.
     * 
     * Current thread model including four threads, reading, video/audio/subtitle
     * decoding threads.
     * 
     * So far, the interfaces play/pause/resume/stop can contorll the work flow.
     * 
     * Next, use real threads to replace the test threads.
     */
    void threadTestFunc(int thread_type);
    void threadLog(int thread_type, int log_type);
    void readThreadTest();

private:
    std::string      m_fileName;
    CSJLogger       *m_pLogger;

    AVFormatContext *m_pFormatCtx   = nullptr;
    AVInputFormat   *m_pInputFormat = nullptr;
    AVDictionary    *m_pFormatOpts  = nullptr;
    AVDictionary    *m_pCodecOpts   = nullptr;
    StdCondVar      *m_pContinueReadCond;
    UniqueThread     m_pReadThread;
    UniqueThread     m_pVideoDecThread;
    UniqueThread     m_pAudioDecThread;
    UniqueThread     m_pSubtitleDecThread;

    Clock       m_audClk;
    Clock       m_vidClk;
    Clock       m_extClk;
    CSJShowMode m_showMode;

    bool        m_abortRequest     = true;
    bool        m_paused           = false;
    
    int         m_realTime;
    int         m_avSyncType       = AV_SYNC_AUDIO_MASTER;
    int         m_eof;
    int         m_inifiteBuffer    = -1;
    int         m_startupVolume    = 100;
    bool        m_displayDisable   = false;
    bool        m_bDisableVideo    = false;
    bool        m_bDisableAudio    = false;
    bool        m_bDisableSubtitle = false;

    const char *m_WantedStreamSpec[AVMEDIA_TYPE_NB] = {0};

    std::mutex  m_pauseMtx;
    StdCondVar  m_pauseCond;

    int         m_seekReq;
    int         m_seekFlags;
    int64_t     m_seekPos;
    int64_t     m_seekRel;

    /* The next two members indicate to play partly of current file.
     * m_startTime means the start time of the playing, and m_playDuration
     * means the duration of the playing.
     * 
     * m_playDuration must be set if users want to play part ofo the file,
     * and this means that if users only set the m_playDuration, m_startTime
     * will be considered as 0.
     */
    int64_t     m_startTime    = AV_NOPTS_VALUE;
    int64_t     m_playDuration = AV_NOPTS_VALUE;

    /* The number of times to play current file, default is 0. */
    int         m_loopNumber = 0;

    int m_forceRrefresh;
    int m_lastPaused;
    int m_queueAttchmentsReq; 
    int m_readPauseReturn;

    FrameQueue          m_audioFrameQueue;
    PacketQueue         m_audioPaketQueue;
    Decoder             m_audDecoder;

    int                 m_audioStreamIndex;
    double              m_audioClock;
    AVStream           *m_pAudioSteam;
    
    int                 m_audioHwBufSize;
    uint8_t            *m_pAudioBuf;
    uint8_t            *m_pAudioBuf1;
    unsigned int        m_audioBufSize;
    unsigned int        m_audioBuf1Size;
    int                 m_audioBufIndex;
    struct AudioParams  m_audioSrc;
    struct AudioParams  m_audioTgt;
    struct SwrContext  *m_swrCtx;

    int                 m_audioWriteBufSize;
    int                 m_audioVolume;
    int                 m_muted;

    int                 m_subtitleStreamIndex;
    AVStream           *m_pSubtitleStream;
    PacketQueue         m_subtitlePacketQueue;
    FrameQueue          m_subtitleFrameQueue;
    Decoder             m_subtitleDecoder;

    int                 m_width;
    int                 m_height;
    int                 m_xleft;
    int                 m_ytop;
    int                 m_step;

    int                 m_videoStreamIndex;
    AVStream           *m_pVideoStream;
    PacketQueue         m_videoPacketQueue;
    FrameQueue          m_videoFrameQueue;
    Decoder             m_videoDecoder;

    int     m_audioDiffAvgCount;
    int     m_audioClockSerial;
    double  m_audioDiffCum;
    double  m_audioDiffAvgCoef;
    double  m_audioDiffThreshold;
    
    int     m_frameDropsEarly;
    int     m_frameDropsLate;

    int16_t m_sampleArray[SAMPLE_ARRAY_SIZE];
    int     m_sampleArrayIndex;
    int     m_lastIStart;

    // 实时离散傅里叶变换上下文
    // RDFTContext *m_pRdft;
    // FFTSample   *m_pRdftData;
    int          m_rdftBits;
    int          m_xpos;
    double       m_rdftSpeed = 0.02;
    double       m_lastVisTime;
    double       m_frameTimer;
    double       m_frameLastReturnedTime;
    double       m_frameLastFilterDelay;
    double       m_maxFrameDuration;

    struct SwsContext *m_pImgConvertCtx;
    struct SwsContext *m_pSubConvertCtx;

    int     m_lastVideoStream;
    int     m_lastAudioStream;
    int     m_lastSubtitleStream;

    int     m_decoderReorderPts = -1;
    int     m_frameDrop         = -1;
    int     m_showStatus        = -1;
    int64_t m_audioCallbackTime;
};

} // namespace csjmediaengine 
