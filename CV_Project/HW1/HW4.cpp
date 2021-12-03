#include "stdafx.h"

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <time.h>

using namespace cv;
using namespace std;


int main()
{
	Mat img_in;

	img_in = imread("C:/Users/CEY/Desktop/Lena.png");
	cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);

	imshow("Original Image", img_in);
	
	waitKey(0);

	return 0;

}



