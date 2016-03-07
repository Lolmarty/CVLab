#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

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
#define FRAMES_PER_SECOND 15

#define MAIN_WINDOW "poots"
#define DEBUG_WINDOW "debug"
#define DEBUG_DOWNSCALE 0.5

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
	VideoWriter tracking_recorder;
	ofstream logger;
	static bool mouse_is_dragging;
	static bool mouse_is_moving;
	static bool rectangle_selected;
	static Point initial_click_point, current_mouse_point;

	//routine runtime parameters
	vector<Mat> debug_images;
	vector<char*> debug_image_labels;
	Mat current_transform, previous_transform;
	Mat curr_bgr_frame, curr_gray, prev_gray;
	bool debug;
	bool recording;
	bool running = true;
	bool paused = false;
public:
	static Scalar hsv_min;
	static Scalar hsv_max;

	GenericClassnameOneTracker9000(string filename, string log_name, string output_path, bool extern_debug, bool extern_recording)
	{
		debug = extern_debug;
		recording = extern_recording;
		capture = VideoCapture(filename);
		logger.open(log_name);
		logger << filename << endl;
		Mat tmp;
		capture.read(tmp);
		tracking_recorder.open(output_path, CV_FOURCC('M', 'P', '4', '3'), FRAMES_PER_SECOND, tmp.size());
	}

	~GenericClassnameOneTracker9000()
	{
		tracking_recorder.release();
		logger.close();
	}

	void AddToDebugImages(Mat image_pointer, char* label_pointer)
	{
		Mat temp = image_pointer;
		debug_images.push_back(temp);
		debug_image_labels.push_back(label_pointer);
	}

	void ShowDebugImages()
	{
		if (!debug_images.empty())
		{
			int item_count = debug_images.size();
			int max_images_width = 4;
			int max_images_height = item_count / max_images_width + ((item_count % max_images_width) ? 1 : 0);
			Mat full_image(1, 1, debug_images[0].type());
			int content_height = 0;
			int content_width = 0;
			int counter = 0;

			for (int ypos = 0; ypos < max_images_height && counter < item_count; ypos++)
			{
				for (int xpos = 0; xpos < max_images_width && counter < item_count; xpos++, counter++)
				{
					Mat temp = debug_images[counter].clone();

					putText(temp, debug_image_labels[counter], Point(20, 20), CV_FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));

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
					temp.copyTo(full_image(image_position));
					content_width += image_position.width;
				}
				content_width = 0;
				content_height = full_image.rows;
			}
			resize(full_image, full_image, Size(0, 0), DEBUG_DOWNSCALE, DEBUG_DOWNSCALE);
			imshow(DEBUG_WINDOW, full_image);
			debug_images.clear();
			debug_image_labels.clear();
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


		int64 start, finish;
		start = getTickCount();
		capture.read(curr_bgr_frame);

		if (curr_bgr_frame.empty())
		{
			running = false;
			return; // I DON'T LIKE IT
		}
		Mat curr_hsv_frame;
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
		// translation + rotation only
		if (prev_corner2.size()>0 && cur_corner2.size() > 0)
		{
			current_transform = estimateRigidTransform(prev_corner2, cur_corner2, false); // false = rigid transform, no scaling/shearing
		}
		if (current_transform.rows == 0)
		{
			current_transform = previous_transform.clone();
		}

		///Diff Section
		Mat stabilized, stab_diff;
		warpAffine(prev_gray, stabilized, current_transform, prev_gray.size());
		absdiff(stabilized, curr_gray, stab_diff);
		AddToDebugImages(stab_diff, "stab_diff");

		Mat block(prev_gray.size(), prev_gray.type(), Scalar(255));
		Mat rotated_block = block.clone();
		int THICKNESS = int(sqrt(current_transform.at<double>(0, 2)*current_transform.at<double>(0, 2) + current_transform.at<double>(1, 2)*current_transform.at<double>(1, 2)));
		rectangle(rotated_block, Rect(0, 0, rotated_block.cols , rotated_block.rows), Scalar(0), THICKNESS);
		warpAffine(rotated_block, rotated_block, current_transform, block.size());
		AddToDebugImages(rotated_block, "rotated-block");
		Mat rotational_compensating_mask;
		absdiff(rotated_block, block, rotational_compensating_mask);
		bitwise_not(rotational_compensating_mask, rotational_compensating_mask);
		bitwise_and(rotational_compensating_mask, stab_diff, stab_diff);
		AddToDebugImages(rotational_compensating_mask, "rotational_compensating_mask");


		Mat element = getStructuringElement(MORPH_RECT, Size(BLUR_SIZE, BLUR_SIZE));
		Mat diff_closed;
		morphologyEx(stab_diff, diff_closed, MORPH_CLOSE, element);
		AddToDebugImages(diff_closed, "diff_closed");

		Mat diff_closed_blur;
		blur(diff_closed, diff_closed_blur, Size(BLUR_SIZE, BLUR_SIZE));
		AddToDebugImages(diff_closed_blur, "diff_closed_blur");

		Mat diff_closed_blur_threshold;
		threshold(diff_closed_blur, diff_closed_blur_threshold, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		AddToDebugImages(diff_closed_blur_threshold, "diff_closed_blur_threshold");
		//Diff Section End

		//Color Section
		Mat hsv_in_range;
		inRange(curr_hsv_frame, hsv_min, hsv_max, hsv_in_range);
		AddToDebugImages(hsv_in_range, "hsv_in_range");

		Mat hsv_in_range_closed;
		morphologyEx(hsv_in_range, hsv_in_range_closed, MORPH_CLOSE, element);
		AddToDebugImages(hsv_in_range_closed, "hsv_in_range_closed");

		Mat hsv_in_range_closed_blur;
		blur(hsv_in_range_closed, hsv_in_range_closed_blur, Size(BLUR_SIZE, BLUR_SIZE));
		AddToDebugImages(hsv_in_range_closed_blur, "hsv_in_range_closed_blur");

		Mat hsv_in_range_closed_blur_threshold;
		threshold(hsv_in_range_closed_blur, hsv_in_range_closed_blur_threshold, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		AddToDebugImages(hsv_in_range_closed_blur_threshold, "hsv_in_range_closed_blur_threshold");
		//Color Section End

		//Union Section
		Mat raw_mask;
		bitwise_and(diff_closed_blur_threshold, hsv_in_range, raw_mask);
		AddToDebugImages(raw_mask, "raw_mask");

		Mat threshold_mask;
		threshold(raw_mask, threshold_mask, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		AddToDebugImages(threshold_mask, "threshold_mask");

		Mat threshold_closed_mask;
		morphologyEx(threshold_mask, threshold_closed_mask, MORPH_CLOSE, element);
		AddToDebugImages(threshold_closed_mask, "threshold_closed_mask");

		Mat threshold_closed_blur_mask;
		blur(threshold_closed_mask, threshold_closed_blur_mask, Size(BLUR_SIZE, BLUR_SIZE));
		AddToDebugImages(threshold_closed_blur_mask, "threshold_closed_blur_mask");

		Mat final_mask;
		threshold(threshold_closed_blur_mask, final_mask, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		AddToDebugImages(final_mask, "final_mask");
		//Union Section end

		Rect object_bounding_rectangle;
		Point2d last_position;
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;

		findContours(final_mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);  // retrieves external contours

		for (vector<Point> contour : contours)
		{
			object_bounding_rectangle = boundingRect(contour);
			rectangle(curr_bgr_frame, object_bounding_rectangle, Scalar(0, 150, 0));
		}
		int x_pos = -1;
		int y_pos = -1;

		//Mat object_cutout(curr_gray.size(), curr_gray.type());

		if (contours.size() > 0)//hotfix. find a better solution
		{
			object_bounding_rectangle = boundingRect(contours.back());
			rectangle(curr_bgr_frame, object_bounding_rectangle, Scalar(0, 0, 0));

			/*Mat bg_model, fg_model, object_mask;
			grabCut(curr_bgr_frame, object_mask, object_bounding_rectangle, bg_model, fg_model, 1, GC_INIT_WITH_RECT);
			compare(object_mask, GC_PR_FGD, object_mask, CMP_EQ);
			curr_gray.copyTo(object_cutout, object_mask);
			AddToDebugImages(object_cutout,"object-cutout");*/

			x_pos = object_bounding_rectangle.x + object_bounding_rectangle.width / 2;
			y_pos = object_bounding_rectangle.y + object_bounding_rectangle.height / 2;
			WritePosition(x_pos, y_pos);
		}

		finish = getTickCount();
		double seconds = getTickFrequency() / (finish - start);
		putText(curr_bgr_frame, to_string(seconds), Point(10, 30), CV_FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));

		if (debug)
		{
			ShowDebugImages();
			char* text = new char[10];
			sprintf(text, "x:%d x:%d", x_pos, y_pos);
			putText(curr_bgr_frame, text, object_bounding_rectangle.tl(), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
		}
		else
		{
			cvDestroyWindow(DEBUG_WINDOW);
		}
		if (recording)
		{
			if (tracking_recorder.isOpened())
			{
				tracking_recorder.write(curr_bgr_frame);
			}
			else throw exception("well shit");
			circle(curr_bgr_frame, Point(10, 10), 8, Scalar(0, 0, 255), -1);
		}
		if (mouse_is_dragging)
		{
			rectangle(curr_bgr_frame, initial_click_point, current_mouse_point, Scalar(0, 0, 0));
		}

		imshow(MAIN_WINDOW, curr_bgr_frame);
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
			case 'r':recording = !recording; break;
			case 'R':recording = !recording; break;
			}
		}
		cvDestroyWindow(MAIN_WINDOW);
		if (debug)
		{
			cvDestroyWindow(DEBUG_WINDOW);
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
	optparse.add_option("--outvideo").help("select file for video output");
	optparse.add_option("--debug").action("store_true").help("show debug windows");
	optparse.add_option("--recording").action("store_false").help("record the tracking process (just the main window)");

	Values& options = optparse.parse_args(argc, argv);
	GenericClassnameOneTracker9000 tracker(options["infile"], options["outlog"], options["outvideo"], options.is_set("debug"), options.is_set("recording"));
	tracker.Routine();
}