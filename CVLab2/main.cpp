#include <iostream>
#include <fstream>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/video/background_segm.hpp>

#include "OptionParser.h"
using namespace cv;
using namespace std;
using namespace optparse;


#define MAIN_WINDOW "poots"
#define DIFF_WINDOW "diff"
#define THRE_WINDOW "thre"


#define SENSITIVITY_VALUE 20
#define BLUR_SIZE 20
#define FRAME_CAP 20
#define MOG_HISTORY 50
#define MIXTURES 3
#define BACKGROUND_RATIO 0.8
#define NOISE_SIGMA 0.05
#define LEARNING_RATE 0.1

class GenericClassnameTracker9000
{
	VideoCapture capture;
	ofstream logger;
	BackgroundSubtractor* substractor;
	static bool mouse_is_dragging;
	static bool mouse_is_moving;
	static bool rectangle_selected;
	static Point initial_click_point, current_mouse_point;

	//routine runtime parameters
	Mat curr_bgr_frame;
	bool debug;
	bool running = true;
	bool paused = false;
public:
	static Scalar hsv_min;
	static Scalar hsv_max;
	GenericClassnameTracker9000(string filename, string log_name, bool extern_debug)
	{
		debug = extern_debug;
		capture = VideoCapture(filename);
		logger.open(log_name);
		logger << filename << endl;
	}

	~GenericClassnameTracker9000()
	{
		logger.close();
	}

	void Prelearn()
	{
		Mat frame;
		substractor = new BackgroundSubtractorMOG(MOG_HISTORY, MIXTURES, BACKGROUND_RATIO, NOISE_SIGMA);
		for (int frame_no = 0; frame_no < FRAME_CAP; frame_no++)
		{
			capture.read(frame);
			if (frame.empty()) throw exception("not enough frames");
			substractor->operator()(frame, frame,LEARNING_RATE);
		}
	}

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

	void WritePosition(int X, int Y)
	{
		if (logger.is_open())
		{
			logger << X << " " << Y << endl;
		}
		else throw exception("well shit");
	}

	void TrackingRoutine()
	{
		capture.read(curr_bgr_frame);
		if (curr_bgr_frame.empty())
		{
			running = false;
			return; // I DON'T LIKE IT
		}
		Mat curr_hsv_frame, diff_frame, thre_frame;
		cvtColor(curr_bgr_frame, curr_hsv_frame, CV_BGR2HSV);

		substractor->operator()(curr_bgr_frame, diff_frame);
		inRange(curr_hsv_frame, hsv_min, hsv_max, thre_frame);
		bitwise_and(diff_frame, thre_frame, diff_frame);


		threshold(diff_frame, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		Mat element = getStructuringElement(MORPH_RECT, Size(BLUR_SIZE, BLUR_SIZE));
		morphologyEx(thre_frame, thre_frame, MORPH_CLOSE, element);
		blur(thre_frame, thre_frame, Size(BLUR_SIZE, BLUR_SIZE));
		threshold(thre_frame, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);

		Rect object_bounding_rectangle;
		Point2d last_position;
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;

		findContours(thre_frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);  // retrieves external contours
		/*for (vector<Point> contour : contours)
		{
			object_bounding_rectangle = boundingRect(contour);
			rectangle(curr_bgr_frame, object_bounding_rectangle, Scalar(0, 0, 0));
		}*/
		object_bounding_rectangle = boundingRect(contours.back());
		rectangle(curr_bgr_frame, object_bounding_rectangle, Scalar(0, 0, 0));
		int x_pos = object_bounding_rectangle.x + object_bounding_rectangle.width / 2;
		int y_pos = object_bounding_rectangle.y + object_bounding_rectangle.height / 2;

		if (mouse_is_dragging)
		{
			rectangle(curr_bgr_frame, initial_click_point, current_mouse_point, Scalar(0, 0, 0));
		}
		if (debug)
		{
			imshow(DIFF_WINDOW, diff_frame);
			imshow(THRE_WINDOW, thre_frame);
			char* text = new char[10];
			sprintf(text, "x:%d x:%d", x_pos, y_pos);
			putText(curr_bgr_frame, text, object_bounding_rectangle.tl(), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
		}
		else
		{
			cvDestroyWindow(DIFF_WINDOW);
			cvDestroyWindow(THRE_WINDOW);
		}
		imshow(MAIN_WINDOW, curr_bgr_frame);
		WritePosition(x_pos, y_pos);
	}

	void PauseRoutine()
	{
		Mat image = curr_bgr_frame.clone();
		if (mouse_is_dragging)
		{
			rectangle(image, initial_click_point, current_mouse_point, Scalar(0, 0, 0));
		}
		imshow(MAIN_WINDOW, image);
	}

	void Routine()
	{
		Prelearn();
		capture.read(curr_bgr_frame);
		namedWindow(MAIN_WINDOW);
		setMouseCallback(MAIN_WINDOW, ClickAndDragRectangle, &curr_bgr_frame);

		while (running)
		{
			if (!paused) TrackingRoutine();
			else PauseRoutine();
			
			switch (waitKey(10))
			{
			case 'd':debug = !debug; break;
			case 'D':debug = !debug; break;
			case 27: running = false; break;
			case 'p':paused = !paused; break;
			case 'P':paused = !paused; break;
			case ' ':paused = !paused; break;
			}
		}
		cvDestroyWindow(MAIN_WINDOW);
		if (debug)
		{
			cvDestroyWindow(DIFF_WINDOW);
			cvDestroyWindow(THRE_WINDOW);
		}
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

void main(int argc, char* argv[])
{
	OptionParser optparse = OptionParser();
	optparse.add_option("--infile").help("select source for tracking");
	optparse.add_option("--outlog").help("select file for output");
	optparse.add_option("--debug").action("store_true").help("show debug windows");


	Values& options = optparse.parse_args(argc, argv);
	GenericClassnameTracker9000 tracker(options["infile"], options["outlog"], options.is_set("debug"));
	tracker.Routine();
}