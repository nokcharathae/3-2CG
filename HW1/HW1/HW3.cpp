#include "stdafx.h"

//#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <time.h>

using namespace cv;
using namespace std;

Mat my_GaussianNoise(cv::Mat input_img, double sigma);
Mat my_SaltAndPepper(cv::Mat input_img, double ratio);
Mat my_filtering(cv::Mat input_img, int n, float* mask);
Mat my_MedianFilter(cv::Mat input_img);

int main()
{
	Mat img_in;

	img_in = imread("C:/Users/CEY/Desktop/Lena.png");
	cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);

	imshow("Original Image", img_in);
	imshow("GaussianNoise Sigma 30", my_GaussianNoise(img_in, 30));
	imshow("SaltAndPepperNoise 5%", my_SaltAndPepper(img_in, 0.05));

	float moving_average_3X3[9] = { (float)1 / 9,(float)1 / 9,(float)1 / 9,(float)1 / 9,(float)1 / 9,
								 (float)1 / 9,(float)1 / 9,(float)1 / 9,(float)1 / 9 };
	float moving_average_5X5[25] = { (float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 259,
								 (float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,
								 (float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,
								 (float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,
								 (float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25,(float)1 / 25 };

	//imshow("GaussianNoise Sigma 15", my_GaussianNoise(img_in, 15));
	//imshow("GaussianNoise Sigma 30", my_GaussianNoise(img_in, 30));
	//imshow("GaussianNoise Sigma 45", my_GaussianNoise(img_in, 45));
	//imshow("Smoothing filter (15, 3X3)", my_filtering(my_GaussianNoise(img_in,15), 1, moving_average_3X3));
	//imshow("Smoothing filter (15, 5X5)", my_filtering(my_GaussianNoise(img_in,15), 2, moving_average_5X5));
	//imshow("Smoothing filter (30, 3X3)", my_filtering(my_GaussianNoise(img_in, 30), 1, moving_average_3X3));
	//imshow("Smoothing filter (30, 5X5)", my_filtering(my_GaussianNoise(img_in, 30), 2, moving_average_5X5));
	//imshow("Smoothing filter (45, 3X3)", my_filtering(my_GaussianNoise(img_in, 45), 1, moving_average_3X3));
	//imshow("Smoothing filter (45, 5X5)", my_filtering(my_GaussianNoise(img_in, 45), 2, moving_average_5X5));
	
	//imshow("SaltAndPepperNoise 5%", my_SaltAndPepper(img_in, 0.05));
	//imshow("SaltAndPepperNoise 10%", my_SaltAndPepper(img_in, 0.1));
	//imshow("SaltAndPepperNoise 25%", my_SaltAndPepper(img_in, 0.25));
	//imshow("Median filter 5%", my_MedianFilter(my_SaltAndPepper(img_in, 0.05)));
	//imshow("Median filter 10%", my_MedianFilter(my_SaltAndPepper(img_in, 0.1)));
	//imshow("Median filter 25%", my_MedianFilter(my_SaltAndPepper(img_in, 0.25)));

	waitKey(0);

	return 0;

}


// 1.1. Gaussian noise 생성 함수 
Mat my_GaussianNoise(cv::Mat input_img, double sigma)
{
	Mat noise_image(input_img.size(), CV_64FC1);
	double mean = 0.0;
	randn(noise_image, Scalar::all(mean), Scalar::all(sigma));

	Mat temp_image;
	input_img.copyTo(temp_image);
	temp_image += noise_image;

	return temp_image;
}

// 1.2. Salt/Pepper noise 생성 함수
Mat my_SaltAndPepper(cv::Mat input_img, double ratio)
{
	Mat noise_image;
	input_img.copyTo(noise_image);
	int height= input_img.rows;
	int width=input_img.cols; 
	int ImageSize = (int)((double)(height * width) * ratio);

	for (int i = 0; i < ImageSize; i++)
	{
		int r = rand() % height;
		int c = rand() % width;
		uchar* pixel = noise_image.ptr<uchar>(r) + c;
		*pixel = (rand() % 2 == 1) ? 255 : 0;
	}

	return noise_image;
}

// 3. 3X3 Median filter
Mat my_MedianFilter(cv::Mat input_img)
{
	int m[9];
	int i, key, j;
	Mat filtered_image;
	input_img.copyTo(filtered_image);

	for (int row = 1; row < input_img.rows-1 ; row++)
	{
		for (int col = 1; col < input_img.cols-1 ; col++)
		{
			m[0] = filtered_image.at<uchar>(row - 1, col - 1);
			m[1] = filtered_image.at<uchar>(row - 1, col);
			m[2] = filtered_image.at<uchar>(row - 1, col + 1);
			m[3] = filtered_image.at<uchar>(row, col - 1);
			m[4] = filtered_image.at<uchar>(row, col);
			m[5] = filtered_image.at<uchar>(row, col + 1);
			m[6] = filtered_image.at<uchar>(row + 1, col - 1);
			m[7] = filtered_image.at<uchar>(row + 1, col );
			m[8] = filtered_image.at<uchar>(row + 1, col + 1);

			for (i = 1; i < 9; i++)
			{
				key = m[i];
				j = i - 1;

				while (j >= 0 && m[j] > key)
				{
					m[j + 1] = m[j];
					j = j - 1;
				}
				m[j + 1] = key;
			}

			filtered_image.at<uchar>(row, col) = m[4];
		}
	}

	return filtered_image;
}

// HW2의 Filtering 함수 
Mat my_filtering(cv::Mat input_img, int n, float* mask) 
{
	unsigned char* pData;
	pData = (unsigned char*)input_img.data;

	float tmp = 0;
	float tmp_B = 0;
	float tmp_G = 0;
	float tmp_R = 0;
	int counter = 0;

	// Gray-scale
	if (input_img.channels() == 1) {
		Mat OutImage_gray = Mat::zeros(input_img.rows + 2*n, input_img.cols + 2*n, CV_8UC1);
		unsigned char* CData = (unsigned char*)OutImage_gray.data;
		Mat Filter_gray = Mat::zeros(input_img.rows + 2 * n, input_img.cols + 2 * n, CV_8UC1);
		unsigned char* GData = (unsigned char*)Filter_gray.data;

		// zero padding
		for (int row = n; row < input_img.rows + n; row++)
		{
			for (int col = n; col < input_img.cols + n; col++)
			{
				CData[row * (input_img.cols + 2*n) + col] = pData[(row - n) * input_img.cols + col - n];
			}
		}

		// apply kernel
		for (int row = n; row < input_img.rows + n; row++)
		{
			for (int col = n; col < input_img.cols + n; col++)
			{
				for (int k = row - n; k < row + n +1; k++)
				{
					for (int l = col -n; l < col + n+1 ; l++)
					{
						tmp += mask[counter] * CData[k * OutImage_gray.cols + l];
						counter++;
					}
				}
				GData[col + row * OutImage_gray.cols] = tmp;
				tmp = 0;
				counter = 0;
			}
		}

		return Filter_gray;
	}
}



