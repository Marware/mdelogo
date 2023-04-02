#ifndef MDELOGO_H
#define MDELOGO_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

#include <unistd.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/text.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/core/ocl.hpp"
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <vector>
#include <chrono>
#define __STDC_CONSTANT_MACROS

extern "C"{
    #include <libavutil/imgutils.h>
    #include <libavutil/avassert.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
    #include <libavutil/avutil.h>
    #include <libavutil/audio_fifo.h>
    #include <libavcodec/avcodec.h>
}

#include <signal.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <tbb/tbb.h>
#include <tbb/parallel_invoke.h>

#define SCALE_FLAGS SWS_BILINEAR
#define STREAM_PIX_FMT  AV_PIX_FMT_YUV420P /* default pix_fmt */

using namespace std;
using namespace cv;
using namespace cv::text;

class mdelogo
{
public:
    mdelogo();
};

//VARIABLES
typedef struct InputStream{
        AVFormatContext* ifmt_ctx;

        int audioStream;
        int videoStream;

        AVFrame* frame;
        AVFrame* rgbFrame;

        AVCodecContext* pVCodecContext;
        AVCodecContext* pACodecContext;
        AVPacket current;
} InputStream;


typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int64_t samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

class VideoStream
{
    public:
        //FUNCS
        VideoStream();

        //VARS PROB DELETE
        pthread_t inpthr;
        std::clock_t master;
        int MAX_WIDTH;
        int MIN_WIDTH;
        int MAX_HEIGHT;
        int MIN_HEIGHT;
        int init_cont;
        int rect_x;
        int rect_y;
        int rect_width;
        int rect_hight;
        int ocframeid;
        string data;
        bool init_v = false;
        bool init_a = false;
        ifstream myfile;
        ofstream myfile1;
        string inputfile;
        string outputfile;
        string format_;
        long bitrate;
        int width;
        int height;
        string code;
        long long iPts;
        AVPacket thepkt;
        int64_t odts;

        InputStream input = {0};
        OutputStream video_st = {0};
        OutputStream audio_st = {0};

        bool process_video_frame(vector<AVPacket> vpkts);

        /* Global timestamp for the audio frames. */
        int64_t pts;
        int init_packet(AVPacket *packet);
        int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context);
        int add_samples_to_fifo(AVAudioFifo *fifo,
                                       uint8_t **converted_input_samples,
                                       const int frame_size);
        int convert_samples(const uint8_t **input_data,
                                   uint8_t **converted_data, const int frame_size,
                                   SwrContext *resample_context);
        int init_converted_samples(uint8_t ***converted_input_samples,
                                          AVCodecContext *output_codec_context,
                                          int frame_size);
        int conv_audio(AVFormatContext *oc, OutputStream *ost, AVPacket apkt);
        int read_decode_convert_and_store(AVAudioFifo *fifo,
                                                 AVFormatContext *input_format_context,
                                                 AVCodecContext *input_codec_context,
                                                 AVCodecContext *output_codec_context,
                                                 SwrContext *resampler_context, AVPacket apkt,
                                                 int *finished);
        int encode_audio_frame(AVFrame *frame,
                                      AVFormatContext *output_format_context,
                                      AVCodecContext *output_codec_context,
                                      int *data_present, OutputStream *ost);
        int load_encode_and_write(AVAudioFifo *fifo,
                                         AVFormatContext *output_format_context,
                                         AVCodecContext *output_codec_context, OutputStream *ost);
        int init_output_frame(AVFrame **frame,
                                     AVCodecContext *output_codec_context,
                                     int frame_size);
        //FUNCS PROB DELETE
        int64_t get_master();

        //FFMPEG
        int init_ffmpeg();
        int run_delogo();
        void add_stream(OutputStream *ost, AVFormatContext *oc,
                               const AVCodec **codec,
                               enum AVCodecID codec_id);
        void open_audio(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);
        int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt, int stream_type);
        void open_video(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg);
        int write_video_frame(AVFormatContext *oc, OutputStream *ost, cuda::GpuMat ocvv);


        const AVOutputFormat *fmt;
        AVFormatContext *oc;
        const AVCodec *audio_codec, *video_codec;
        int ret;
        int have_video = 0;
        int have_audio = 0;
        AVDictionary *opt = NULL;
        bool initial_z = false;
        bool found = false;

        //OpenCV
        int init_opencv(std::string template_path);
        Point track(cv::Mat &img, const cv::Mat &templ);
        Point track2(cv::Mat &img, const cv::Mat &templ);
        bool process_vframe(cuda::GpuMat ocv);
        std::vector<int> get_rect();
        Point process_track(cuda::GpuMat ocv);

        cv::Ptr<cv::cuda::TemplateMatching> b_TemplateMatching;
        cuda::GpuMat templ_Gpu;
        Mat mytemplate;
        int falpos = 0;
        bool initial = false;
        Point p;
        int sw = 0;
        int sh = 0;
        int num_ocrs = 4;
        int img_counter = 0;
        int gx;
        int gy;
        int gxe;
        int gye;

        cv::Mat ocv_frame;
        cv::Mat flipped;
        cv::Mat cpy;
        cv::Mat gray;
        cv::Mat msk;
        cv::Mat zeros;
        cv::Mat empmat;
        cv::Mat zs;
        cv::Mat inp;
        cv::Mat suma;

        bool reset = false;
        uint8_t * fdata;

        int max_dst_nb_samples;
        int64_t m_dtsOffset;
        int64_t dts;
        int afcnt = 0;
        int bfsz = 9216;

        int pfrm = 200;
        vector<cuda::GpuMat> umats;
        int vcn = 0;
        int fcnt = 0;
        int afrms = 0;
        bool fdone = false;
        double last_maxVal = 0.0f;
        cv::Ptr<cv::cuda::Filter> lapFilter0;
        cv::Ptr<cv::cuda::Filter> cu_dilate;
        int fdc = 0;
};





#endif // MDELOGO_H
