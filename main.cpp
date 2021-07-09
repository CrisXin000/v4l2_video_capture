#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>
#include <linux/videodev2.h>

#include "v4l2Capture.h"

using namespace std;
using namespace cv;

static void printUsage()
{
    cout <<
         "视频捕捉\n\n"
         "video_capture [video_name1 video_name2 ...]\n"
         "\t--format [format1 format2 ...]   MJPG|YUYV422|RG10\n";
}

vector<string> video_names;
vector<int> video_formats;
vector<Mat> images;
static int parseCmdArgs(int argc, char* argv[])
{
    if (argc == 1)
    {
        printUsage();
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--help" || string(argv[i]) == "/?"){
            printUsage();
            return -1;
        } else if (string(argv[i]) == "--format")
        {
            for (int j = i+1; j < argc; ++j) {
                int format_code = -1;
                if (string(argv[j]) == "MJPG")
                    format_code = V4L2_PIX_FMT_MJPEG;
                else if (string(argv[j]) == "YUYV422")
                    format_code = V4L2_PIX_FMT_YUV422M;
                else if (string(argv[j]) == "RG10")
                    format_code = V4L2_PIX_FMT_SRGGB10;
                else
                {
                    cout << "Can not recognize the format." << endl;
                    printUsage();
                    return -1;
                }
                video_formats.push_back(format_code);
            }
            break;
        }
        else
        {
            video_names.push_back(argv[i]);
        }
    }
    if (video_names.empty())
    {
        printUsage();
        return -1;
    }
    if (video_names.size() != video_formats.size())
    {
        cout << "pixel format cannot match video numbers!" << endl;
        printUsage();
        return -1;
    }

    return 0;
}

void VideoPlayer(string video_name) {
    unsigned char *yuv422frame = NULL;
    unsigned long yuvframeSize = 0;

    string videoDev = video_name;
    V4L2Capture *vcap = new V4L2Capture(const_cast<char*>(videoDev.c_str()), 800, 600, video_formats[0]);
    vcap->openDevice();
    vcap->initDevice();
    vcap->startCapture();

    cvNamedWindow("Capture",CV_WINDOW_AUTOSIZE);
    IplImage* img;
    CvMat cvmat;
    double t;
    while(1){
        t = (double)cvGetTickCount();
        vcap->getFrame((void **) &yuv422frame, (size_t *)&yuvframeSize);
        cvmat = cvMat(IMAGEHEIGHT,IMAGEWIDTH,CV_8UC3,(void*)yuv422frame);		//CV_8UC3

        //解码
        img = cvDecodeImage(&cvmat,1);
        if(!img){
            printf("DecodeImage error!\n");
        }

        cvShowImage("Capture",img);
        cvReleaseImage(&img);

        vcap->backFrame();
        if((cvWaitKey(1)&255) == 27){
            exit(0);
        }
        t = (double)cvGetTickCount() - t;
        printf("Used time is %g ms\n",( t / (cvGetTickFrequency()*1000)));
    }
    vcap->stopCapture();
    vcap->freeBuffers();
    vcap->closeDevice();

}

void showMultiImages(const vector<Mat> &srcImage, Size imgSize)
{
    int numImages = srcImage.size();
    Size windowSize;
    if (numImages > 6)
    {
        cout << "Not more than 6 videos" << endl;
        return;
    }
    // According number of videos ensure number of sub windows
    if (numImages<=4)
        windowSize = Size(numImages, 1);
    else if (numImages <= 6)
        windowSize = Size(3, 2);

    // set sub images attributes
    Size showImageSize = imgSize;
    int splitLineSize = 15; // gap
    int aroundLineSize = 50; // board
    // built the output window
    const int windowHeight = showImageSize.width * windowSize.width +
                             aroundLineSize + (windowSize.width-1) * splitLineSize;
    const int windowWidth = showImageSize.height * windowSize.height +
                            aroundLineSize + (windowSize.height-1) * splitLineSize;
    Mat showWindowImages(windowWidth, windowHeight, CV_8UC3, Scalar(0, 0,0));

    // corner coordinates X and Y
    int posX = aroundLineSize/2;
    int posY = aroundLineSize/2;
    int tempX = posX;
    int tempY = posY;
    for (int i = 0; i < numImages; ++i) {
        if (i % windowSize.width == 0 && tempX != posX)
        {
            tempX = posX;
            tempY += splitLineSize + showImageSize.height;
        }
        Mat tempImage = showWindowImages(Rect(tempX, tempY, showImageSize.width, showImageSize.height));
        resize(srcImage[i], tempImage, showImageSize);
        tempX += splitLineSize + showImageSize.width;
    }
    imshow("Videos", showWindowImages);
    waitKey(30);
}

void MultiVideoPlayer(vector<string> &videoNames)
{
    int videoNum = videoNames.size();

    unsigned char *yuv422frame = NULL;
    unsigned long yuvframeSize = 0;
    vector<string> windowName(videoNum);
    vector<V4L2Capture*> cap(videoNum);
    for (int i = 0; i < videoNum; ++i) {
        cap[i] = new V4L2Capture(const_cast<char*>(videoNames[i].c_str()), 800, 600, video_formats[i]);
        cap[i]->openDevice();
        cap[i]->initDevice();
        cap[i]->startCapture();
        windowName[i] = "Capture" + to_string(i);
        cvNamedWindow(windowName[i].c_str(),CV_WINDOW_AUTOSIZE);
    }

    IplImage* img;
    CvMat cvmat;
    double t;
    while(1){
        t = (double)cvGetTickCount();
        for (int i = 0; i < videoNum; ++i) {
            cap[i]->getFrame((void **) &yuv422frame, (size_t *)&yuvframeSize);
            cvmat = cvMat(IMAGEHEIGHT,IMAGEWIDTH,CV_8UC3,(void*)yuv422frame);		//CV_8UC3
            //解码
            img = cvDecodeImage(&cvmat, 1);
            if(!img){
                printf("DecodeImage error!\n");
            }
            images.push_back(cvarrToMat(img));
            cvShowImage(windowName[i].c_str(),img);
            cvReleaseImage(&img);
            cap[i]->backFrame();
            if((cvWaitKey(1)&255) == 27){
                exit(0);
            }
        }
//        showMultiImages(images, Size(800, 600));
//        images.clear();
        t = ((double)cvGetTickCount() - t)/(cvGetTickFrequency()*1000);
        cout << "Used time is " << t << "ms\n";
    }
    for (int i = 0; i < videoNum; ++i) {
        cap[i]->stopCapture();
        cap[i]->freeBuffers();
        cap[i]->closeDevice();
    }
}



int main(int argc, char* argv[]) {
    parseCmdArgs(argc, argv);
    if (video_names.size() <= 0)
        return -1;
    else if (video_names.size() == 1)
        VideoPlayer(video_names[0]);
    else
        MultiVideoPlayer(video_names);
    return 0;
}