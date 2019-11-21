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

#define HEIGHT 800
#define WIDTH 1280
#define SIZE HEIGHT*WIDTH*2

int main()
{
    // cvNamedWindow("Capture",CV_WINDOW_AUTOSIZE);
    std::ifstream ifs("test.raw", std::ios::in | std::ios::binary);
    char yuv422frame[SIZE];
    // char mono8frame[SIZE];
    ifs.read(yuv422frame, SIZE);
    uint16_t a = *(uint16_t*)(yuv422frame);
    std::cout<< a << std::endl;
    while (true)
    {
        // unsigned char yuv422frame[SIZE*2] = {0};
        cv::Mat cvmat(HEIGHT, WIDTH, CV_8UC1);

        cv::Mat(HEIGHT, WIDTH, CV_16UC1, (void *)yuv422frame).convertTo(cvmat, CV_8UC1, 1.0/256);

        cv::imshow("Capture", cvmat);

        if ((cv::waitKey(1) & 255) == 27)
        {
            exit(0);
        }
        // cvReleaseImage(&img);
    }

    return 0;
}