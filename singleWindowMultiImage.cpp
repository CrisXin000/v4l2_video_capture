//
// Created by gxlinux on 2021/6/28.
//
#include <opencv2/opencv.hpp>
#include <iostream>
#include "v4l2Capture.h"

using namespace cv;
using namespace std;

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

int main()
{
    unsigned char *yuv422frame = NULL;
    unsigned long yuvframeSize = 0;

    string video_name = "/dev/video2";
    V4L2Capture *vcap = new V4L2Capture(const_cast<char*>(video_name.c_str()), 800, 600);
    vcap->openDevice();
    vcap->initDevice();
    vcap->startCapture();

    vector<Mat> images;
    int num = 2;
    IplImage *img;
    CvMat cvmat;

    while (1)
    {
        vcap->getFrame((void **) &yuv422frame, (size_t *)&yuvframeSize);
        cvmat = cvMat(IMAGEHEIGHT,IMAGEWIDTH,CV_8UC3,(void*)yuv422frame);
        //解码
        img = cvDecodeImage(&cvmat,1);
        if(!img){
            printf("DecodeImage error!\n");
        }
        for (int i = 0; i < num; ++i) {
            images.push_back(cvarrToMat(img));
        }
        showMultiImages(images, Size(800, 600));
        images.clear();
        cvReleaseImage(&img);

        vcap->backFrame();
        if((cvWaitKey(1)&255) == 27){
            exit(0);
        }
    }
}


