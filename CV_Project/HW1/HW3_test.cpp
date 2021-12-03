#include <opencv2/opencv.hpp>



IplImage* GaussNoise(IplImage* img);



void main()

{

	IplImage* img = 0;

	IplImage* noise = 0;



	img = cvLoadImage("lena.jpg", 0);



	cvNamedWindow("origin", CV_WINDOW_AUTOSIZE);

	cvNamedWindow("noise", CV_WINDOW_AUTOSIZE);



	cvShowImage("origin", img);



	noise = GaussNoise(img);



	cvShowImage("noise", noise);



	cvWaitKey(0);



	cvReleaseImage(&img);

	cvReleaseImage(&noise);

}



IplImage* GaussNoise(IplImage* img)

{

	int height, width, step;

	uchar* data;



	height = img->height;

	width = img->width;

	step = img->widthStep;

	data = (uchar*)img->imageData;



	int r1, r2, img_size;

	double rand1, rand2, normal, std_normal, tmp;

	time_t t;

	double std = 20;

	img_size = width * height;

	srand(time(&t));



	do {

		r1 = rand() % width;

		r2 = rand() % height;



		rand1 = (double)rand() / RAND_MAX;

		rand2 = (double)rand() / RAND_MAX;



		std_normal = sqrt(-2.0 * log(rand1)) * cos(2 * 3.141592 * rand2);

		normal = std * std_normal;



		tmp = data[r1 * step + r2] + normal;



		if (tmp < 0)

			data[r1 * step + r2] = 0;

		else if (tmp > 255)

			data[r1 * step + r2] = 255;

		else

			data[r1 * step + r2] = (unsigned char)tmp;



		img_size--;

	} while (img_size > 0);

	return img;

}


