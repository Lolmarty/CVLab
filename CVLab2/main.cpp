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
	bool mouse_is_dragging;//used for showing a rectangle on screen as user clicks and drags mouse
	bool mouse_is_moving;
	bool rectangle_selected;
	Point initial_click_point, current_mouse_point; //keep track of initial point clicked and current position of mouse
	Rect rectangleROI;
public:
	Scalar hsv_min, hsv_max;
	GenericClassnameTracker9000(){}
	~GenericClassnameTracker9000(){}

	void GetHSVBoundaries(Mat frame)
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

	void clickAndDragRectangle(int event, int x, int y, int flags, void* param){
		//only if calibration mode is true will we use the mouse to change HSV values
		//get handle to video feed passed in as "param" and cast as Mat pointer
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
			}
			/* user has released left button */
			case CV_EVENT_LBUTTONUP:
			{
				//set rectangle ROI to the rectangle that the user has selected
				rectangleROI = Rect(initial_click_point, current_mouse_point);

				//reset boolean variables
				mouse_is_dragging = false;
				mouse_is_moving = false;
				rectangle_selected = true;
			}
			}

			imshow(MAIN_WINDOW, *videoFeed);
		
		}

		if (event == CV_EVENT_RBUTTONDOWN){
			//user has clicked right mouse button
			//Reset HSV Values
			H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 255;
			S_MAX = 255;
			V_MAX = 255;

		}
		if (event == CV_EVENT_MBUTTONDOWN){

			//user has clicked middle mouse button
			//enter code here if needed.
		}

	}
};



void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame){

	//save HSV values for ROI that user selected to a vector
	if (mouseMove == false && rectangleSelected == true){

		//clear previous vector values
		if (H_ROI.size()>0) H_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		if (V_ROI.size()>0)V_ROI.clear();
		//if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
		if (rectangleROI.width<1 || rectangleROI.height<1) cout << "Please drag a rectangle, not a line" << endl;
		else{
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++){
				//iterate through both x and y direction and save HSV values at each and every point
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++){
					//save HSV value at this point
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		//reset rectangleSelected so user can select another region if necessary
		rectangleSelected = false;
		//set min and max HSV values from min and max elements of each array

		if (H_ROI.size()>0){
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			cout << "MIN 'H' VALUE: " << H_MIN << endl;
			cout << "MAX 'H' VALUE: " << H_MAX << endl;
		}
		if (S_ROI.size()>0){
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			cout << "MIN 'S' VALUE: " << S_MIN << endl;
			cout << "MAX 'S' VALUE: " << S_MAX << endl;
		}
		if (V_ROI.size()>0){
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			cout << "MIN 'V' VALUE: " << V_MIN << endl;
			cout << "MAX 'V' VALUE: " << V_MAX << endl;
		}

	}

	if (mouseMove == true){
		//if the mouse is held down, we will draw the click and dragged rectangle to the screen
		;
	}


}

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
	VideoCapture capture = VideoCapture("../assets/poop0.avi");
	Mat prev_gray_frame, curr_gray_frame, curr_bgr_frame, curr_hsv_frame, diff_frame, thre_frame;
	bool running = true;
	setMouseCallback(MAIN_WINDOW, clickAndDrag_Rectangle, &curr_bgr_frame);
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
			Mat chunk(curr_bgr_frame,object_bounding_rectangle);
			Scalar hsv_min, hsv_max;
			GetHSVBoundaries(chunk, hsv_min, hsv_max);
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
		case 'p': 
			{
				bool paused = true;
				while (paused)
				{
					if (waitKey(50) == 'p' || waitKey(50) == 'P') 
						paused = false; // or paused = waitKey(50) != 'p' && waitKey(50) != 'P';
				}
				break;
			}
		}
	}
	cvDestroyWindow(MAIN_WINDOW);
}