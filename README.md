# Mdelogo

Remove logos or static overlays from a live stream

### How it works
Utilizing CUDA hardware decoder `cuvid` each frame is decoded and copied to a `cuda::GpuMat` and saved in frames queue, the frames queue is processed when it reaches the set value of `pfrm` (200 by default), once the queue is filled it's passed to `process_video_frame` and using `cv::addWeighted` all the queued frames are added with alpha value 0.1, this way the created a single frame the shows clearly the desired template that should be matched in `process_vframe`, because matching the template frame by frame is much slower and less accurate, once a template detected `cv::inpaint` applied to add smooth blurring over the object and essentially hiding it in the output stream

### Install dependencies

    Ubuntu
    sudo apt install libopencv-dev libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev
OR

    git clone https://github.com/Marware/opencv-ffmpeg-cuda-build
    Follow the insructions in the repo

Using `opencv-ffmpeg-cuda-build` will get and build the latest code from OpenCV and FFmpeg codebases

### Build
    
    cd mdelogo
    qmake
    make

### Run

    ./mdelogo input=[URL/FILE] output=[URL/FILE]

### Notes
* It the feature to detect text using OCR `Tesseract` is available but commented due to limited testing
* The ability to use CPU-only processing in decoding, encoding and detection is available, needs more optimization
* Contributions are welcome!