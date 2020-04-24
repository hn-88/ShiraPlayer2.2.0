#ifndef AUDIOCLASS_H
#define AUDIOCLASS_H

#include <QObject>
#include <QString>

extern "C"{
#include <libavutil/avstring.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avfft.h>
#include <libswresample/swresample.h>
#include <SDL.h>
#include <SDL_thread.h>
}

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 5

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

//static int64_t sws_flags = SWS_BICUBIC;
#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))
#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};
#define isnan(x) ((x) != (x))
//#define INT64_MIN    ((int64_t)_I64_MIN)
//#define INT64_MAX    _I64_MAX

class MyAVPacketList: public QObject
{
public:
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
};

class PacketQueue : public QObject 
{
public:
    MyAVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
};

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

class Clock : public QObject 
{
public:
    double pts;           /* clock base */
    double pts_drift;     /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
};

class Frame : public QObject 
{
public:
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
};

class FrameQueue : public QObject
{
public:
    Frame queue[FRAME_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int keep_last;
    int rindex_shown;
    SDL_mutex *mutex;
    SDL_cond *cond;
    PacketQueue *pktq;
};

class Decoder : public QObject
{
public:
    AVPacket pkt;
    AVPacket pkt_temp;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    SDL_cond *empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    SDL_Thread *decoder_tid;
};

class VideoState : public QObject
{
public:
    SDL_Thread *read_tid;
    AVInputFormat *iformat;
    int abort_request;
    int paused;
    int last_paused;
    int queue_attachments_req;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;
    int realtime;

    Clock audclk;
    Clock vidclk;
    Clock extclk;

    FrameQueue pictq;
    FrameQueue sampq;

    Decoder auddec;
    int audio_stream;

    int av_sync_type;

    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t silence_buf[SDL_AUDIO_MIN_BUFFER_SIZE];
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    AudioParams audio_src;

    AudioParams audio_tgt;
    struct SwrContext *swr_ctx;

    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;

    double frame_timer;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity

    struct SwsContext *img_convert_ctx;
    int eof;

    char filename[1024];
    int step;

    int last_audio_stream;

    SDL_cond *continue_read_thread;

};

class AudioClass : public QObject
{
    Q_OBJECT

public:
    AudioClass(QObject *parent = 0);
    ~AudioClass();

    bool Load_Audio(QString input_filename);
    void Start_Audio();
    void Stop_Audio();
    void audioSeekToPositionSn(double seekSn);
    void audioSeekToPosition(int64_t t, double frameRate);
    QString getCurrentClockStr();
    //QString getTimeSnFromFrameStr(int64_t t);
    int64_t getEnd();
    int64_t getEndSn();
    QString getEndSnStr();
    double getCurrentFrameSn();
    bool isFinised();
    void toggle_pause(void);
    int set_volume(int var);
    void setLoop(int _loop);
    bool IsOpened() { return isOpened;}
    bool IsStarted() { return isStarted;}

    static AudioClass& getInstance() {Q_ASSERT(singleton); return *singleton;}
private:
    bool isOpened;
    bool isStarted;
    static AudioClass* singleton;

    static VideoState *cur_stream;
    static AVInputFormat *file_iformat;
    static AVDictionary *format_opts, *codec_opts, *resample_opts;
    static int av_sync_type;
    static int seek_by_bytes;
    static int64_t start_time;
    static int64_t duration;
    static const char* wanted_stream_spec[AVMEDIA_TYPE_NB];
    static int infinite_buffer;
    static AVPacket flush_pkt;
    static int autoexit;
    static int loop;
    static int volume;
    static const char *audio_codec_name;
    static int64_t audio_callback_time;

    static VideoState *stream_open(const char *filename, AVInputFormat *iformat);
    static void packet_queue_init(PacketQueue *q);
    static int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last);
    static void init_clock(Clock *c, int *queue_serial);

    static int read_thread(void *arg);

    static int decode_interrupt_cb(void *ctx);
    static AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                                      AVDictionary *codec_opts);

    static AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                           AVFormatContext *s, AVStream *st, AVCodec *codec);

    static int is_realtime(AVFormatContext *s);

    /* open a given stream. Return 0 if OK */
    static int stream_component_open(VideoState *is, int stream_index);
    static void packet_queue_flush(PacketQueue *q);
    static int packet_queue_put(PacketQueue *q, AVPacket *pkt);
    static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);
    static void set_clock_at(Clock *c, double pts, int serial, double time);
    static void set_clock(Clock *c, double pts, int serial);
    static void step_to_next_frame(VideoState *is);
    static void stream_toggle_pause(VideoState *is);
    static void toggle_pause(VideoState *is);
    static double get_clock(Clock *c);
    static int packet_queue_put_nullpacket(PacketQueue *q, int stream_index);
    /* return the number of undisplayed frames in the queue */
    static int frame_queue_nb_remaining(FrameQueue *f);
    static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes);
    static void stream_component_close(VideoState *is, int stream_index);
    static void decoder_abort(Decoder *d, FrameQueue *fq);
    static void decoder_destroy(Decoder *d) ;
    static void packet_queue_abort(PacketQueue *q);
    static void frame_queue_signal(FrameQueue *f);
    static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params);
    /* prepare a new audio buffer */
    static void sdl_audio_callback(void *opaque, Uint8 *stream, int len);
    static int audio_decode_frame(VideoState *is);
    static Frame *frame_queue_peek_readable(FrameQueue *f);
    static void frame_queue_next(FrameQueue *f);
    static void frame_queue_unref_item(Frame *vp);
    static int synchronize_audio(VideoState *is, int nb_samples);
    static int get_master_sync_type(VideoState *is);
    static double get_master_clock(VideoState *is);
    static int audio_thread(void *arg);
    static void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond) ;
    static void decoder_start(Decoder *d, int (*fn)(void *), void *arg);
    static void stream_close(VideoState *is);
    static void packet_queue_destroy(PacketQueue *q);
    static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub);
    static void packet_queue_start(PacketQueue *q);
    static Frame *frame_queue_peek_writable(FrameQueue *f);
    static void frame_queue_push(FrameQueue *f);
    static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);
    static void sync_clock_to_slave(Clock *c, Clock *slave);

signals:
    void emitFunction(double i);

};


#endif // AUDIOCLASS_H
