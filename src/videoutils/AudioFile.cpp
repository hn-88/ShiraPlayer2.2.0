
/* Copyright (c) 2001 Fabrice Bellard
*  Copyright (C) 2011 Asaf Yurdakul
*
* This file is part of FFmpeg.
*
* FFmpeg is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* ShiraPlayer is a trademark of Sureyyasoft.
*/

#include "AudioFile.h"
#include "cmdutils.h"

#ifdef _WIN32
#undef main
#endif

#undef exit
#undef printf
#undef fprintf

#include <QMessageBox>
#include <QDebug>

/* packet queue handling */
static void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
    packet_queue_put(q, &flush_pkt);
}

static void packet_queue_flush(PacketQueue *q)
{
    AVPacketList *pkt, *pkt1;

    SDL_LockMutex(q->mutex);
    for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    SDL_UnlockMutex(q->mutex);
}

static void packet_queue_end(PacketQueue *q)
{
    packet_queue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pkt1;

    /* duplicate the packet */
    if (pkt!=&flush_pkt && av_dup_packet(pkt) < 0)
        return -1;

    pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;


    SDL_LockMutex(q->mutex);

    if (!q->last_pkt)

        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);
    /* XXX: should duplicate packet data in DV case */
    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
    return 0;
}

static void packet_queue_abort(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);

    q->abort_request = 1;

    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for(;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

/* get the current audio clock value */
static double get_audio_clock(VideoState *is)
{
    double pts;
    int hw_buf_size, bytes_per_sec;
    pts = is->audio_clock;
    hw_buf_size = audio_write_get_buf_size(is);
    bytes_per_sec = 0;
    if (is->audio_st) {
        bytes_per_sec = is->audio_st->codec->sample_rate *
                2 * is->audio_st->codec->channels;
    }
    if (bytes_per_sec)
        pts -= (double)hw_buf_size / bytes_per_sec;
    return pts;
}

/* get the current video clock value */
static double get_video_clock(VideoState *is)
{
    if (is->paused) {
        return is->video_current_pts;
    } else {
        return is->video_current_pts_drift + av_gettime() / 1000000.0;
    }
}

/* get the current external clock value */
static double get_external_clock(VideoState *is)
{
    int64_t ti;
    ti = av_gettime();
    return is->external_clock + ((ti - is->external_clock_time) * 1e-6);
}

/* get the current master clock value */
static double get_master_clock(VideoState *is)
{
    double val;

    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st)
            val = get_video_clock(is);
        else
            val = get_audio_clock(is);
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st)
            val = get_audio_clock(is);
        else
            val = get_video_clock(is);
    } else {
        val = get_external_clock(is);
    }
    return val;
}

/* seek in the stream */
static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_rel = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
            is->seek_flags |= AVSEEK_FLAG_BYTE;
        is->seek_req = 1;
    }
}

/* pause or resume the video */
static void stream_pause(VideoState *is)
{
    if (is->paused) {
        is->frame_timer += av_gettime() / 1000000.0 + is->video_current_pts_drift - is->video_current_pts;
        if(is->read_pause_return != AVERROR(ENOSYS)){
            is->video_current_pts = is->video_current_pts_drift + av_gettime() / 1000000.0;
        }
        is->video_current_pts_drift = is->video_current_pts - av_gettime() / 1000000.0;
    }
    is->paused = !is->paused;
}

static void stream_close(VideoState *is)
{
    is->abort_request = 1;
    SDL_WaitThread(is->parse_tid, NULL);
    SDL_WaitThread(is->refresh_tid, NULL);

    SDL_DestroyMutex(is->pictq_mutex);
    SDL_DestroyCond(is->pictq_cond);

    av_free(is);
}

static void do_exit(void)
{
    int i;
    if (cur_stream) {
        stream_close(cur_stream);
        cur_stream = NULL;
    }
    for (i = 0; i < AVMEDIA_TYPE_NB; i++)
        av_free(avcodec_opts[i]);
    av_free(avformat_opts);
    av_free(sws_opts);

    if (show_status)
        printf("\n");
    SDL_Quit();
    exit(0);
}

static int video_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    AVFrame *frame= avcodec_alloc_frame();
    int ret;

    for(;;) {

        AVPacket pkt;
        while (is->paused && !is->videoq.abort_request)
            SDL_Delay(10);

        ret = packet_queue_get(&is->videoq, &pkt, 1);

        av_free_packet(&pkt);

        if (ret < 0)
            goto the_end;

        if (step)
            if (cur_stream)
                stream_pause(cur_stream);
    }
the_end:
    av_free(frame);
    return 0;
}


/* return the new audio buffer size (samples can be added or deleted
   to get better sync if video or external master clock) */
static int synchronize_audio(VideoState *is, short *samples,
                             int samples_size1, double pts)
{
    int n, samples_size;
    double ref_clock;

    n = 2 * is->audio_st->codec->channels;
    samples_size = samples_size1;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (((is->av_sync_type == AV_SYNC_VIDEO_MASTER && is->video_st) ||
         is->av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
        double diff, avg_diff;
        int wanted_size, min_size, max_size, nb_samples;

        ref_clock = get_master_clock(is);
        diff = get_audio_clock(is) - ref_clock;

        if (diff < AV_NOSYNC_THRESHOLD) {
            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
            if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                is->audio_diff_avg_count++;
            } else {
                /* estimate the A-V difference */
                avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

                if (fabs(avg_diff) >= is->audio_diff_threshold) {
                    wanted_size = samples_size + ((int)(diff * is->audio_st->codec->sample_rate) * n);
                    nb_samples = samples_size / n;

                    min_size = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
                    max_size = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
                    if (wanted_size < min_size)
                        wanted_size = min_size;
                    else if (wanted_size > max_size)
                        wanted_size = max_size;

                    /* add or remove samples to correction the synchro */
                    if (wanted_size < samples_size) {
                        /* remove samples */
                        samples_size = wanted_size;
                    } else if (wanted_size > samples_size) {
                        uint8_t *samples_end, *q;
                        int nb;

                        /* add samples */
                        nb = (samples_size - wanted_size);
                        samples_end = (uint8_t *)samples + samples_size - n;
                        q = samples_end + n;
                        while (nb > 0) {
                            memcpy(q, samples_end, n);
                            q += n;
                            nb -= n;
                        }
                        samples_size = wanted_size;
                    }
                }

            }
        } else {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            is->audio_diff_avg_count = 0;
            is->audio_diff_cum = 0;
        }
    }

    return samples_size;
}

/* decode one audio frame and returns its uncompressed size */
static int audio_decode_frame(VideoState *is, double *pts_ptr)
{
    AVPacket *pkt_temp = &is->audio_pkt_temp;
    AVPacket *pkt = &is->audio_pkt;
    AVCodecContext *dec= is->audio_st->codec;
    int n, len1, data_size;
    double pts;

    for(;;) {
        /* NOTE: the audio packet can contain several frames */
        while (pkt_temp->size > 0) {
            data_size = sizeof(is->audio_buf1);
            len1 = avcodec_decode_audio3(dec,
                                         (int16_t *)is->audio_buf1, &data_size,
                                         pkt_temp);
            if (len1 < 0) {
                /* if error, we skip the frame */
                pkt_temp->size = 0;
                break;
            }

            pkt_temp->data += len1;
            pkt_temp->size -= len1;
            if (data_size <= 0)
                continue;

            if (dec->sample_fmt != is->audio_src_fmt) {
                if (is->reformat_ctx)
                    av_audio_convert_free(is->reformat_ctx);
                is->reformat_ctx= av_audio_convert_alloc(SAMPLE_FMT_S16, 1,
                                                         dec->sample_fmt, 1, NULL, 0);
                if (!is->reformat_ctx) {
                    fprintf(stderr, "Cannot convert %s sample format to %s sample format\n",
                            avcodec_get_sample_fmt_name(dec->sample_fmt),
                            avcodec_get_sample_fmt_name(SAMPLE_FMT_S16));
                    break;
                }
                is->audio_src_fmt= dec->sample_fmt;
            }

            if (is->reformat_ctx) {
                const void *ibuf[6]= {is->audio_buf1};
                void *obuf[6]= {is->audio_buf2};
                int istride[6]= {av_get_bits_per_sample_format(dec->sample_fmt)/8};
                int ostride[6]= {2};
                int len= data_size/istride[0];
                if (av_audio_convert(is->reformat_ctx, obuf, ostride, ibuf, istride, len)<0) {
                    printf("av_audio_convert() failed\n");
                    break;
                }
                is->audio_buf= is->audio_buf2;
                /* FIXME: existing code assume that data_size equals framesize*channels*2
                          remove this legacy cruft */
                data_size= len*2;
            }else{
                is->audio_buf= is->audio_buf1;
            }

            /* if no pts, then compute it */
            pts = is->audio_clock;
            *pts_ptr = pts;
            n = 2 * dec->channels;
            is->audio_clock += (double)data_size /
                    (double)(n * dec->sample_rate);

            return data_size;
        }

        /* free the current packet */
        if (pkt->data)
            av_free_packet(pkt);

        if (is->paused || is->audioq.abort_request) {
            return -1;
        }

        /* read next packet */
        if (packet_queue_get(&is->audioq, pkt, 1) < 0)
            return -1;
        if(pkt->data == flush_pkt.data){
            avcodec_flush_buffers(dec);
            continue;
        }

        pkt_temp->data = pkt->data;
        pkt_temp->size = pkt->size;

        /* if update the audio clock with the pts */
        if (pkt->pts != AV_NOPTS_VALUE) {
            is->audio_clock = av_q2d(is->audio_st->time_base)*pkt->pts;
        }
    }
}

/* get the current audio output buffer size, in samples. With SDL, we
   cannot have a precise information */
static int audio_write_get_buf_size(VideoState *is)
{
    return is->audio_buf_size - is->audio_buf_index;
}

/* prepare a new audio buffer */
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    VideoState *is = (VideoState *)opaque;
    int audio_size, len1;
    double pts;

    audio_callback_time = av_gettime();

    while (len > 0) {
        if (is->audio_buf_index >= is->audio_buf_size) {
            audio_size = audio_decode_frame(is, &pts);
            if (audio_size < 0) {
                /* if error, just output silence */
                is->audio_buf = is->audio_buf1;
                is->audio_buf_size = 1024;
                memset(is->audio_buf, 0, is->audio_buf_size);
            } else
            {
                audio_size = synchronize_audio(is, (int16_t *)is->audio_buf, audio_size,
                                               pts);
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
            len1 = len;
        //memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        SDL_MixAudio(stream, (uint8_t *)is->audio_buf +is->audio_buf_index, len1, volume);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}
static int opt_name_count;
void set_context_opts(void *ctx, void *opts_ctx, int flags)
{
    int i;
    for(i=0; i<opt_name_count; i++){
        char buf[256];
        const AVOption *opt;
        const char *str= av_get_string(opts_ctx, opt_names[i], &opt, buf, sizeof(buf));
        /* if an option with name opt_names[i] is present in opts_ctx then str is non-NULL */
        if(str && ((opt->flags & flags) == flags))
            av_set_string3(ctx, opt_names[i], str, 1, NULL);
    }
}


/* open a given stream. Return 0 if OK */
static int stream_component_open(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;
    AVCodec *codec;
    SDL_AudioSpec wanted_spec, spec;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    avctx = ic->streams[stream_index]->codec;

    /* prepare audio output */
    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        if (avctx->channels > 0) {
            avctx->request_channels = FFMIN(2, avctx->channels);
        } else {
            avctx->request_channels = 2;
        }
    }

    codec = avcodec_find_decoder(avctx->codec_id);
    avctx->debug_mv = debug_mv;
    avctx->debug = debug;
    avctx->workaround_bugs = workaround_bugs;
    avctx->lowres = lowres;
    if(lowres) avctx->flags |= CODEC_FLAG_EMU_EDGE;
    avctx->idct_algo= idct;
    if(fast) avctx->flags2 |= CODEC_FLAG2_FAST;
    avctx->skip_frame= skip_frame;
    avctx->skip_idct= skip_idct;
    avctx->skip_loop_filter= skip_loop_filter;
    avctx->error_recognition= error_recognition;
    avctx->error_concealment= error_concealment;
    avcodec_thread_init(avctx, thread_count);

    set_context_opts(avctx, avcodec_opts[avctx->codec_type], 0);

    if (!codec ||
            avcodec_open(avctx, codec) < 0)
        return -1;

    qDebug()<<QString("codec Name:%1").arg(codec->long_name);

    /* prepare audio output */
    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        wanted_spec.freq = avctx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = avctx->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = sdl_audio_callback;
        wanted_spec.userdata = is;
        if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
            fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
            return -1;
        }
        is->audio_hw_buf_size = spec.size;
        is->audio_src_fmt= SAMPLE_FMT_S16;
    }

    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch(avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_stream = stream_index;
        is->audio_st = ic->streams[stream_index];
        is->audio_buf_size = 0;
        is->audio_buf_index = 0;

        /* init averaging filter */
        is->audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        is->audio_diff_avg_count = 0;
        /* since we do not have a precise anough audio fifo fullness,
           we correct audio sync only if larger than this threshold */
        is->audio_diff_threshold = 2.0 * SDL_AUDIO_BUFFER_SIZE / avctx->sample_rate;

        memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
        packet_queue_init(&is->audioq);
        SDL_PauseAudio(0);
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_stream = stream_index;
        is->video_st = ic->streams[stream_index];

        packet_queue_init(&is->videoq);
        is->video_tid = SDL_CreateThread(video_thread, is);
        break;
    default:
        break;
    }
    return 0;
}

static void stream_component_close(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    avctx = ic->streams[stream_index]->codec;

    switch(avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        packet_queue_abort(&is->audioq);

        SDL_CloseAudio();

        packet_queue_end(&is->audioq);
        if (is->reformat_ctx)
            av_audio_convert_free(is->reformat_ctx);
        is->reformat_ctx = NULL;
        break;
    case AVMEDIA_TYPE_VIDEO:
        packet_queue_abort(&is->videoq);

        SDL_WaitThread(is->video_tid, NULL);

        packet_queue_end(&is->videoq);
        break;
    default:
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    avcodec_close(avctx);
    switch(avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_st = NULL;
        is->audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_st = NULL;
        is->video_stream = -1;
        break;
    default:
        break;
    }
}

/* since we have only one decoding thread, we can use a global
   variable instead of a thread local variable */
static VideoState *global_video_state;

static int decode_interrupt_cb(void)
{
    return (global_video_state && global_video_state->abort_request);
}

/* this thread gets the stream from the disk or the network */
static int decode_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    AVFormatContext *ic;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    int st_count[AVMEDIA_TYPE_NB]={0};
    int st_best_packet_count[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    AVFormatParameters params, *ap = &params;
    int eof=0;
    int pkt_in_play_range = 0;

    ic = avformat_alloc_context();

    memset(st_index, -1, sizeof(st_index));
    memset(st_best_packet_count, -1, sizeof(st_best_packet_count));
    is->video_stream = -1;
    is->audio_stream = -1;
    is->subtitle_stream = -1;

    global_video_state = is;
    url_set_interrupt_cb(decode_interrupt_cb);

    memset(ap, 0, sizeof(*ap));

    ap->prealloced_context = 1;
    ap->width = frame_width;
    ap->height= frame_height;
    AVRational* avr=new AVRational();
    avr->den = 25;avr->num = 1;
    ap->time_base= *avr;
    ap->pix_fmt = frame_pix_fmt;

    set_context_opts(ic, avformat_opts, AV_OPT_FLAG_DECODING_PARAM);

    err = av_open_input_file(&ic, is->filename, is->iformat, 0, ap);
    if (err < 0) {
        ret = -1;
        goto fail;
    }
    is->ic = ic;

    if(genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    err = av_find_stream_info(ic);
    if (err < 0) {
        fprintf(stderr, "%s: could not find codec parameters\n", is->filename);
        ret = -1;
        goto fail;
    }
    if(ic->pb)
        ic->pb->eof_reached= 0; //FIXME hack, ffplay maybe should not use url_feof() to test for the end

    if(seek_by_bytes<0)
        seek_by_bytes= !!(ic->iformat->flags & AVFMT_TS_DISCONT);

    /* if seeking requested, we execute it */
    if (start_time != AV_NOPTS_VALUE) {
        int64_t timestamp;

        timestamp = start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1,_I64_MIN, timestamp, _I64_MAX, 0);
        if (ret < 0) {
            fprintf(stderr, "%s: could not seek to position %0.3f\n",
                    is->filename, (double)timestamp / AV_TIME_BASE);
        }
    }

    for(i = 0; i < ic->nb_streams; i++) {
        AVStream *st= ic->streams[i];
        AVCodecContext *avctx = st->codec;
        ic->streams[i]->discard = AVDISCARD_ALL;
        if(avctx->codec_type >= (unsigned)AVMEDIA_TYPE_NB)
            continue;
        if(st_count[avctx->codec_type]++ != wanted_stream[avctx->codec_type] && wanted_stream[avctx->codec_type] >= 0)
            continue;

        if(st_best_packet_count[avctx->codec_type] >= st->codec_info_nb_frames)
            continue;
        st_best_packet_count[avctx->codec_type]= st->codec_info_nb_frames;

        switch(avctx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            if (!audio_disable)
                st_index[AVMEDIA_TYPE_AUDIO] = i;
            break;
        case AVMEDIA_TYPE_VIDEO:
            if (!video_disable)
                st_index[avctx->codec_type] = i;
            break;
        default:
            break;
        }
    }

    /* open the streams */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
    }

    ret=-1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret= stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
    }

    if (is->video_stream < 0 && is->audio_stream < 0) {
        fprintf(stderr, "%s: could not open codecs\n", is->filename);
        ret = -1;
        goto fail;
    }

    for(;;) {
        if (is->abort_request)
            break;
        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
                is->read_pause_return= av_read_pause(ic);
            else
                av_read_play(ic);
        }
        if (is->seek_req) {
            int64_t seek_target= is->seek_pos;
            int64_t seek_min= is->seek_rel > 0 ? seek_target - is->seek_rel + 2: _I64_MIN;
            int64_t seek_max= is->seek_rel < 0 ? seek_target - is->seek_rel - 2: _I64_MAX;
            //FIXME the +-2 is due to rounding being not done in the correct direction in generation
            //      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0) {
                fprintf(stderr, "%s: error while seeking\n", is->ic->filename);
            }else{
                if (is->audio_stream >= 0) {
                    packet_queue_flush(&is->audioq);
                    packet_queue_put(&is->audioq, &flush_pkt);
                }
//                if (is->subtitle_stream >= 0) {
//                    packet_queue_flush(&is->subtitleq);
//                    packet_queue_put(&is->subtitleq, &flush_pkt);
//                }
//                if (is->video_stream >= 0) {
//                    packet_queue_flush(&is->videoq);
//                    packet_queue_put(&is->videoq, &flush_pkt);
//                }
            }
            is->seek_req = 0;
            eof= 0;
        }

        //        // seek stuff goes here
        //        if (is->seek_req) {
        //            if (av_seek_frame(is->ic, is->audio_stream, is->seek_pos, AVSEEK_FLAG_BACKWARD) < 0) {
        //                qDebug()<<QString("%0: error while seeking\n").arg(is->ic->filename);
        //            }
        ////            if (avformat_seek_file(is->ic,is->audio_stream,0,is->seek_pos,is->seek_pos,AVSEEK_FLAG_BACKWARD)<0 )
        ////            {
        ////                            qDebug()<<QString("%0: error while seeking\n").arg(is->ic->filename);
        ////            }
        //            else
        //            {
        //                //after seek,  we'll have to flush buffers
        //                //            if (!is->videoq.flush())
        //                //                fprintf(stderr, "%s: error while seeking\n", is->ic->filename);
        //                if (is->audio_stream >= 0) {
        //                    packet_queue_flush(&is->audioq);
        //                    packet_queue_put(&is->audioq, &flush_pkt);
        //                }
        //            }
        //            //is->seek_backward = false;
        //            is->seek_req = false;
        //            eof= 0;
        //        }

        /* if the queue are full, no need to read more */
        if (   is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE
               || (   (is->audioq   .size  > MIN_AUDIOQ_SIZE || is->audio_stream<0)
                      && (is->videoq   .nb_packets > MIN_FRAMES || is->video_stream<0)
                      && (is->subtitleq.nb_packets > MIN_FRAMES || is->subtitle_stream<0))) {
            /* wait 10 ms */
            SDL_Delay(10);
            continue;
        }
        if(url_feof(ic->pb) || eof) {
            if(is->video_stream >= 0){
                av_init_packet(pkt);
                pkt->data=NULL;
                pkt->size=0;
                pkt->stream_index= is->video_stream;
                packet_queue_put(&is->videoq, pkt);
            }
            SDL_Delay(10);
            if(is->audioq.size + is->videoq.size + is->subtitleq.size ==0){
                if(loop!=1 && (!loop || --loop)){
                    stream_seek(cur_stream, start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
                }else if(autoexit){
                    ret=AVERROR_EOF;
                    goto fail;
                }
            }
            continue;
        }
        ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF)
                eof=1;
            if (url_ferror(ic->pb))
                break;
            SDL_Delay(100); /* wait for user event */
            continue;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                (pkt->pts - ic->streams[pkt->stream_index]->start_time) *
                av_q2d(ic->streams[pkt->stream_index]->time_base) -
                (double)(start_time != AV_NOPTS_VALUE ? start_time : 0)/1000000
                <= ((double)duration/1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range) {
            packet_queue_put(&is->audioq, pkt);
        } else if (pkt->stream_index == is->video_stream && pkt_in_play_range) {
            packet_queue_put(&is->videoq, pkt);
        } else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range) {
            packet_queue_put(&is->subtitleq, pkt);
        } else {
            av_free_packet(pkt);
        }
    }
    /* wait until the end */
    while (!is->abort_request) {
        SDL_Delay(100);
    }

    ret = 0;
fail:
    /* disable interrupting */
    global_video_state = NULL;

    /* close each stream */
    if (is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);
    if (is->video_stream >= 0)
        stream_component_close(is, is->video_stream);
    if (is->subtitle_stream >= 0)
        stream_component_close(is, is->subtitle_stream);
    if (is->ic) {
        av_close_input_file(is->ic);
        is->ic = NULL; /* safety */
    }
    url_set_interrupt_cb(NULL);

    if (ret != 0) {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    return 0;
}

static VideoState *stream_open(const char *filename, AVInputFormat *iformat)
{
    VideoState *is;

    is = (VideoState *)av_mallocz(sizeof(VideoState));
    if (!is)
        return NULL;
    av_strlcpy(is->filename, filename, sizeof(is->filename));
    is->iformat = iformat;
    is->ytop = 0;
    is->xleft = 0;

    is->av_sync_type = av_sync_type;
//    is->parse_tid = SDL_CreateThread(decode_thread, is);
//    if (!is->parse_tid) {
//        av_free(is);
//        return NULL;
//    }
    return is;
}


void toggle_pause(void)
{
    if (cur_stream)
        stream_pause(cur_stream);
    step = 0;
}

static void step_to_next_frame(void)
{
    if (cur_stream) {
        /* if the stream is paused unpause it, then step */
        if (cur_stream->paused)
            stream_pause(cur_stream);
    }
    step = 1;
}

VideoState* Load_Audio(char* input_filename)
{
    int flags, i;

    /* register all codecs, demux and protocols */
    av_register_all();

    for(i=0; i<AVMEDIA_TYPE_NB; i++){
        avcodec_opts[i]= avcodec_alloc_context2((AVMediaType)i);
    }
    avformat_opts = avformat_alloc_context();

    if (!input_filename) {
        fprintf(stderr, "An input file must be specified\n");
        fprintf(stderr, "Use -h to get full help or, even better, run 'man ffplay'\n");
        exit(1);
    }

    flags = SDL_INIT_AUDIO | SDL_INIT_TIMER;

    if (SDL_Init (flags)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    av_init_packet(&flush_pkt);
    flush_pkt.data= (uint8_t*)"FLUSH";

    cur_stream = stream_open(input_filename, file_iformat);

    return 0;
}

VideoState* Start_Audio(char* input_filename)
{
    //Load_Audio(input_filename);
    if(cur_stream)
    {
        cur_stream->parse_tid = SDL_CreateThread(decode_thread, cur_stream);
        if (!cur_stream->parse_tid) {
            av_free(cur_stream);
            return NULL;
        }
    }
    return cur_stream;
}

void Stop_Audio()
{
    int i;

    if (cur_stream)
    {
        stream_close(cur_stream);
        cur_stream = NULL;
    }
    else
        return;

    for (i = 0; i < AVMEDIA_TYPE_NB; i++)
        av_free(avcodec_opts[i]);
    av_free(avformat_opts);
    av_free(sws_opts);

    SDL_Quit();
}
int set_volume(int var)
{
    volume = var;
    return 0;
}

int64_t getBegin()
{
    return ((cur_stream->audio_st&& cur_stream->audio_st->start_time != (int64_t) AV_NOPTS_VALUE) ? cur_stream->audio_st->start_time : 0);
}

int64_t getEnd()
{

    if (!cur_stream)
        return 0;

    // normal way to get duration of the video stream
    if ( cur_stream->audio_st->duration != (int64_t) AV_NOPTS_VALUE)
        return cur_stream->audio_st->duration + getBegin();

    //	qDebug("end = duration %d", av_rescale_q(pFormatCtx->duration, AV_TIME_BASE_Q, video_st->time_base) + getBegin());

    // if former failed, try this
    AVRational a;
    a.num = 1;
    a.den = AV_TIME_BASE;

    return av_rescale_q(cur_stream->ic->duration, a, cur_stream->audio_st->time_base) + getBegin();
    
}

int getTimeSnFromFrame(int64_t t)
{
    double play_speed = 1.0;
    double time = (double) (t- cur_stream->audio_st->start_time) * av_q2d(cur_stream->audio_st->time_base) / play_speed;
    int s = (int) time;
    time -= s;
    return s;
}

//void seekToPosition(int64_t t)
//{
//    if (!cur_stream)
//        return;
//    //cur_stream->seek_pos = t;
//    //cur_stream->audio_st->nb_frames =t;
//    //cur_stream->seek_req = true;
//    double pos;
//    if(global_video_state) {
//        pos = get_master_clock(global_video_state);
//        pos += t;
//        //QMessageBox::information(0,"",QString("%1").arg((int64_t)(pos * AV_TIME_BASE)),0,0);
//        //QMessageBox::information(0,"",QString("%1").arg(pos),0,0);
//        stream_seek(global_video_state,(int64_t)(pos * AV_TIME_BASE),0, true);
//    }
//}

void audioSeekToPosition(int64_t t,double frameRate) {

    if (cur_stream->ic && !cur_stream->seek_req)
    {
        //cur_stream->seek_pos = qBound(getBegin(), t, getEnd());
        //cur_stream->seek_flags |= AVSEEK_FLAG_ANY;
        //cur_stream->seek_req = true;
        //qDebug()<<"frame rate"<<frameRate;
        //qDebug()<<"audio base"<<cur_stream->audio_st->time_base.den;
        //qDebug()<<"video base"<<cur_stream->video_st->time_base.den;
        double seekSn = (t / cur_stream->video_st->time_base.den);
        //double pos = get
        double pos = get_master_clock(cur_stream);
        //qDebug()<<"t:"<<t<<"curr pos:"<<pos;
        pos += seekSn-pos;
        qDebug()<<"seekSn:"<<seekSn;
        stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(seekSn * AV_TIME_BASE), 0);
    }

}
