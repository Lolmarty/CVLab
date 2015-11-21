#include <math.h>
#include <chrono>
#include <random>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/video/background_segm.hpp>

#include "Definitions.cpp"

using namespace std;
using namespace cv;

enum VideoSourceMode
{
	StaticImageMode,
	DynamicImageMode,
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

	VideoSourceWrapper(Mat extern_image, bool dynamic = false)
	{
		image = extern_image;
		mode = dynamic ? DynamicImageMode : StaticImageMode;
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
		case StaticImageMode: frame = image.clone();
			break;
		case VideoMode: if (!video_capture.isOpened())
			throw exception("well shit");
			video_capture >> frame;
			break;
		}
		return frame;
	}
	
	~VideoSourceWrapper()
	{
		switch (mode)
		{
		case StaticImageMode: break;
		case VideoMode: video_capture.release(); break;
		}
	}
};

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

class VideoInterface
{
public:
	Mat GetNextFrame();
};

class StaticImageVideo : VideoInterface
{
	Mat image;
public:
	StaticImageVideo(Mat extern_image)
	{
		image = extern_image.clone();
	}

	Mat GetNextFrame()
	{
		return image.clone();
	};

	~StaticImageVideo()
	{
		image.release();
	}
};

class DynamicImageVideo :VideoInterface
{
	Mat image;
	Rect center_domain;
	int frame_side;
	minstd_rand gen;
	int center_x, center_y;
	double angle, velocity;
	int sign_a, sign_v;
public:
	DynamicImageVideo(Mat extern_image)
	{
		image = extern_image.clone();
		frame_side = min(image.rows, image.cols) / sqrt(2.0);
		center_domain = Rect((int)frame_side / 2 + 1, (int)frame_side / 2 + 1, image.cols - (int)frame_side - 1, image.rows - (int)frame_side - 1);
		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		gen = minstd_rand(seed);
		double angle = gen() % 360; //in degrees
		double velocity = gen() % VELOCITY;
		int sign_v = (gen() % 2) ? -1 : 1;
		int sign_a = (gen() % 2) ? -1 : 1;
		center_x = gen() % center_domain.width + center_domain.x;
		center_y = gen() % center_domain.height + center_domain.y;
		
		//Debug stuff
		Mat temp = image.clone();
		rectangle(temp, center_domain, Scalar(0, 0, 0));
		arrowedLine(temp, 
			Point(center_x, center_y), 
			Point(center_x + (int)(velocity*cos(CV_PI*angle / 180)), 
				center_y + (int)(velocity*sin(CV_PI*angle / 180))), 
			Scalar(0, 0, 0));
		imshow(MAIN_WINDOW_NAME, temp);
		waitKey();
	}

	Mat GetNextFrame()
	{
		return image.clone();
	}
};