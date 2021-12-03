#include "stdafx.h"

//#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <random>
#include <time.h>

using namespace cv;
using namespace std;

Mat K_Means(Mat Input, int K);

int main()
{    
	Mat Input_Image = imread("C:/Users/CEY/Desktop/Lena.png"); // Lena 영상을 input
	// cvtColor(Input_Image, Input_Image, cv::COLOR_RGB2GRAY); // Lena 영상이 gray scale일 때
	cout << "Height: " << Input_Image.rows << ", Width: " << Input_Image.rows << ", Channels: " << Input_Image.channels() << endl; // input 영상의 height, width, channel 정보를 반환
	int Clusters = 3; // 몇 개로 segmentaion할 지 정해주는 k parameter
	Mat Clustered_Image = K_Means(Input_Image, Clusters); // k-mean clustering 통해 segmentaion을 해주는 함수
	
	imshow("Clustered_Image.png", Clustered_Image); // segmentation된 output 영상

	waitKey(0);
	return 0;

}

// k - mean clustering를 통해 segmentaion하는 함수
Mat K_Means(Mat Input, int K) {
	// cv2.kmeans에 입력하는 sample 영상은 np.float32 데이터 유형이어야 하며 각 피쳐는 단일 열에 배치되어 있어야 하므로 이 과정을 수행
	Mat samples(Input.rows * Input.cols, Input.channels(), CV_32F); // np.float32 데이터 유형의 input 영상 크기의 영상 채널 수의 영상 생성 
	for (int y = 0; y < Input.rows; y++) // input.rows만큼 반복
		for (int x = 0; x < Input.cols; x++) // input.cols만큼 반복
			for (int z = 0; z < Input.channels(); z++) // input.chnnels만큼 반복
				if (Input.channels() == 3) { // input 영상이 3 channel일 경우, 즉 color 영상일 경우
					samples.at<float>(y + x * Input.rows, z) = Input.at<Vec3b>(y, x)[z]; // sample 영상에 3개의 unsigned char를 가지는 벡터를 이용하여 input 영상을 삽입
				}
				else { // input 영상이 3 channel이 아닐 경우, 즉 color 영상이 아닐 경우
					samples.at<float>(y + x * Input.rows, z) = Input.at<uchar>(y, x); // sample 영상에 uchar를 이용하여 input 영상을 삽입
				}

	Mat labels; // kmeans에 사용할 cluster index를 저장하는 bestLabels parameter 
	int attempts = 5; // kmeans에 사용할 초기 레이블링을 사용하여 알고리즘이 실행되는 횟수를 지정하는 attempts parameter
	Mat centers; // kmeans에 사용할 cluster center를 출력하는 행렬의 centers parameter
	kmeans(samples, K, labels, TermCriteria(TermCriteria::MAX_ITER | TermCriteria::EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers);
	// 군집의 중심을 찾고 군집 주변에서 입력 표본을 그룹화하는 함수로 cluster_count 군집의 중심을 찾고 군집 주위에 입력 표본을 그룹화하는 k-평균 알고리즘
	// kmeans	(	InputArray 	data,						// 데이터는 np.float32 데이터 유형이어야 하며 각 피쳐는 단일 열에 배치되어야 함
	//				int 	K,								// 집합을 분할할 군집 수
	//				InputOutputArray 	bestLabels,			// 모든 표본에 대한 cluster index를 저장하는 입력/출력 정수 배열
	//				TermCriteria 	criteria,				// 알고리즘 종료 기준, 즉 최대 반복 횟수 및/또는 원하는 정확도 
	//														   - cv.TERM_CRITERia_EPS - 지정된 정확도인 epsilon에 도달하면 알고리즘 반복을 중지
	//														   - cv.TERM_CRITERIA_MAX_ITER - 지정된 반복 횟수 max_iter 후 알고리즘을 중지
	//														   - cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER - 위 조건 중 하나라도 충족되면 반복을 중지
	//																max_iter - 최대 반복 횟수를 지정하는 정수
	//																epsilon - 필요한 정확도
	//				int 	attempts,						// 다른 초기 레이블링을 사용하여 알고리즘이 실행되는 횟수를 지정하는 플래그로 알고리즘은 최상의 압축도를 산출하는 레이블을 출력으로 반환
	//				int 	flags,							// 이 플래그는 초기 중심을 취하는 방법을 지정하는 데 사용(KMEANS_PP_CENTS, KMEANS_RANDOM_CENTERS, KMEANS_USE_INITIAL_LABELS)
	//				OutputArray 	centers = noArray()	)	// cluster center(각 cluster center 당 하나의 행)의 출력 행렬

	Mat new_image(Input.size(), Input.type()); // segmentaion된 영상을 저장하는 output 영상
	for (int y = 0; y < Input.rows; y++) // input.rows만큼 반복
		for (int x = 0; x < Input.cols; x++) // input.cols만큼 반복
		{
			int cluster_idx = labels.at<int>(y + x * Input.rows, 0); // kmeans를 거져 저장된 labels를 통해 cluster index
			if (Input.channels() == 3) { // input 영상이 3 channel일 경우, 즉 color 영상일 경우
				for (int z = 0; z < Input.channels(); z++) { // input.chnnels만큼 반복
					new_image.at<Vec3b>(y, x)[z] = centers.at<float>(cluster_idx, z); // output 영상에 cluster index에 해당하는 cluster center를 삽입
				}
			}
			else { // input 영상이 3 channel이 아닐 경우, 즉 color 영상이 아닐 경우
				new_image.at<uchar>(y, x) = centers.at<float>(cluster_idx, 0); // output 영상에 cluster index에 해당하는 cluster center를 삽입
			}
		}

	return new_image; // output 영상 return
}

// 참고 코드 : https://github.com/abubakr-shafique/K_Means_Clustering_CPP/blob/master/K_Means_Clustering.cpp