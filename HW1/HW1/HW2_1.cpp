// PSNR.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <math.h>
#include <limits>

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;


// HW2_3. Filtering 함수 작성
Mat my_filtering(cv::Mat input_img, int n, float*mask){
	unsigned char* pData;
	pData = (unsigned char*)input_img.data;

	float tmp = 0;
	float tmp_B = 0;
	float tmp_G = 0;
	float tmp_R = 0;
	int counter = 0;

	// Gray-scale
	if (input_img.channels() == 1) {
		Mat OutImage_gray = Mat::zeros(input_img.rows + 2, input_img.cols + 2, CV_8UC1);
		unsigned char* CData= (unsigned char*)OutImage_gray.data;
		Mat Filter_gray = Mat::zeros(input_img.rows + 2, input_img.cols + 2, CV_8UC1);
		unsigned char* GData = (unsigned char*)Filter_gray.data;

		// zero padding
		for (int row = 1; row < input_img.rows + 1; row++)
		{
			for (int col = 1; col < input_img.cols +1; col++)
			{
				CData[row * (input_img.cols + 2)  + col] = pData[(row - 1) * input_img.cols  + col - 1];
			}
		}

		// apply kernel
		for (int row = 1; row < OutImage_gray.rows-1; row++)
		{
			for (int col = 1; col < OutImage_gray.cols; col++)
			{
				for (int k = row - 1; k < row + 2*n; k++)
				{
					for (int l = col - 1; l < col + 2*n; l++)
					{
						tmp += mask[counter] * CData[k * OutImage_gray.cols + l];
						counter++;
					}
				}
				GData[col+row* OutImage_gray.cols] = tmp;
				tmp = 0;
				counter = 0;
			}
		}

		return Filter_gray;
	}

	// Color-scale
	else if (input_img.channels() == 3) {
		Mat OutImage_color = Mat::zeros(input_img.rows + 2, input_img.cols + 2, CV_8UC3);
		unsigned char* CData= (unsigned char*)OutImage_color.data;
		Mat Filter_color = Mat::zeros(input_img.rows + 2, input_img.cols + 2, CV_8UC3);
		unsigned char* FData = (unsigned char*)Filter_color.data;
		
		// zero padding
		for (int row = 1; row < input_img.rows + 1; row++)
		{
			for (int col = 3; col < input_img.cols * 3 + 3; col++)
			{
				CData[row * (input_img.cols + 2) * 3 + col] = pData[(row - 1) * (input_img.cols) * 3 + col - 3];
			}
		}

		// filtering
		for (int row = 1; row < OutImage_color.rows-1; row++)
		{
			for (int col = 1; col < OutImage_color.cols; col++)
			{
				// apply kernel
				for (int k = row - 1; k < row + 2 * n; k++)
				{
					for (int l = col - 1; l < col+ 2 * n; l++)
					{
						tmp_B += mask[counter] * CData[k * OutImage_color.cols * 3 + l * 3];
						tmp_G += mask[counter] * CData[k * OutImage_color.cols * 3 + l * 3 + 1];
						tmp_R += mask[counter] * CData[k * OutImage_color.cols * 3 + l * 3 + 2];
						counter++;
					}
				}
				FData[row * OutImage_color.cols * 3 + col * 3] = tmp_B ;
				FData[row * OutImage_color.cols * 3 + col * 3+1] = tmp_G ;
				FData[row * OutImage_color.cols * 3 + col * 3+2] = tmp_R ;
				tmp_B = 0;
				tmp_G = 0;
				tmp_R = 0;
				counter = 0;
			}
		}
		
		return Filter_color;
	}
}

int main()
{
	Mat img_in;
	Mat img_copy;

	img_in = imread("C:/Users/CEY/Desktop/Lena.png");
	cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);
	img_in.copyTo(img_copy);
	
	unsigned char* pData;
	pData = (unsigned char*)img_in.data;

	imshow("source img", img_in);

	// HW2_1. RGB 영상 포멧 확인
	/*
	for (int row = 0; row < img_in.rows; row++)
	{
		for (int col = 0; col < img_in.cols; col++)
		{
			uchar c1 = pData[row * img_in.cols * 3 + col * 3];
			uchar c2 = pData[row * img_in.cols * 3 + col * 3 + 1];
			uchar c3 = pData[row * img_in.cols * 3 + col * 3 + 2];
			printf("(%3d, %3d, %3d)", c1, c2, c3);
		}
		cout << "\n" << endl;
	}
	*/

	// HW2_2. YUV (YCbCr) color space
	//cvtColor(img_copy, img_copy, cv::COLOR_RGB2YCrCb);
	//imshow("output image_cvtcolor", img_copy);
	/*
	for (int row = 0; row < img_in.rows; row++)
	{
		for (int col = 0; col < img_in.cols; col++)
		{
			uchar B = pData[row * img_in.cols * 3 + col * 3];
			uchar G = pData[row * img_in.cols * 3 + col * 3 + 1];
			uchar R = pData[row * img_in.cols * 3 + col * 3 + 2];
			uchar Y = (0.257 * R) + (0.504 * G) + (0.098 * B) + 16;
			uchar U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128;
			uchar V = (0.439 * R) - (0.368 * G) - (0.071 * B) + 128;
			
			pData[row * img_in.cols * 3 + col * 3] = Y;
			pData[row * img_in.cols * 3 + col * 3 + 1] = U;
			pData[row * img_in.cols * 3 + col * 3 + 2] = V;
		}
	}
	*/
	//printf("\n%d, %d\n", img_in.cols, img_in.rows);
	//imshow("output image_YUV", img_in);
	
	// HW2_4. 다양한 filter 적용
	float Original_kernel[9] = { 0,0,0,0,1,0,0,0,0 };
	float moving_average[9] = { (float)1/9,(float)1 / 9,(float)1 / 9,(float)1 / 9,(float)1 / 9,(float)1 / 9,(float)1/9,(float)1 / 9,(float)1 / 9 };
	float laplace_mask[9] = { 0, 1, 0, 1, -4, 1, 0, 1, 0 };
	
	// apply sharpening filter
	if (img_in.channels() == 1) {
		Mat Original = Mat::zeros(img_in.rows + 2, img_in.cols + 2, CV_8UC1);
		Mat blur = Mat::zeros(img_in.rows + 2, img_in.cols + 2, CV_8UC1);
		Original = my_filtering(img_in, 1, Original_kernel);
		blur = my_filtering(img_in, 1, moving_average);
		//imshow("output image_SharpenFilter", Original - blur + Original);
	}
	else if (img_in.channels() == 3) {
		Mat Original = Mat::zeros(img_in.rows + 2, img_in.cols + 2, CV_8UC3);
		Mat blur = Mat::zeros(img_in.rows + 2, img_in.cols + 2, CV_8UC3);
		Original = my_filtering(img_in, 1, Original_kernel);
		blur = my_filtering(img_in, 1, moving_average);
		//imshow("output image_SharpenFilter", Original - blur + Original);
	}
	//imshow("output image_Original", my_filtering(img_in, 1, Original_kernel));
	imshow("output image_moving_average", my_filtering(img_in, 2, moving_average));
	//imshow("output image_laplacian", my_filtering(img_in, 1, laplace_mask));

	waitKey(0);

	return 0;
}

