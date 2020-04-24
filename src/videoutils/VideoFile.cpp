
/*
 *  ShiraPlayer(TM)
 *  Created on: Jul 10, 2009 Author: bh
 *
 *  This file is part of GLMixer.
 *
 *   GLMixer is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   GLMixer is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GLMixer.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Copyright 2009, 2012 Bruno Herbelin
 *   Copyright (C) 2011 Asaf Yurdakul
 */
#include <stdint.h>
#include <QDateTime>

#ifndef INT64_MIN
#define INT64_MIN       (-0x7fffffffffffffffLL-1)
#endif

#ifndef INT64_MAX
#define INT64_MAX INT64_C(9223372036854775807)
#endif

#define INT64_C

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/common.h>
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52,30,0)
#include <libavutil/pixdesc.h>
#endif
}

// Buffer between Parsing thread and Decoding thread
#define MEGABYTE 1048576
#define MAX_VIDEOQ_SIZE (3 * 1048576)

#include "VideoFile.h"

#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QThread>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
class ParsingThread: public QThread {
public:
    ParsingThread(VideoFile *video = 0) : QThread(), is(video) {
    }

    void run();
private:
    VideoFile *is;

};

class DecodingThread: public QThread {
public:
    DecodingThread(VideoFile *video = 0) : QThread(), is(video) {
        // allocate a frame to fill
        _pFrame = avcodec_alloc_frame();
        Q_CHECK_PTR(_pFrame);
    }
    ~DecodingThread() {
        // free the allocated frame
        av_free(_pFrame);
    }

    void run();
private:
    VideoFile *is;
    AVFrame *_pFrame;
};


VideoPicture::VideoPicture() : oldframe(0), width(0), height(0), allocated(false), usePalette(false), pts(0.0) {

    img_convert_ctx_filtering = NULL;
    rgb.data[0] = NULL;
}

bool VideoPicture::allocate(SwsContext *img_convert_ctx, int w, int h, enum PixelFormat format, bool palettized) {

    img_convert_ctx_filtering = img_convert_ctx;
    usePalette = palettized;
    width = w;
    height = h;
    pixelformat = format;

    // if we already have one, make another
    // free image
    if (rgb.data[0]) {
        avpicture_free(&rgb);
        rgb.data[0] = NULL;
    }

    // Determine required buffer size and allocate buffer
    int numBytes = avpicture_get_size(pixelformat, width, height);
    uint8_t *buffer = static_cast<uint8_t*>(av_mallocz(numBytes + FF_INPUT_BUFFER_PADDING_SIZE));
    Q_CHECK_PTR(buffer);
    // initialize buffer to black
    for (int i = 0; i < numBytes; ++i)
        buffer[i] = 0;

    // create & fill the picture
    if ( avpicture_alloc( &rgb, pixelformat, width, height) < 0)
        return false;
    avpicture_fill( &rgb, buffer, pixelformat, width, height);

    allocated = true;
    return true;
}

VideoPicture::~VideoPicture()
{
    if (rgb.data[0]) {
        avpicture_free(&rgb);
        rgb.data[0] = NULL;
    }
}

void VideoPicture::saveToPPM(QString filename) const {

    if ( allocated && pixelformat != PIX_FMT_RGBA) {
        FILE *pFile;
        int  y;

        // Open file
        pFile = fopen(filename.toUtf8().data(), "wb");
        if(pFile==NULL)
            return;

        // Write header
        fprintf(pFile, "P6\n%d %d\n255\n", width, height);

        // Write pixel data
        for(y=0; y<height; y++)
            fwrite(rgb.data[0] + y*rgb.linesize[0], 1, width*3, pFile);

        // Close file
        fclose(pFile);
    }
}

void VideoPicture::refilter() const
{
    if (!allocated || !oldframe || !img_convert_ctx_filtering || usePalette)
        return;

    sws_scale(img_convert_ctx_filtering, oldframe->data, oldframe->linesize, 0, height, (uint8_t**) rgb.data, (int*) rgb.linesize);
}

void VideoPicture::fill(AVPicture *frame, double timestamp)
{
    if (!allocated || !frame || !img_convert_ctx_filtering)
        return;

    // remember frame (for refilter())
    oldframe = frame;
    // remember pts
    pts = timestamp;

    if (!usePalette) {
        // Convert the image with ffmpeg sws

        //if (!this-> useYUVData )
        sws_scale(img_convert_ctx_filtering, frame->data, frame->linesize, 0, height, (uint8_t**) rgb.data, (int *) rgb.linesize);
        //else
        //    av_picture_data_copy( (uint8_t**)rgb.data , (int *)rgb.linesize,frame->data, frame->linesize, pixelformat, width, height);
        return;
    }
    // I reimplement here sws_convertPalette8ToPacked32 which does not work...
    else {
        // get pointer to the palette
        uint8_t *palette = frame->data[1];
        // clear rgb to 0 when alpha is 0
        for (int i = 0; i < 4 * 256; i += 4)
        {
            if (palette[i + 3] == 0)
            {
                palette[i + 0] = 0;
                palette[i + 1] = 0;
                palette[i + 2] = 0;
            }
        }
        // copy BGR palette color from frame to RGB buffer of VideoPicture
        uint8_t *map = frame->data[0];
        uint8_t *bgr = rgb.data[0];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                *bgr++ = palette[ 4 * map[x] + 2];  // B
                *bgr++ = palette[ 4 * map[x] + 1];  // G
                *bgr++ = palette[ 4 * map[x]];      // R
                if (pixelformat == PIX_FMT_RGBA)
                    *bgr++ = palette[ 4 * map[x] + 3];  // A
            }
            map += frame->linesize[0];
        }
    }

}


bool VideoFile::ffmpegregistered = false;

VideoFile::VideoFile(QObject *parent, bool generatePowerOfTwo, int swsConversionAlgorithm, int destinationWidth, int destinationHeight) :
    QObject(parent), filename(QString()), powerOfTwo(generatePowerOfTwo),
    targetWidth(destinationWidth), targetHeight(destinationHeight),
    conversionAlgorithm(swsConversionAlgorithm) {

    if (!ffmpegregistered) {
        avcodec_register_all();
        av_register_all();
#ifdef NDEBUG
        av_log_set_level( AV_LOG_QUIET ); /* don't print warnings from ffmpeg */
#endif
        ffmpegregistered = true;
    }

    // Init some pointers to NULL
    videoStream = -1;
    video_st = NULL;
    deinterlacing_buffer = NULL;
    pFormatCtx = NULL;
    img_convert_ctx = NULL;
    filter = NULL;
    firstFrame = NULL;
    resetPicture = NULL;
    ignoreAlpha = false;
    seek_pos = 0;
    seek_req = false;

    // Contruct some objects
    parse_tid = new ParsingThread(this);
    Q_CHECK_PTR(parse_tid);
    decod_tid = new DecodingThread(this);
    Q_CHECK_PTR(decod_tid);
    pictq_mutex = new QMutex;
    Q_CHECK_PTR(pictq_mutex);
    pictq_cond = new QWaitCondition;
    Q_CHECK_PTR(pictq_cond);

    QObject::connect(parse_tid, SIGNAL(finished()), this, SLOT(thread_terminated()));
    QObject::connect(decod_tid, SIGNAL(finished()), this, SLOT(thread_terminated()));

    ptimer = new QTimer(this);
    Q_CHECK_PTR(ptimer);
    ptimer->setSingleShot(true);
    QObject::connect(ptimer, SIGNAL(timeout()), this, SLOT(video_refresh_timer()));

    // initialize behavior
    play_speed = 1.0; // normal play speed
    seek_any = false; // NOT dirty seek
    seek_backward = false;
    pause_video = false; // not paused by default
    loop_video = false; // loop by default
    quit = true; // not running yet
    restart_where_stopped = true;
    mark_stop = 0;

    // reset
    reset();
}

void VideoFile::close() {

    // request ending
    quit = true;
    // wait for threads to end properly
    parse_tid->wait();
    pictq_cond->wakeAll();
    decod_tid->wait();

    // free context & filter
    if (img_convert_ctx)
        sws_freeContext(img_convert_ctx);
    if (filter)
        sws_freeFilter(filter);

    // close file
    if (pFormatCtx)
        av_close_input_file(pFormatCtx);

}

VideoFile::~VideoFile() {

    close();

    // delete threads
    delete parse_tid;
    delete decod_tid;
    delete pictq_mutex;
    delete pictq_cond;
    delete ptimer;

    if (deinterlacing_buffer)
        av_free(deinterlacing_buffer);

    QObject::disconnect(this,0,0,0);
}


void VideoFile::reset() {

    // reset variables to 0
    pictq_skip = false;
    video_clock = 0.0;
    pictq_windex = 0;
    pictq_size = 0;
    pictq_rindex = 0;

    pause_video = false;
    pause_video_last = true;
    setPlaySpeed(getPlaySpeed());

    if (video_st) {
        frame_last_delay = 1.0 / getFrameRate();
        video_current_pts =   (double) getBegin() * av_q2d( video_st->time_base) / play_speed ;
    } else {
        frame_last_delay = 40e-3; // 40 ms
        video_current_pts =  0.0 ;
    }
    frame_last_pts = video_current_pts ;

    // initial clock
    video_current_pts_time = av_gettime();
    frame_timer = (double) (video_current_pts_time ) / (double) AV_TIME_BASE;
}

void VideoFile::stop() {

    if (!quit && pFormatCtx) {
        // request quit
        quit = true;
        // wait fo threads to end properly
        parse_tid->wait();
        pictq_cond->wakeAll();
        decod_tid->wait();

        // remember where we are for next restart
        mark_stop = getCurrentFrameTime();
        if (!restart_where_stopped) {
            fill_first_frame(true);
            // display firstPicture frame
            emit frameReady(-1);
        }

        /* say if we are running or not */
        emit running(!quit);
        //ASAF
        emit endofVideo();
        qDebug()<<filename<<tr("|Video stopped.");

    }

}

void VideoFile::start() {

    // nothing to play if there is ONE frame only...
    if (getEnd() - getBegin() < 2)
        return;

    if (quit && pFormatCtx) {
        // reset quit flag
        quit = false;

        // reset internal state
        reset();

        // where shall we (re)start ?
        // enforces seek to the frame before ; it is needed because we may miss the good frame
        seek_backward = true;
        // restart where we where
        if (restart_where_stopped && mark_stop < mark_out)
            seekToPosition(mark_stop);
        // restart at beginning
        else
            seekToPosition(mark_in);

        // start parsing and decoding threads
        parse_tid->start();
        decod_tid->start();
        ptimer->start(0);

        /* say if we are running or not */
        emit running(!quit);
        qDebug()<<filename<<tr("|Video playing.");
    }

}

void VideoFile::play(bool startorstop) {

    if (startorstop) {
        start();
    } else {
        stop();
    }
}

void VideoFile::setPlaySpeed(int playspeed) {

    switch (playspeed) {

    case SPEED_QUARTER:
        play_speed = 0.25;
        break;
    case SPEED_THIRD:
        play_speed = 0.333;
        break;
    case SPEED_HALF:
        play_speed = 0.5;
        break;
    case SPEED_DOUBLE:
        play_speed = 2.0;
        break;
    case SPEED_TRIPLE:
        play_speed = 3.0;
        break;
    case SPEED_NORMAL:
    default:
        play_speed = 1.0;
    }

    if (video_st) {
        min_frame_delay = 0.5 / ( getFrameRate() * play_speed );
        max_frame_delay = 10.0 / ( getFrameRate() * play_speed );
    } else
    {
        min_frame_delay = 0.01;
        max_frame_delay = 2.0;
    }
}

int VideoFile::getPlaySpeed(){

    if (play_speed - 0.25 < 0.1 )
        return SPEED_QUARTER;
    if (play_speed - 0.333 < 0.1 )
        return SPEED_THIRD;
    if (play_speed - 0.5 < 0.1 )
        return SPEED_HALF;
    if (play_speed - 1.0 < 0.1 )
        return SPEED_NORMAL;
    if (play_speed - 2.0 < 0.1 )
        return SPEED_DOUBLE;
    if (play_speed - 3 < 0.1 )
        return SPEED_TRIPLE;

    return SPEED_NORMAL;
}

void VideoFile::thread_terminated() {

    // recieved this message while 'quit' was not requested ?
    if (!quit) {
        //  this means an error occured...
        qWarning() << filename << tr("|Decoding interrupted.");
        // stop properly if possible
        stop();
    }
}

const VideoPicture *VideoFile::getPictureAtIndex(int index) const {

    // return a firstPicture picture if wrong index (e.g. -1)
    if (index < 0 || index > VIDEO_PICTURE_QUEUE_SIZE)
        return (resetPicture);

    return (&pictq[index]);
}

bool VideoFile::open(QString file, int64_t markIn, int64_t markOut, bool ignoreAlphaChannel,PixelFormat mytargetPixelFormat) {

    AVFormatContext *_pFormatCtx;

    filename = file;
    ignoreAlpha = ignoreAlphaChannel;

    // Check file
    int err = av_open_input_file(&_pFormatCtx, qPrintable(filename), NULL, 0, NULL);
    if (err < 0) {
        switch (err) {
        case AVERROR_NUMEXPECTED:
            qWarning()<<filename<< tr("|Incorrect numbered image sequence syntax.");
            break;
        case AVERROR_INVALIDDATA:
            qWarning()<<filename<< tr("|Error while parsing header.");
            break;
        case AVERROR_NOFMT:
            qWarning()<<filename<< tr("|Unknown format.");
            break;
        case AVERROR(EIO):
            qWarning()<<filename<< tr("|I/O error. Usually that means that input file is truncated and/or corrupted");
            break;
        case AVERROR(ENOMEM):
            qWarning()<<filename<< tr("|Memory allocation error.");
            break;
        case AVERROR(ENOENT):
            qWarning()<<filename<< tr("|No such file.");
            break;
        default:
            qWarning()<<filename<< tr("|Cannot open file.");
            break;
        }
        return false;
    }

    err = av_find_stream_info(_pFormatCtx);
    if (err < 0) {
        switch (err) {
        case AVERROR_NUMEXPECTED:
            qWarning()<<filename<< tr("|Incorrect numbered image sequence syntax.");
            break;
        case AVERROR_INVALIDDATA:
            qWarning()<<filename<< tr("|Error while parsing header.");
            break;
        case AVERROR_NOFMT:
            qWarning()<<filename<< tr("|Unknown format.");
            break;
        case AVERROR(EIO):
            qWarning()<<filename<< tr("|I/O error. Usually that means that input file is truncated and/or corrupted");
            break;
        case AVERROR(ENOMEM):
            qWarning()<<filename<< tr("|Memory allocation error");
            break;
        case AVERROR(ENOENT):
            qWarning()<<filename<< tr("|No such entry.");
            break;
        default:
            qWarning()<<filename<< tr("|Unsupported format.");
            break;
        }
        return false;
    }

    // if video_index not set (no video stream found) or stream open call failed
    videoStream = stream_component_open(_pFormatCtx);
    if (videoStream < 0) {
        //could not open Codecs (error message already emitted)
        return false;
    }

    // Init the stream and timing info
    video_st = _pFormatCtx->streams[videoStream];

    // all ok, we can set the internal pointer to the good value
    pFormatCtx = _pFormatCtx;

    // check the parameters for mark in and out and setup marking accordingly
    if (markIn == 0)
        mark_in = getBegin();
    else {
        mark_in = qBound(getBegin(), markIn, getEnd());
        // openning a file with a mark in ; go there!
        seek_pos = mark_in;
        seek_req = true;
    }

    if (markOut == 0)
        mark_out = getEnd();  // default to end of file
    else
        mark_out = qBound(mark_in, markOut, getEnd());

    // emit that marking changed only once
    if (markIn != 0 || markOut != 0)
        emit markingChanged();

    // init picture size
    //if (targetWidth == 0)
    targetWidth = video_st->codec->width;
    //if (targetHeight == 0)
    targetHeight = video_st->codec->height;

    // round target picture size to power of two size
    if ( powerOfTwo ) {
        targetWidth = VideoFile::roundPowerOfTwo(targetWidth);
        targetHeight = VideoFile::roundPowerOfTwo(targetHeight);
    }

    // Default targetFormat to PIX_FMT_RGB24, not using color palette
    enum PixelFormat targetFormat = PIX_FMT_RGB24;
    if(useYUVData)
        targetFormat = PIX_FMT_YUV420P;
    bool paletized = false;

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52,30,0)
    // Change target format to keep Alpha channel if exist
    if ( pixelFormatHasAlphaChannel()
         // this is a fix for some jpeg formats with YUVJ format
         || av_pix_fmt_descriptors[video_st->codec->pix_fmt].log2_chroma_h >0  ){
        if(useYUVData)
            targetFormat = PIX_FMT_YUV420P;
        else
            targetFormat = PIX_FMT_RGBA;
    }
    // special case of PALLETIZED pixel formats
    else if ( av_pix_fmt_descriptors[video_st->codec->pix_fmt].flags & PIX_FMT_PAL ){
        if(useYUVData)
            targetFormat = PIX_FMT_YUV420P;
        else
            targetFormat = PIX_FMT_RGB24;//PIX_FMT_RGBA;
        paletized = true;
    }
#else
    // Change target format to keep Alpha channel if exist
    if (pixelFormatHasAlphaChannel()){
        targetFormat = PIX_FMT_RGBA;
    }
    // special case of PALLETIZED pixel formats
    else if (video_st->codec->pix_fmt == PIX_FMT_PAL8 ){
        targetFormat = PIX_FMT_RGBA;
        paletized = true;
    }
#endif
    // Decide for optimal scaling algo if it was not specified
    // NB: the algo is used only if the conversion is scaled or with filter
    // (i.e. optimal 'unscaled' converter is used by default)
    if (conversionAlgorithm == 0) {
        if (getEnd() - getBegin() < 2)
            conversionAlgorithm = SWS_LANCZOS;  	// optimal quality scaling for 1 frame sources (images)
        else
            conversionAlgorithm = SWS_POINT;		// optimal speed scaling for videos
    }

#ifndef NDEBUG
    // print all info if in debug
    conversionAlgorithm |= SWS_PRINT_INFO;
#endif

    if (ignoreAlpha) {
        if(useYUVData)
            targetFormat = PIX_FMT_YUV420P;
        else
            targetFormat = PIX_FMT_RGB24;
        // The ignore alpha flag is normally requested when the source is rgba
        // and in this case, optimal conversion from rgba to rgba is to do nothing : but
        // this means there is no conversion, and no brightness/contrast is applied
        // So, I change the filter to enforce a per-pixel conversion (here a slight blur)
        filter = sws_getDefaultFilter(0.0, 1.0, 0.0 ,0.0, 0.0, 0.0, 0);
    }
    else
        // default filter for doing nothing
        //    	filter = sws_getDefaultFilter(0.0, 0.0, 0.0 ,0.0, 0.0, 0.0, 0);
        filter = NULL;
    if (mytargetPixelFormat!= -1)
        targetFormat = mytargetPixelFormat;

    // MMX should be faster !
    conversionAlgorithm |= SWS_CPU_CAPS_MMX;
    conversionAlgorithm |= SWS_CPU_CAPS_MMX2;

    // create conversion context
    img_convert_ctx = sws_getCachedContext(NULL, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt,
                                           targetWidth, targetHeight, targetFormat, conversionAlgorithm,
                                           filter, NULL, NULL);
    if (img_convert_ctx == NULL) {
        // Cannot initialize the conversion context!
        qWarning() << filename << tr("|Cannot create a suitable conversion context.");
        return false;
    }

    /* allocate the buffers */
    // not need for the picture queue if there is ONE frame only...
    if (getEnd() - getBegin() > 1)
        for (int i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; ++i) {
            if (!pictq[i].allocate(img_convert_ctx, targetWidth, targetHeight, targetFormat, paletized)) {
                // Cannot allocate Video Pictures!
                qWarning() << filename << tr("|Cannot allocate pictures buffer.");
                return false;
            }
        }

    // we may need a black Picture frame to return when stopped
    if (!blackPicture.allocate(0, targetWidth, targetHeight)) {
        // Cannot allocate Video Pictures!
        qWarning() << filename << tr("|Cannot allocate buffer.");
        return false;
    }

    // we need a picture to display when not decoding
    if (!firstPicture.allocate(img_convert_ctx, targetWidth, targetHeight, targetFormat, paletized)) {
        // Cannot allocate Video Pictures!
        qWarning() << filename << tr("|Cannot allocate picture buffer.");
        return false;
    }

    // read firstPicture (not a big problem if fails; it would just be black)
    fill_first_frame(false);
    resetPicture = &firstPicture;

    // display a firstPicture frame ; this shows that the video is open
    emit frameReady(-1);

    pause_video = false; // not paused when loaded
    /* say if we are running or not */
    emit running(false);

    // tells everybody we are set !
    if (mytargetPixelFormat == -1)
    {
        qDebug()<<filename<<tr("|Media opened (nb. %1).").arg(getExactFrameFromFrame(getEnd()));
        //qDebug()<<tr("Video codec name:%1").arg(getCodecName());
    }
    return true;
}

void VideoFile::fillFirstFrame()//ASAF
{
    fill_first_frame(true);
}

bool VideoFile::pixelFormatHasAlphaChannel() const {

    if (!video_st)
        return false;

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52,30,0)
    return ( av_pix_fmt_descriptors[video_st->codec->pix_fmt].nb_components > 3 );
#else
    return (video_st->codec->pix_fmt == PIX_FMT_RGBA || video_st->codec->pix_fmt == PIX_FMT_BGRA ||
            video_st->codec->pix_fmt == PIX_FMT_ARGB || video_st->codec->pix_fmt == PIX_FMT_ABGR ||
            video_st->codec->pix_fmt == PIX_FMT_YUVJ420P );
#endif
}


void VideoFile::fill_first_frame(bool seek) {
    AVPacket pkt1;
    AVPacket *packet = &pkt1;
    int frameFinished = 0;

    if (seek)
        av_seek_frame(pFormatCtx, videoStream, mark_in, AVSEEK_FLAG_BACKWARD);

    while (!frameFinished) {
        // read a packet
        if (av_read_frame(pFormatCtx, packet) != 0)
            break;
        // ignore non-video stream packets
        if (packet->stream_index != videoStream)
            continue;

        // create an empty frame
        if (firstFrame)
            av_free(firstFrame);
        firstFrame = avcodec_alloc_frame();
        Q_CHECK_PTR(firstFrame);
        // if we can decode it
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52,30,0)
        if (avcodec_decode_video2(video_st->codec, firstFrame, &frameFinished, packet) >= 0) {
#else
        if ( avcodec_decode_video(video_st->codec, firstFrame, &frameFinished, packet->data, packet->size) >= 0) {
#endif
            // and if the frame is finished (should be in one shot for video stream)
            if (frameFinished)
            {
                firstPicture.useYUVData = this->useYUVData;
                firstPicture.skipNextFrame = this->skipNextFrame;
                firstPicture.fill((AVPicture *)firstFrame);
            }
        }
    }

    // free packet
    av_free_packet(packet);

    if(seek)
        avcodec_flush_buffers(video_st->codec);

    //    firstPicture.saveToPPM("firstpict.ppm");

}


int VideoFile::stream_component_open(AVFormatContext *pFCtx) {

    AVCodecContext *codecCtx;
    AVCodec *codec;
    int stream_index = -1;

    // Find the first video stream index
    for (int i = 0; i < (int) pFCtx->nb_streams; i++) {
        if (pFCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
            stream_index = i;
            break;
        }
    }

    if (stream_index < 0 || stream_index >= (int) pFCtx->nb_streams) {
        qWarning() << filename << tr("|This is not a video or an image file.");
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    codecCtx = pFCtx->streams[stream_index]->codec;

    codec = avcodec_find_decoder(codecCtx->codec_id);
    if (!codec || (avcodec_open(codecCtx, codec) < 0)) {
        qWarning() << filename << tr("|The codec ")<< codecCtx->codec_name << tr("is not supported.");
        return -1;
    }

    codecname = QString(codec->name);

    return stream_index;
}

void VideoFile::video_refresh_timer() {

    VideoPicture *vp;

    double actual_delay = 0.0, delay = 0.0;

    if (video_st) {

        if (pictq_size < 1) {
            // no picture yet ? Quickly retry...
            ptimer->start(10);
            return;

        } else {

            // use vp as the current picture at the read index
            vp = &pictq[pictq_rindex];

            // update time to now
            video_current_pts = vp->pts;
            video_current_pts_time = av_gettime();

            // how long since last frame ?
            delay = video_current_pts - frame_last_pts; /* the pts from last time */

            if (delay <= min_frame_delay || delay >= max_frame_delay ) {			/* if incorrect delay, use previous one */
                delay = frame_last_delay;
                // as incorrect delay is generally due to seek, set the last pts to the seek pos
                frame_last_pts = (double) seek_pos * av_q2d(video_st->time_base) / play_speed ;
            }
            else
                frame_last_delay = delay;               /* save for next time */

            // remember current pts for next time
            frame_last_pts = video_current_pts;

            // the next frame should be displayed at frame_timer
            frame_timer += delay;

            /* compute the REAL delay */
            actual_delay = frame_timer - ((double) av_gettime() / (double) AV_TIME_BASE);

            // clamp the delay
            actual_delay = qBound(min_frame_delay, actual_delay, max_frame_delay );

            /* show the picture at the current read index
                             * (this delay is for next frame) */
            emit frameReady(pictq_rindex);

            if (!pause_video) {
                // knowing the delay (in seconds), we can schedule the next display (in ms)
                ptimer->start((int) (actual_delay * 1000.0 + 0.5)); // round up
            }

            /* update queue for next picture at the read index */
            if (++pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
                pictq_rindex = 0;
            pictq_mutex->lock();

            // if we hit the lower bound, we skip next frame
            //qDebug()<<"useyuvdata:"<<this->useYUVData;
            //qDebug()<<"skipNextframe:"<<this->skipNextFrame;

            if (this->skipNextFrame) //ASAF
                if ( !(actual_delay > min_frame_delay ))
                    pictq_skip = true;

            // decrease the number of frames in the queue
            pictq_size--;
            // tell video thread that it can go on...
            pictq_cond->wakeAll();
            pictq_mutex->unlock();


        }

    } else {
        // wait for a video_st to be created
        ptimer->start(SLEEP_DELAY);
    }
}

int64_t VideoFile::getCurrentFrameTime() const {

    return ((int64_t)(play_speed * video_current_pts / av_q2d(video_st->time_base)));
}

double VideoFile::getFrameRate() const {

    //    if (video_st && video_st->avg_frame_rate.den > 0) // never true !!!
    //        return ((double) (video_st->avg_frame_rate.num )/ (double) video_st->avg_frame_rate.den);
    //    else
    if (video_st && video_st->r_frame_rate.den > 0)
        return ((double) av_q2d(video_st->r_frame_rate) );
    else
        return 1.0;
}

void VideoFile::setOptionAllowDirtySeek(bool dirty) {

    seek_any = dirty;

}

void VideoFile::setOptionRevertToBlackWhenStop(bool black) {
    if (black)
        resetPicture = &blackPicture;
    else
        resetPicture = &firstPicture;
    // if the option is toggled while being stopped, then we should show the good frame now!
    if (quit)
        emit frameReady(-1);
}

void VideoFile::seekToPosition(int64_t t) {

    if (pFormatCtx && !quit && !seek_req) {
        seek_pos = qBound(getBegin(), t, getEnd());
        seek_req = true;
    }
}

void VideoFile::seekBySeconds(double ss) {

    if (pFormatCtx && !quit && !seek_req) {
        int64_t pos = (int64_t)((get_clock() + ss) * AV_TIME_BASE * play_speed);
        pos = av_rescale_q(pos, AV_TIME_BASE_Q, video_st->time_base);

        seekToPosition(pos);
    }
}


void VideoFile::seekByFrames(int64_t ss) {

    if (pFormatCtx && !quit && !seek_req) {
        int64_t pos = getCurrentFrameTime() + ss;

        seekToPosition(pos);
    }
}

//ASAF
int VideoFile::getTimeSnFromFrame(int64_t t) const {

    double time = (double) (t- video_st->start_time) * av_q2d(video_st->time_base) / play_speed;
    int s = (int) time;
    time -= s;
    return s;
}
//

QString VideoFile::getTimeFromFrame(int64_t t) const {

    double time = (double) (t- video_st->start_time) * av_q2d(video_st->time_base) / play_speed;
    int s = (int) time;
    time -= s;
    int h = s / 3600;
    int m = (s % 3600) / 60;
    s = (s % 3600) % 60;
    int ds = (int) (time * 100.0);
    return QString("%1h %2m %3.%4s").arg(h, 2).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(ds, 2, 10, QChar('0'));
}


QString VideoFile::getExactFrameFromFrame(int64_t t) const {
    if ( video_st->nb_frames > 0 )
        return (QString("Frame; %1").arg( (int)( video_st->nb_frames * double(t - video_st->start_time) / double(video_st->duration) ) ) );
    else
        return (QString("Frame; %1").arg( (int)( double (t - video_st->start_time) / getFrameRate()) ));

}

int64_t VideoFile::getFrameFromTime(QString t) const {

    QStringList list1 = t.split(":");

    if (list1.size() > 3) {
        int h = list1.at(0).toInt();
        int m = list1.at(1).toInt();
        double s = list1.at(2).toDouble();

        double time = (double) h * 3600.0 + (double) m * 60 +  s ;
        time = (time * play_speed) / av_q2d(video_st->time_base);
        return ((int64_t) time);
    }

    return -1;
}

int64_t VideoFile::getBegin() const {
    return ((video_st && video_st->start_time != (int64_t) AV_NOPTS_VALUE) ? video_st->start_time : 0);
}

double VideoFile::getDuration() const {

    //    return (pFormatCtx ? (double) pFormatCtx->duration / (double) AV_TIME_BASE : 0.0);
    AVRational a;
    a.num = 1;
    a.den = AV_TIME_BASE;

    return (pFormatCtx ? (double)pFormatCtx->duration * av_q2d(a) : 0.0);
}

int64_t VideoFile::getEnd() const {

    if (!video_st)
        return 0;

    // normal way to get duration of the video stream
    if ( video_st->duration != (int64_t) AV_NOPTS_VALUE)
        return video_st->duration + getBegin();

    //	qDebug("end = duration %d", av_rescale_q(pFormatCtx->duration, AV_TIME_BASE_Q, video_st->time_base) + getBegin());

    // if former failed, try this
    AVRational a;
    a.num = 1;
    a.den = AV_TIME_BASE;

    return av_rescale_q(pFormatCtx->duration, a, video_st->time_base) + getBegin();
}

void VideoFile::setMarkIn(int64_t t) {
    mark_in = qBound(getBegin(), t, mark_out);
    emit markingChanged();
}

void VideoFile::setMarkOut(int64_t t) {
    mark_out = qBound(mark_in, t, getEnd());
    emit markingChanged();
}


double VideoFile::getStreamAspectRatio() const {
    double aspect_ratio = 0;

    if (video_st->sample_aspect_ratio.num)
        aspect_ratio = av_q2d(video_st->sample_aspect_ratio);
    else if (video_st->codec->sample_aspect_ratio.num)
        aspect_ratio = av_q2d(video_st->codec->sample_aspect_ratio);
    else
        aspect_ratio = 0;
    if (aspect_ratio <= 0.0)
        aspect_ratio = 1.0;
    aspect_ratio *= (double)video_st->codec->width / video_st->codec->height;

    return aspect_ratio;
}

/**
 *
 * ParsingThread
 *
 */
void ParsingThread::run() {

    AVPacket pkt1, *packet = &pkt1;

    while (is && !is->quit) {

        if (is->pause_video != is->pause_video_last) {
            is->pause_video_last = is->pause_video;
            if (is->pause_video)
                av_read_pause(is->pFormatCtx);
            else
                av_read_play(is->pFormatCtx);
        }

        // seek stuff goes here
        if (is->seek_req) {

            int flags = 0;
            if ( is->seek_any )
                flags |= AVSEEK_FLAG_ANY;
            if ( is->seek_backward || is->seek_pos <= is->getCurrentFrameTime() )
                flags |= AVSEEK_FLAG_BACKWARD;

            if (av_seek_frame(is->pFormatCtx, is->videoStream, is->seek_pos, flags) < 0) {
                qDebug()<< is->filename << tr("|Could not seek to frame (%1); jumping where I can!").arg(is->seek_pos);
            }
            // after seek,  we'll have to flush buffers
            if (!is->videoq.flush())
                qWarning()<< is->filename << tr("|Flushing error.");

            is->seek_backward = false;
            is->seek_req = false;
            //qDebug()<<"Finished-SeekVideo time:"<<QDateTime().currentDateTime().toString("hh:mm:ss.zzz");
        }

        /* if the queue is full, no need to read more */
        if (is->videoq.isFull()) {
            msleep(SLEEP_DELAY);
            continue;
        }

        // avoid reading packet if we know its the end of the file!
        if ( url_feof(is->pFormatCtx->pb) ) {
            msleep(SLEEP_DELAY);
            continue;
        }

        // MAIN call ; reading the frame
        if (av_read_frame(is->pFormatCtx, packet) < 0) {
            // if could NOT read full frame, was it an error?
            if (is->pFormatCtx->pb->error) {
                qWarning()<< is->filename << tr("|Could not read frame.");
                break;
            }
            // no error; just wait a bit for the end of the packet and continue
            msleep(SLEEP_DELAY);
            continue;
        }

        // 1) test if it was NOT a video stream packet : if yes, the OR ignores the second part and frees the packet
        // 2) if no, the OR tests the second part, which puts the video packet in the queue
        // 3) if this fails we free the packet anyway
        if ( packet->stream_index != is->videoStream || !is->videoq.put(packet) )
            // we need to free the packet if it was not put in the queue
            av_free_packet(packet);

    } // quit

    // request flushing of the video queue (this will end the decoding thread)
    if (!is->videoq.flush())
        qWarning()<< is->filename << tr("|Flushing error at end.");

}

void VideoFile::queue_picture(AVFrame *pFrame, double pts) {

    /* wait until we have space for a new pic */
    pictq_mutex->lock();
    while (pictq_size >= (VIDEO_PICTURE_QUEUE_SIZE - 1) && !quit)
        pictq_cond->wait(pictq_mutex); // the condition is released in VideoFile::video_refresh_timer()
    pictq_mutex->unlock();

    if (pictq_skip) {
        pictq_skip = false;
        return;
    }

    // if have to quit, discard the rest but let the video thread continue
    if (quit) {
        pictq_mutex->lock();
        pictq_size = 0;
        pictq_mutex->unlock();
        return;
    }

    // set vp as the picture in the queue to be written
    // (write index is set to 0 initially)
    VideoPicture *vp = &pictq[pictq_windex];
    if (vp->isAllocated() && pictq_size <= VIDEO_PICTURE_QUEUE_SIZE) {

        AVPicture *picture = (AVPicture*)pFrame;

        if (pFrame->interlaced_frame) {
            // create temporary picture the first time we need it
            if (deinterlacing_buffer == NULL) {
                int size = avpicture_get_size(video_st->codec->pix_fmt, video_st->codec->width, video_st->codec->height);
                deinterlacing_buffer= (uint8_t *)av_malloc(size);
                avpicture_fill(&deinterlacing_picture, deinterlacing_buffer, video_st->codec->pix_fmt, video_st->codec->width, video_st->codec->height);
            }

            // try to deinterlace into the temporary picture
            if(avpicture_deinterlace(&deinterlacing_picture, picture, video_st->codec->pix_fmt, video_st->codec->width, video_st->codec->height) == 0)
                // if deinterlacing was successfull, use it
                picture = &deinterlacing_picture;
        }

        // Fill the Video Picture queue with the current frame
        vp->useYUVData = this->useYUVData;
        vp->skipNextFrame = this->skipNextFrame;
        vp->fill(picture, pts);

        // set to write indexto next in queue
        if (++pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)
            pictq_windex = 0;

        /* now we inform our display thread that we have a pic ready */
        pictq_mutex->lock();
        pictq_size++;
        pictq_mutex->unlock();
    }

}

/**
 * compute the exact PTS for the picture if it is omitted in the stream
 * @param pts1 the dts of the pkt / pts of the frame
 */
double VideoFile::synchronize_video(AVFrame *src_frame, double pts1) {

    // static variables ; no need to re-allocate each time function is called...
    static double frame_delay = 0.0, pts = 0.0;

    pts = pts1;

    if (pts >= 0)
        /* if we have pts, set video clock to it */
        video_clock = pts;
    else
        /* if we aren't given a pts, set it to the clock */
        // this happens rarely (I noticed it on last frame)
        pts = video_clock;

    frame_delay = av_q2d(video_st->codec->time_base);

    /* for MPEG2, the frame can be repeated, so we update the clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);

    /* update the video clock */
    video_clock += frame_delay;

    return pts;
}


/**
 * DecodingThread
 */

void DecodingThread::run() {

    AVPacket pkt1, *packet = &pkt1;
    int len1 = 0, frameFinished = 0;
    double pts = 0.0;					// Presentation time stamp
    int64_t dts = 0;					// Decoding time stamp

    while (is) {
        // sleep a bit if paused
        if (is->pause_video && !is->quit) {
            msleep(10);
            continue;
        }

        // get front packet (not blocking if we are quitting)
        if (!is->videoq.get(packet, !is->quit)) {
            // this is the exit condition
            break;
        }

        // special case of flush packet
        if (is->videoq.isFlush(packet)) {
            // means we have to flush buffers
            avcodec_flush_buffers(is->video_st->codec);
            // go on to next packet
            continue;
        }

        // Decode video frame
        is->video_st->codec->reordered_opaque = packet->pts;

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52,30,0)
        len1 = avcodec_decode_video2(is->video_st->codec, _pFrame, &frameFinished, packet);
#else
        len1 = avcodec_decode_video(is->video_st->codec, _pFrame, &frameFinished, packet->data, packet->size);
#endif

        // get decompression time stamp
        // this is the code in ffplay:
        dts = 0.0;
        if (packet->dts != (int64_t) AV_NOPTS_VALUE) {
            dts = packet->dts;
        } else if (_pFrame->reordered_opaque != (int64_t) AV_NOPTS_VALUE) {
            dts = _pFrame->reordered_opaque;
        }

        // Did we get a full video frame?
        if (frameFinished) {
            // compute presentation time stamp
            pts = dts * av_q2d(is->video_st->time_base) / is->play_speed;
            pts = is->synchronize_video(_pFrame, pts);
            // yes, frame is ready ; add it to the queue of pictures
            is->queue_picture(_pFrame, pts);
        }

        // test if this was the last frame (also for unfinished frames)
        // we test after the above queue_picture : the queue is empty only if we really get to the end
        if ( dts >= is->mark_out  || (is->videoq.isEmpty() && dts + packet->duration >= is->getEnd() - 1) ){
            // if no more frames because we approach the upper limit of frames and there is no more in the queue
            // react according to loop mode
            if (!is->loop_video) {
                // if loop mode off, stop at the end (mark_out >= getEnd)
                is->stop();
                is->mark_stop = is->mark_out; // this ensures the start will jump back to beginning
            } else  // if loop more on, loop to mark in
                is->seekToPosition(is->mark_in);

        }

        // packet was decoded, should be removed
        av_free_packet(packet);
    }

}

void VideoFile::pause(bool pause) {

    if (!quit && pause != pause_video) {
        pause_video = pause;

        if (!pause_video) {
            video_current_pts = get_clock();
            frame_timer += (av_gettime() - video_current_pts_time) / (double) AV_TIME_BASE;

            ptimer->start(10);
        }

        emit paused(pause);
    }
}

double VideoFile::get_clock() {

    double delta;

    if (pause_video)
        delta = 0.0;
    else
        delta = (double) (av_gettime() - video_current_pts_time) / (double) AV_TIME_BASE;

    return video_current_pts + delta;
}


/**
 * VideoFile::PacketQueue
 */

// put a packet at the tail of the queue.
bool VideoFile::PacketQueue::put(AVPacket *pkt) {

    if (pkt != flush_pkt && av_dup_packet(pkt) < 0)
        return false;

    AVPacketList *pkt1 = (AVPacketList*) av_malloc(sizeof(AVPacketList));
    Q_CHECK_PTR(pkt1);

    if (!pkt1)
        return false;

    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    mutex->lock();

    if (!last_pkt)
        first_pkt = pkt1;
    else
        last_pkt->next = pkt1;

    last_pkt = pkt1;
    nb_packets++;
    size += pkt1->pkt.size;

    cond->wakeAll();
    mutex->unlock();

    return true;
}

// gets the front of the queue
// this blocks and repeats if block parameter is true
bool VideoFile::PacketQueue::get(AVPacket *pkt, bool block) {

    bool ret = false;
    mutex->lock();

    for (;;) {

        AVPacketList *pkt1 = first_pkt;
        if (pkt1) {
            first_pkt = pkt1->next;
            if (!first_pkt)
                last_pkt = NULL;
            nb_packets--;
            size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);

            ret = true;
            break;
        } else if (!block) {
            break;
        } else {
            cond->wait(mutex);
        }
    }

    mutex->unlock();
    return ret;
}

VideoFile::PacketQueue::~PacketQueue() {
    delete mutex;
    delete cond;
}

AVPacket *VideoFile::PacketQueue::flush_pkt = 0;

VideoFile::PacketQueue::PacketQueue() {

    if (!flush_pkt) {
        flush_pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
        Q_CHECK_PTR(flush_pkt);
        av_init_packet(flush_pkt);
        uint8_t *msg = (uint8_t *) av_malloc(sizeof(uint8_t));
        Q_CHECK_PTR(msg);
        msg[0] = 'F';
        flush_pkt->data = msg;
    }

    last_pkt = NULL;
    first_pkt = NULL;
    nb_packets = 0;
    size = 0;
    mutex = new QMutex();
    Q_CHECK_PTR(mutex);
    cond = new QWaitCondition();
    Q_CHECK_PTR(cond);
}

bool VideoFile::PacketQueue::flush() {
    AVPacketList *pkt, *pkt1;

    mutex->lock();

    for (pkt = first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    last_pkt = NULL;
    first_pkt = NULL;
    nb_packets = 0;
    size = 0;

    mutex->unlock();

    return put(flush_pkt);
}

bool VideoFile::PacketQueue::isFlush(AVPacket *pkt) const {
    return (pkt->data == flush_pkt->data);
}


bool VideoFile::PacketQueue::isFull() const {
    return (size > MAX_VIDEOQ_SIZE);
}

int VideoFile::roundPowerOfTwo(int v){

    int k;
    if (v == 0)
        return 1;
    for (k = sizeof(int) * 8 - 1; ((static_cast<int>(1U) << k) & v) == 0; k--);
    if (((static_cast<int>(1U) << (k - 1)) & v) == 0)
        return static_cast<int>(1U) << k;
    return static_cast<int>(1U) << (k + 1);

}

void VideoFile::displayFormatsCodecsInformation(QString iconfile) {

    if (!ffmpegregistered) {
        avcodec_register_all();
        av_register_all();
        ffmpegregistered = true;
    }

    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTreeWidget *availableFormatsTreeWidget;
    QLabel *label_2;
    QTreeWidget *availableCodecsTreeWidget;
    QDialogButtonBox *buttonBox;

    QDialog *ffmpegInfoDialog = new QDialog;
    QIcon icon;
    icon.addFile(iconfile, QSize(), QIcon::Normal, QIcon::Off);
    ffmpegInfoDialog->setWindowIcon(icon);
    ffmpegInfoDialog->resize(510, 588);
    verticalLayout = new QVBoxLayout(ffmpegInfoDialog);
    label = new QLabel(ffmpegInfoDialog);
    label_2 = new QLabel(ffmpegInfoDialog);

    availableFormatsTreeWidget = new QTreeWidget(ffmpegInfoDialog);
    availableFormatsTreeWidget->setProperty("showDropIndicator", QVariant(false));
    availableFormatsTreeWidget->setAlternatingRowColors(true);
    availableFormatsTreeWidget->setRootIsDecorated(false);
    availableFormatsTreeWidget->header()->setVisible(true);

    availableCodecsTreeWidget = new QTreeWidget(ffmpegInfoDialog);
    availableCodecsTreeWidget->setAlternatingRowColors(true);
    availableCodecsTreeWidget->setRootIsDecorated(false);
    availableCodecsTreeWidget->header()->setVisible(true);

    buttonBox = new QDialogButtonBox(ffmpegInfoDialog);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);

    ffmpegInfoDialog->setWindowTitle(tr("FFMPEG formats and codecs"));
    label->setText(tr("Compiled with libavcodec %1.%2.%3 \n\nReadable VIDEO codecs").arg(LIBAVCODEC_VERSION_MAJOR).arg(LIBAVCODEC_VERSION_MINOR).arg(LIBAVCODEC_VERSION_MICRO));
    label_2->setText(tr("Available formats"));

    QTreeWidgetItem *title = availableFormatsTreeWidget->headerItem();
    title->setText(1, tr("Description"));
    title->setText(0, tr("Name"));

    title = availableCodecsTreeWidget->headerItem();
    title->setText(1, tr("Description"));
    title->setText(0, tr("Name"));

    QObject::connect(buttonBox, SIGNAL(accepted()), ffmpegInfoDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), ffmpegInfoDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(ffmpegInfoDialog);

    QTreeWidgetItem *formatitem;

    AVInputFormat *ifmt = NULL;
    AVCodec *p = NULL, *p2;
    const char *last_name;

    last_name = "000";
    for (;;) {
        const char *name = NULL;
        const char *long_name = NULL;

        while ((ifmt = av_iformat_next(ifmt))) {
            if ((name == NULL || strcmp(ifmt->name, name) < 0) && strcmp(ifmt->name, last_name) > 0) {
                name = ifmt->name;
                long_name = ifmt->long_name;
            }
        }
        if (name == NULL)
            break;
        last_name = name;

        formatitem = new QTreeWidgetItem(availableFormatsTreeWidget);
        formatitem->setText(0, QString(name));
        formatitem->setText(1, QString(long_name));
        availableFormatsTreeWidget->addTopLevelItem(formatitem);
    }

    last_name = "000";
    for (;;) {
        int decode = 0;
        int cap = 0;

        p2 = NULL;
        while ((p = av_codec_next(p))) {
            if ((p2 == NULL || strcmp(p->name, p2->name) < 0) && strcmp(p->name, last_name) > 0) {
                p2 = p;
                decode = cap = 0;
            }
            if (p2 && strcmp(p->name, p2->name) == 0) {
                if (p->decode)
                    decode = 1;
                cap |= p->capabilities;
            }
        }
        if (p2 == NULL)
            break;
        last_name = p2->name;

        if (decode && p2->type == CODEC_TYPE_VIDEO) {
            formatitem = new QTreeWidgetItem(availableCodecsTreeWidget);
            formatitem->setText(0, QString(p2->name));
            formatitem->setText(1, QString(p2->long_name));
            availableCodecsTreeWidget->addTopLevelItem(formatitem);
        }
    }

    verticalLayout->addWidget(label);
    verticalLayout->addWidget(availableCodecsTreeWidget);
    verticalLayout->addWidget(label_2);
    verticalLayout->addWidget(availableFormatsTreeWidget);
    verticalLayout->addWidget(buttonBox);

    ffmpegInfoDialog->exec();

    delete verticalLayout;
    delete label;
    delete label_2;
    delete availableFormatsTreeWidget;
    delete availableCodecsTreeWidget;
    delete buttonBox;

    delete ffmpegInfoDialog;
}



int VideoFile::getBrightness(){
    int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;

    if ( -1 != sws_getColorspaceDetails(img_convert_ctx, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {

        return (((brightness*100) + (1<<15))>>16);
    }
    else return 0;

}
int VideoFile::getContrast(){
    int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;

    if ( -1 != sws_getColorspaceDetails(img_convert_ctx, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {

        return ((((contrast  *100) + (1<<15))>>16) - 100);
    }
    else return 0;

}
int VideoFile::getSaturation(){
    int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;

    if ( -1 != sws_getColorspaceDetails(img_convert_ctx, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {

        return ((((saturation*100) + (1<<15))>>16) - 100);
    }
    else return 0;
}


void VideoFile::setBrightness(int b){

    int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;
    if ( b >= -100 && b <= 100 &&  -1 != sws_getColorspaceDetails(img_convert_ctx, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {
        // ok, got all the details, modify one:
        brightness = (( b <<16) + 50)/100;
        // apply it
        sws_setColorspaceDetails(img_convert_ctx, inv_table, srcrange, table, dstrange, brightness, contrast, saturation);

        emit prefilteringChanged();
    }
}


void VideoFile::setContrast(int c){
    //QMessageBox::information(0,"",QString::number(c),0,0);
    int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;
    if ( c >= -99 && c <= 100 &&  -1 != sws_getColorspaceDetails(img_convert_ctx, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {
        // ok, got all the details, modify one:
        contrast   = ((( c +100)<<16) + 50)/100;
        // apply it
        sws_setColorspaceDetails(img_convert_ctx, inv_table, srcrange, table, dstrange, brightness, contrast, saturation);

        emit prefilteringChanged();
    }
}



void VideoFile::setSaturation(int s){

    int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;
    if ( s >= -100 && s <= 100 &&  -1 != sws_getColorspaceDetails(img_convert_ctx, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {
        // ok, got all the details, modify one:
        saturation = ((( s +100)<<16) + 50)/100;
        // apply it
        sws_setColorspaceDetails(img_convert_ctx, inv_table, srcrange, table, dstrange, brightness, contrast, saturation);

        emit prefilteringChanged();
    }

}


#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52,30,0)

QString VideoFile::getPixelFormatName(PixelFormat ffmpegPixelFormat) const {

    if (ffmpegPixelFormat == PIX_FMT_NONE && video_st)
        ffmpegPixelFormat = video_st->codec->pix_fmt;

    QString pfn(av_pix_fmt_descriptors[ffmpegPixelFormat].name);
    pfn += QString(" (%1bpp)").arg(av_get_bits_per_pixel( &av_pix_fmt_descriptors[ffmpegPixelFormat]));

    return pfn;
}
#else
QString VideoFile::getPixelFormatName(PixelFormat ffmpegPixelFormat) const {

    if (ffmpegPixelFormat == PIX_FMT_NONE)
        ffmpegPixelFormat =video_st->codec->pix_fmt;

    switch (ffmpegPixelFormat ) {

    case PIX_FMT_YUV420P: return QString("planar YUV 4:2:0, 12bpp");
    case PIX_FMT_YUYV422: return QString("packed YUV 4:2:2, 16bpp");
    case PIX_FMT_RGB24:
    case PIX_FMT_BGR24: return QString("packed RGB 8:8:8, 24bpp");
    case PIX_FMT_YUV422P: return QString("planar YUV 4:2:2, 16bpp");
    case PIX_FMT_YUV444P: return QString("planar YUV 4:4:4, 24bpp");
    case PIX_FMT_RGB32: return QString("packed RGB 8:8:8, 32bpp");
    case PIX_FMT_YUV410P: return QString("planar YUV 4:1:0,  9bpp");
    case PIX_FMT_YUV411P: return QString("planar YUV 4:1:1, 12bpp");
    case PIX_FMT_RGB565: return QString("packed RGB 5:6:5, 16bpp");
    case PIX_FMT_RGB555: return QString("packed RGB 5:5:5, 16bpp");
    case PIX_FMT_GRAY8: return QString("Y,  8bpp");
    case PIX_FMT_MONOWHITE:
    case PIX_FMT_MONOBLACK: return QString("Y,  1bpp");
    case PIX_FMT_PAL8: return QString("RGB32 palette, 8bpp");
    case PIX_FMT_YUVJ420P: return QString("planar YUV 4:2:0, 12bpp (JPEG)");
    case PIX_FMT_YUVJ422P: return QString("planar YUV 4:2:2, 16bpp (JPEG)");
    case PIX_FMT_YUVJ444P: return QString("planar YUV 4:4:4, 24bpp (JPEG)");
    case PIX_FMT_XVMC_MPEG2_MC: return QString("XVideo Motion Acceleration (MPEG2)");
    case PIX_FMT_UYVY422: return QString("packed YUV 4:2:2, 16bpp");
    case PIX_FMT_UYYVYY411: return QString("packed YUV 4:1:1, 12bpp");
    case PIX_FMT_BGR32: return QString("packed RGB 8:8:8, 32bpp");
    case PIX_FMT_BGR565: return QString("packed RGB 5:6:5, 16bpp");
    case PIX_FMT_BGR555: return QString("packed RGB 5:5:5, 16bpp");
    case PIX_FMT_BGR8: return QString("packed RGB 3:3:2, 8bpp");
    case PIX_FMT_BGR4: return QString("packed RGB 1:2:1, 4bpp");
    case PIX_FMT_BGR4_BYTE: return QString("packed RGB 1:2:1,  8bpp");
    case PIX_FMT_RGB8: return QString("packed RGB 3:3:2,  8bpp");
    case PIX_FMT_RGB4: return QString("packed RGB 1:2:1,  4bpp");
    case PIX_FMT_RGB4_BYTE: return QString("packed RGB 1:2:1,  8bpp");
    case PIX_FMT_NV12:
    case PIX_FMT_NV21: return QString("planar YUV 4:2:0, 12bpp");
    case PIX_FMT_RGB32_1:
    case PIX_FMT_BGR32_1: return QString("packed RGB 8:8:8, 32bpp");
    case PIX_FMT_GRAY16BE:
    case PIX_FMT_GRAY16LE: return QString("Y, 16bpp");
    case PIX_FMT_YUV440P: return QString("planar YUV 4:4:0");
    case PIX_FMT_YUVJ440P: return QString("planar YUV 4:4:0 (JPEG)");
    case PIX_FMT_YUVA420P: return QString("planar YUV 4:2:0, 20bpp");
    case PIX_FMT_VDPAU_H264: return QString("H.264 HW");
    case PIX_FMT_VDPAU_MPEG1: return QString("MPEG-1 HW");
    case PIX_FMT_VDPAU_MPEG2: return QString("MPEG-2 HW");
    case PIX_FMT_VDPAU_WMV3: return QString("WMV3 HW");
    case PIX_FMT_VDPAU_VC1: return QString("VC-1 HW");
    case PIX_FMT_RGB48BE: return QString("packed RGB 16:16:16, 48bpp");
    case PIX_FMT_RGB48LE: return QString("packed RGB 16:16:16, 48bpp");
    case PIX_FMT_VAAPI_MOCO:
    case PIX_FMT_VAAPI_IDCT:
    case PIX_FMT_VAAPI_VLD: return QString("HW acceleration through VA API");
    default:
        return QString("unknown");
    }
}
#endif
