//#include "ffmpeg.h"

//FFmpeg::FFmpeg()
//{

//}

#include "mdelogo.h"

VideoStream::VideoStream()
{

}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    //    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

//    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
//           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
//           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
//           pkt->stream_index);
}

int VideoStream::write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt, int stream_type)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    log_packet(fmt_ctx, pkt);
    int ret = av_interleaved_write_frame(fmt_ctx, pkt);
    av_packet_unref(pkt);
    return ret;
}

void VideoStream::add_stream(OutputStream *ost, AVFormatContext *oc,
                       const AVCodec **codec,
                       enum AVCodecID codec_id)
{
        AVCodecContext *c;
        int i;

        /* find the encoder */
        if(codec_id == AV_CODEC_ID_H264 )
        {

            *codec = avcodec_find_encoder_by_name("h264_nvenc");
        }
        else
        {
            //*codec = avcodec_find_encoder(codec_id);
            *codec = avcodec_find_encoder_by_name("aac");
        }
        ost->st = avformat_new_stream(oc, *codec);
        if (!ost->st) {
            fprintf(stderr, "Could not allocate stream\n");
            exit(1);
        }
        c = avcodec_alloc_context3(*codec);

        ost->st->id = oc->nb_streams-1;
        ost->enc = c;
        //c = ost->st->codec;
        //avcodec_get_context_defaults3(c, *codec);
        //c->refcounted_frames = 1;


        switch ((*codec)->type) {
        case AVMEDIA_TYPE_AUDIO:

            c->codec_id = codec_id; //AV_SAMPLE_FMT_S16;
            c->sample_fmt  = input.pACodecContext->sample_fmt;//(*codec)->sample_fmts ?
            //(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
            c->bit_rate    =  input.pACodecContext->bit_rate;
            c->sample_rate = input.pACodecContext->sample_rate;

            if ((*codec)->supported_samplerates) {
                    c->sample_rate = (*codec)->supported_samplerates[0];
                    for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                            if ((*codec)->supported_samplerates[i] == 44100)
                                    c->sample_rate = 44100;
                    }
            }
            c->channel_layout = AV_CH_LAYOUT_STEREO;
            if ((*codec)->channel_layouts) {
                c->channel_layout = (*codec)->channel_layouts[0];
                for (i = 0; (*codec)->channel_layouts[i]; i++) {
                    if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                        c->channel_layout = AV_CH_LAYOUT_STEREO;
                }
            }

            c->channels        = av_get_channel_layout_nb_channels(input.pACodecContext->channel_layout);
            c->channel_layout = AV_CH_LAYOUT_STEREO;
            if ((*codec)->channel_layouts) {
                c->channel_layout = (*codec)->channel_layouts[0];
                for (i = 0; (*codec)->channel_layouts[i]; i++) {
                    if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                        c->channel_layout = AV_CH_LAYOUT_STEREO;
                }
            }
            c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
            //
            ost->st->time_base = (AVRational){ 1, c->sample_rate };
            //ost->st->time_base = (AVRational){ 1, 1000 };
            break;

        case AVMEDIA_TYPE_VIDEO:
            c->codec_id = codec_id;

            c->bit_rate = bitrate;//input.pVCodecContext->bit_rate;//bitrate;
            /* Resolution must be a multiple of two. */
            c->width    = input.pVCodecContext->width; //width
            c->height   = input.pVCodecContext->height; //height
            /* timebase: This is the fundamental unit of time (in seconds) in terms
                 * of which frame timestamps are represented. For fixed-fps content,
                 * timebase should be 1/framerate and timestamp increments should be
                 * identical to 1. */
            //  ost->st->time_base = (AVRational){ 1, input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.num };

            cout << "FRAME NUM = "  << input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.num << endl;
            cout << "FRAME DEN = "  << input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.den << endl;
            //            ost->st->time_base = (AVRational){ input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.num,
            //                                               input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.den };

            //            if(input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.num
            //               >  input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.den)
            //                ost->st->time_base = (AVRational){ input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.den,
            //                                                   input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.num };

            ost->st->time_base = (AVRational){ 1,25};

            c->time_base       = ost->st->time_base;
            c->framerate = (AVRational){ input.ifmt_ctx->streams[input.videoStream]->r_frame_rate.num };

        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        c->max_b_frames=2;
        //c->max_b_frames = 2;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
                     * This does not happen with normal video, it just happens here as
                     * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
        break;

        default:
            break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    ost->enc = c;
    int ret = avcodec_parameters_from_context(ost->st->codecpar,c);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream\n");
        exit(1);
    }
}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

void VideoStream::open_audio(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;

    //c = ost->st->codec;
    c = ost->enc;
    //fdata = static_cast<uint8_t *>(malloc(1024));//(uint8_t*)malloc(1024 * sizeof(uint8_t));
    /* open it */
    //c->strict_std_compliance = -2;

    //av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    //av_dict_free(&opt);

    if (ret < 0) {
        exit(1);
    }

    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    ost->frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                       c->sample_rate, nb_samples);

    ost->tmp_frame = alloc_audio_frame(input.pACodecContext->sample_fmt, input.pACodecContext->channel_layout,
                                       input.pACodecContext->sample_rate, input.pACodecContext->frame_size);

    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    /* create resampler context */
    //    ost->swr_ctx = swr_alloc();

    ost->swr_ctx = swr_alloc_set_opts(NULL,  // we're allocating a new context
                                      (c->channel_layout),  // out_ch_layout
                                      c->sample_fmt,     // out_sample_fmt
                                      c->sample_rate,                // out_sample_rate
                                      (input.pACodecContext->channel_layout),  // in_ch_layout
                                      input.pACodecContext->sample_fmt,   // in_sample_fmt
                                      input.pACodecContext->sample_rate,                // in_sample_rate
                                      0,                    // log_offset
                                      NULL);

    if (!ost->swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        exit(1);
    }

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context: %d\n", ret);
        exit(1);
    }
}

static int decode_audio_frame(AVFrame *frame,
                              AVFormatContext *input_format_context,
                              AVCodecContext *input_codec_context,
                              int *data_present, int *finished, AVPacket input_packet)
{
    /* Packet used for temporary storage. */
    //    AVPacket *input_packet;
    int error;

    //    error = init_packet(input_packet);
    //    if (error < 0)
    //        return error;


    /* Read one audio frame from the input file into a temporary packet. */
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0) {
        /* If we are at the end of the file, flush the decoder below. */
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            fprintf(stderr, "Could not read frame (error '%s')\n",
                    /*av_err2str*/(error));
            goto cleanup;
        }
    }
    printf("ONE TIME\n");
    /* Send the audio frame stored in the temporary packet to the decoder.
     * The input audio stream decoder is used to do this. */
    if ((error = avcodec_send_packet(input_codec_context, &input_packet)) < 0) {
        fprintf(stderr, "Could not send packet for decoding (error '%s')\n",
                /*av_err2str*/(error));
        goto cleanup;
    }

    /* Receive one frame from the decoder. */
    error = avcodec_receive_frame(input_codec_context, frame);
    /* If the decoder asks for more data to be able to decode a frame,
     * return indicating that no data is present. */
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
        /* If the end of the input file is reached, stop decoding. */
    } else if (error == AVERROR_EOF) {
        *finished = 1;
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not decode frame (error '%s')\n",
                /*av_err2str*/(error));
        goto cleanup;
        /* Default case: Return decoded data. */
    } else {
        *data_present = 1;
        goto cleanup;
    }

cleanup:
    //av_packet_free(input_packet);
    return error;
}

int VideoStream::init_packet(AVPacket *packet)
{
    //av_init_packet(packet);
    packet = av_packet_alloc();
    //av_new_packet(packet, 0);
    /* Set the packet data and size so that it is recognized as being empty. */
    //packet->data = NULL;
    //packet->size = 0;

    return 0;
}

AVAudioFifo *fifo = NULL;
int VideoStream::init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context)
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(*fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt,
                                      output_codec_context->channels, output_codec_context->frame_size))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int VideoStream::add_samples_to_fifo(AVAudioFifo *fifo,
                               uint8_t **converted_input_samples,
                               const int frame_size)
{
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
                            frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}


int convert_samples(const uint8_t **input_data,
                           uint8_t **converted_data, const int frame_size,
                           SwrContext *resample_context)
{
    int error;
    /* Convert the samples using the resampler. */
    if ((error = swr_convert(resample_context,
                             converted_data, frame_size,
                             input_data    , frame_size)) < 0) {
        fprintf(stderr, "Could not convert input samples (error '%d')\n", error);
        return error;
    }
    return 0;
}


int init_converted_samples(uint8_t ***converted_input_samples,
                                  AVCodecContext *output_codec_context,
                                  int frame_size)
{
    int error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = static_cast<uint8_t**>(calloc(output_codec_context->channels,
                                                                   sizeof(**converted_input_samples))))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  output_codec_context->channels,
                                  frame_size,
                                  output_codec_context->sample_fmt, 0)) < 0) {
        fprintf(stderr,
                "Could not allocate converted input samples (error '%d')\n", error);
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}

int VideoStream::conv_audio(AVFormatContext *oc, OutputStream *ost, AVPacket apkt)
{
    AVCodecContext *c;
    // data and size must be 0;
    //pkt = apkt;
    //pkt.pts = 0;

    int ret = 0;
    int got_packet = 0;
    int dst_nb_samples = 0;
    int out_samples = 0;
    int swr_ret = 0;
    AVFrame *input_frame = NULL;
    c = ost->enc;

    const int output_frame_size = c->frame_size;
    if(apkt.size < 1024)
    {
        return 0;
    }
    printf("APKT size %d\n", apkt.size);

    int finished                = 0;


    while (av_audio_fifo_size(fifo) < output_frame_size) {

        read_decode_convert_and_store(fifo, input.ifmt_ctx,
                                      input.pACodecContext,
                                      c,
                                      ost->swr_ctx, apkt, &finished);
    }
    while (av_audio_fifo_size(fifo) >= output_frame_size )
    {
        load_encode_and_write(fifo, oc, c, ost);
        //printf("OUT %d\n", av_audio_fifo_size(fifo));
    }


    return ret;
    //cleanup:

}

int VideoStream::read_decode_convert_and_store(AVAudioFifo *fifo,
                                         AVFormatContext *input_format_context,
                                         AVCodecContext *input_codec_context,
                                         AVCodecContext *output_codec_context,
                                         SwrContext *resampler_context, AVPacket apkt,
                                         int *finished)
{
    /* Temporary storage of the input samples of the frame read from the file. */


    /* Temporary storage for the converted input samples. */
    uint8_t **converted_input_samples = NULL;
    int data_present = 0;
    int ret = AVERROR_EXIT;

    AVFrame *input_frame;// = av_frame_alloc();
    //input_frame = get_audio_frame(AVFormatContext *input_format_context, apkt);

    decode_audio_frame(input_frame,
                       input_format_context,
                       input_codec_context,
                       &data_present, finished, apkt);

    AVFrame *out_frame = alloc_audio_frame(output_codec_context->sample_fmt, output_codec_context->channel_layout,
                                           output_codec_context->sample_rate, input_frame->nb_samples);
    if(!input_frame)
    {
        printf("NO AUDIO FRAME\n");
        return -1;
    }

    if (input_frame) {
        /* Initialize the temporary storage for the converted input samples. */
        //        if (init_converted_samples(*out_frame->data, output_codec_context,
        //                                   input_frame->nb_samples))
        //            goto cleanup;
        /* Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples. */
        //        if (convert_samples((const uint8_t**)input_frame->extended_data, out_frame->data,
        //                            input_frame->nb_samples, resampler_context))

        ret = swr_config_frame(resampler_context, out_frame, input_frame);
        if (ret < 0) {
            fprintf(stderr, "Error while CONFIG\n");
            goto cleanup;
            //exit(1);
        }

        ret = swr_convert_frame(resampler_context, out_frame, input_frame);

        if (ret < 0) {
            fprintf(stderr, "Error while converting %d\n", ret);
            goto cleanup;
            //exit(1);
        }
        // goto cleanup;
        /* Add the converted input samples to the FIFO buffer for later processing. */
        if (add_samples_to_fifo(fifo, out_frame->extended_data,
                                input_frame->nb_samples))
            goto cleanup;
        ret = 0;
    }
    else
    {
        printf("NO INPUT AUDIO FRAME\n");
    }
    ret = 0;
    *finished = 1;
cleanup:
    //    if (converted_input_samples) {
    //        av_freep(&converted_input_samples[0]);
    //        free(converted_input_samples);
    //    }
    av_frame_free(&out_frame);
    av_frame_free(&input_frame);
    return ret;
}

int VideoStream::encode_audio_frame(AVFrame *frame,
                              AVFormatContext *output_format_context,
                              AVCodecContext *output_codec_context,
                              int *data_present, OutputStream *ost)
{
    /* Packet used for temporary storage. */
    AVPacket *output_packet;
    int error;
    int got_packet = -1;
    int dst_nb_samples = 0;
    int ret = 0;
    init_packet(output_packet);
    output_packet = av_packet_alloc();
    AVCodecContext *c = output_codec_context;

    /* Set a timestamp based on the sample rate for the container. */
    if (frame)
    {

        //ost->frame = frame;
        av_frame_unref(ost->frame);
        av_frame_move_ref(ost->frame, frame);
        //if (frame) {
        AVRational sr = { 1, output_codec_context->sample_rate };
        //frame->pkt_pts = av_rescale_q(ost->samples_count, ost->st->time_base, output_codec_context->time_base);
        ost->frame->pts = av_rescale_q(ost->samples_count, (AVRational){1, output_codec_context->sample_rate}, output_codec_context->time_base);
        //ost->frame->pkt_pts = av_rescale_q(ost->samples_count, (AVRational){1, output_codec_context->sample_rate}, output_codec_context->time_base);

        //frame->dts = frame->pts;
        ost->samples_count += ost->frame->nb_samples;
        //printf("ost->samples_count nb_samples %lld %lld\n", ost->samples_count,  ost->frame->nb_samples);
        //}


        error = avcodec_send_frame(output_codec_context, ost->frame);
        /* The encoder signals that it has nothing more to encode. */
        if (error == AVERROR_EOF) {
            error = 0;
            //goto cleanup;
        } else if (error < 0) {
            fprintf(stderr, "Could not send packet for encoding (error '%s')\n",
                    /*av_err2str*/(error));
            // goto cleanup;
        }

        /* Receive one encoded frame from the encoder. */
        error = avcodec_receive_packet(output_codec_context, output_packet);
        //        int ret = avcodec_encode_audio2(output_codec_context, &output_packet, ost->frame, data_present);
        //        if (ret < 0) {
        //            printf("Error encoding audio frame: %d\n", ret);
        //            return ret;
        //            // exit(1);
        //        }

        //*data_present = 1;
        /* Write one audio frame from the temporary packet to the output file. */
        if(*data_present)
        {
            //printf("Write Audio packet %3"PRId64" (size=%5d)\n", output_packet.pts, output_packet.size);
            ret = write_frame(output_format_context, &output_codec_context->time_base, ost->st, output_packet, input.audioStream);
            if(ret < 0)
            {
                printf("Audio packet write error %d\n", ret);
            }

        }
    }
    av_frame_free(&frame);
    av_frame_unref(ost->frame);
}

int VideoStream::init_output_frame(AVFrame **frame,
                             AVCodecContext *output_codec_context,
                             int frame_size)
{
    int error;
    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }
    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        fprintf(stderr, "Could not allocate output frame samples (error '%d')\n", error);
        av_frame_free(frame);
        return error;
    }
    return 0;
}

int VideoStream::load_encode_and_write(AVAudioFifo *fifo,
                                 AVFormatContext *output_format_context,
                                 AVCodecContext *output_codec_context, OutputStream *ost)
{
    /* Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /* Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size. */
    const int frame_size =   FFMIN(av_audio_fifo_size(fifo),
                                   output_codec_context->frame_size);
    int data_written;
    /* Initialize temporary storage for one output frame. */
    //printf("FRAME SIZE %d\n", frame_size);
    if (init_output_frame(&output_frame, output_codec_context, frame_size))
        return AVERROR_EXIT;
    /* Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily. */
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    /* Encode one frame worth of audio samples. */
    if (encode_audio_frame(output_frame, output_format_context,
                           output_codec_context, &data_written, ost)) {
        //        if(output_frame)
        //        {
        //            av_frame_free(&output_frame);
        //        }
        return AVERROR_EXIT;
    }
    return 0;
}

AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}


SwsContext *scale_context;

void VideoStream::open_video(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c;// =  ost->st->codec;
    c = ost->enc;
    AVDictionary *opt = NULL;


    //av_dict_copy(&opt, opt_arg, 0);

    //av_opt_set_int(oc, "threads", (int64_t)10, 0);
    //av_opt_set_int(oc, "fps", (int64_t)60, 0);

    /* open the codec */
    //av_dict_set(&opt, "r", "25", 0);
    ret = avcodec_open2(c, codec, &opt);
    //av_dict_free(&opt);
    if (ret < 0) {
        //fprintf(stderr, "Could not open video codec: %s\n", /*av_err2str*/(ret));
        exit(1);
    }

    /* allocate and init a re-usable frame */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    //    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, input.pVCodecContext->width, input.pVCodecContext->height, 24);
    //    uint8_t * bgr_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    //    input.rgbFrame = av_frame_alloc();
    //    ret = av_image_fill_arrays(input.rgbFrame->data, input.rgbFrame->linesize, bgr_buffer, AV_PIX_FMT_BGR24, input.pVCodecContext->width,
    //                               input.pVCodecContext->height, 24);

    scale_context = sws_getCachedContext(NULL,input.pVCodecContext->width, input.pVCodecContext->height,
                                         AV_PIX_FMT_YUV420P, input.pVCodecContext->width,
                                         input.pVCodecContext->height,
                                         AV_PIX_FMT_BGR24, SWS_BICUBIC,
                                         NULL, NULL, NULL);
    //    av_freep(bgr_buffer);
}

bool VideoStream::process_video_frame(vector<AVPacket> vvpkts)
{
    //AVCodecContext *c = ost->enc;//ost->st->codec;
    //AVCodec* in_codec = avcodec_find_decoder(input.pVCodecContext->codec_id);
    if(!init_v)
    {
        //const AVCodec* in_codec = avcodec_find_decoder_by_name("h264_cuvid"/*input.pVCodecContext->codec_id*/);
        const AVCodec* in_codec = avcodec_find_decoder(input.pVCodecContext->codec_id);
        avcodec_open2(input.pVCodecContext, in_codec, NULL);
        init_v = true;
    }


    int ret = 0;
    int got = 0;

    for(int i=0; i<vvpkts.size(); i++)
    {

        AVFrame* decframe = av_frame_alloc();
        //        ret = avcodec_decode_video2(input.pVCodecContext, decframe, &got, &vvpkts[i]);

        ret = avcodec_send_packet(input.pVCodecContext, &vvpkts[i]);
        if (ret < 0) {
            continue;
        }

        AVFrame* frame = NULL;

        while (ret >= 0) {

            ret = avcodec_receive_frame(input.pVCodecContext, decframe);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            frame = av_frame_alloc();

            int imgsize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, input.pVCodecContext->width, input.pVCodecContext->height, 32);
            if (imgsize < 0)
            {
                printf("Bad av_image_get_buffer_size: %d\n", imgsize);
            }

            std::vector<uint8_t> framebuf(imgsize);
            imgsize = av_image_fill_arrays(frame->data, frame->linesize, framebuf.data(), AV_PIX_FMT_BGR24, input.pVCodecContext->width, input.pVCodecContext->height, 32);
            if (imgsize < 0)
            {
                printf("Bad av_image_fill_arrays: %d\n", imgsize);
            }

            sws_scale(scale_context, decframe->data, decframe->linesize, 0, decframe->height, frame->data, frame->linesize);

            cv::Mat img(input.pVCodecContext->height,input.pVCodecContext->width,CV_8UC3,frame->data[0]);
            if(img.empty())
            {
                img.release();
                av_frame_free(&frame);
                av_frame_free(&decframe);
                av_frame_unref(frame);
                av_frame_unref(decframe);
                free(decframe);
                free(frame);
                framebuf.clear();
                continue;
            }
            cuda::GpuMat gMat;
            gMat.upload(img);
            umats.push_back(gMat);
            //cout << "VCN = " << vcn << endl;
            //img.copyTo(umats[vcn]);

            if(!initial_z){
                Mat msk = img;
                zeros = cv::Mat::zeros(msk.rows, msk.cols, msk.type());
                cv::cvtColor(zeros, zeros, cv::COLOR_RGB2GRAY);
                empmat = zeros;
                initial_z = true;

            }
            vcn++;
            img.release();

        }

        av_frame_free(&frame);
        av_frame_free(&decframe);
        av_frame_unref(frame);
        av_frame_unref(decframe);
        free(decframe);
        free(frame);
        //framebuf.clear();
    }

    //found = process_vframe();
    //found = true;
    //return true;
    if(umats.empty())
    {
        //cout << "empty" << endl;
        return false;
    }


    for (auto it = umats.cbegin(); it != umats.cend() /* not hoisted */; /* no increment */)
    {
        if ((*it).empty())
        {
            it = umats.erase(it);
        }
        else
        {
            //cv::cvtColor((*it), (*it), cv::COLOR_RGB2GRAY);
            ++it;
        }
    }

    cuda::GpuMat tmpMat(umats.at(0).rows,umats.at(0).cols, empmat.type());
    //Mat tmpMat(umats.at(0).rows,umats.at(0).cols, empmat.type());

    //    for (auto it = umats.cbegin(); it != umats.cend() /* not hoisted */; /* no increment */)
    //    {
    //        Mat tmn = (*it);
    //        cvtColor(tmn, tmn, cv::COLOR_RGB2GRAY);
    //        Canny( tmn, tmn, 240, 255, 3);
    //        addWeighted(tmpMat, 1.0, tmn, 1.0/umats.size(), 0, tmpMat);
    //        ++it;
    //    }

    cv::Mat fmat;
    int kernel_size = 3;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_8U;

    if(!initial)
    {
        //lapFilter0 = cv::cuda::createLaplacianFilter(ddepth, ddepth, kernel_size, scale, delta, BORDER_DEFAULT);
        cu_dilate = cv::cuda::createMorphologyFilter(cv::MORPH_DILATE, mytemplate.type(), fmat, Point(-1,-1), 1);
        initial = true;
    }


    for (auto it = umats.cbegin(); it != umats.cend() /* not hoisted */; /* no increment */)
    {
        cuda::GpuMat tmn = *it;
        //tmn.upload(*it);
        //cvtColor(tmn, tmn, cv::COLOR_RGB2GRAY);
        cv::cuda::cvtColor(tmn, tmn, cv::COLOR_RGB2GRAY);
        cuda::GpuMat tmp(tmn.size() , tmn.type());

        Mat c_tmp, c_tmn;
        tmn.download(c_tmn);


        Laplacian( c_tmn, c_tmp, ddepth, kernel_size, scale, delta, BORDER_DEFAULT );
        convertScaleAbs( c_tmp, c_tmn );

        //lapFilter0->apply(tmn, tmp);
        //RECHECK

        tmn.upload(c_tmn);
        cv::cuda::addWeighted(tmpMat, 1.0, tmn,  1.0/umats.size(), 0, tmpMat);
        //addWeighted(tmpMat, 1.0, tmn, 1.0/umats.size(), 0, tmpMat);

        ++it;

    }


    //cuda::GpuMat aMat(tmpMat.rows, tmpMat.cols, mytemplate.type());
    Mat aMat(tmpMat.rows, tmpMat.cols, mytemplate.type());
    //    cv::dilate(aMat,aMat,Mat(),Point(-1,-1), 1);
    //    cv::erode (aMat,aMat,Mat(),Point(-1,-1), 1);




    cv::dilate(aMat,aMat,Mat(),Point(-1,-1), 1);
    cuda::GpuMat g_aMat;
    g_aMat.upload(aMat);
    //cu_dilate->apply(tmpMat, aMat);
    found = process_vframe(g_aMat);
    aMat.release();
    tmpMat.release();
    return found;
}
/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
int tbs = 0;
int VideoStream::write_video_frame(AVFormatContext *oc, OutputStream *ost, cuda::GpuMat ocvv)
{
    Mat occv;
    ocvv.download(occv);
    int ret;
    AVCodecContext *c;
    AVFrame *frame;
    int got_packet = 0;
    AVPacket pkt = { 0 };

    //c = ost->st->codec;
    c = ost->enc;
    //AVCodecContext *c;// = output_codec_context;
    //ret = avcodec_parameters_to_context(c, ost->st->codecpar); // should be okay
    //    auto t1 = std::chrono::high_resolution_clock::now();
    //    //cv::inpaint(occv, zeros, inp, 2, cv::INPAINT_NS);
    //    auto t2 = std::chrono::high_resolution_clock::now();
    //    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();



    init_packet(&pkt);
    //output_packet
    if(found)
    {

         cv::inpaint(occv, zeros, inp, 1, cv::INPAINT_TELEA);
//        //
//        int border = 3;
//        int sborder = 1;
//        cv::Rect region(gx, gy, gxe, gye);
//        cv::Rect Bregion(gx-border, gy-border, gxe+border*2, gye+border*2);
//        cv::Rect Sregion(gx+sborder, gy+sborder, gxe-sborder*2, gye-sborder*2);

//        //inp = occv.clone();
//        cv::Mat blurred_padded_region = inp;

//        //t1 = std::chrono::high_resolution_clock::now();

//        // cout << "GGGG " << gx << " - " << gy << " - " << gxe << " - " << gye <<endl;
//        Mat src, erosion_dst, dilation_dst;
//        int erosion_elem = 0;
//        int erosion_size = 39;
//        int dilation_elem = 0;
//        int dilation_size = 7;
//        int const max_elem = 2;
//        int const max_kernel_size = 21;

//        int erosion_type = 0;
//        if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
//        else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
//        else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }
//        Mat element = getStructuringElement( erosion_type,
//                             Size( erosion_size, erosion_size ),
//                             Point( -1, -1 ) );

//        cv::erode( occv(Sregion), occv(Sregion), element );

//        int dilation_type = 0;
//        if( dilation_elem == 0 ){ dilation_type = MORPH_RECT; }
//        else if( dilation_elem == 1 ){ dilation_type = MORPH_CROSS; }
//        else if( dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }
//        element = getStructuringElement( dilation_type,
//                             Size( dilation_size, dilation_size ),
//                             Point( -1, -1 ) );



//        //dilate( zeros(Bregion), zeros(Bregion), element );
//       // blurred_padded_region.copyTo(occv(region));


//        //cv::erode( occv(region), blurred_padded_region, element );

//        //cv::GaussianBlur(occv(Bregion), occv(Bregion), Size(93, 93), 11,11, BORDER_REPLICATE);
//        //cv::medianBlur(occv(region), occv(region), 39);
//cv::GaussianBlur(occv(Sregion), occv(Sregion), Size(13, 13), 11,11, BORDER_DEFAULT);

////dilate(occv(Bregion), occv(Bregion), element );
//        //cv::boxFilter(occv(region), occv(region), -1, Size(33,33), Point(-1,-1), 1, BORDER_DEFAULT);
//        //cv::Mat blurred_region = blurred_padded_region;
//        // and you can then copy that back into the original image if you want:
//        //Nzeros.copyTo(occv(Bregion));
//        //dilate( occv(region), occv(region), element );
//       // inp = occv;


//cv::inpaint(occv, zeros, inp, 1, cv::INPAINT_TELEA);
////       inp = occv;
////cv::medianBlur(inp(Sregion), inp(Sregion), 77);
//cv::GaussianBlur(inp(region), inp(region), Size(19, 19), 100,100, BORDER_DEFAULT);
        //t2 = std::chrono::high_resolution_clock::now();
        //duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
        //std::cout << "11 = " << duration << endl;
    }
    else
    {
        inp = occv;
    }




    if (!ost->sws_ctx) {
        printf("CRASH HERE %d\n", input.pVCodecContext->width);
        ost->sws_ctx = sws_getCachedContext(NULL, input.pVCodecContext->width, input.pVCodecContext->height,
                                            AV_PIX_FMT_BGR24,
                                            c->width, c->height, AV_PIX_FMT_YUV420P, SCALE_FLAGS, NULL, NULL, NULL);

        if (!ost->sws_ctx) {
            fprintf(stderr,
                    "Could not initialize the conversion context\n");
            exit(1);
        }
    }



    //t1 = std::chrono::high_resolution_clock::now();
//    av_frame_make_writable(ost->frame);
//    const int stride[] = { static_cast<int>(inp.step[0]) };


//    sws_scale(ost->sws_ctx,
//              &inp.data, stride,
//              0, input.pVCodecContext->height, ost->frame->data, ost->frame->linesize);

    if(!ost->frame)
    {
        ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    }
    frame = alloc_picture(c->pix_fmt, c->width, c->height);
    //create a AVPicture frame from the opencv Mat input image
//    avpicture_fill((AVPicture *)ost->frame,
//                   (uint8_t *)inp.data,
//                   AV_PIX_FMT_BGR24,
//                   inp.cols,
//                   inp.rows);


    //std::vector<uint8_t> framebuf(avpicture_get_size(AV_PIX_FMT_BGR24, inp.cols, inp.rows));
    av_image_fill_arrays(ost->frame->data, ost->frame->linesize, (uint8_t *)inp.data, AV_PIX_FMT_BGR24, input.pVCodecContext->width, input.pVCodecContext->height, 32);


    //convert the frame to the color space and pixel format specified in the sws context
    sws_scale(ost->sws_ctx,
              ost->frame->data,
              ost->frame->linesize, 0,
              c->height,
              frame->data,
              frame->linesize);



    //t2 = std::chrono::high_resolution_clock::now();
    //duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    //std::cout << "22 = " << duration << endl;

    //t1 = std::chrono::high_resolution_clock::now();
    ost->frame->pts = ost->next_pts++;
    frame->pts = ost->frame->pts;
    //frame = ost->frame;
    /* encode the image */

    if(!frame)
    {
        av_frame_unref(frame);
        av_packet_unref(&pkt);
        occv.release();
        ocvv.release();
        inp.release();
        return -1;
    }

    //ninp.release();

        ret = avcodec_send_frame(c, frame);
        if (ret < 0) {
            fprintf(stderr, "Error sending a frame for encoding\n");
            exit(1);
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(c, &pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return -1;
            else if (ret < 0) {
                fprintf(stderr, "Error during encoding\n");
                exit(1);
            }

            //printf("Write Video packet %3"PRId64" (size=%5d)\n", pkt.pts, pkt.size);
            //fwrite(pkt->data, 1, pkt->size, outfile);
            ret = write_frame(oc, &c->time_base, ost->st, &pkt, input.videoStream);
            if(ret < 0)
            {
                printf("Video packet write error %d\n", ret);
            }
            av_packet_unref(&pkt);
        }
//    ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);

//    if (ret < 0) {
//        cout << "Error encoding video" << endl;
//        //        fprintf(stderr, "Error encoding video frame: %s\n", /*av_err2str*/(ret));
//        exit(1);
//    }

//    if (got_packet) {
//        ret = write_frame(oc, &c->time_base, ost->st, &pkt, input.videoStream);
//    } else {
//        //cout << "NO PACKET" << endl;
//        ret = 0;
//    }

//    if (ret < 0) {
//        cout <<  "Error writing video frame" << endl;
//        //        fprintf(stderr, "Error while writing video frame: %s\n", /*av_err2str*/(ret));
//        //exit(1);
//    }

    av_frame_unref(frame);
    av_packet_unref(&pkt);
    occv.release();
    ocvv.release();
    inp.release();
    //t2 = std::chrono::high_resolution_clock::now();
    //duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    //std::cout << "part 2 encoding = " << duration << endl;

    return 0;
    //return (frame || got_packet) ? 0 : 1;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

bool compare_pts(const AVPacket &a, const AVPacket &b)
{
    return a.pts < b.pts;
}

int64_t VideoStream::get_master()
{
    int64_t ms = (std::clock() - master) / (double) (CLOCKS_PER_SEC / 1000);
    //cout << "MASTER = " << ms << endl;
    return ms;
}

int64_t i1 = 0;
void *target_obj;

int VideoStream::init_ffmpeg()
{
    avformat_network_init();
    av_log_set_level(AV_LOG_INFO);

start:

    input.ifmt_ctx = avformat_alloc_context();
    //av_dict_set(&opt, "flvflags", "no_duration_filesize", 0);
    av_dict_set(&opt, "reconnect", "1", 0);
    av_dict_set(&opt, "reconnect_at_eof", "1", 0);
    av_dict_set(&opt, "reconnect_streamed", "1", 0);
    av_dict_set(&opt, "reconnect_delay_max", "1800", 0);
    char buf[80];
    if ((ret = avformat_open_input(&input.ifmt_ctx, inputfile.c_str(), 0, &opt)) < 0) {
        av_strerror(ret, buf, 80);
        cout << "File not accessible, retrying... " << buf <<endl;
        sleep(4);
        if(reset)
        {
            exit(0);
        }
        goto start;
    }

    if ((ret = avformat_find_stream_info(input.ifmt_ctx, 0)) < 0) {
        printf("Failed to retrieve input stream information");
        exit(1);
    }


    for(int i = 0; i < input.ifmt_ctx->nb_streams; i++){

        if(input.ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            input.videoStream = i;
            //input.pVCodecContext = input.ifmt_ctx->streams[i]->codec;
            const AVCodec *dec = avcodec_find_decoder(input.ifmt_ctx->streams[i]->codecpar->codec_id);
            input.pVCodecContext = avcodec_alloc_context3(dec);
            avcodec_parameters_to_context(input.pVCodecContext, input.ifmt_ctx->streams[i]->codecpar);

        }

        if(input.ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            input.audioStream = i;
            //input.pACodecContext = input.ifmt_ctx->streams[i]->codec;
            const AVCodec *dec = avcodec_find_decoder(input.ifmt_ctx->streams[i]->codecpar->codec_id);
            input.pACodecContext = avcodec_alloc_context3(dec);
            avcodec_parameters_to_context(input.pACodecContext, input.ifmt_ctx->streams[i]->codecpar);
        }
    }

    //    pthread_t ocvthr;
    //     pthread_create(&ocvthr, NULL, ocv_thread, NULL);
    //

    av_dump_format(input.ifmt_ctx, 0, inputfile.c_str(), 0);

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, format_.c_str(), outputfile.c_str()); //outputfile.c_str()


    if(oc && strcmp(format_.c_str(),  "mpegts") == 0){

        //        AVOutputFormat* ofmt = oc->oformat;
        //        const AVOption *o = av_opt_find2(ofmt, "hls_list_size", NULL, 0, AV_OPT_SEARCH_CHILDREN, &target_obj);
        //
        //        av_opt_get_int(ofmt, "hls_list_size", AV_OPT_SEARCH_CHILDREN, &i1);
        //        av_opt_set_int(ofmt, "hls_list_size", 10, AV_OPT_SEARCH_CHILDREN);
    }

    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, format_.c_str(), outputfile.c_str()); //
    }

    if (!oc)
        return 1;

    fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        //enum AVCodecID codec_id = avcodec_find_encoder(fmt->video_codec);
        //add_stream(&video_st, oc, &video_codec, codec_id);
        add_stream(&video_st, oc, &video_codec, input.pVCodecContext->codec_id);
        have_video = 1;
    }

    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        //enum AVCodecID codec_id = avcodec_find_encoder_by_name("aac")->id;
        //add_stream(&audio_st, oc, &audio_codec, codec_id);//input.pACodecContext->codec_id /*AV_CODEC_ID_MP3*/ );// AV_CODEC_ID_AAC
        add_stream(&audio_st, oc, &audio_codec, input.pACodecContext->codec_id /*AV_CODEC_ID_MP3*/ );
        have_audio = 1;
    }

    av_dict_set_int(&opt, "threads", (int64_t)0, 0);
    av_dict_set_int(&opt, "framerate", (int64_t)25, 0);
    av_dict_set_int(&opt, "r", (int64_t)25, 0);

    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if (have_video)
        open_video(oc, video_codec, &video_st, opt);

    if (have_audio)
        open_audio(oc, audio_codec, &audio_st, opt);

    init_fifo(&fifo, audio_st.enc);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, outputfile.c_str(), AVIO_FLAG_WRITE);
        char errBuffer[80];

        if (ret < 0) {
            av_strerror(ret, errBuffer, 80);
            cout << "Could not open file" << errBuffer <<endl;
            return 1;
        }
    }

    /* Write the stream header, if any. */

    AVDictionary * fopts = NULL;
    av_dict_set(&fopts, "flvflags", "no_duration_filesize", 0);
    av_dict_set(&fopts, "flvflags", "aac_seq_header_detect", 0);
    ret = avformat_write_header(oc, &fopts);
    if(ret < 0)
    {
        printf("Write Header %d\n", ret);
    }
    av_dump_format(oc, 0, outputfile.c_str(), 1);

    if (ret < 0) {
        return 1;
    }

}

int VideoStream::run_delogo()
{
    int counter = 0;
    int ret = 0;
    bool br = false;

    master = std::clock();
    while (1) {
        std::clock_t start = std::clock();
        int iv=0;
        int ia=0;
        //  int stt[pfrm];
        vector<int> stt (pfrm);
        //        AVPacket tpkt;
        //        tpkt.data = NULL;
        vector<AVPacket> vpkts;// (pfrm);
        vector<AVPacket> apkts;// (pfrm);
        //std::clock_t start = std::clock();
        for(int i=0; i<pfrm; i++)
        {
            ret = av_read_frame(input.ifmt_ctx, &input.current);
            if(ret < 0)
            {
                br = true;
                break;
            }


            if(input.current.stream_index == input.videoStream){
                if(!input.current.data)
                {
                    continue;
                }
                vpkts.push_back(input.current);

                iv++;
                stt[i] = input.videoStream;
            }
            else if(input.current.stream_index == input.audioStream){

                if(!input.current.data)
                {
                    continue;
                }
                apkts.push_back(input.current);
                ia++;

                stt[i] = input.audioStream;
            }
        }

        if(br)
        {
            break;
        }

        if(apkts.size() == 0)
        {
            goto restart;
        }

        vpkts.shrink_to_fit();
        apkts.shrink_to_fit();
        stt.shrink_to_fit();

        process_video_frame(vpkts);

        umats.shrink_to_fit();

        iv = 0;
        ia = 0;

        bool cont = false;
        for(int i=0; i<stt.size(); i++)
        {
            if(stt[i] == input.videoStream){

                if(iv >= umats.size())
                {
                    iv++;
                    cont = true;
                    continue;
                }
                if(umats[iv].empty())
                {
                    iv++;
                    cont = true;
                    continue;
                }

                //t1 = std::chrono::high_resolution_clock::now();
                write_video_frame(oc, &video_st, umats[iv]);
                //t2 = std::chrono::high_resolution_clock::now();
                //duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
                //std::cout << "write_video_frame = " << duration << endl;

                cont = false;
                iv++;
            }
            else if(stt[i] == input.audioStream){
                if(cont == true)
                {
                    continue;
                }
                if(ia >= apkts.size())
                {
                    ia++;
                    continue;
                }
                // write_audio_frame(oc, &audio_st, apkts.at(ia));
                conv_audio(oc, &audio_st, apkts.at(ia));
                ia++;

            }

            counter++;
        }

        //RECHECK
        for(int i=0; i<vpkts.size(); i++)
        {
            av_packet_unref(&vpkts.at(i));
        }
        for(int i=0; i<apkts.size(); i++)
        {
            av_packet_unref(&apkts.at(i));
        }

        vpkts.resize(0);
        apkts.resize(0);
        vpkts.clear();
        apkts.clear();
        stt.clear();
        for(int i=0; i<umats.size(); i++)
        {
            umats.at(i).release();
        }

        for (auto it = umats.cbegin(); it != umats.cend();)
        {
            umats.erase(it);

        }

        umats.clear();
        //umats.resize(0);
        vcn = 0;
        //        int ms = (std::clock() - start) / (double) (CLOCKS_PER_SEC / 1000);
        //        std::cout << "Finished in " << ms << "ms" << std::endl;
        //vpkts.clear();
    }
    cout << "FRAME COUNT = " << afrms << endl;
restart:
    av_write_trailer(oc);

    /* Close each codec. */
    if (have_video)
        close_stream(oc, &video_st);
    if (have_audio)
        close_stream(oc, &audio_st);

    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&oc->pb);

    /* free the stream */
    avformat_free_context(oc);
    reset = true;
}

