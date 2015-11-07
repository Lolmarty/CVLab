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
	static bool mouse_is_dragging;
	static bool mouse_is_moving;
	static bool rectangle_selected;
	static Point initial_click_point, current_mouse_point;
public:
	static Scalar hsv_min;
	static Scalar hsv_max;
	GenericClassnameTracker9000(){}
	~GenericClassnameTracker9000(){}

	static void GetHSVBoundaries(Mat frame)
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

	static void ClickAndDragRectangle(int event, int x, int y, int flags, void* param){
		Mat* videoFeed = (Mat*)param;
		if (!mouse_is_dragging && event == CV_EVENT_LBUTTONDOWN)
		{
			//keep track of initial point clicked
			initial_click_point = cv::Point(x, y);
			//user has begun dragging the mouse
			mouse_is_dragging = true;

		}
		if (mouse_is_dragging){
			switch (event)
			{
				/* user is dragging the mouse */
			case CV_EVENT_MOUSEMOVE:
			{
				//keep track of current mouse point
				current_mouse_point = cv::Point(x, y);
				//user has moved the mouse while clicking and dragging
				mouse_is_moving = true;
				break;
			}
			/* user has released left button */
			case CV_EVENT_LBUTTONUP:
			{
				//reset boolean variables
				mouse_is_dragging = false;
				mouse_is_moving = false;
				rectangle_selected = true;
				Mat hsv_feed;
				cvtColor(*videoFeed, hsv_feed, CV_BGR2HSV);
				Mat chunk(hsv_feed, Rect(initial_click_point, current_mouse_point));
				GetHSVBoundaries(chunk);
				break;
			}
			}
		}

		if (event == CV_EVENT_RBUTTONDOWN){
			//user has clicked right mouse button
			//Reset HSV Values
			/*H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 255;
			S_MAX = 255;
			V_MAX = 255;*/

		}
		if (event == CV_EVENT_MBUTTONDOWN){

			//user has clicked middle mouse button
			//enter code here if needed.
		}
	}

	void Pause(Mat curr_bgr_frame)
	{
		bool paused = true;
		while (paused)
		{
			Mat image = curr_bgr_frame.clone();
			if (mouse_is_dragging)
			{
				rectangle(image, initial_click_point, current_mouse_point, Scalar(0, 0, 0));  //IIIIITS SHIIIT
			}
			imshow(MAIN_WINDOW, image);
			switch (waitKey(50))
			{
			case 'p':paused = false; break;
			case 'P':paused = false; break;
			case 27: paused = false; break;
			}
		}
	}

	void Routine()
	{
		bool debug = false;
		bool running = true;
		VideoCapture capture = VideoCapture("../assets/poop0.avi");
		Mat prev_gray_frame, curr_gray_frame, curr_bgr_frame, curr_hsv_frame, diff_frame, thre_frame;
		capture.read(curr_bgr_frame);
		namedWindow(MAIN_WINDOW);
		setMouseCallback(MAIN_WINDOW, ClickAndDragRectangle, &curr_bgr_frame);
		capture.read(prev_gray_frame);
		cvtColor(prev_gray_frame, prev_gray_frame, COLOR_BGR2GRAY);

		while (running)
		{
			capture.read(curr_bgr_frame);
			if (curr_bgr_frame.empty()) break;
			cvtColor(curr_bgr_frame, curr_gray_frame, COLOR_BGR2GRAY);

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
				rectangle(curr_bgr_frame, object_bounding_rectangle, Scalar(0, 0, 0));
				cvtColor(curr_bgr_frame, curr_hsv_frame, CV_BGR2HSV);
				Mat chunk(curr_hsv_frame, object_bounding_rectangle);
				GetHSVBoundaries(chunk);
			}
			
			if (mouse_is_dragging)
			{
				rectangle(curr_bgr_frame, initial_click_point, current_mouse_point, Scalar(0, 0, 0));
			}
			imshow(MAIN_WINDOW, curr_bgr_frame);
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
			case 'p': Pause(curr_bgr_frame); break;
			case 'P': Pause(curr_bgr_frame); break;
			}
		}
		cvDestroyWindow(MAIN_WINDOW);
	}
};

bool GenericClassnameTracker9000::mouse_is_dragging = false;
bool GenericClassnameTracker9000::mouse_is_moving = false;
bool GenericClassnameTracker9000::rectangle_selected = false;
Point GenericClassnameTracker9000::initial_click_point = Point();
Point GenericClassnameTracker9000::current_mouse_point = Point();
Scalar GenericClassnameTracker9000::hsv_min = Scalar(0, 0, 0);
Scalar GenericClassnameTracker9000::hsv_max = Scalar(255, 255, 255);
//gee this is stupid




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
	GenericClassnameTracker9000 tracker;
	tracker.Routine();

	//bool debug = false;
	//VideoCapture capture = VideoCapture("../assets/poop0.avi");
	//Mat prev_gray_frame, curr_gray_frame, curr_bgr_frame, curr_hsv_frame, diff_frame, thre_frame;
	//bool running = true;
	//setMouseCallback(MAIN_WINDOW, clickAndDrag_Rectangle, &curr_bgr_frame);
	//capture.read(prev_gray_frame);
	//cvtColor(prev_gray_frame, prev_gray_frame, COLOR_BGR2GRAY);

	//while (running)
	//{
	//	capture.read(curr_bgr_frame);
	//	if (curr_bgr_frame.empty()) break;
	//	cvtColor(curr_bgr_frame, curr_gray_frame, COLOR_BGR2GRAY);

	//	absdiff(curr_gray_frame, prev_gray_frame, diff_frame);
	//	threshold(diff_frame, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);
	//	//blur(thre_frame, thre_frame, Size(BLUR_SIZE, BLUR_SIZE));
	//	//threshold(thre_frame, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);

	//	Mat element = getStructuringElement(MORPH_RECT, Size(BLUR_SIZE, BLUR_SIZE));
	//	morphologyEx(thre_frame, thre_frame, MORPH_CLOSE, element);

	//	Rect object_bounding_rectangle;
	//	Point2d last_position;
	//	vector< vector<Point> > contours;
	//	vector<Vec4i> hierarchy;
	//	
	//	findContours(thre_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);// retrieves external contours
	//	for (vector<Point> contour : contours)
	//	{
	//		object_bounding_rectangle = boundingRect(contour);
	//		rectangle(curr_bgr_frame, object_bounding_rectangle, Scalar(0, 0, 0));
	//		Mat chunk(curr_bgr_frame,object_bounding_rectangle);
	//		Scalar hsv_min, hsv_max;
	//		GetHSVBoundaries(chunk, hsv_min, hsv_max);
	//	}

	//	imshow(MAIN_WINDOW, curr_bgr_frame);
	//	if (debug){
	//		imshow(MAIN_WINDOW, diff_frame);
	//		waitKey(100);
	//		imshow(MAIN_WINDOW, thre_frame);
	//		waitKey(100);
	//		imshow(MAIN_WINDOW, curr_gray_frame);
	//		waitKey(100);
	//	}
	//	prev_gray_frame = curr_gray_frame.clone();
	//	switch (waitKey(200))
	//	{
	//	case 'd':debug = !debug; break;
	//	case 'D':debug = !debug; break;
	//	case 27: running = false; break;
	//	case 'p': 
	//		{
	//			bool paused = true;
	//			while (paused)
	//			{
	//				if (waitKey(50) == 'p' || waitKey(50) == 'P') 
	//					paused = false; // or paused = waitKey(50) != 'p' && waitKey(50) != 'P';
	//			}
	//			break;
	//		}
	//	}
	//}
	//cvDestroyWindow(MAIN_WINDOW);
}