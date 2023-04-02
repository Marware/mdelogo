#include <iostream>
#include "mdelogo.h"
#include "ffmpeg.h"
#include "opencv.h"
#include "utils.h"

using namespace std;

char name_buffer[80];

static void parse_argument(VideoStream *vs, char* arg){

    string args = arg;


    vector<string> vrgs = splits(args, " ");
    for(int i=0; i < vrgs.size(); i++)
    {
        if(vrgs.at(i).find("input=") != std::string::npos)
        {
            string inp = vrgs.at(i);
            inp.erase(0, string("input=").size());
            vs->inputfile = inp;
            cout << "Input = " << vs->inputfile << endl;
        }
        if(vrgs.at(i).find("output=") != std::string::npos)
        {
            string inp = vrgs.at(i);
            inp.erase(0, string("output=").size());
            vs->outputfile = inp;
            cout << "Output = " << vs->outputfile << endl;
        }
        if(vrgs.at(i).find("bitrate=") != std::string::npos)
        {
            std::string::size_type sz;   // alias of size_t


            string inp = vrgs.at(i);
            inp.erase(0, string("bitrate=").size());
            vs->bitrate = (std::stol (inp,&sz))*1024;
            cout << "Bitrate = " << vs->bitrate << endl;
        }
        if(vrgs.at(i).find("format=") != std::string::npos)
        {
            string inp = vrgs.at(i);
            inp.erase(0, string("format=").size());
            vs->format_ = inp;
            cout << "Format = " << vs->format_ << endl;
        }
        if(vrgs.at(i).find("width=") != std::string::npos)
        {
            std::string::size_type sz;   // alias of size_t
            string inp = vrgs.at(i);
            inp.erase(0, string("width=").size());
            vs->width = std::stol (inp,&sz);
            cout << "Width = " << vs->width << endl;
        }
        if(vrgs.at(i).find("height=") != std::string::npos)
        {
            std::string::size_type sz;   // alias of size_t
            string inp = vrgs.at(i);
            inp.erase(0, string("height=").size());
            vs->height = std::stol (inp,&sz);
            cout << "Height = " << vs->height << endl;
        }
    }
}

void init_arguments(VideoStream *vs, int argc, char **argv){

    vs->width = 1920;
    vs->height = 1080;
    vs->bitrate = 2000000;
    vs->inputfile = "";
    vs->outputfile = "";
    vs->format_ = "flv";

    for(int i = 1; i < argc; i++){
        parse_argument(vs, argv[i]);
    }
}

int main(int argc, char **argv)
{
    VideoStream *vs = new VideoStream();
    init_arguments(vs, argc, argv);

    if(vs->inputfile.empty()){
        cout << "You must provide a input url" << endl;
        exit(1);
    }

    if(vs->outputfile.empty()){
        cout << "You must provide a output url" << endl;
        exit(1);
    }

    vs->init_opencv("temp.png");
    vs->init_ffmpeg();
    vs->run_delogo();

    return 0;
}
