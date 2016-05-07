#ifndef WINDOW_HANDLER_HEADER
#define WINDOW_HANDLER_HEADER
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

		void ShowMain(cv::Mat image);

		void ShowDebug(cv::Mat image);

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
		static void ClickAndDragRectangle(int event, int x, int y, int flags, void* param);

		~WindowHandler()
		{
			cv::destroyAllWindows();
		}
	};
}
#endif