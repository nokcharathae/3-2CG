// PSNR.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <math.h>
#include <iostream>

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;
#define pi 3.141592653589793238462643383279

int main()
{
	Mat img_in;

	float x=0;

	// image 읽고 gray로 바꾸기
	img_in = imread("C:/Users/CEY/Desktop/Lena.png");
	cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);
	imshow("source img", img_in);

	unsigned char* pData;
	pData = (unsigned char*)img_in.data;

	Mat img_out = Mat::zeros(img_in.rows, img_in.cols, CV_8UC1);

	unsigned char* outData;
	outData = (unsigned char*)img_out.data;

	cout << "회전할 각도를 입력하시오: ";

	cin >> x;

	///////////////////// 처리하기 ///////////////////
	
		int orig_x, orig_y;
		int pixel;
		double radian = x * pi / 180.0; // (1)
		double co = cos(radian), si = sin(-radian);
		double xcenter = (double)img_in.cols / 2.0, ycenter = (double)img_in.rows / 2.0;

		for (int py = 0; py < img_in.rows; py++)
		{
			for (int px = 0; px < img_in.cols; px++)
			{
				orig_x = (int)(xcenter + ((double)py - ycenter) * si + ((double)px - xcenter) * co);
				orig_y = (int)(ycenter + ((double)py - ycenter) * co - ((double)px - xcenter) * si);
				pixel = 0; // (3)

				if ((orig_y >= 0 && orig_y < img_in.rows) && (orig_x >= 0 && orig_x < img_in.cols)) 

					pixel = pData[orig_x + orig_y * img_in.cols]; // (5)

				outData[px  + py * img_in.cols] = pixel; // (6)

			} // x-loop


		} // y-loop

		
	imshow("output image", img_out);

	waitKey(0);

	return 0;
}

