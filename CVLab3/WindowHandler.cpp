#include "WindowHandler.h"
#include "Settings.h"

namespace Tracking
{
	bool WindowHandler::mouseIsDragging = false;
	bool WindowHandler::mouseIsMoving = false;
	bool WindowHandler::rectangleSelected = false;
	cv::Point WindowHandler::initialMousePoint = cv::Point();
	cv::Point WindowHandler::currentMousePoint = cv::Point();

	WindowHandler::WindowHandler()//TODO: manage to integrate QT into this.
	{
		trackingObserver = nullptr;
		mainWindowName = Settings::Instance().StringGet("MAIN_WINDOW");
		debugWindowName = Settings::Instance().StringGet("DEBUG_WINDOW");
		cv::namedWindow(mainWindowName);
		cv::setMouseCallback(mainWindowName, ClickAndDragRectangle);
	}
	void WindowHandler::ShowMain(cv::Mat image)
	{
		cv::Mat showable = image.clone();
		if (mouseIsDragging) cv::rectangle(showable, initialMousePoint, currentMousePoint, cv::Scalar(0, 0, 0));
		cv::imshow(mainWindowName, showable);
		switch (tolower(cv::waitKey(1)))
		{
		case 'd':trackingObserver->ToggleDebug(); break;
		case 27: trackingObserver->Stop(); break;
		case 'p':trackingObserver->TogglePause(); break;
		case ' ':trackingObserver->TogglePause(); break;
		}
	}
	void WindowHandler::ShowDebug(cv::Mat image)
	{
		cv::imshow(debugWindowName, image);
	}
	void WindowHandler::ClickAndDragRectangle(int event, int x, int y, int flags, void* param)
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
}
