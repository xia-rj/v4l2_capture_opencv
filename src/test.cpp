/*
 * @Author: your name
 * @Date: 2019-11-19 10:51:31
 * @LastEditTime: 2019-11-20 19:23:48
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /camera_opencv/src/test.cpp
 */
#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
// #include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define HEIGHT 800
#define WIDTH 1280
#define SIZE HEIGHT*WIDTH*2
#define TARGET_EXPOSURE_LEVEL 0.3

int main()
{
    // cvNamedWindow("Capture",CV_WINDOW_AUTOSIZE);
    std::ifstream ifs("test.raw", std::ios::in | std::ios::binary);
    char yuv422frame[SIZE];
    // char mono8frame[SIZE];
    ifs.read(yuv422frame, SIZE);
    // uint16_t a = *(uint16_t*)(yuv422frame);
    // std::cout<< a << std::endl;

    cv::Mat cvmat(HEIGHT, WIDTH, CV_8UC1);
    cv::Mat(HEIGHT, WIDTH, CV_16UC1, (void *)yuv422frame).convertTo(cvmat, CV_8UC1, 1.0/256);

    cv::Mat hist;
    int hist_size[1] = {64};
    float hist_range[] = {0, 256};
    const float* ranges[]={hist_range};
    cv::calcHist(&cvmat, 1, 0, cv::Mat(),
                 hist, 1, hist_size, ranges, 
                 true, false);

    // std::cout<< hist.type() <<std::endl;
    int index = 0;
    int num = 0;
    for (index; index < hist_size[0]; index++){
        // std::cout<< hist.at<uint8_t>(index) <<std::endl;
        num +=hist.at<float>(index);
        if (num > SIZE/4.0){
            break;
        }
    }
    float center_exposure_level  = (float)++index/(float)hist_size[0];

    std::cout<< index << "  " << center_exposure_level << std::endl;
    std::cout<<"hist = "<<std::endl<<hist<<std::endl;

    // while (true)
    // {
    //     // unsigned char yuv422frame[SIZE*2] = {0};


    //     cv::imshow("Capture", cvmat);

    //     if ((cv::waitKey(1) & 255) == 27)
    //     {
    //         exit(0);
    //     }
    //     // cvReleaseImage(&img);
    // }

    return 0;
}