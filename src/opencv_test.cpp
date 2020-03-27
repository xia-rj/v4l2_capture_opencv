#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::VideoCapture cap;
    cap.open("/dev/video0");
    if (!cap.isOpened()){
        std::cout<<"OPEN FAILED"<<std::endl;
    }
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE,0);
    double exposure = cap.get(cv::CAP_PROP_AUTO_EXPOSURE);
    std::cout<<exposure<<std::endl;

    cv::Mat frame;
    while (true){
        cap>>frame;
        cv::imshow("Capture", frame);

        if ((cv::waitKey(1) & 255) == 27)
        {
            exit(0);
        }
    }


    return 0;
}