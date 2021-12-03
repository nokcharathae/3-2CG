// PSNR.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <math.h>
#include <limits>

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;

int main()
{
	Mat img_in;
	int x;

	int nw = 512;
	int nh = 512;

	// image 읽고 gray로 바꾸기
	img_in = imread("C:/Users/CEY/Desktop/Lena_256x256.png");
	cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);
	imshow("source img", img_in);

	unsigned char* pData;
	pData = (unsigned char*)img_in.data;

	Mat img_out = Mat::zeros(nh, nw, CV_8UC1);

	unsigned char* outData;
	outData = (unsigned char*)img_out.data;


	///////////////////// 처리하기 ///////////////////
	//for (int i = 0; i < img_in.cols * img_in.rows; i++)	// 예제...
	//{
		//pData[i] = 0;
		
	//}

	int index;
	int off = 0;
	float x_ratio = ((float)(img_in.cols-1) / nw);
	float y_ratio = ((float)(img_in.rows-1) / nh);
	float x_diff, y_diff;

	for (int h = 0; h < nh; h++) {
		for (int w = 0; w < nw; w++) {
			int px = (int)(w * x_ratio);
			int py = (int)(h * y_ratio);
			x_diff = (w * x_ratio) - px;
			y_diff = (h * y_ratio) - py;
			index = h * img_in.cols + w;

			outData[off++] = (int)(pData[index] * (1 - x_diff) * (1 - y_diff) 
				+ pData[index + 1] * (x_diff) * (1 - y_diff) 
				+ pData[index + img_in.cols] * (1 - x_diff) * (y_diff)
				+ pData[index + img_in.cols + 1] * (x_diff) * (y_diff));
		}
		}
	}
	printf("\n%f\n\n", x_ratio);
	imshow("output image", img_out);

	waitKey(0);

	return 0;
}


/*
int px = (int)(w * img_in.cols) / nw;
		int py = (int)(h  * img_in.rows) / nh;

		double fx1 = (double)((w-1) * img_in.cols) / (double)(nw - 1) - (double)px;
		double fx2 = 1 - fx1;
		double fy1 = (double)((h-1) * img_in.rows)/ (double)(nh-1) - (double)py;
		double fy2 = 1 - fy1;

		double w1 = fx2 * fy2;
		double w2 = fx1 * fy2;
		double w3 = fx2 * fy1;
		double w4 = fx1 * fy1;

		uchar P1 = img_in.at<uchar>(py, px);
		uchar P2 = img_in.at<uchar>(py, px+1);
		uchar P3 = img_in.at<uchar>(py+1, px);
		uchar P4 = img_in.at<uchar>(py+1, px+1);
		pData[nh*h+w] = w1 * P1 + w2 * P2 + w3 * P3 + w4 * P4;

			}
		}

//printf("끝");


int w = img_in.cols;
int h = img_in.rows;

//imgDst.CreateImage(nw, nh);

int l, m, x1, y1, x2, y2;
double rx, ry, p, q, value;

for (m = 0; m < nh; m++)
	for (l = 0; l < nw; l++)
	{
		rx = static_cast<double>(w - 1) * l / (nw - 1);
		ry = static_cast<double>(h - 1) * m / (nh - 1);

		x1 = static_cast<int>(rx);
		y1 = static_cast<int>(ry);

		x2 = x1 + 1; if (x2 == w) x2 = w - 1;
		y2 = y1 + 1; if (y2 == h) y2 = h - 1;

		p = rx - x1;
		q = ry - y1;

		value = (1. - p) * (1. - q) * pData[y1 * nh + x1]
			+ p * (1. - q) * pData[y1 * nh + x2]
			+ (1. - p) * q * pData[y2 * nh + x1]
			+ p * q * pData[y2 * nh + x2];

		img_out = static_cast<BYTE>(value);
	}



	int index;
		float x_ratio = ((float)(img_in.cols ) / nw);
		float y_ratio = ((float)(img_in.rows ) / nh);
		float x_diff, y_diff;

		for (int h = 0; h < nh; h++) {
			for (int w = 0; w < nw; w++) {
				int px = (int)(w * x_ratio);
				int py = (int)(h * y_ratio);
				x_diff = (w * x_ratio) - px;
				y_diff = (h * y_ratio) - py;
				index = h * img_in.cols + w;

				outData[i] = (int)(pData[index] * (1 - x_diff) * (1 - y_diff) +
					+pData[index + 1] * (x_diff) * (1 - y_diff)
					+ pData[index + img_in.cols] * (1 - x_diff) * (y_diff)
					+pData[index + img_in.cols + 1] * (x_diff) * (y_diff));

				//printf("%d ",index);

			}
	*/