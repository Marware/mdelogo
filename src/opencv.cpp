//#include "opencv.h"

//OpenCV::OpenCV()
//{

//}

#include "mdelogo.h"

int VideoStream::init_opencv(std::string template_path)
{
    setNumThreads(16);
    setUseOptimized(1);
    ocl::setUseOpenCL(true);
    if (!ocl::haveOpenCL())
    {
        cout << "OpenCL is not available..." << endl;
        //return;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_ALL))
    {
        cout << "Failed creating the context..." << endl;
        //return;
    }

    cout << context.ndevices() << " GPU devices are detected." << endl; //This bit provides an overview of the OpenCL devices you have in your computer
    for (int i = 0; i < context.ndevices(); i++)
    {
        cv::ocl::Device device = context.device(i);
        cout << "name:              " << i << device.name() << endl;
        cout << "available:         " << device.available() << endl;
        cout << "imageSupport:      " << device.imageSupport() << endl;
        cout << "OpenCL_C_Version:  " << device.OpenCL_C_Version() << endl;
        cout << endl;
    }

    cv::ocl::Device(context.device(0));

    mytemplate = imread(template_path, -1);

    templ_Gpu.upload(mytemplate);
    b_TemplateMatching = cuda::createTemplateMatching(CV_8U, cv::TM_CCOEFF_NORMED);

    //cv::cvtColor(tmpt, tmpt, cv::COLOR_RGB2GRAY);
    int sw = mytemplate.size().width;
    int sh = mytemplate.size().height;

    cout << "SIZE 1111= " << sw << "  " << sh << endl;
    //cout << "TH: " << getNumThreads() << "\tOpt : "<< useOptimized() << "\nBuild: " << getBuildInformation() << endl;

    return 0;
}

Point VideoStream::process_track(cuda::GpuMat ocv)
{
    int sw = mytemplate.size().width;
    int sh = mytemplate.size().height;

    //cout << "SIZE = " << sw << "  " << sh << endl;
    cuda::GpuMat img_Gpu;//(sh, sw, CV_8U);
    cuda::GpuMat result_Gpu;//(CV_32F);

    //img_Gpu.upload(ocv);
    img_Gpu = ocv;

    b_TemplateMatching->match(img_Gpu, templ_Gpu, result_Gpu);

    double minVal = 0.0f; double maxVal = 0.0f; Point minLoc = Point(0,0); Point maxLoc = Point(0,0);
    Point matchLoc;
    cv::cuda::minMaxLoc(result_Gpu, &minVal, &maxVal, &minLoc, &maxLoc, cuda::GpuMat() );

    //CPU
    {
        //    cv::Mat result(sh, sw, CV_8U);
        //    matchTemplate( ocv, mytemplate, result, cv::TM_CCOEFF_NORMED );
        //    /// Localizing the best match with minMaxLoc
        //    double minVal = 0.0f; double maxVal = 0.0f; Point minLoc = Point(0,0); Point maxLoc = Point(0,0);
        //    Point matchLoc;
        //    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

    }

    matchLoc = maxLoc;

    std::cout << img_counter << "_" << matchLoc << "\tTTT_MAX = " << maxVal << "_Min = " << minVal << endl;

    if(maxVal >= 0.60)
    {
        return matchLoc;
    }

    matchLoc = Point(0,0);
    return matchLoc;

}

bool VideoStream::process_vframe(cuda::GpuMat ocv){
        falpos++;
        //cout << "CD = " << img_counter << endl;
        p = process_track(ocv);
        //42:50:506:46
        std::vector<int> rect = get_rect();

        //cout << "x = " << rect[0] << endl;
        //cout << "y = " << rect[1] << endl;
        //cout << "w = " << rect[2] << endl;
        //cout << "h = " << rect[3] << endl;

        p.x = rect[0];
        p.y = rect[1];
        sw = rect[2];
        sh = rect[3];
        if(p.x > 0 && p.x <= width)
        {
            rect_x = p.x;//*1.43
            rect_y = p.y;//*1.43
            cout << "MATCH: X: " << rect_x << "\tY: " << rect_y << "\tF: " << img_counter <<endl;

            gx = rect_x;
            gy = rect_y;
            gxe = sw-1;
            gye = sh-1;
            cv::Range cols = cv::Range(rect_x, rect_x + sw-1);
            cv::Range rows = cv::Range(rect_y, rect_y + sh-1);
            zeros(rows, cols) = 255;
            falpos = 0;
            p.x = 0;
            p.y = 0;
            found = true;
        }

        if(falpos > 6)
        {

            if(!zeros.empty())
            {
                //cout << "EMP" << endl;
                zeros = cv::Mat::zeros(empmat.rows, empmat.cols, empmat.type());
                //zeros = empmat.clone();
            }
            found = false;
        }

    return found;
    // inp = ocv.clone();

}

std::vector<int> VideoStream::get_rect()
{
    string recval;
    std::vector<int> rect(4);
    // Read from the text file
    ifstream rectFile("rect.txt");

    // Use a while loop together with the getline() function to read the file line by line
    int i = 0;
    while (getline (rectFile, recval)) {
      // Output the text from the file
      //cout << recval << endl;
      rect[i] = stoi(recval);
      i++;
    }

    // Close the file
    rectFile.close();

    return rect;
}
