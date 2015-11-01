#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <math.h>

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

using namespace std;
using namespace cv;
using namespace optparse;

#define MAIN_WINDOW_NAME "poots"
#define FRAMES_PER_SECOND 15
#define MAX_VELOCITY 20
#define VELOCITY 10
#define STEP_X 10
#define STEP_Y 10
#define STEP_V 5
#define STEP_A 5
#define SPRITE_BG_WIDTH 5
#define SPRITE_BG_HEIGHT 5
#define SPRITE_ITERATIONS 1

enum VideoSourceMode
{
	ImageMode,
	VideoMode,
	CamMode,
};

class VideoSourceWrapper
{
private:
	Mat image;
	VideoCapture video_capture;
	VideoSourceMode mode;

public:
	VideoSourceWrapper(){}
	
	VideoSourceWrapper(Mat extern_image)
	{
		image = extern_image;
		mode = ImageMode;
	}

	VideoSourceWrapper(VideoCapture extern_capture)
	{
		video_capture = extern_capture;
		mode = VideoMode;
	}

	Mat GetNextFrame()
	{
		Mat frame;
		switch (mode)
		{
		case ImageMode: frame = image.clone(); 
						break;
		case VideoMode: if (!video_capture.isOpened())
						throw exception("well shit");
						video_capture >> frame;
						break;
		}
		return frame;
	}

	int GetEncoding() // quite potentially useless
	{
		video_capture.get(CV_CAP_PROP_FOURCC);
		switch (mode)
		{
		case ImageMode: return CV_FOURCC('M', 'S', 'V', 'C');
		case VideoMode: return video_capture.get(CV_CAP_PROP_FOURCC);
		}
	}

	~VideoSourceWrapper()
	{
		switch (mode)
		{
		case ImageMode: break;
		case VideoMode: video_capture.release(); break;
		}
	}
};

class GenericClassnameOneGenerator9000
{
private:
	VideoSourceWrapper wrapper;
	Mat sprite;
	Mat sprite_mask;
	VideoWriter writer;
	ofstream logger;
	bool debug;

public:

	GenericClassnameOneGenerator9000(VideoSourceWrapper extern_wrapper, string output_path,string log_name, string sprite_name, bool extern_debug)
	{
		debug = extern_debug;
		sprite = imread(sprite_name, CV_LOAD_IMAGE_UNCHANGED);
		Rect sprite_roi_rect(SPRITE_BG_WIDTH, SPRITE_BG_HEIGHT, sprite.cols - SPRITE_BG_WIDTH, sprite.rows - SPRITE_BG_HEIGHT);
		Mat bg_model, fg_model;
		grabCut(sprite, sprite_mask, sprite_roi_rect, bg_model, fg_model, SPRITE_ITERATIONS, GC_INIT_WITH_RECT);
		compare(sprite_mask, GC_PR_FGD, sprite_mask, CMP_EQ);
		imshow(MAIN_WINDOW_NAME, sprite_mask);
		wrapper = extern_wrapper;

		writer.open(output_path, CV_FOURCC('M', 'P', '4', '3'), FRAMES_PER_SECOND, wrapper.GetNextFrame().size());
		//ones that worked CV_FOURCC('M','P','4','3') CV_FOURCC('A','P','4','1') CV_FOURCC('M','S','V','C')
		if (!writer.isOpened()) throw exception("well shit");

		logger.open(log_name);
		logger << output_path << endl;
	}

	void WriteFrame(Mat frame)
	{
		if (writer.isOpened())
		{
			writer.write(frame);
		}
		else throw exception("well shit");
	}

	void WritePosition(int X, int Y)
	{
		if (logger.is_open())
		{
			logger << X << " " << Y << endl;
		}
		else throw exception("well shit");
	}

	Mat PlaceSpriteOnFrameAt(Mat frame, int X, int Y)
	{
		sprite.copyTo(frame.rowRange(X, X + sprite.rows).colRange(Y, Y + sprite.cols), sprite_mask);
		return frame;
	}

	void Routine()
	{
		Mat current_frame = wrapper.GetNextFrame();
		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		minstd_rand gen(seed);
		int x = gen() % (current_frame.rows - sprite.rows);
		int y = gen() % (current_frame.cols - sprite.cols);
		int sign_x = (gen() % 2) ? -1 : 1;
		int sign_y = (gen() % 2) ? -1 : 1;
		while (true)
		{
			current_frame = wrapper.GetNextFrame();
			int dx = gen() % STEP_X;
			int dy = gen() % STEP_Y;
			if (x + sign_x*dx < 0 || x + sign_x*dx + sprite.rows > current_frame.rows) sign_x *= -1;
			if (y + sign_y*dy < 0 || y + sign_y*dy + sprite.cols > current_frame.cols) sign_y *= -1;
			x += sign_x*dx;
			y += sign_y*dy;
			current_frame = PlaceSpriteOnFrameAt(current_frame, x, y);
			imshow(MAIN_WINDOW_NAME, current_frame);
			WriteFrame(current_frame);
			WritePosition(x, y);
			if (waitKey(50) >= 0)
				break;
		}
	}

	Mat Deform(Mat image, double velocity)
	{
		return image;
	}

	static Mat Rotate(Mat image, double angle)
	{
		// get rotation matrix for rotating the image around its center
		Point2f center(image.cols / 2.0, image.rows / 2.0);
		Mat rot = getRotationMatrix2D(center, angle, 1.0);
		// determine bounding rectangle
		Rect bbox = RotatedRect(center, image.size(), angle).boundingRect();
		// adjust transformation matrix
		rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
		rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

		Mat result;
		warpAffine(image, result, rot, bbox.size());
		return result;
	}

	void Routine2()
	{
		Mat current_frame = wrapper.GetNextFrame();
		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		minstd_rand gen(seed);
		double angle = gen() % 360; //in degrees
		double velocity = gen() % VELOCITY;
		int sign_v = (gen() % 2) ? -1 : 1;
		int sign_a = (gen() % 2) ? -1 : 1;
		Mat new_sprite = Rotate(Deform(sprite, velocity), angle);
		int y = gen() % (current_frame.rows - new_sprite.rows);
		int x = gen() % (current_frame.cols - new_sprite.cols);
		bool running = true;
		while (running)
		{
			current_frame = wrapper.GetNextFrame();
			if (current_frame.empty()) break;
			double dv = gen() % STEP_V;
			double da = gen() % STEP_A;
			if (velocity + sign_v*dv <= 0 || velocity + sign_v*dv >= MAX_VELOCITY) sign_v *= -1;
			velocity += sign_v*dv;
			angle += sign_a*da;
			int dx = (int)(velocity*cos(CV_PI*angle / 180));
			int dy = (int)(velocity*sin(CV_PI*angle / 180));
			new_sprite = Rotate(Deform(sprite, velocity), angle);
			if (y + dy <= 0 || y + dy + new_sprite.rows >= current_frame.rows) angle *= -1;
			if (x + dx <= 0) angle = 180 - angle;
			if (x + dx + new_sprite.cols >= current_frame.cols) angle = 180 - angle;
			dx = (int)(velocity*cos(CV_PI*angle / 180));
			dy = (int)(velocity*sin(CV_PI*angle / 180));
			y += dy;
			x += dx;
			new_sprite = Rotate(Deform(sprite, velocity), -angle);
			Mat new_mask = Rotate(Deform(sprite_mask, velocity), -angle);
			new_sprite.copyTo(current_frame.rowRange(y, y + new_sprite.rows).colRange(x, x + new_sprite.cols), new_mask);
			WriteFrame(current_frame);
			int object_center_x = x + new_sprite.cols / 2;
			int object_center_y = y + new_sprite.rows / 2;
			WritePosition(object_center_x, object_center_y);
			if (debug)
			{
				rectangle(current_frame, Rect(x, y, new_sprite.cols, new_sprite.rows), Scalar(0, 0, 0), 1);
				char* text = new char[10];
				sprintf(text, "a:%f v:%f", angle, velocity);
				putText(current_frame, text, Point(x, y), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0, 255), 1);
				line(current_frame, Point(object_center_x, object_center_y), Point(object_center_x + 5, object_center_y + 5), Scalar(0, 0, 0), 2);
			}
			imshow(MAIN_WINDOW_NAME, current_frame);
			switch (waitKey(50))
			{
			case 'd':debug = !debug; break;
			case 'D':debug = !debug; break;
			case 27: running = false; break;
			}
		}
	}

	~GenericClassnameOneGenerator9000()
	{
		cvDestroyWindow(MAIN_WINDOW_NAME);
		writer.release();
		logger.close();
	}
};

template<typename T> struct map_init_helper
{
	T& data;
	map_init_helper(T& d) : data(d) {}
	map_init_helper& operator() (typename T::key_type const& key, typename T::mapped_type const& value)
	{
		data[key] = value;
		return *this;
	}
};

template<typename T> map_init_helper<T> map_init(T& item)
{
	return map_init_helper<T>(item);
}

int main(int argc, char* argv[])
{
	map<const string, VideoSourceMode> bgtype_map;
	map_init(bgtype_map)
		("img", ImageMode)
		("vid", VideoMode)
		("cam", CamMode)
		;
	vector<string> bgtype;
	for (map<const string, VideoSourceMode>::iterator it = bgtype_map.begin(); it != bgtype_map.end(); ++it)
	{
		bgtype.push_back(it->first);
	}
	OptionParser optparse = OptionParser();
	optparse.add_option("--bgtype").choices(bgtype.begin(), bgtype.end()).help("select the type for background");
	optparse.add_option("--bgfile").help("select source for background");
	optparse.add_option("--spfile").help("select source for sprite");
	optparse.add_option("--outvideo").help("select file for video output");
	optparse.add_option("--outlog").help("select file for log output");
	optparse.add_option("--debug").action("store_true").help("show debug box");


	Values& options = optparse.parse_args(argc, argv);

	VideoSourceWrapper wrapper;
	switch (bgtype_map.at(options["bgtype"]))
	{
	case VideoMode:wrapper = VideoSourceWrapper(VideoCapture(options["bgfile"])); break;
	case ImageMode:wrapper = VideoSourceWrapper(imread(options["bgfile"])); break;
	case CamMode:wrapper = VideoSourceWrapper(VideoCapture(0)); break;
	}

	GenericClassnameOneGenerator9000 gc(wrapper, options["outvideo"], options["outlog"], options["spfile"], options.is_set("debug"));
	gc.Routine2();
	return 0;
}