#include <iostream>


#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/video/background_segm.hpp>
using namespace cv;
using namespace std;

#define MAIN_WINDOW "poots"
#define SENSITIVITY_VALUE 20
#define BLUR_SIZE 20

class GenericClassnameTracker9000
{
private:
	Scalar HSVMin, HSVMax;
public:
	GenericClassnameTracker9000(){}
	~GenericClassnameTracker9000(){}
};

void GetHSVBoundaries(Mat frame, Scalar& hsv_min, Scalar& hsv_max)
{
	hsv_min = Scalar(0, 0, 0);
	hsv_max = Scalar(255, 255, 255);
	if (!frame.empty())
	{
		vector<int> h_values, s_values, v_values;
		for (int i = 0; i < frame.cols; i++)
		{
			for (int j = 0; j < frame.rows; j++)
			{
				h_values.push_back((int)frame.at<cv::Vec3b>(j, i)[0]);
				s_values.push_back((int)frame.at<cv::Vec3b>(j, i)[1]);
				v_values.push_back((int)frame.at<cv::Vec3b>(j, i)[2]);
			}
		}
		hsv_min = Scalar(*min_element(h_values.begin(), h_values.end()), *min_element(s_values.begin(), s_values.end()), *min_element(v_values.begin(), v_values.end()));
		hsv_max = Scalar(*max_element(h_values.begin(), h_values.end()), *max_element(s_values.begin(), s_values.end()), *max_element(v_values.begin(), v_values.end()));
	}
}

void main()
{
	bool debug = false;
	VideoCapture capture = VideoCapture("../assets/poop8.avi");
	Mat prev_gray_frame, curr_gray_frame, curr_frame, diff_frame, thre_frame;
	bool running = true;
	capture.read(prev_gray_frame);
	cvtColor(prev_gray_frame, prev_gray_frame, COLOR_BGR2GRAY);

	while (running)
	{
		capture.read(curr_frame);
		if (curr_frame.empty()) break;
		cvtColor(curr_frame, curr_gray_frame, COLOR_BGR2GRAY);

		absdiff(curr_gray_frame, prev_gray_frame, diff_frame);
		threshold(diff_frame, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		//blur(thre_frame, thre_frame, Size(BLUR_SIZE, BLUR_SIZE));
		//threshold(thre_frame, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);

		Mat element = getStructuringElement(MORPH_RECT, Size(BLUR_SIZE, BLUR_SIZE));
		morphologyEx(thre_frame, thre_frame, MORPH_CLOSE, element);


		Rect object_bounding_rectangle;
		Point2d last_position;
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		
		findContours(thre_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);// retrieves external contours
		for (vector<Point> contour : contours)
		{
			object_bounding_rectangle = boundingRect(contour);
			rectangle(curr_frame, object_bounding_rectangle, Scalar(0, 0, 0));
			Mat chunk(curr_frame,object_bounding_rectangle);
			Scalar hsv_min, hsv_max;
			GetHSVBoundaries(chunk, hsv_min, hsv_max);
		}

		imshow(MAIN_WINDOW, curr_frame);
		if (debug){
			imshow(MAIN_WINDOW, diff_frame);
			waitKey(100);
			imshow(MAIN_WINDOW, thre_frame);
			waitKey(100);
			imshow(MAIN_WINDOW, curr_gray_frame);
			waitKey(100);
		}
		prev_gray_frame = curr_gray_frame.clone();
		switch (waitKey(200))
		{
		case 'd':debug = !debug; break;
		case 'D':debug = !debug; break;
		case 27: running = false; break;
		}
	}
	cvDestroyWindow(MAIN_WINDOW);
}