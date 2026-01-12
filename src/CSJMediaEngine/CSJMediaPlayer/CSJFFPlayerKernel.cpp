#include "CSJFFPlayerKernel.h"

#include <chrono>
#include <iostream>

#include "CSJUtils/CSJLogger.h"
#include "CSJUtils/CSJPathTool.h"

using namespace csjutils;
namespace csjmediaengine {

/* The operation below is the filters. */
#if CONFIG_AVFILTER
static int configure_filtergraph(AVFilterGraph *graph, const char *filtergraph,
                                 AVFilterContext *source_ctx, AVFilterContext *sink_ctx)
{
    int ret, i;
    int nb_filters = graph->nb_filters;
    AVFilterInOut *outputs = NULL, *inputs = NULL;

    if (filtergraph) {
        outputs = avfilter_inout_alloc();
        inputs  = avfilter_inout_alloc();
        if (!outputs || !inputs) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        outputs->name       = av_strdup("in");
        outputs->filter_ctx = source_ctx;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;

        inputs->name        = av_strdup("out");
        inputs->filter_ctx  = sink_ctx;
        inputs->pad_idx     = 0;
        inputs->next        = NULL;

        if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, NULL)) < 0)
            goto fail;
    } else {
        if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0)
            goto fail;
    }

    /* Reorder the filters to ensure that inputs of the custom filters are merged first */
    for (i = 0; i < graph->nb_filters - nb_filters; i++)
        FFSWAP(AVFilterContext*, graph->filters[i], graph->filters[i + nb_filters]);

    ret = avfilter_graph_config(graph, NULL);
fail:
    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);
    return ret;
}

static int configure_video_filters(AVFilterGraph *graph, VideoState *is, const char *vfilters, AVFrame *frame)
{
    enum AVPixelFormat pix_fmts[FF_ARRAY_ELEMS(sdl_texture_format_map)];
    char sws_flags_str[512] = "";
    char buffersrc_args[256];
    int ret;
    AVFilterContext *filt_src = NULL, *filt_out = NULL, *last_filter = NULL;
    AVCodecParameters *codecpar = m_pVideoStream->codecpar;
    AVRational fr = av_guess_frame_rate(is->ic, m_pVideoStream, NULL);
    const AVDictionaryEntry *e = NULL;
    int nb_pix_fmts = 0;
    int i, j;

    for (i = 0; i < renderer_info.num_texture_formats; i++) {
        for (j = 0; j < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; j++) {
            if (renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt) {
                pix_fmts[nb_pix_fmts++] = sdl_texture_format_map[j].format;
                break;
            }
        }
    }
    pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;

    while ((e = av_dict_get(sws_dict, "", e, AV_DICT_IGNORE_SUFFIX))) {
        if (!strcmp(e->key, "sws_flags")) {
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", "flags", e->value);
        } else
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", e->key, e->value);
    }
    if (strlen(sws_flags_str))
        sws_flags_str[strlen(sws_flags_str)-1] = '\0';

    graph->scale_sws_opts = av_strdup(sws_flags_str);

    snprintf(buffersrc_args, sizeof(buffersrc_args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             frame->width, frame->height, frame->format,
             m_pVideoStream->time_base.num, m_pVideoStream->time_base.den,
             codecpar->sample_aspect_ratio.num, FFMAX(codecpar->sample_aspect_ratio.den, 1));
    if (fr.num && fr.den)
        av_strlcatf(buffersrc_args, sizeof(buffersrc_args), ":frame_rate=%d/%d", fr.num, fr.den);

    if ((ret = avfilter_graph_create_filter(&filt_src,
                                            avfilter_get_by_name("buffer"),
                                            "ffplay_buffer", buffersrc_args, NULL,
                                            graph)) < 0)
        goto fail;

    ret = avfilter_graph_create_filter(&filt_out,
                                       avfilter_get_by_name("buffersink"),
                                       "ffplay_buffersink", NULL, NULL, graph);
    if (ret < 0)
        goto fail;

    if ((ret = av_opt_set_int_list(filt_out, "pix_fmts", pix_fmts,  AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto fail;

    last_filter = filt_out;

/* Note: this macro adds a filter before the lastly added filter, so the
 * processing order of the filters is in reverse */
#define INSERT_FILT(name, arg) do {                                          \
    AVFilterContext *filt_ctx;                                               \
                                                                             \
    ret = avfilter_graph_create_filter(&filt_ctx,                            \
                                       avfilter_get_by_name(name),           \
                                       "ffplay_" name, arg, NULL, graph);    \
    if (ret < 0)                                                             \
        goto fail;                                                           \
                                                                             \
    ret = avfilter_link(filt_ctx, 0, last_filter, 0);                        \
    if (ret < 0)                                                             \
        goto fail;                                                           \
                                                                             \
    last_filter = filt_ctx;                                                  \
} while (0)

    if (autorotate) {
        int32_t *displaymatrix = (int32_t *)av_stream_get_side_data(m_pVideoStream, AV_PKT_DATA_DISPLAYMATRIX, NULL);
        double theta = get_rotation(displaymatrix);

        if (fabs(theta - 90) < 1.0) {
            INSERT_FILT("transpose", "clock");
        } else if (fabs(theta - 180) < 1.0) {
            INSERT_FILT("hflip", NULL);
            INSERT_FILT("vflip", NULL);
        } else if (fabs(theta - 270) < 1.0) {
            INSERT_FILT("transpose", "cclock");
        } else if (fabs(theta) > 1.0) {
            char rotate_buf[64];
            snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
            INSERT_FILT("rotate", rotate_buf);
        }
    }

    if ((ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0)
        goto fail;

    is->in_video_filter  = filt_src;
    is->out_video_filter = filt_out;

fail:
    return ret;
}

static int configure_audio_filters(VideoState *is, const char *afilters, int force_output_format)
{
    static const enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int sample_rates[2] = { 0, -1 };
    AVFilterContext *filt_asrc = NULL, *filt_asink = NULL;
    char aresample_swr_opts[512] = "";
    const AVDictionaryEntry *e = NULL;
    AVBPrint bp;
    char asrc_args[256];
    int ret;

    avfilter_graph_free(&is->agraph);
    if (!(is->agraph = avfilter_graph_alloc()))
        return AVERROR(ENOMEM);
    is->agraph->nb_threads = filter_nbthreads;

    av_bprint_init(&bp, 0, AV_BPRINT_SIZE_AUTOMATIC);

    while ((e = av_dict_get(swr_opts, "", e, AV_DICT_IGNORE_SUFFIX)))
        av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
    if (strlen(aresample_swr_opts))
        aresample_swr_opts[strlen(aresample_swr_opts)-1] = '\0';
    av_opt_set(is->agraph, "aresample_swr_opts", aresample_swr_opts, 0);

    av_channel_layout_describe_bprint(&is->audio_filter_src.ch_layout, &bp);

    ret = snprintf(asrc_args, sizeof(asrc_args),
                   "sample_rate=%d:sample_fmt=%s:time_base=%d/%d:channel_layout=%s",
                   is->audio_filter_src.freq, av_get_sample_fmt_name(is->audio_filter_src.fmt),
                   1, is->audio_filter_src.freq, bp.str);

    ret = avfilter_graph_create_filter(&filt_asrc,
                                       avfilter_get_by_name("abuffer"), "ffplay_abuffer",
                                       asrc_args, NULL, is->agraph);
    if (ret < 0)
        goto end;


    ret = avfilter_graph_create_filter(&filt_asink,
                                       avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
                                       NULL, NULL, is->agraph);
    if (ret < 0)
        goto end;

    if ((ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts,  AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto end;
    if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto end;

    if (force_output_format) {
        sample_rates   [0] = m_audioTgt.freq;
        if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0)
            goto end;
        if ((ret = av_opt_set(filt_asink, "ch_layouts", bp.str, AV_OPT_SEARCH_CHILDREN)) < 0)
            goto end;
        if ((ret = av_opt_set_int_list(filt_asink, "sample_rates"   , sample_rates   ,  -1, AV_OPT_SEARCH_CHILDREN)) < 0)
            goto end;
    }


    if ((ret = configure_filtergraph(is->agraph, afilters, filt_asrc, filt_asink)) < 0)
        goto end;

    is->in_audio_filter  = filt_asrc;
    is->out_audio_filter = filt_asink;

end:
    if (ret < 0)
        avfilter_graph_free(&is->agraph);
    av_bprint_finalize(&bp, NULL);

    return ret;
}
#endif  /* CONFIG_AVFILTER */

static void (*program_exit)(int ret);

void register_exit(void (*cb)(int ret)) {
    program_exit = cb;
}

void exit_program(int ret) {
    if (program_exit) {
        program_exit(ret);
    }

    exit(ret);
}

int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec) {
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0) {
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    }

    return ret;
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, const AVCodec *codec) {
    AVDictionary *ret = NULL;
    const AVDictionaryEntry *t = NULL;
    int flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
    char prefix = 0;
    const AVClass *cc = avcodec_get_class();

    if (!codec) {
        codec = s->oformat ? avcodec_find_encoder(codec_id) : avcodec_find_decoder(codec_id);
    }

    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        prefix  = 'v';
        flags  |= AV_OPT_FLAG_VIDEO_PARAM;
        break;
    case AVMEDIA_TYPE_AUDIO:
        prefix  = 'a';
        flags  |= AV_OPT_FLAG_AUDIO_PARAM;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        prefix  = 's';
        flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
        break;
    }

    while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX)) {
        const AVClass *priv_class;
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p) {
            switch (check_stream_specifier(s, st, p + 1)) {
            case 1: 
                *p = 0; 
                break;
            case 0:
                continue;
            default: 
                exit_program(1);
            }
        }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) || !codec ||
            ((priv_class = codec->priv_class) && 
                av_opt_find(&priv_class, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ))) {
                av_dict_set(&ret, t->key, t->value, 0);
        } else if (t->key[0] == prefix && 
                   av_opt_find(&cc, t->key + 1, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ)) {
            av_dict_set(&ret, t->key + 1, t->value, 0);
        }

        if (p) {
            *p = ':';
        }
    }

    return ret;
}

AVDictionary** CSJFFPlayerKernel::setup_find_stream_info_opts(AVFormatContext *s,
                                                             AVDictionary *codec_opts) {
   int i;
   AVDictionary **opts;

   if (!s->nb_streams)
       return NULL;
   opts = (AVDictionary **)av_calloc(s->nb_streams, sizeof(*opts));
   if (!opts) {
       av_log(NULL, AV_LOG_ERROR,
              "Could not alloc memory for stream options.\n");
       exit_program(1);
   }
   for (i = 0; i < s->nb_streams; i++) {
       opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id,
                                   s, s->streams[i], NULL);
   }

   return opts;
}

int CSJFFPlayerKernel::packet_queue_put_private(PacketQueue *q, AVPacket *pkt) {
    MyAVPacketList pkt1;

    if (q->abort_request) {
        return -1;
    }

    pkt1.pkt = pkt;
    pkt1.serial = q->serial;

    int ret = av_fifo_write(q->pkt_list, &pkt1, 1);
    if (ret < 0) {
        return ret;
    }

    q->nb_packets++;
    q->size += pkt1.pkt->size + sizeof(pkt1);
    q->duration += pkt1.pkt->duration;

    q->cond->notify_one();
    return 0;
}

int CSJFFPlayerKernel::packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacket *pkt1;

    pkt1 = av_packet_alloc();
    if (!pkt1) {
        av_packet_unref(pkt);
        return -1;
    }

    av_packet_move_ref(pkt1, pkt);
    q->mutex->lock();
    int ret = packet_queue_put_private(q, pkt1);
    q->mutex->unlock();

    if (ret < 0) {
        av_packet_free(&pkt1);
    }

    return ret;
}

int CSJFFPlayerKernel::packet_queue_put_nullpacket(PacketQueue *q, AVPacket *pkt,
                                                   int stream_index) {
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

int CSJFFPlayerKernel::packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));

    q->pkt_list = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
    if (!q->pkt_list) {
        return AVERROR(ENOMEM);
    }

    q->mutex = new std::mutex();
    if (!q->mutex) {
        av_log(NULL, AV_LOG_FATAL, "Create mutex: %s\n", "failed");
        return AVERROR(ENOMEM);
    }

    q->cond = new std::condition_variable();
    if (!q->cond) {
        av_log(NULL, AV_LOG_FATAL, "Create Condition: %s\n", "failed");
        return AVERROR(ENOMEM);
    }
    q->abort_request = 1;

    return 0;
}

void CSJFFPlayerKernel::packet_queue_flush(PacketQueue *q) {
    MyAVPacketList pkt1;

    q->mutex->lock();
    while (av_fifo_read(q->pkt_list, &pkt1, 1) >= 0) {
        av_packet_free(&pkt1.pkt);
    }

    q->nb_packets = 0;
    q->size = 0;
    q->duration = 0;
    q->serial++;
    q->mutex->unlock();
}

void CSJFFPlayerKernel::packet_queue_destory(PacketQueue *q) {
    packet_queue_flush(q);
    av_fifo_freep2(&q->pkt_list);
    delete q->mutex;
    delete q->cond;
}

void CSJFFPlayerKernel::packet_queue_abort(PacketQueue *q) {
    q->mutex->lock();
    q->abort_request = 1;
    q->cond->notify_one();
    q->mutex->unlock();
}

void CSJFFPlayerKernel::packet_queue_start(PacketQueue *q) {
    q->mutex->lock();
    q->abort_request = 0;
    q->serial++;
    q->mutex->unlock();
}

void CSJFFPlayerKernel::decoder_destroy(Decoder *d) {
    av_packet_free(&d->pkt);
    avcodec_free_context(&d->avctx);
}

void CSJFFPlayerKernel::frame_queue_unref_item(Frame *vp) {
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

/* return < 0 if aborted, 0 if no packet and >0 if packet. */
int CSJFFPlayerKernel::packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial) {
    MyAVPacketList pkt1;
    int ret;

    q->mutex->lock();
    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        if (av_fifo_read(q->pkt_list, &pkt1, 1) >= 0) {
            q->nb_packets--;
            q->size -= pkt1.pkt->size + sizeof(pkt1);
            q->duration -= pkt1.pkt->duration;
            av_packet_move_ref(pkt, pkt1.pkt);
            if (serial) {
                *serial = pkt1.serial;
            }
            av_packet_free(&pkt1.pkt);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            std::unique_lock uniLock(*(q->mutex));
            q->cond->wait(uniLock);
        }
    }

    q->mutex->unlock();
    return ret;
}

int CSJFFPlayerKernel::decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue,
                        std::condition_variable *empty_queue_cond) {
    memset(d, 0, sizeof(Decoder));
    d->pkt = av_packet_alloc();
    if (!d->pkt) {
        return AVERROR(ENOMEM);
    }

    d->avctx = avctx;
    d->queue = queue;
    d->empty_queue_cond = empty_queue_cond;
    d->start_pts = AV_NOPTS_VALUE;
    d->pkt_serial = -1;

    return 0;
}

void CSJFFPlayerKernel::display_video() {
    // Update video to render layer.
}

int CSJFFPlayerKernel::decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub) {
    int ret = AVERROR(EAGAIN);

    for (;;) {
        if (d->queue->serial == d->pkt_serial) {
            do {
                if (d->queue->abort_request) {
                    return -1;
                }

                switch (d->avctx->codec_type) {
                case AVMEDIA_TYPE_VIDEO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if (ret >= 0) {
                        if (m_decoderReorderPts == -1) {
                            frame->pts = frame->best_effort_timestamp;
                        } else if (!m_decoderReorderPts) {
                            frame->pts = frame->pkt_dts;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if (ret >= 0) {
                        AVRational tb = {1, frame->sample_rate};
                        if (frame->pts != AV_NOPTS_VALUE) {
                            frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                        } else if (d->next_pts != AV_NOPTS_VALUE) {
                            frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                        }

                        if (frame->pts != AV_NOPTS_VALUE) {
                            d->next_pts = frame->pts + frame->nb_samples;
                            d->next_pts_tb = tb;
                        }
                    }
                    break;
                }

                if (ret == AVERROR_EOF) {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }

                if (ret >= 0) {
                    return 1;
                }

            } while (ret != AVERROR(EAGAIN));
        }
    }

    do {
        if (d->queue->nb_packets == 0) {
            d->empty_queue_cond->notify_one();
        }

        if (d->packet_pending) {
            d->packet_pending = 0;
        } else {
            int old_serial = d->pkt_serial;
            if (packet_queue_get(d->queue, d->pkt, 1, &d->pkt_serial) < 0) {
                return -1;
            }

            if (old_serial != d->pkt_serial) {
                avcodec_flush_buffers(d->avctx);
                d->finished    = 0;
                d->next_pts    = d->start_pts;
                d->next_pts_tb = d->start_pts_tb;
            }
        }

        if (d->queue->serial == d->pkt_serial) {
            break;
        }

        av_packet_unref(d->pkt);
    } while (1);

    if (d->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
        int got_frame = 0;
        ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, d->pkt);
        if (ret < 0) {
            ret = AVERROR(EAGAIN);
        } else {
            if (got_frame && !d->pkt->data) {
                d->packet_pending = 1;
            }

            ret = got_frame ? 0 : (d->pkt->data ? AVERROR(EAGAIN) : AVERROR_EOF);
        }

        av_packet_unref(d->pkt);
    } else {
        if (avcodec_send_packet(d->avctx, d->pkt) == AVERROR(EAGAIN)) {
            av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, \
                                            which is an API violation.\n");
            d->packet_pending = 1;
        } else {
            av_packet_unref(d->pkt);
        }
    }

    return ret;
}

int CSJFFPlayerKernel::frame_queue_init(FrameQueue *f, PacketQueue *pktq,
                                        int max_size, int keep_last) {
    memset(f, 0, sizeof(FrameQueue));
    f->mutex     = new std::mutex();
    f->cond      = new std::condition_variable();
    f->pktq      = pktq;
    f->max_size  = FFMAX(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = !!keep_last;
    for (int i = 0; i < f->max_size; i++) {
        AVFrame *frame = av_frame_alloc();
        if (!(f->queue[i].frame = av_frame_alloc())) {
            return AVERROR(ENOMEM);
        }
    }

    return 0;
}

void CSJFFPlayerKernel::frame_queue_destory(FrameQueue *f) {
    for (int i = 0; i < f->max_size; i++) {
        Frame *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }

    delete f->mutex;
    f->mutex = nullptr;
    delete f->cond;
    f->cond = nullptr;
}

void CSJFFPlayerKernel::frame_queue_signal(FrameQueue *f) {
    if (!f) {
        return ;
    }

    f->mutex->lock();
    f->cond->notify_one();
    f->mutex->unlock();
}

Frame* CSJFFPlayerKernel::frame_queue_peek(FrameQueue *f) {
    if (!f) {
        return nullptr;
    }

    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

Frame* CSJFFPlayerKernel::frame_queue_peek_next(FrameQueue *f) {
    if (!f) {
        return nullptr;
    }
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

Frame* CSJFFPlayerKernel::frame_queue_peek_last(FrameQueue *f) {
    if (!f) {
        return nullptr;
    }

    return &f->queue[f->rindex];
}

Frame* CSJFFPlayerKernel::frame_queue_peek_writable(FrameQueue *f) {
    if (!f) {
        return nullptr;
    }

    f->mutex->lock();
    while (f->size >= f->max_size && !f->pktq->abort_request) {
        std::unique_lock uniLock(*f->mutex);
        f->cond->wait(uniLock);
    }
    f->mutex->unlock();

    if (f->pktq->abort_request) {
        return nullptr;
    }

    return &f->queue[f->windex];
}

Frame* CSJFFPlayerKernel::frame_queue_peek_readable(FrameQueue *f) {
    if (!f) {
        return nullptr;
    }

    f->mutex->lock();
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        std::unique_lock uniLock(*f->mutex);
        f->cond->wait(uniLock);
    }
    f->mutex->unlock();

    if (f->pktq->abort_request) {
        return nullptr;
    }

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

void CSJFFPlayerKernel::frame_queue_push(FrameQueue *f) {
    if (++f->windex == f->max_size) {
        f->windex = 0;
    }

    f->mutex->lock();
    f->cond->notify_one();
    f->mutex->unlock();
}

void CSJFFPlayerKernel::frame_queue_next(FrameQueue *f) {
    if (!f) {
        return ;
    }

    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return ;
    }

    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size) {
        f->rindex = 0;
    }

    f->mutex->lock();
    f->size--;
    f->cond->notify_one();
    f->mutex->unlock();
}

int CSJFFPlayerKernel::frame_queue_nb_remaining(FrameQueue *f) {
    return f->size - f->rindex_shown;
}

int64_t CSJFFPlayerKernel::frame_queue_last_pos(FrameQueue *f) {
    if (!f) {
        return -1;
    }

    Frame *fp = &f->queue[f->rindex];
    if (f->rindex_shown && fp->serial == f->pktq->serial) {
        return fp->pos;
    } else {
        return -1;
    }
}

void CSJFFPlayerKernel::decoder_abort(Decoder *d, FrameQueue *fq) {
    packet_queue_abort(d->queue);
    frame_queue_signal(fq);

    if (d->decoder_thr->joinable()) {
        d->decoder_thr->join();
    }

    packet_queue_flush(d->queue);
}

double CSJFFPlayerKernel::get_clock(Clock *c) {
    if (!c) {
        return NAN;
    }

    if (*c->queue_serial != c->serial) {
        return NAN;
    }

    if (c->paused) {
        return c->pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return c->pts_drift + time - (time - c->last_updated);
    }
}

void CSJFFPlayerKernel::set_clock_at(Clock *c, double pts, int serial, double time) {
    if (!c) {
        return ;
    }

    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

void CSJFFPlayerKernel::set_clock(Clock *c, double pts, int serial) {
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

void CSJFFPlayerKernel::set_clock_speed(Clock *c, double speed) {
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

void CSJFFPlayerKernel::init_clock(Clock *c, int *queue_serial) {
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

void CSJFFPlayerKernel::sync_clock_to_slave(Clock *c, Clock *slave) {
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) ||
                                fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD)) {
        set_clock(c, slave_clock, slave->serial);
    }
}

int CSJFFPlayerKernel::get_master_sync_type() {
    if (m_avSyncType == AV_SYNC_VIDEO_MASTER) {
        if (m_pVideoStream) {
            return AV_SYNC_VIDEO_MASTER;
        } else {
            return AV_SYNC_AUDIO_MASTER;
        }
    } else if (m_avSyncType == AV_SYNC_AUDIO_MASTER) {
        if (m_pAudioSteam) {
            return AV_SYNC_AUDIO_MASTER;
        } else {
            return AV_SYNC_EXTERNAL_CLOCK;
        }
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

double CSJFFPlayerKernel::get_master_clock() {
    double val;

    switch (get_master_sync_type()) {
    case AV_SYNC_VIDEO_MASTER:
        val = get_clock(&m_vidClk);
        break;
    case AV_SYNC_AUDIO_MASTER:
        val = get_clock(&m_audClk);
        break;
    default:
        val = get_clock(&m_extClk);
        break;
    }

    return val;
}

void CSJFFPlayerKernel::check_external_clock_speed() {
    if (m_videoStreamIndex >= 0 && m_videoPacketQueue.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES ||
        m_audioStreamIndex >= 0 && m_audioPaketQueue.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES) {
        set_clock_speed(&m_extClk, FFMAX(EXTERNAL_CLOCK_SPEED_MIN,
                                         m_extClk.speed - EXTERNAL_CLOCK_SPEED_STEP ));
    } else if (m_videoStreamIndex < 0 || m_videoPacketQueue.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES ||
               m_audioStreamIndex < 0 || m_audioPaketQueue.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) {
        set_clock_speed(&m_extClk, FFMIN(EXTERNAL_CLOCK_SPEED_MAX,
                                         m_extClk.speed + EXTERNAL_CLOCK_SPEED_STEP));
    } else {
        double speed = m_extClk.speed;
        if (speed != 1.0) {
            set_clock_speed(&m_extClk,
                            speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
        }
    }

}

void CSJFFPlayerKernel::stream_seek(int64_t pos, int64_t rel, int by_bytes) {
    if (m_seekReq) {
        m_seekPos = pos;
        m_seekRel = rel;
        m_seekFlags &= ~AVSEEK_FLAG_BYTE;
        if (by_bytes) {
            m_seekFlags != AVSEEK_FLAG_BYTE;
        }
        m_seekReq = 1;
        m_pContinueReadCond->notify_one();
    }
}

void CSJFFPlayerKernel::stream_component_close(int stream_index) {
    AVCodecParameters *codecpar;

    if (stream_index < 0 || stream_index >= m_pFormatCtx->nb_streams) {
        return ;
    }

    codecpar = m_pFormatCtx->streams[stream_index]->codecpar;
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        decoder_abort(&m_audDecoder, &m_audioFrameQueue);
        // TODO: stop audio device;
        decoder_destroy(&m_audDecoder);
        swr_free(&m_swrCtx);
        av_freep(&m_pAudioBuf1);
        /* m_pAudioBuf and m_pAudioBuf1 point the same address, 
         * so m_pAudioBuf needn't to be free. 
         */
        m_pAudioBuf = nullptr;
        m_audioBuf1Size = 0;

        // 释放傅里叶离散变换的相应数据;
        // if (m_pRdft) {
        //     av_rdft_end(m_pRdft);
        //     av_freep(m_pRdftData);
        //     m_pRdft = nullptr;
        //     m_rdftBits = 0;
        // }
        break;
    case AVMEDIA_TYPE_VIDEO:
        decoder_abort(&m_videoDecoder, &m_videoFrameQueue);
        decoder_destroy(&m_videoDecoder);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        decoder_abort(&m_subtitleDecoder, &m_subtitleFrameQueue);
        decoder_destroy(&m_subtitleDecoder);
        break;
    default:
        break;
    }

    m_pFormatCtx->streams[stream_index]->discard = AVDISCARD_ALL;
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        m_pAudioSteam = NULL;
        m_audioStreamIndex = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        m_pVideoStream = NULL;
        m_videoStreamIndex = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        m_pSubtitleStream = NULL;
        m_subtitleStreamIndex = -1;
    default:
        break;
    }
}

void CSJFFPlayerKernel::stream_close() {
    m_abortRequest = 1;

    /* In current status is pause, resume the play first. */
    if (m_paused) {
        resume();
    }

    if (m_pReadThread->joinable()) {
        m_pReadThread->join();
    }

    if (m_audioStreamIndex >= 0) {
        stream_component_close(m_audioStreamIndex);
    }

    if (m_videoStreamIndex > 0) {
        stream_component_close(m_videoStreamIndex);
    }

    if (m_subtitleStreamIndex) {
        stream_component_close(m_subtitleStreamIndex);
    }

    if (m_pFormatCtx) {
        avformat_close_input(&m_pFormatCtx);
    }
    
    packet_queue_destory(&m_videoPacketQueue);
    packet_queue_destory(&m_audioPaketQueue);
    packet_queue_destory(&m_subtitlePacketQueue);

    frame_queue_destory(&m_videoFrameQueue);
    frame_queue_destory(&m_audioFrameQueue);
    frame_queue_destory(&m_subtitleFrameQueue);

    if (m_pImgConvertCtx) {
        sws_freeContext(m_pImgConvertCtx);
    }

    if (m_pSubConvertCtx) {
        sws_freeContext(m_pSubConvertCtx);
    }
}

void CSJFFPlayerKernel::stream_toggle_pause() {
    if (m_paused) {
        m_frameTimer += av_gettime_relative() / 1000000.0 - m_vidClk.last_updated;
        if (m_readPauseReturn != AVERROR(ENOSYS)) {
            m_vidClk.paused = 0;
        }

        set_clock(&m_vidClk, get_clock(&m_vidClk), m_vidClk.serial);
    }

    set_clock(&m_extClk, get_clock(&m_extClk), m_extClk.serial);
    m_paused = m_audClk.paused = m_vidClk.paused = m_extClk.paused = !m_paused;
}

void CSJFFPlayerKernel::resetPlayState() {
    m_abortRequest = true;

    if (m_paused) {
        resume();
    }

    if (m_pReadThread && m_pReadThread->joinable()) {
        m_pReadThread->join();
    }

    if (m_pVideoDecThread && m_pVideoDecThread->joinable()) {
        m_pVideoDecThread->join();
    }

    if (m_pAudioDecThread && m_pAudioDecThread->joinable()) {
        m_pAudioDecThread->join();
    }

    if (m_pSubtitleDecThread && m_pSubtitleDecThread->joinable()) {
        m_pSubtitleDecThread->join();
    }
}

void CSJFFPlayerKernel::toggle_mute() {
    m_muted = !m_muted;
}

void CSJFFPlayerKernel::update_volume(int sign, double step) {
    // SDL_MIX_MAXVOLUME
    int SDL_MIX_MAXVOLUME = 1000;
    double volume_level = m_audioVolume ? (20 * log(m_audioVolume / (double) SDL_MIX_MAXVOLUME) / log(10)) : -1000.0;
    int new_volume = lrint(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) / 20.0));
    m_audioVolume = av_clip(m_audioVolume == new_volume ? (m_audioVolume + sign) :new_volume, 0, SDL_MIX_MAXVOLUME);
}

void CSJFFPlayerKernel::step_to_next_frame() {
    if (m_paused) {
        stream_toggle_pause();
    }
    m_step = 1;
}

double CSJFFPlayerKernel::compute_target_delay(double delay) {
    double sync_threshold, diff = 0;

    if (get_master_sync_type() != AV_SYNC_VIDEO_MASTER) {
        diff = get_clock(&m_vidClk) - get_master_clock();

        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isnan(diff) && fabs(diff) < m_maxFrameDuration) {
            if (diff == -sync_threshold) {
                delay = FFMAX(0, delay + diff);
            } else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
                delay = delay + diff;
            } else if (diff >= sync_threshold) {
                delay = 2 * delay;
            }
        }
    }

    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n", delay, -diff);
    return delay;
}

double CSJFFPlayerKernel::vp_duration(Frame *vp, Frame *nextvp) {
    if (!vp || !nextvp) {
        return 0.0;
    }

    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > m_maxFrameDuration) {
            return vp->duration;
        } else {
            return duration;
        }
    } else {
        return 0.0;
    }
}

void CSJFFPlayerKernel::update_video_pts(double pts, int64_t pos, int serial) {
    /* update current video pts. */
    set_clock(&m_vidClk, pts, serial);
    sync_clock_to_slave(&m_extClk, &m_vidClk);
}

void CSJFFPlayerKernel::video_refresh(double *remaining_time) {
    double time;

    Frame *sp, *sp2;

    if (m_paused && get_master_sync_type() == AV_SYNC_EXTERNAL_CLOCK && m_realTime) {
        check_external_clock_speed();
    }

    if (!m_displayDisable && m_showMode != SHOW_MODE_VIDEO && m_pAudioSteam) {
        time = av_gettime_relative() / 1000000.0;
        if (m_forceRrefresh || m_lastVisTime + m_rdftSpeed < time) {
            //video_display();
            m_lastVisTime = time;
        }

        *remaining_time = FFMIN(*remaining_time, m_lastVisTime + m_rdftSpeed - time);
    }

    if (m_pVideoStream) {
        while (frame_queue_nb_remaining(&m_videoFrameQueue) != 0) {
            double last_duration, duration, delay;
            Frame *vp, *lastvp;

            lastvp = frame_queue_peek_last(&m_videoFrameQueue);
            vp = frame_queue_peek(&m_videoFrameQueue);

            if (vp->serial != m_videoPacketQueue.serial) {
                frame_queue_next(&m_videoFrameQueue);
                continue;
            }

            if (lastvp->serial != vp->serial) {
                m_frameTimer = av_gettime_relative() / 1000000.0;
            }

            if (m_paused) {
                break;
            }

            /* compute nominal last_duration */
            last_duration = vp_duration(lastvp, vp);
            delay = compute_target_delay(last_duration);

            time= av_gettime_relative() / 1000000.0;
            if (time < m_frameTimer + delay) {
                 *remaining_time = FFMIN(m_frameTimer + delay - time, *remaining_time);
                 break;
            }

            m_frameTimer += delay;
            if (delay > 0 && time - m_frameTimer > AV_SYNC_THRESHOLD_MAX){
                m_frameTimer = time;
            }

            m_videoFrameQueue.mutex->lock();
            if (!isnan(vp->pts)) {
                update_video_pts(vp->pts, vp->pos, vp->serial);
            }
            m_videoFrameQueue.mutex->unlock();

            if (frame_queue_nb_remaining(&m_videoFrameQueue) > 1) {
                Frame *nextvp = frame_queue_peek_next(&m_videoFrameQueue);
                duration = vp_duration(vp, nextvp);
                if (!m_step &&
                   (m_frameDrop > 0 || (m_frameDrop && get_master_sync_type() != AV_SYNC_VIDEO_MASTER)) &&
                   time > m_frameTimer + duration) {
                    m_frameDropsLate++;
                    frame_queue_next(&m_videoFrameQueue);
                    continue ;
                }
            }

            if (m_pSubtitleStream) {
                while (frame_queue_nb_remaining(&m_subtitleFrameQueue) > 0) {
                    sp = frame_queue_peek(&m_subtitleFrameQueue);
                    if (frame_queue_nb_remaining(&m_subtitleFrameQueue) > 1) {
                        sp2 = frame_queue_peek_next(&m_subtitleFrameQueue);
                    } else {
                        sp2 = NULL;
                    }

                    if (sp->serial != m_subtitlePacketQueue.serial ||
                        (m_vidClk.pts > (sp->pts + ((float) sp->sub.end_display_time / 1000))) ||
                        (sp2 && m_vidClk.pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000)))) {
                        if (sp->uploaded) {
                            int i;
                            for (i = 0; i < sp->sub.num_rects; i++) {
                                AVSubtitleRect *sub_rect = sp->sub.rects[i];
                                uint8_t *pixels;
                                int pitch, j;

                                //if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)&pixels, &pitch)) {
                                //    for (j = 0; j < sub_rect->h; j++, pixels += pitch)
                                //        memset(pixels, 0, sub_rect->w << 2);
                                //    SDL_UnlockTexture(is->sub_texture);
                                //}
                            }
                        }
                        frame_queue_next(&m_subtitleFrameQueue);
                    } else {
                        break;
                    }
                }
            }

            frame_queue_next(&m_videoFrameQueue);
            m_forceRrefresh = 1;

            if (m_step && !m_paused) {
                stream_toggle_pause();
            }
        }

        if (!m_displayDisable &&
            m_forceRrefresh &&
            m_showMode == SHOW_MODE_VIDEO &&
            m_videoFrameQueue.rindex_shown) {
            // video_display();
        }
    }

    m_forceRrefresh = 0;
    if (m_showStatus) {
        AVBPrint buf;
        static int64_t last_time;
        int64_t cur_time;
        int aqsize, vqsize, sqsize;
        double av_diff;

        cur_time = av_gettime_relative();
        if (!last_time || (cur_time - last_time) >= 30000) {
            aqsize = 0;
            vqsize = 0;
            sqsize = 0;
            if (m_pAudioSteam) {
                aqsize = m_audioPaketQueue.size;
            }

            if (m_pVideoStream) {
                vqsize = m_videoPacketQueue.size;
            }

            if (m_pSubtitleStream) {
                sqsize = m_subtitlePacketQueue.size;
            }

            av_diff = 0;
            if (m_pAudioSteam && m_pVideoStream) {
                av_diff = get_clock(&m_audClk) - get_clock(&m_vidClk);
            } else if (m_pVideoStream) {
                av_diff = get_master_clock() - get_clock(&m_vidClk);
            } else if (m_pAudioSteam) {
                av_diff = get_master_clock() - get_clock(&m_audClk);
            }

            av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
            av_bprintf(&buf,
                      "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%" PRId64"/%" PRId64"   \r",
                      get_master_clock(),
                      (m_pAudioSteam && m_pVideoStream) ? "A-V" : (m_pVideoStream ? "M-V" : (m_pAudioSteam ? "M-A" : "   ")),
                      av_diff,
                      m_frameDropsEarly + m_frameDropsLate,
                      aqsize / 1024,
                      vqsize / 1024,
                      sqsize);

            if (m_showStatus == 1 && AV_LOG_INFO > av_log_get_level()) {
                fprintf(stderr, "%s", buf.str);
            } else {
                av_log(NULL, AV_LOG_INFO, "%s", buf.str);
            }

            fflush(stderr);
            av_bprint_finalize(&buf, NULL);

            last_time = cur_time;
        }
    }
}

int CSJFFPlayerKernel::queue_picture(AVFrame *src_frame, double pts, double duration,
                                     int64_t pos, int serial) {
    Frame *vp;

#if defined(DEBUG_SYNC)
    printf("frame_type=%c pts=%0.3f\n",
           av_get_picture_type_char(src_frame->pict_type), pts);
#endif

    if (!(vp = frame_queue_peek_writable(&m_videoFrameQueue))) {
        return -1;
    }

    vp->sar      = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width    = src_frame->width;
    vp->height   = src_frame->height;
    vp->format   = src_frame->format;

    vp->pts      = pts;
    vp->duration = duration;
    vp->pos      = pos;
    vp->serial   = serial;

    //set_default_window_size(vp->width, vp->height, vp->sar);

    av_frame_move_ref(vp->frame, src_frame);
    frame_queue_push(&m_videoFrameQueue);
    return 0;
}

int CSJFFPlayerKernel::get_video_frame(AVFrame *frame) {
    int got_picture;

    if ((got_picture = decoder_decode_frame(&m_videoDecoder, frame, NULL)) < 0) {
        return -1;
    }

    if (got_picture) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE) {
            dpts = av_q2d(m_pVideoStream->time_base) * frame->pts;
        }

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(m_pFormatCtx, m_pVideoStream, frame);

        if (m_frameDrop > 0 || (m_frameDrop && get_master_sync_type() != AV_SYNC_VIDEO_MASTER)) {
            if (frame->pts != AV_NOPTS_VALUE) {
                double diff = dpts - get_master_clock();
                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                    diff - m_frameLastFilterDelay < 0 &&
                    m_videoDecoder.pkt_serial == m_vidClk.serial &&
                    m_videoPacketQueue.nb_packets) {
                    m_frameDropsEarly++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }

    return got_picture;
}

int CSJFFPlayerKernel::audio_decode_task() {
    AVFrame *frame = av_frame_alloc();
    Frame *af;
#if CONFIG_AVFILTER
    int last_serial = -1;
    int reconfigure;
#endif
    int got_frame = 0;
    AVRational tb;
    int ret = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    do {
        if ((got_frame = decoder_decode_frame(&m_audDecoder, frame, NULL)) < 0) {
            goto the_end;
        }

        if (got_frame) {
            tb = {1, frame->sample_rate};

#if CONFIG_AVFILTER
            
            int audio_fmts_same = cmp_audio_fmts(is->audio_filter_src.fmt, 
                                                 is->audio_filter_src.ch_layout.nb_channels,
                                                 frame->format, 
                                                 frame->ch_layout.nb_channels);

            int audio_channel_layout_same = av_channel_layout_compare(&is->audio_filter_src.ch_layout, &frame->ch_layout);

            reconfigure = audio_fmts_same || 
                          audio_channel_layout_same || 
                          (is->audio_filter_src.freq != frame->sample_rate) || 
                          m_audDecoder.pkt_serial != last_serial;

            if (reconfigure) {
                char buf1[1024], buf2[1024];
                av_channel_layout_describe(&is->audio_filter_src.ch_layout, buf1, sizeof(buf1));
                av_channel_layout_describe(&frame->ch_layout, buf2, sizeof(buf2));
                av_log(NULL, AV_LOG_DEBUG,
                       "Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d\n",
                       is->audio_filter_src.freq, 
                       is->audio_filter_src.ch_layout.nb_channels, 
                       av_get_sample_fmt_name(is->audio_filter_src.fmt), 
                       buf1, last_serial,
                       frame->sample_rate, 
                       frame->ch_layout.nb_channels, 
                       av_get_sample_fmt_name(frame->format), buf2, m_audDecoder.pkt_serial);

                is->audio_filter_src.fmt            = frame->format;
                ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &frame->ch_layout);
                if (ret < 0) {
                    goto the_end;
                }

                is->audio_filter_src.freq           = frame->sample_rate;
                last_serial                         = m_audDecoder.pkt_serial;

                if ((ret = configure_audio_filters(is, afilters, 1)) < 0) {
                    goto the_end;
                }
            }

            if ((ret = av_buffersrc_add_frame(is->in_audio_filter, frame)) < 0) {
                goto the_end;
            }

            while ((ret = av_buffersink_get_frame_flags(is->out_audio_filter, frame, 0)) >= 0) {
                tb = av_buffersink_get_time_base(is->out_audio_filter);
#endif
                if (!(af = frame_queue_peek_writable(&m_audioFrameQueue))) {
                    goto the_end;
                }

                af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
                af->pos = frame->pts;
                af->serial = m_audDecoder.pkt_serial;
                af->duration = av_q2d({frame->nb_samples, frame->sample_rate});

                av_frame_move_ref(af->frame, frame);
                frame_queue_push(&m_audioFrameQueue);

#if CONFIG_AVFILTER
                if (m_audioPaketQueue.serial != m_audDecoder.pkt_serial) {
                    break;
                }
            }

            if (ret == AVERROR_EOF) {
                m_audDecoder.finished = m_audDecoder.pkt_serial;
            }
#endif
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:
#if CONFIG_AVFILTER
    avfilter_graph_free(&is->agraph);
#endif
    av_frame_free(&frame);
    return ret;
}

int CSJFFPlayerKernel::video_decode_task() {
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = m_pVideoStream->time_base;
    AVRational frame_rate = av_guess_frame_rate(m_pFormatCtx, m_pVideoStream, NULL);

#if CONFIG_AVFILTER
    AVFilterGraph *graph = NULL;
    AVFilterContext *filt_out = NULL, *filt_in = NULL;
    int last_w = 0;
    int last_h = 0;
    enum AVPixelFormat last_format = -2;
    int last_serial = -1;
    int last_vfilter_idx = 0;
#endif

    if (!frame) {
        return AVERROR(ENOMEM);
    }

    for (;;) {
        ret = get_video_frame(frame);
        if (ret < 0) {
            goto the_end;
        }

        if (!ret) {
            continue;
        }

#if CONFIG_AVFILTER
        if (last_w != frame->width ||
            last_h != frame->height ||
            last_format != frame->format ||
            last_serial != m_videoDecoder.pkt_serial ||
            last_vfilter_idx != is->vfilter_idx) {
            av_log(NULL, AV_LOG_DEBUG,
                   "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                   last_w, last_h,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                   frame->width, frame->height,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(frame->format), "none"), m_videoDecoder.pkt_serial);
            avfilter_graph_free(&graph);
            graph = avfilter_graph_alloc();
            if (!graph) {
                ret = AVERROR(ENOMEM);
                goto the_end;
            }
            graph->nb_threads = filter_nbthreads;
            if ((ret = configure_video_filters(graph, is, vfilters_list ? vfilters_list[is->vfilter_idx] : NULL, frame)) < 0) {
                SDL_Event event;
                event.type = FF_QUIT_EVENT;
                event.user.data1 = is;
                SDL_PushEvent(&event);
                goto the_end;
            }
            filt_in  = is->in_video_filter;
            filt_out = is->out_video_filter;
            last_w = frame->width;
            last_h = frame->height;
            last_format = frame->format;
            last_serial = m_videoDecoder.pkt_serial;
            last_vfilter_idx = is->vfilter_idx;
            frame_rate = av_buffersink_get_frame_rate(filt_out);
        }

        ret = av_buffersrc_add_frame(filt_in, frame);
        if (ret < 0) {
            goto the_end;
        }

        while (ret >= 0) {
            is->frame_last_returned_time = av_gettime_relative() / 1000000.0;

            ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
            if (ret < 0) {
                if (ret == AVERROR_EOF)
                    m_videoDecoder.finished = m_videoDecoder.pkt_serial;
                ret = 0;
                break;
            }

            is->frame_last_filter_delay = av_gettime_relative() / 1000000.0 - is->frame_last_returned_time;
            if (fabs(is->frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0) {
                is->frame_last_filter_delay = 0;
            }
            tb = av_buffersink_get_time_base(filt_out);
#endif
            duration = (frame_rate.num && frame_rate.den ? av_q2d({frame_rate.den, frame_rate.num}) : 0);
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            ret = queue_picture(frame, pts, duration, frame->pts, m_videoDecoder.pkt_serial);
            av_frame_unref(frame);
#if CONFIG_AVFILTER
            if (m_videoPacketQueue.serial != m_videoDecoder.pkt_serial) {
                break;
            }
        }
#endif

        if (ret < 0) {
            goto the_end;
        }
    }
the_end:
#if CONFIG_AVFILTER
    avfilter_graph_free(&graph);
#endif
    av_frame_free(&frame);
    return 0;
}

int CSJFFPlayerKernel::subtitle_decode_task() {
    Frame *sp;
    int got_subtitle;
    double pts;

    for (;;) {
        if (!(sp = frame_queue_peek_writable(&m_subtitleFrameQueue))) {
            return 0;
        }

        if ((got_subtitle = decoder_decode_frame(&m_subtitleDecoder, NULL, &sp->sub)) < 0) {
            break;
        }

        pts = 0;

        if (got_subtitle && sp->sub.format == 0) {
            if (sp->sub.pts != AV_NOPTS_VALUE) {
                pts = sp->sub.pts / (double)AV_TIME_BASE;
            }

            sp->pts = pts;
            sp->serial = m_subtitleDecoder.pkt_serial;
            sp->width = m_subtitleDecoder.avctx->width;
            sp->height = m_subtitleDecoder.avctx->height;
            sp->uploaded = 0;

            /* now we can update the picture count */
            frame_queue_push(&m_subtitleFrameQueue);
        } else if (got_subtitle) {
            avsubtitle_free(&sp->sub);
        }
    }

    return 0;
}

void CSJFFPlayerKernel::update_sample_display(short *samples, int samples_size) {
    if (!samples) {
        return ;
    }

    int size, len;

    size = samples_size / sizeof(short);
    while (size > 0) {
        len = SAMPLE_ARRAY_SIZE - m_sampleArrayIndex;
        if (len > size) {
            len = size;
        }

        memcpy(m_sampleArray + m_sampleArrayIndex, samples, len * sizeof(short));
        samples += len;
        m_sampleArrayIndex += len;
        if (m_sampleArrayIndex >= SAMPLE_ARRAY_SIZE) {
            m_sampleArrayIndex = 0;
        }
        size -= len;
    }
}

int CSJFFPlayerKernel::synchronize_audio(int nb_samples) {
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (get_master_sync_type() != AV_SYNC_AUDIO_MASTER) {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = get_clock(&m_audClk) - get_master_clock();

        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
            m_audioDiffCum = diff + m_audioDiffAvgCoef * m_audioDiffCum;
            if (m_audioDiffAvgCount < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate */
                m_audioDiffAvgCount++;
            } else {
                /* estimate the A-V difference */
                avg_diff = m_audioDiffCum * (1.0 - m_audioDiffAvgCoef);

                if (fabs(avg_diff) >= m_audioDiffThreshold) {
                    wanted_nb_samples = nb_samples + (int)(diff * m_audioSrc.freq);
                    min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(NULL, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                       diff, avg_diff, wanted_nb_samples - nb_samples,
                       m_audioClock, m_audioDiffThreshold);
            }
        } else {
            /* too big difference : may be initial PTS errors, so
             * reset A-V filter
             */
            m_audioDiffAvgCount = 0;
            m_audioDiffCum = 0;
        }
    }

    return wanted_nb_samples;
}

int CSJFFPlayerKernel::audio_decode_frame() {
    int data_size, resampled_data_size;
    av_unused double audio_clock0;
    int wanted_nb_samples;
    Frame *af;

    if (m_paused) {
        return -1;
    }

    do {
#if defined(_WIN32)
        while (frame_queue_nb_remaining(&m_audioFrameQueue) == 0) {
            if ((av_gettime_relative() - m_audioCallbackTime) > 1000000LL *
                    m_audioHwBufSize/ m_audioTgt.bytes_per_sec / 2) {
                return -1;
            }
            av_usleep (1000);
        }
#endif
        if (!(af = frame_queue_peek_readable(&m_audioFrameQueue))) {
            return -1;
        }

        frame_queue_next(&m_audioFrameQueue);
    } while (af->serial != m_audioPaketQueue.serial);

    data_size = av_samples_get_buffer_size(NULL, af->frame->ch_layout.nb_channels,
                                           af->frame->nb_samples,
                                           (AVSampleFormat)af->frame->format, 1);

    wanted_nb_samples = synchronize_audio(af->frame->nb_samples);

    if (af->frame->format != m_audioSrc.fmt ||
        av_channel_layout_compare(&af->frame->ch_layout, &m_audioSrc.ch_layout) ||
        af->frame->sample_rate != m_audioSrc.freq ||
        (wanted_nb_samples != af->frame->nb_samples && !m_swrCtx)) {
        swr_free(&m_swrCtx);
        swr_alloc_set_opts2(&m_swrCtx,
                            &m_audioTgt.ch_layout,
                            m_audioTgt.fmt,
                            m_audioTgt.freq,
                            &af->frame->ch_layout,
                            (AVSampleFormat)af->frame->format,
                            af->frame->sample_rate,
                            0, NULL);

        if (!m_swrCtx || swr_init(m_swrCtx) < 0) {
            av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                   af->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)af->frame->format), af->frame->ch_layout.nb_channels,
                   m_audioTgt.freq, av_get_sample_fmt_name(m_audioTgt.fmt), m_audioTgt.ch_layout.nb_channels);
            swr_free(&m_swrCtx);
            return -1;
        }

        if (av_channel_layout_copy(&m_audioSrc.ch_layout, &af->frame->ch_layout) < 0) {
            return -1;
        }

        m_audioSrc.freq = af->frame->sample_rate;
        m_audioSrc.fmt = (AVSampleFormat)af->frame->format;
    }

    if (m_swrCtx) {
        const uint8_t **in = (const uint8_t **)af->frame->extended_data;
        uint8_t **out = &m_pAudioBuf1;
        int out_count = (int64_t)wanted_nb_samples * m_audioTgt.freq / af->frame->sample_rate + 256;
        int out_size  = av_samples_get_buffer_size(NULL, m_audioTgt.ch_layout.nb_channels, out_count, m_audioTgt.fmt, 0);
        int len2;
        if (out_size < 0) {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }

        if (wanted_nb_samples != af->frame->nb_samples) {
            if (swr_set_compensation(m_swrCtx,
                                     (wanted_nb_samples - af->frame->nb_samples) * m_audioTgt.freq / af->frame->sample_rate,
                                     wanted_nb_samples * m_audioTgt.freq / af->frame->sample_rate) < 0) {
                av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                return -1;
            }
        }

        av_fast_malloc(&m_pAudioBuf1, &m_audioBuf1Size, out_size);
        if (!m_pAudioBuf1) {
            return AVERROR(ENOMEM);
        }

        len2 = swr_convert(m_swrCtx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0) {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }

        if (len2 == out_count) {
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(m_swrCtx) < 0)
                swr_free(&m_swrCtx);
        }

        m_pAudioBuf = m_pAudioBuf1;
        resampled_data_size = len2 * m_audioTgt.ch_layout.nb_channels * av_get_bytes_per_sample(m_audioTgt.fmt);
    } else {
        m_pAudioBuf = af->frame->data[0];
        resampled_data_size = data_size;
    }

    audio_clock0 = m_audioClock;
    /* update the audio clock with the pts */
    if (!isnan(af->pts)) {
        m_audioClock = af->pts + (double) af->frame->nb_samples / af->frame->sample_rate;
    } else {
        m_audioClock = NAN;
    }
    m_audioClockSerial = af->serial;
#ifdef DEBUG
    {
        static double last_clock;
        printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
               is->audio_clock - last_clock,
               is->audio_clock, audio_clock0);
        last_clock = is->audio_clock;
    }
#endif
    return resampled_data_size;
}

void CSJFFPlayerKernel::sdl_audio_callback(uint8_t *stream, int len) {
    int audio_size, len1;

    m_audioCallbackTime = av_gettime_relative();

    while (len > 0) {
        if (m_audioBufIndex >= m_audioBufSize) {
            audio_size = audio_decode_frame();
            if (audio_size < 0) {
                /* if error, just output silence */
                m_pAudioBuf = NULL;
#define SDL_AUDIO_MIN_BUFFER_SIZE 10
                m_audioBufSize = SDL_AUDIO_MIN_BUFFER_SIZE / m_audioTgt.frame_size * m_audioTgt.frame_size;
            } else {
                if (m_showMode != SHOW_MODE_VIDEO)
                    update_sample_display((int16_t *)m_pAudioBuf, audio_size);
                m_audioBufSize = audio_size;
            }
            m_audioBufIndex = 0;
        }

        len1 = m_audioBufSize - m_audioBufIndex;
        if (len1 > len) {
            len1 = len;
        }
#define SDL_MIX_MAXVOLUME 10
        if (!m_muted && m_pAudioBuf && m_audioVolume == SDL_MIX_MAXVOLUME) {
            memcpy(stream, (uint8_t *)m_pAudioBuf + m_audioBufSize, len1);
        } else {
            memset(stream, 0, len1);
            if (!m_muted && m_pAudioBuf) {
                //SDL_MixAudioFormat(stream, (uint8_t *)m_pAudioBuf + m_audioBufIndex, AUDIO_S16SYS, len1, is->audio_volume);
            }
        }
        len -= len1;
        stream += len1;
        m_audioBufIndex += len1;
    }
    m_audioWriteBufSize = m_audioBufSize - m_audioBufIndex;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(m_audioClock)) {
        set_clock_at(&m_audClk, m_audioClock - (double)(2 * m_audioHwBufSize + m_audioWriteBufSize) / m_audioTgt.bytes_per_sec, m_audioClockSerial, m_audioCallbackTime / 1000000.0);
        sync_clock_to_slave(&m_extClk, &m_audClk);
    }

}

int CSJFFPlayerKernel::audio_open(AVChannelLayout *wanted_channel_layout,
                                  int wanted_sample_rate,
                                  AudioParams *audio_hw_params) {
    /*
    SDL_AudioSpec wanted_spec, spec;
    const char *env;
    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;
    int wanted_nb_channels = wanted_channel_layout->nb_channels;

    env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if (env) {
        wanted_nb_channels = atoi(env);
        av_channel_layout_uninit(wanted_channel_layout);
        av_channel_layout_default(wanted_channel_layout, wanted_nb_channels);
    }
    if (wanted_channel_layout->order != AV_CHANNEL_ORDER_NATIVE) {
        av_channel_layout_uninit(wanted_channel_layout);
        av_channel_layout_default(wanted_channel_layout, wanted_nb_channels);
    }
    wanted_nb_channels = wanted_channel_layout->nb_channels;
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
        return -1;
    }
    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
        next_sample_rate_idx--;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = opaque;
    while (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
        av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
               wanted_spec.channels, wanted_spec.freq, SDL_GetError());
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq) {
                av_log(NULL, AV_LOG_ERROR,
                       "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        av_channel_layout_default(wanted_channel_layout, wanted_spec.channels);
    }
    if (spec.format != AUDIO_S16SYS) {
        av_log(NULL, AV_LOG_ERROR,
               "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels) {
        av_channel_layout_uninit(wanted_channel_layout);
        av_channel_layout_default(wanted_channel_layout, spec.channels);
        if (wanted_channel_layout->order != AV_CHANNEL_ORDER_NATIVE) {
            av_log(NULL, AV_LOG_ERROR,
                   "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }

    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    if (av_channel_layout_copy(&audio_hw_params->ch_layout, wanted_channel_layout) < 0)
        return -1;
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->ch_layout.nb_channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->ch_layout.nb_channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    return spec.size;
    */
    return 0;
}

int CSJFFPlayerKernel::stream_component_open(int stream_index) {
    AVFormatContext *ic = m_pFormatCtx;
    AVCodecContext  *avctx;
    const AVCodec   *codec;
    const char      *forced_codec_name = NULL;
    AVDictionary    *opts = NULL;
    
    int             sample_rate;
    AVChannelLayout ch_layout = { (AVChannelOrder)0 };
    int             stream_lowres = 0;//lowres;
    const AVDictionaryEntry *t = NULL;    
    int             ret = 0;

    if (stream_index < 0 || stream_index >= ic->nb_streams) {
        return -1;
    }

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx) {
        return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if (ret < 0) {
        return ret;
    }

    avctx->pkt_timebase = ic->streams[stream_index]->time_base;
    codec = avcodec_find_decoder(avctx->codec_id);

    static const char* audio_codec_name = "AAC";
    static const char* subtitle_codec_name = "subtitle";
    static const char* video_codec_name = "x264";
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        m_lastAudioStream = stream_index;
        //forced_codec_name = audio_codec_name;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        m_lastSubtitleStream = stream_index;
        //forced_codec_name = subtitle_codec_name;
        break;
    case AVMEDIA_TYPE_VIDEO:
        m_lastVideoStream = stream_index;
        //forced_codec_name = video_codec_name;
        break;
    }

    // It's unnecessary to find decoder with codec name.
//    if (forced_codec_name) {
//        codec = avcodec_find_decoder_by_name(forced_codec_name);
//    }

//    if (!codec) {
//        if (forced_codec_name) {
//            av_log(NULL, AV_LOG_WARNING,
//                                     "No codec could be found with name '%s'\n", forced_codec_name);
//        } else {
//            av_log(NULL, AV_LOG_WARNING,
//                                      "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
//        }

//        ret = AVERROR(EINVAL);
//        return ret;
//    }

    avctx->codec_id = codec->id;
    if (stream_lowres > codec->max_lowres) {
        // av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
        //        codec->max_lowres);
        m_pLogger->log_warn("The maximum value for lowres supported by the decoder is %d\n",
                                codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;

    int fast = 0;
    if (fast) {
        avctx->flags2 |= AV_CODEC_FLAG2_FAST;
    }

    AVDictionary *codec_opts = NULL;
    opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
    /* Set multi-thread decoder mode. 
     * "auto" indiecates FFMpeg will select thread numbers automatically with the system properties  
     */
    if (!av_dict_get(opts, "threads", NULL, 0)) {
        av_dict_set(&opts, "threads", "auto", 0);
    }

    /* If low sample video frame */
    if (stream_lowres) {
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    }

    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
        m_pLogger->log_fatal("Open codec failed!");
        goto fail;
    }
    
    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        //av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        m_pLogger->log_error("Option %s not found.\n", t->key);
        ret =  AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    m_eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
#if CONFIG_AVFILTER
    {
        AVFilterContext *sink;

        is->audio_filter_src.freq           = avctx->sample_rate;
        ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &avctx->ch_layout);
        if (ret < 0) {
            goto fail;
        }

        is->audio_filter_src.fmt            = avctx->sample_fmt;
        if ((ret = configure_audio_filters(is, afilters, 0)) < 0) {
            goto fail;
        }

        sink = is->out_audio_filter;
        sample_rate    = av_buffersink_get_sample_rate(sink);
        ret = av_buffersink_get_ch_layout(sink, &ch_layout);
        if (ret < 0) {
            goto fail;
        }
    }
#else
        sample_rate    = avctx->sample_rate;
        ret = av_channel_layout_copy(&ch_layout, &avctx->ch_layout);
        if (ret < 0) {
            goto fail;
        }
#endif

        /* prepare audio output */
        /* Audio output will use the native library, this module isn't implemented. */
        if ((ret = audio_open(&ch_layout, sample_rate, &m_audioTgt)) < 0) {
            goto fail;
        }

        m_audioHwBufSize = ret;
        m_audioSrc = m_audioTgt;
        m_audioBufSize = 0;
        m_audioBufIndex = 0;

        /* init averaging filter */
        m_audioDiffAvgCoef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        m_audioDiffAvgCount = 0;

        /*
         * since we do not have a precise anough audio FIFO fullness,
         * we correct audio sync only if larger than this threshold
         */
        m_audioDiffThreshold = (double)(m_audioHwBufSize) / m_audioTgt.bytes_per_sec;

        m_audioStreamIndex = stream_index;
        m_pAudioSteam = ic->streams[stream_index];

        if ((ret = decoder_init(&m_audDecoder, avctx, &m_audioPaketQueue, m_pContinueReadCond)) < 0) {
            m_pLogger->log_error("Audio decoder init failed!");
            goto fail;
        }

        if (ic->iformat->flags & AVFMT_NOTIMESTAMPS) {
            m_audDecoder.start_pts = m_pAudioSteam->start_time;
            m_audDecoder.start_pts_tb = m_pAudioSteam->time_base;
        }

        // decoder start 方法还没完成，启动C++11的线程来执行方法.
        packet_queue_start(m_audDecoder.queue);
        m_audDecoder.decoder_thr.reset(new std::thread(&CSJFFPlayerKernel::audio_decode_task, this));
        break;
    case AVMEDIA_TYPE_VIDEO:
        m_videoStreamIndex = stream_index;
        m_pVideoStream = ic->streams[stream_index];

        if ((ret = decoder_init(&m_videoDecoder, avctx, &m_videoPacketQueue, m_pContinueReadCond)) < 0) {
            m_pLogger->log_error("Video decoder init failed!");
            goto fail;
        }

        // 此方法需要启动C++11的线程来执行
        packet_queue_start(m_videoDecoder.queue);
        m_videoDecoder.decoder_thr.reset(new std::thread(&CSJFFPlayerKernel::video_decode_task, this));
        m_queueAttchmentsReq = 1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        m_subtitleStreamIndex = stream_index;
        m_pSubtitleStream = ic->streams[stream_index];

        if ((ret = decoder_init(&m_subtitleDecoder, avctx, &m_subtitlePacketQueue, m_pContinueReadCond)) < 0) {
            m_pLogger->log_error("Subtitle decoder init failed!");
            goto fail;
        }

        // 此方法需要启动C++11的线程来执行
        packet_queue_start(m_subtitleDecoder.queue);
        m_subtitleDecoder.decoder_thr.reset(new std::thread(&CSJFFPlayerKernel::subtitle_decode_task, this));
        break;
    default:
        break;
    }
    goto out;

fail:
    avcodec_free_context(&avctx);
out:
    av_channel_layout_uninit(&ch_layout);
    av_dict_free(&opts);

    return ret;
}

static int decode_interrupt_cb(void *ctx) {
    CSJFFPlayerKernel *kernel = (CSJFFPlayerKernel *)ctx;
    return kernel ? kernel->getAbortRequest() : 0;
}

int CSJFFPlayerKernel::stream_has_enough_packets(AVStream *st,
                                                 int stream_id,
                                                 PacketQueue *queue) {
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
                queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

int CSJFFPlayerKernel::is_realtime(AVFormatContext *s) {
    if(!strcmp(s->iformat->name, "rtp") ||
       !strcmp(s->iformat->name, "rtsp") ||
       !strcmp(s->iformat->name, "sdp")) {
        return 1;
    }

    if(s->pb && (!strncmp(s->url, "rtp:", 4) ||
                 !strncmp(s->url, "udp:", 4))) {
        return 1;
    }

    return 0;
}

int CSJFFPlayerKernel::read_thread() {
    if (m_fileName.size() == 0) {
        m_pLogger->log_error("There's no file need to play");
        return -1;
    }

    if (!CSJPathTool::fileExists(m_fileName)) {
        m_pLogger->log_error("The {} is not exists!", m_fileName);
        return -1;
    }

    CSJLogger *logger = CSJLogger::getLoggerInst();
    logger->log_info("Reading thread start!");

    AVFormatContext *ic = NULL;
    int err, ret;
    const AVDictionaryEntry *t;
    int scan_all_pmts_set = 0;

    int st_index[AVMEDIA_TYPE_NB];
    memset(st_index, -1, sizeof(st_index));

    bool canStartRead = false;
    do {
        ic = avformat_alloc_context();
        if (!ic) {
            // av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
            logger->log_fatal("Could not allocate context.");
            ret = AVERROR(ENOMEM);
            break;
        }

        ic->interrupt_callback.callback = decode_interrupt_cb;
        ic->interrupt_callback.opaque = this;

        // scan all the pmt infos for TS
        if (!av_dict_get(m_pFormatOpts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
            av_dict_set(&m_pFormatOpts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
            scan_all_pmts_set = 1;
        }

        err = avformat_open_input(&ic, m_fileName.c_str(), m_pInputFormat, &m_pFormatOpts);
        if (err < 0) {
            //print_error(m_pFileName, err);
            logger->log_info("Open input stream failed!");
            ret = -1;
            break;
        }

        if (scan_all_pmts_set) {
            av_dict_set(&m_pFormatOpts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);
        }

        t = av_dict_get(m_pFormatOpts, "", NULL, AV_DICT_IGNORE_SUFFIX);
        if (t) {
            //av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
            logger->log_error("Option %s not found.\n", t->key);
            ret = AVERROR_OPTION_NOT_FOUND;
            break;
        }

        static int genpts = 0;
        if (genpts) {
            ic->flags |= AVFMT_FLAG_GENPTS;
        }

        //av_format_inject_global_side_data(ic);

        static int find_stream_info = 1;
        AVDictionary *codec_opts = NULL;
        if (find_stream_info) {
            AVDictionary **opts = setup_find_stream_info_opts(ic, codec_opts);
            int orig_nb_streams = ic->nb_streams;

            err = avformat_find_stream_info(ic, &m_pFormatOpts);

            for (int i = 0; i < orig_nb_streams; i++) {
                av_dict_free(&opts[i]);
            }
            av_freep(&opts);

            if (err < 0) {
                //av_log(NULL, AV_LOG_WARNING, "%s: could not find codec parameters\n", m_pFileName);
                logger->log_warn("{}: could not find codec parameters\n", m_fileName);
                ret = -1;
                break;
            }
        }
        m_pCodecOpts = codec_opts;

        if (ic->pb) {
            ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end
        }

        static int seek_by_bytes = 0;
        if (seek_by_bytes < 0) {
            seek_by_bytes = !(ic->iformat->flags & AVFMT_NO_BYTE_SEEK) &&
                            !!(ic->iformat->flags & AVFMT_TS_DISCONT) &&
                            strcmp("ogg", ic->iformat->name);
        }

        m_maxFrameDuration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

        /* if seeking requested, we execute it */
        if (m_startTime != AV_NOPTS_VALUE) {
            int64_t timestamp;

            timestamp = m_startTime;
            /* add the stream start time */
            if (ic->start_time != AV_NOPTS_VALUE) {
                timestamp += ic->start_time;
            }

            ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
            if (ret < 0) {
                // av_log(NULL, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
                //        m_pFileName, (double)timestamp / AV_TIME_BASE);
                logger->log_warn("{}: could not seek to position %0.3f.",
                                    m_fileName, (double)timestamp / AV_TIME_BASE);
            }
        }

        m_realTime = is_realtime(ic);
        if (m_realTime) {
            logger->log_info("Current content is real-time!");
        }

        if (m_showStatus) {
            av_dump_format(ic, 0, m_fileName.c_str(), 0);
        }

        for (int i = 0; i < ic->nb_streams; i++) {
            AVStream *st = ic->streams[i];
            enum AVMediaType type = st->codecpar->codec_type;
            st->discard = AVDISCARD_ALL;
            if (type >= 0 && m_WantedStreamSpec[type] && st_index[type] == -1) {
                if (avformat_match_stream_specifier(ic, st, m_WantedStreamSpec[type]) > 0) {
                    st_index[type] = i;
                }
            }
        }

        for (int i = 0; i < AVMEDIA_TYPE_NB; i++) {
            if (m_WantedStreamSpec[i] && st_index[i] == -1) {
                // av_log(NULL,
                //        AV_LOG_ERROR,
                //        "Stream specifier %s does not match any %s stream\n",
                //        m_WantedStreamSpec[i], av_get_media_type_string((AVMediaType)i));

                logger->log_error("Stream specifier %s does not match any %s stream\n",
                                    m_WantedStreamSpec[i], av_get_media_type_string((AVMediaType)i));
                st_index[i] = INT_MAX;
            }
        }

        if (!m_bDisableVideo) {
            st_index[AVMEDIA_TYPE_VIDEO] =
                    av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                        st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);

            logger->log_info("Find the best video stream, id: %d", st_index[AVMEDIA_TYPE_VIDEO]);
        }

        if (!m_bDisableAudio) {
            st_index[AVMEDIA_TYPE_AUDIO] =
                    av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                        st_index[AVMEDIA_TYPE_AUDIO],
                                        st_index[AVMEDIA_TYPE_VIDEO],
                                        NULL, 0);
            logger->log_info("Find the best audio stream, id: %d", st_index[AVMEDIA_TYPE_AUDIO]);
        }

        if (!m_bDisableVideo && !m_bDisableSubtitle) {
            st_index[AVMEDIA_TYPE_SUBTITLE] =
                    av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                        st_index[AVMEDIA_TYPE_SUBTITLE], 
                                        st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? 
                                            st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO],
                                        NULL,
                                        0);
            
            logger->log_info("Find the best subtitle stream, id: %d", st_index[AVMEDIA_TYPE_SUBTITLE]);
        }

        //m_showMode = show_mode;
        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
            AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
            AVCodecParameters *codecpar = st->codecpar;
            AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
            if (codecpar->width) {
                logger->log_info("From guessing aspect ratio, size: %d x %d", 
                                    codecpar->width, codecpar->height);
            }
        }

        /* open the streams */
        if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
            ret = stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
            if (ret == -1) {
                logger->log_warn("Open audio component failed!");
            }
        }
        
        ret = -1;
        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
            ret = stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
            if (ret == -1) {
                logger->log_warn("Open video component failed!");
            }
        }

        if (m_showMode == SHOW_MODE_NONE) {
            m_showMode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;
        }

        if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
            ret = stream_component_open(st_index[AVMEDIA_TYPE_SUBTITLE]);
            if (ret == -1) {
                logger->log_warn("Open subtitle component failed!");
            }
        }

        if (m_videoStreamIndex < 0 && m_audioStreamIndex < 0) {
            //av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n", m_pFileName);
            logger->log_fatal("Failed to open file '{}' or configure filtergraph!", m_fileName);
            ret = -1;
            break;
        }

        if (m_inifiteBuffer < 0 && m_realTime) {
            m_inifiteBuffer = 1;
        }
    } while (false);

    if (!canStartRead) {
        if (ic) {
            avformat_close_input(&ic);
        }

        logger->log_error("Stop reading stream due to exceptions!");
        return ret;
    }

    AVPacket *pkt = NULL;
    pkt = av_packet_alloc();
    if (!pkt) {
        //av_log(NULL, AV_LOG_FATAL, "Could not allocate packet.\n");
        logger->log_fatal("Could not allocate packet.");
        ret = AVERROR(ENOMEM);
        return ret;
    }

    int64_t pkt_ts = 0;
    int64_t stream_start_time = 0;
    int     pkt_in_play_range = 0;

    m_pFormatCtx = ic;
    m_eof = 0;

    std::mutex wait_mutex;
    std::unique_lock<std::mutex> uniqueLock(wait_mutex);
    // start entering the read cycle.
    logger->log_info("Start reading stream packets ...");
    for (;;) {
        if (m_abortRequest) {
            break;
        }

        if (m_paused != m_lastPaused) {
            m_lastPaused = m_paused;
            if (m_paused) {
                m_readPauseReturn = av_read_pause(m_pFormatCtx);
            } else {
                av_read_play(m_pFormatCtx);
            }
        }
#if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
        if (m_paused &&
            (!strcmp(m_pFormatCtx->iformat->name, "rtsp") ||
             (m_pFormatCtx->pb && !strncmp(input_filename, "mmsh:", 5)))) {
            /* wait 10 ms to avoid trying to get another packet */
            /* XXX: horrible */
            SDL_Delay(10);
            continue;
        }
#endif
        if (m_seekReq) {
            logger->log_info("Seeking stream ...");
            int64_t seek_target = m_seekPos;
            int64_t seek_min    = m_seekRel > 0 ? seek_target - m_seekRel + 2: INT64_MIN;
            int64_t seek_max    = m_seekRel < 0 ? seek_target - m_seekRel - 2: INT64_MAX;
            // FIXME the +-2 is due to rounding being not done in the correct direction in generation
            //      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(m_pFormatCtx, -1, seek_min, seek_target, seek_max, m_seekFlags);
            if (ret < 0) {
                //av_log(NULL, AV_LOG_ERROR, "%s: error while seeking\n", m_pFormatCtx->url);
                logger->log_error("%s: error while seeking\n", m_pFormatCtx->url);
            } else {
                if (m_audioStreamIndex >= 0) {
                    packet_queue_flush(&m_audioPaketQueue);
                }

                if (m_subtitleStreamIndex >= 0) {
                    packet_queue_flush(&m_subtitlePacketQueue);
                }

                if (m_videoStreamIndex >= 0) {
                    packet_queue_flush(&m_videoPacketQueue);
                }

                if (m_seekFlags & AVSEEK_FLAG_BYTE) {
                    set_clock(&m_extClk, NAN, 0);
                } else {
                    set_clock(&m_extClk, seek_target / (double)AV_TIME_BASE, 0);
                }
            }

            m_seekReq = 0;
            m_queueAttchmentsReq = 1;
            m_eof = 0;
            if (m_paused) {
                step_to_next_frame();
            }
        } // m_seekReq;

        if (m_queueAttchmentsReq) {
            if (m_pVideoStream && m_pVideoStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                if ((ret = av_packet_ref(pkt, &m_pVideoStream->attached_pic)) < 0) {
                    break;
                }

                packet_queue_put(&m_videoPacketQueue, pkt);
                packet_queue_put_nullpacket(&m_videoPacketQueue, pkt, m_videoStreamIndex);
            }

            m_queueAttchmentsReq = 0;
        }

        /* if the queue are full, no need to read more */
        if (m_inifiteBuffer < 1 &&
            (m_audioPaketQueue.size + m_videoPacketQueue.size + m_subtitlePacketQueue.size > MAX_QUEUE_SIZE || 
                (stream_has_enough_packets(m_pAudioSteam, m_audioStreamIndex, &m_audioPaketQueue) &&
                 stream_has_enough_packets(m_pVideoStream, m_videoStreamIndex, &m_videoPacketQueue) &&
                 stream_has_enough_packets(m_pSubtitleStream, m_subtitleStreamIndex, &m_subtitlePacketQueue)))) {
            /* wait 10 ms */
            wait_mutex.lock();
            m_pContinueReadCond->wait_for(uniqueLock, std::chrono::milliseconds(10));
            wait_mutex.unlock();
            continue;
        }

        // Loop play, 
        if (!m_paused &&
            (!m_pAudioSteam || (m_audDecoder.finished == m_audioPaketQueue.serial && frame_queue_nb_remaining(&m_audioFrameQueue) == 0)) &&
            (!m_pVideoStream || (m_videoDecoder.finished == m_videoPacketQueue.serial && frame_queue_nb_remaining(&m_videoFrameQueue) == 0))) {
           if (m_loopNumber != 1 && (!m_loopNumber || --m_loopNumber)) {
                // seek the file to beginning.
                stream_seek(0, 0, 0);
           } else {
                // exit current process. 
                break;
           }
        }

        ret = av_read_frame(m_pFormatCtx, pkt);
        if (ret < 0) {
            if ((ret == AVERROR_EOF || avio_feof(m_pFormatCtx->pb)) && !m_eof) {
                if (m_videoStreamIndex >= 0) {
                    packet_queue_put_nullpacket(&m_videoPacketQueue, pkt, m_videoStreamIndex);
                }

                if (m_audioStreamIndex >= 0) {
                    packet_queue_put_nullpacket(&m_audioPaketQueue, pkt, m_audioStreamIndex);
                }

                if (m_subtitleStreamIndex >= 0) {
                    packet_queue_put_nullpacket(&m_subtitlePacketQueue, pkt, m_subtitleStreamIndex);
                }

                m_eof = 1;
            }

            if (m_pFormatCtx->pb && m_pFormatCtx->pb->error) {
                break;
            }

            wait_mutex.lock();
            m_pContinueReadCond->wait_for(uniqueLock, std::chrono::milliseconds(10));
            wait_mutex.unlock();
            continue;
        } else {
            m_eof = 0;
        }

        /* check if packet is in play range specified by user, then queue, otherwise discard */
        stream_start_time = m_pFormatCtx->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;

        // Check the packet is in the duration that users indicate or not, and if users don't set the 
        // m_playDuration, means curretn packet is always in right range. 
        if (m_playDuration == AV_NOPTS_VALUE) {
            pkt_in_play_range = true;
        } else {
            // the duration from current stream's starting.
            int64_t time_from_stream = (pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
                                        av_q2d(m_pFormatCtx->streams[pkt->stream_index]->time_base);
            // the duration from playing.
            double time_from_play = (double)(m_startTime != AV_NOPTS_VALUE ? m_startTime : 0) / 1000000;

            double time_diff = time_from_stream - time_from_play;

            pkt_in_play_range = m_playDuration == AV_NOPTS_VALUE || time_diff <= ((double)m_playDuration / 1000000);
        }

        if (pkt->stream_index == m_audioStreamIndex && pkt_in_play_range) {
            packet_queue_put(&m_audioPaketQueue, pkt);
        } else if (pkt->stream_index == m_videoStreamIndex && pkt_in_play_range
                   && !(m_pVideoStream->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            packet_queue_put(&m_videoPacketQueue, pkt);
        } else if (pkt->stream_index == m_subtitleStreamIndex && pkt_in_play_range) {
            packet_queue_put(&m_subtitlePacketQueue, pkt);
        } else {
            av_packet_unref(pkt);
        }
    }

    if (m_pFormatCtx) {
        avformat_close_input(&m_pFormatCtx);
    }

    av_packet_free(&pkt);
    logger->log_info("Read thread quit successfully!");
    return 0;
}

bool CSJFFPlayerKernel::stream_open() {
     CSJLogger *logger = CSJLogger::getLoggerInst();

    if (!m_abortRequest) {
        logger->log(CSJLogger::LogLevel::INFO_LOG, "Current status is playing, stop playing first!");
        stream_close();
    }

    logger->log(CSJLogger::LogLevel::INFO_LOG, "Initialize play components, it's going to play.");
    m_abortRequest = false;

    bool ret = false;
    m_lastVideoStream = m_videoStreamIndex = -1;
    m_lastAudioStream = m_audioStreamIndex = -1;
    m_lastSubtitleStream = m_subtitleStreamIndex = -1;

    m_ytop = 0;
    m_xleft = 0;

    /* Initialize queues and clocks, and then start the read thread. */
    do {
        if (frame_queue_init(&m_videoFrameQueue, &m_videoPacketQueue, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
            break;
        if (frame_queue_init(&m_subtitleFrameQueue, &m_subtitlePacketQueue, SUBPICTURE_QUEUE_SIZE, 0) < 0)
            break;
        if (frame_queue_init(&m_audioFrameQueue, &m_audioPaketQueue, SAMPLE_QUEUE_SIZE, 1) < 0)
            break;

        if (packet_queue_init(&m_videoPacketQueue) < 0 ||
            packet_queue_init(&m_audioPaketQueue) < 0 ||
            packet_queue_init(&m_subtitlePacketQueue) < 0) {
            break;
        }

        init_clock(&m_vidClk, &m_videoPacketQueue.serial);
        init_clock(&m_audClk, &m_audioPaketQueue.serial);
        init_clock(&m_extClk, &m_extClk.serial);

        m_audioClockSerial = -1;
        if (m_startupVolume < 0) {
            //av_log(NULL, AV_LOG_WARNING, "-volume=%d < 0, setting to 0\n", m_startupVolume);
            logger->log_warn("-volume=%d < 0, setting to 0\n", m_startupVolume);
        }

        if (m_startupVolume > 100) {
            //av_log(NULL, AV_LOG_WARNING, "-volume=%d > 100, setting to 100\n", m_startupVolume);
            logger->log_warn("-volume=%d > 100, setting to 100\n", m_startupVolume);
        }

        m_startupVolume = av_clip(m_startupVolume, 0, 100);
        m_startupVolume = av_clip(SDL_MIX_MAXVOLUME * m_startupVolume / 100, 0, SDL_MIX_MAXVOLUME);
        //m_audioVolume = m_startupVolume;
        m_muted = 0;
        //m_avSyncType = av_sync_type;

        m_pReadThread.reset(new std::thread(&CSJFFPlayerKernel::read_thread, this));
        ret = true;
    } while (false);

    logger->log(CSJLogger::LogLevel::INFO_LOG, "Stream open succussfully!");

    return ret;
}

void CSJFFPlayerKernel::do_exit() {
    stream_close();
}

void CSJFFPlayerKernel::threadTestFunc(int t) {
    while (1) {

        // pause logic;
        if (m_paused) {
            std::unique_lock<std::mutex> lock(m_pauseMtx);
            threadLog(t, 1);
            m_pauseCond.wait(lock);
            threadLog(t, 2);
        }

        if (m_abortRequest) {
            threadLog(t, 3);
            break;
        }

        threadLog(t, 4);
        
        // a sleep logic;
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void CSJFFPlayerKernel::threadLog(int thread_type, int log_type) {
    static std::string threadType[] = {
        "Video decoder thread",
        "Audio decoder thread",
        "Subtitle decoder thread",
        "Reading thread"
    };

    static std::string logString[] = {
        "start",
        "pause",
        "resume",
        "stop",
        "decoding packet",
        "read packet",
        "exit"
    };

    std::string outputLog = "";
    outputLog.append("[Thread Debug] ");
    outputLog.append(threadType[thread_type]);
    outputLog.append(": ");
    outputLog.append(logString[log_type]);

    std::cout << outputLog;
}

void CSJFFPlayerKernel::readThreadTest() {
    while (1) {

        // TODO: add pause logic;
        if (m_paused) {
            std::unique_lock<std::mutex> lock(m_pauseMtx);
            threadLog(3, 1);
            m_pauseCond.wait(lock);
            threadLog(3, 2);
        }

        if (m_abortRequest) {
            threadLog(3, 3);
            break;
        }

        threadLog(3, 5);

        // a sleep logic;
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    threadLog(3, 5);
}

void CSJFFPlayerKernel::play() {
    stream_open();

    // m_pRenderThread.reset(new std::thread(&CSJVideoRendererWidget::internalRender, this));

    // m_pReadThread.reset(new std::thread(&CSJFFPlayerKernel::readThreadTest, this));

    // m_pVideoDecThread.reset(new std::thread(&CSJFFPlayerKernel::threadTestFunc, this, 0));
    // m_pAudioDecThread.reset(new std::thread(&CSJFFPlayerKernel::threadTestFunc, this, 1));
    // m_pSubtitleDecThread.reset(new std::thread(&CSJFFPlayerKernel::threadTestFunc, this, 2));

}

void CSJFFPlayerKernel::pause() {
    // stream_toggle_pause();
    // m_step = 0;

    m_paused = true;
}

void CSJFFPlayerKernel::resume() {
    // stream_toggle_pause();
    // m_step = 0;

    //std::lock_guard<std::mutex> lock(m_pauseMtx);

    m_paused = false;
    m_pauseCond.notify_all();
}

void CSJFFPlayerKernel::stop() {
    // m_abortRequest = true;
    /* In case current state is pause. */
    resetPlayState();
}

void CSJFFPlayerKernel::seek(double timeStamp) {

}

bool CSJFFPlayerKernel::isPlaying() {
    return !m_abortRequest;
}

bool CSJFFPlayerKernel::isPause() {
    return m_paused;
}

bool CSJFFPlayerKernel::isStop() {
    return m_abortRequest;
}

CSJFFPlayerKernel::CSJFFPlayerKernel() 
    : m_pLogger(CSJLogger::getLoggerInst()) {
    m_abortRequest = true;
    m_paused = false;
}

CSJFFPlayerKernel::~CSJFFPlayerKernel() {
    resetPlayState();
}

void CSJFFPlayerKernel::setPlayFile(std::string &filePath) {

}

bool CSJFFPlayerKernel::initPlayer() {
    return true;
}

int CSJFFPlayerKernel::getDuration() {
    return 0;
}

} // namespace csjmediaengine 