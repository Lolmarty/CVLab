#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv/highgui.h>

#include "Observer.h"
#include "Tracker.h"

namespace Tracking
{		
	class WindowHandler		//implement observer with features of the object
	{
	public:
		static WindowHandler& Instance()
		{
			static WindowHandler singletonWindowHandlerInstance;
			return singletonWindowHandlerInstance;
		}

		void ShowMain(class cv::Mat image)
		{
			if (mouseIsDragging) cv::rectangle(image, initialMousePoint, currentMousePoint, cv::Scalar(0,0,0));
			cv::imshow(mainWindowName, image);
			switch (tolower(cv::waitKey(1)))
			{
			case 'd':trackingObserver->ToggleDebug(); break;
			case 27: trackingObserver->Stop(); break;
			case 'p':trackingObserver->TogglePause(); break;
			case ' ':trackingObserver->TogglePause(); break;
			//case 'r':recording = !recording; break;
			}
		}

		void AttachTracking(Tracker* tracker)
		{
			trackingObserver = tracker;
		}

	private:
		//singleton stuff
		WindowHandler(WindowHandler const&) = delete;
		void operator=(WindowHandler const&) = delete;
		//observer-subject stuff
		Tracker* trackingObserver;
		std::vector<Observer*> featureObservers;	//TODO: implement IMaskGenerator with this

		std::string mainWindowName;
		std::string debugWindowName;

		WindowHandler();
		
		//mouse handling
		static bool mouseIsDragging;
		static bool mouseIsMoving;
		static bool rectangleSelected;
		static cv::Point initialMousePoint;
		static cv::Point currentMousePoint;
		static void ClickAndDragRectangle(int event, int x, int y, int flags, void* param)
		{
			if (!mouseIsDragging && event == CV_EVENT_LBUTTONDOWN)
			{
				initialMousePoint = cv::Point(x, y);
				mouseIsDragging = true;
				rectangleSelected = false;
			}
			if (mouseIsDragging){
				switch (event)
				{
				case CV_EVENT_MOUSEMOVE:
					currentMousePoint = cv::Point(x, y);
					mouseIsMoving = true;
					break;
				case CV_EVENT_LBUTTONUP:
					mouseIsDragging = false;
					mouseIsMoving = false;
					rectangleSelected = true;
					break;
				}
			}
		}

		~WindowHandler()
		{
			cv::destroyAllWindows();
		}
	};
}