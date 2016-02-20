#include <iostream>
#include <fstream>
#include <algorithm>

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
#define HORIZONTAL_BORDER_CROP 20


#define MAIN_WINDOW "poots"
#define DIFF_WINDOW "diff"
#define THRE_WINDOW "thre"
#define DEBUG_WINDOW "debug"


#define SENSITIVITY_VALUE 15
#define BLUR_SIZE 20
#define FRAME_CAP 20
#define MOG_HISTORY 50
#define MIXTURES 3
#define BACKGROUND_RATIO 0.8
#define NOISE_SIGMA 0.05
#define LEARNING_RATE 0.1

class GenericClassnameOneTracker9000
{
	VideoCapture capture;
	ofstream logger;
	static bool mouse_is_dragging;
	static bool mouse_is_moving;
	static bool rectangle_selected;
	static Point initial_click_point, current_mouse_point;

	//routine runtime parameters
	vector<Mat*> debug_images;
	Mat current_transform, previous_transform;
	Mat curr_bgr_frame, curr_gray, prev_gray;
	bool debug;
	bool running = true;
	bool paused = false;
public:
	static Scalar hsv_min;
	static Scalar hsv_max;

	GenericClassnameOneTracker9000(string filename, string log_name, bool extern_debug)
	{
		debug = extern_debug;
		capture = VideoCapture(filename);
		logger.open(log_name);
		logger << filename << endl;
	}

	~GenericClassnameOneTracker9000()
	{
		logger.close();
	}

	void AddToDebugImages(Mat* image_pointer)
	{
		if (find(debug_images.begin(), debug_images.end(), image_pointer) == debug_images.end())
		{
			debug_images.push_back(image_pointer);
		}
	}

	void ShowDebugImages()
	{
		if (!debug_images.empty())
		{
			Mat full_image(*debug_images[0]);
			int middle = debug_images.size() / 2;
			int content_height = 0;
			int content_width = full_image.cols;
			for (int xpos = 1; xpos < debug_images.size(); xpos++)
			{
				if (xpos == middle + 1)//this is really stupid and could be fixed by ypos but noo, look at mister fancy pants
				{
					content_width = 0;
					content_height = full_image.rows;
				}
				Mat temp = *debug_images[xpos];
				Rect image_position(content_width, content_height, temp.cols, temp.rows);
				Rect full_image_rect(0, 0, full_image.cols, full_image.rows);
				bool is_inside = (full_image_rect & image_position) == image_position;
				if (!is_inside)
				{
					Mat temp_full_image((full_image.rows > (image_position.y + image_position.height)) ? full_image.rows : image_position.y + image_position.height,
						(full_image.cols > (image_position.x + image_position.width)) ? full_image.cols : image_position.x + image_position.width, full_image.type());
					full_image.copyTo(temp_full_image(Rect(0, 0, full_image.cols, full_image.rows)));
					full_image = temp_full_image.clone();
				}
				//Mat proxy_full(full_image, image_position);
				temp.copyTo(full_image(image_position));
				content_width += image_position.width;
			}
			imshow(DEBUG_WINDOW, full_image);
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
			cout << " hsv min " << hsv_min[0] << " " << hsv_min[1] << " " << hsv_min[2] << " " << endl;
			cout << " hsv max " << hsv_max[0] << " " << hsv_max[1] << " " << hsv_max[2] << " " << endl;
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
		Mat curr_hsv_frame, thre_frame;
		cvtColor(curr_bgr_frame, curr_hsv_frame, CV_BGR2HSV);
		cvtColor(curr_bgr_frame, curr_gray, CV_BGR2GRAY);
		vector <Point2f> prev_corner, cur_corner;
		vector <Point2f> prev_corner2, cur_corner2;
		vector <uchar> status;
		vector <float> err;

		goodFeaturesToTrack(prev_gray, prev_corner, 200, 0.01, 30);
		calcOpticalFlowPyrLK(prev_gray, curr_gray, prev_corner, cur_corner, status, err);

		// weed out bad matches
		for (size_t i = 0; i < status.size(); i++) {
			if (status[i]) {
				prev_corner2.push_back(prev_corner[i]);
				cur_corner2.push_back(cur_corner[i]);
			}
		}
		//EXCEPTION CAUSE START
		// translation + rotation only
		if (prev_corner2.size()>0 && cur_corner2.size() > 0)
		{
			current_transform = estimateRigidTransform(prev_corner2, cur_corner2, false); // false = rigid transform, no scaling/shearing
		}
		if (current_transform.rows == 0)
		{
			current_transform = previous_transform.clone();
		}
		//EXCEPTION CAUSE END
		Mat stabilized, current_diff;
		warpAffine(prev_gray, stabilized, current_transform, prev_gray.size());
		absdiff(stabilized, curr_gray, current_diff);
		AddToDebugImages(&current_diff);//"stab-diff"

		Mat element = getStructuringElement(MORPH_RECT, Size(BLUR_SIZE, BLUR_SIZE));
		Mat current_diff_closed;
		morphologyEx(current_diff, current_diff_closed, MORPH_CLOSE, element);
		AddToDebugImages(&current_diff_closed);//"pre-blur"

		Mat current_diff_closed_blurred;
		blur(current_diff_closed, current_diff_closed_blurred, Size(BLUR_SIZE, BLUR_SIZE));
		AddToDebugImages(&current_diff_closed_blurred);//"post-blur"

		Mat current_diff_closed_blurred_threshold;
		threshold(current_diff_closed_blurred, current_diff_closed_blurred_threshold, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		AddToDebugImages(&current_diff_closed_blurred_threshold);//"thresh-diff"

		Mat current_in_hsv_range;
		Mat raw_mask;
		inRange(curr_hsv_frame, hsv_min, hsv_max, current_in_hsv_range);
		bitwise_and(current_diff_closed_blurred_threshold, current_in_hsv_range, raw_mask);

		Mat threshold_mask;
		threshold(raw_mask, threshold_mask, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		Mat threshold_closed_mask;
		morphologyEx(threshold_mask, threshold_closed_mask, MORPH_CLOSE, element);
		Mat threshold_closed_blur_mask;
		blur(threshold_closed_mask, threshold_closed_blur_mask, Size(BLUR_SIZE, BLUR_SIZE));
		threshold(threshold_closed_blur_mask, thre_frame, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		AddToDebugImages(&thre_frame);


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
		if (contours.size() > 0)//hotfix. find a better solution
		{
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
				ShowDebugImages();
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
		previous_transform = current_transform.clone();
		prev_gray = curr_gray.clone();

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
		capture.read(curr_bgr_frame);
		cvtColor(curr_bgr_frame, prev_gray, CV_BGR2GRAY);
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

bool GenericClassnameOneTracker9000::mouse_is_dragging = false;
bool GenericClassnameOneTracker9000::mouse_is_moving = false;
bool GenericClassnameOneTracker9000::rectangle_selected = false;
Point GenericClassnameOneTracker9000::initial_click_point = Point();
Point GenericClassnameOneTracker9000::current_mouse_point = Point();
Scalar GenericClassnameOneTracker9000::hsv_min = Scalar(0, 0, 0);
Scalar GenericClassnameOneTracker9000::hsv_max = Scalar(255, 255, 255);
//gee this is stupid

void main(int argc, char* argv[])
{
	OptionParser optparse = OptionParser();
	optparse.add_option("--infile").help("select source for tracking");
	optparse.add_option("--outlog").help("select file for output");
	optparse.add_option("--debug").action("store_true").help("show debug windows");


	Values& options = optparse.parse_args(argc, argv);
	GenericClassnameOneTracker9000 tracker(options["infile"], options["outlog"], options.is_set("debug"));
	tracker.Routine();
}