#include <opencv/highgui.h>

#include "WindowHandler.h"

namespace Tracking
{
	class Tracker
	{
	public:
		Tracker(){};
		~Tracker(){};
		void Main()
		{
			while (true)
			{
				WindowHandler::Instance().ShowMain(cv::imread("../assets/sky_xl.jpg"));
				if (cv::waitKey(10)=='p') break;
			}
		};
	};
}