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
	ImageMode,
	DynamicImageMode,
	VideoMode,
	CamMode,
};

//class VideoSourceWrapper
//{
//private:
//	Mat image;
//	VideoCapture video_capture;
//	VideoSourceMode mode;
//
//public:
//	VideoSourceWrapper(){}
//
//	VideoSourceWrapper(Mat extern_image, bool dynamic = false)
//	{
//		image = extern_image;
//		mode = ImageMode;
//	}
//
//	VideoSourceWrapper(VideoCapture extern_capture)
//	{
//		video_capture = extern_capture;
//		mode = VideoMode;
//	}
//
//	Mat GetNextFrame()
//	{
//		Mat frame;
//		switch (mode)
//		{
//		case ImageMode: frame = image.clone();
//			break;
//		case VideoMode: if (!video_capture.isOpened())
//			throw exception("well shit");
//			video_capture >> frame;
//			break;
//		}
//		return frame;
//	}
//	
//	~VideoSourceWrapper()
//	{
//		switch (mode)
//		{
//		case ImageMode: break;
//		case VideoMode: video_capture.release(); break;
//		}
//	}
//};

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
	StaticImageVideo(){}

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
	DynamicImageVideo(){}

	DynamicImageVideo(Mat extern_image)
	{
		image = extern_image.clone();
		float diag = 0.75 * min(image.rows, image.cols);
		frame_side = diag / sqrt(2.0);
		center_domain = Rect((int)diag / 2 + 1, (int)diag / 2 + 1, image.cols - (int)diag - 1, image.rows - (int)diag - 1);
		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		gen = minstd_rand(seed);
		angle = gen() % 360; //in degrees
		velocity = gen() % VELOCITY;
		sign_v = (gen() % 2) ? -1 : 1;
		sign_a = (gen() % 2) ? -1 : 1;
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
		double dv = gen() % STEP_V;
		double da = gen() % STEP_A;
		if (velocity + sign_v*dv <= 0 || velocity + sign_v*dv >= MAX_VELOCITY) sign_v *= -1;
		velocity += sign_v*dv;
		angle += sign_a*da;
		int dx = (int)(velocity*cos(CV_PI*angle / 180));
		int dy = (int)(velocity*sin(CV_PI*angle / 180));
		if (center_y + dy <= center_domain.y || center_y + dy >= center_domain.height + center_domain.y) angle *= -1;
		if (center_x + dx <= center_domain.x || center_x + dx >= center_domain.width + center_domain.x) angle = 180 - angle;
		dx = (int)(velocity*cos(CV_PI*angle / 180));
		dy = (int)(velocity*sin(CV_PI*angle / 180));
		center_y += dy;
		center_x += dx;

		Mat M, rotated, cropped;
		Size rect_size = Size(frame_side, frame_side);
		// thanks to http://felix.abecassis.me/2011/10/opencv-rotation-deskewing/
		if (angle < -45.) {
			angle += 90.0;
			swap(rect_size.width, rect_size.height);
		}
		// get the rotation matrix
		M = getRotationMatrix2D(Point2f(center_x, center_y), angle, 1.0);
		// perform the affine transformation
		warpAffine(image, rotated, M, image.size(), INTER_CUBIC);
		// crop the resulting image
		getRectSubPix(rotated, rect_size, Point2f(center_x, center_y), cropped);
		return cropped.clone();
	}

	~DynamicImageVideo()
	{
		image.release();
	}
};

class VideoVideo : VideoInterface
{
	VideoCapture video_capture;
public:
	VideoVideo(){}

	VideoVideo(VideoCapture extern_capture)
	{
		video_capture = extern_capture;
	}

	Mat GetNextFrame()
	{
		Mat frame;
		if (!video_capture.isOpened())
			throw exception("well shit");
		video_capture >> frame;
		return frame;
	};

	~VideoVideo()
	{
		video_capture.release();
	}
};


class VideoSourceWrapper
{
	StaticImageVideo static_image_video;
	DynamicImageVideo dynamic_image_video;
	VideoVideo video_video;
	VideoSourceMode video_source_mode;
public:
	VideoSourceWrapper(){}

	VideoSourceWrapper(Mat extern_image, bool dynamic = false)
	{
		if (dynamic)
		{
			video_source_mode = DynamicImageMode;
			dynamic_image_video = DynamicImageVideo(extern_image);
		}
		else
		{
			video_source_mode = ImageMode;
			static_image_video = StaticImageVideo(extern_image);
		}
	}

	VideoSourceWrapper(VideoCapture extern_capture)
	{
		video_source_mode = VideoMode;
		video_video = VideoVideo(extern_capture);
	}

	Mat GetNextFrame()
	{
		switch (video_source_mode)
		{
		case VideoMode: return video_video.GetNextFrame();
		case ImageMode: return static_image_video.GetNextFrame();
		case DynamicImageMode: return dynamic_image_video.GetNextFrame();
		default: return Mat();
		}
	}
};