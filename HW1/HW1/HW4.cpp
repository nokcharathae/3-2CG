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
	Mat Input_Image = imread("C:/Users/CEY/Desktop/Lena.png"); // Lena ������ input
	// cvtColor(Input_Image, Input_Image, cv::COLOR_RGB2GRAY); // Lena ������ gray scale�� ��
	cout << "Height: " << Input_Image.rows << ", Width: " << Input_Image.rows << ", Channels: " << Input_Image.channels() << endl; // input ������ height, width, channel ������ ��ȯ
	int Clusters = 3; // �� ���� segmentaion�� �� �����ִ� k parameter
	Mat Clustered_Image = K_Means(Input_Image, Clusters); // k-mean clustering ���� segmentaion�� ���ִ� �Լ�
	
	imshow("Clustered_Image.png", Clustered_Image); // segmentation�� output ����

	waitKey(0);
	return 0;

}

// k - mean clustering�� ���� segmentaion�ϴ� �Լ�
Mat K_Means(Mat Input, int K) {
	// cv2.kmeans�� �Է��ϴ� sample ������ np.float32 ������ �����̾�� �ϸ� �� ���Ĵ� ���� ���� ��ġ�Ǿ� �־�� �ϹǷ� �� ������ ����
	Mat samples(Input.rows * Input.cols, Input.channels(), CV_32F); // np.float32 ������ ������ input ���� ũ���� ���� ä�� ���� ���� ���� 
	for (int y = 0; y < Input.rows; y++) // input.rows��ŭ �ݺ�
		for (int x = 0; x < Input.cols; x++) // input.cols��ŭ �ݺ�
			for (int z = 0; z < Input.channels(); z++) // input.chnnels��ŭ �ݺ�
				if (Input.channels() == 3) { // input ������ 3 channel�� ���, �� color ������ ���
					samples.at<float>(y + x * Input.rows, z) = Input.at<Vec3b>(y, x)[z]; // sample ���� 3���� unsigned char�� ������ ���͸� �̿��Ͽ� input ������ ����
				}
				else { // input ������ 3 channel�� �ƴ� ���, �� color ������ �ƴ� ���
					samples.at<float>(y + x * Input.rows, z) = Input.at<uchar>(y, x); // sample ���� uchar�� �̿��Ͽ� input ������ ����
				}

	Mat labels; // kmeans�� ����� cluster index�� �����ϴ� bestLabels parameter 
	int attempts = 5; // kmeans�� ����� �ʱ� ���̺��� ����Ͽ� �˰����� ����Ǵ� Ƚ���� �����ϴ� attempts parameter
	Mat centers; // kmeans�� ����� cluster center�� ����ϴ� ����� centers parameter
	kmeans(samples, K, labels, TermCriteria(TermCriteria::MAX_ITER | TermCriteria::EPS, 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers);
	// ������ �߽��� ã�� ���� �ֺ����� �Է� ǥ���� �׷�ȭ�ϴ� �Լ��� cluster_count ������ �߽��� ã�� ���� ������ �Է� ǥ���� �׷�ȭ�ϴ� k-��� �˰���
	// kmeans	(	InputArray 	data,						// �����ʹ� np.float32 ������ �����̾�� �ϸ� �� ���Ĵ� ���� ���� ��ġ�Ǿ�� ��
	//				int 	K,								// ������ ������ ���� ��
	//				InputOutputArray 	bestLabels,			// ��� ǥ���� ���� cluster index�� �����ϴ� �Է�/��� ���� �迭
	//				TermCriteria 	criteria,				// �˰��� ���� ����, �� �ִ� �ݺ� Ƚ�� ��/�Ǵ� ���ϴ� ��Ȯ�� 
	//														   - cv.TERM_CRITERia_EPS - ������ ��Ȯ���� epsilon�� �����ϸ� �˰��� �ݺ��� ����
	//														   - cv.TERM_CRITERIA_MAX_ITER - ������ �ݺ� Ƚ�� max_iter �� �˰����� ����
	//														   - cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER - �� ���� �� �ϳ��� �����Ǹ� �ݺ��� ����
	//																max_iter - �ִ� �ݺ� Ƚ���� �����ϴ� ����
	//																epsilon - �ʿ��� ��Ȯ��
	//				int 	attempts,						// �ٸ� �ʱ� ���̺��� ����Ͽ� �˰����� ����Ǵ� Ƚ���� �����ϴ� �÷��׷� �˰����� �ֻ��� ���൵�� �����ϴ� ���̺��� ������� ��ȯ
	//				int 	flags,							// �� �÷��״� �ʱ� �߽��� ���ϴ� ����� �����ϴ� �� ���(KMEANS_PP_CENTS, KMEANS_RANDOM_CENTERS, KMEANS_USE_INITIAL_LABELS)
	//				OutputArray 	centers = noArray()	)	// cluster center(�� cluster center �� �ϳ��� ��)�� ��� ���

	Mat new_image(Input.size(), Input.type()); // segmentaion�� ������ �����ϴ� output ����
	for (int y = 0; y < Input.rows; y++) // input.rows��ŭ �ݺ�
		for (int x = 0; x < Input.cols; x++) // input.cols��ŭ �ݺ�
		{
			int cluster_idx = labels.at<int>(y + x * Input.rows, 0); // kmeans�� ���� ����� labels�� ���� cluster index
			if (Input.channels() == 3) { // input ������ 3 channel�� ���, �� color ������ ���
				for (int z = 0; z < Input.channels(); z++) { // input.chnnels��ŭ �ݺ�
					new_image.at<Vec3b>(y, x)[z] = centers.at<float>(cluster_idx, z); // output ���� cluster index�� �ش��ϴ� cluster center�� ����
				}
			}
			else { // input ������ 3 channel�� �ƴ� ���, �� color ������ �ƴ� ���
				new_image.at<uchar>(y, x) = centers.at<float>(cluster_idx, 0); // output ���� cluster index�� �ش��ϴ� cluster center�� ����
			}
		}

	return new_image; // output ���� return
}

// ���� �ڵ� : https://github.com/abubakr-shafique/K_Means_Clustering_CPP/blob/master/K_Means_Clustering.cpp