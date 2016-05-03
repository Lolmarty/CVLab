#include "WindowHandler.h"
#include "Settings.h"

namespace Tracking
{
	bool WindowHandler::mouseIsDragging = false;
	bool WindowHandler::mouseIsMoving = false;
	bool WindowHandler::rectangleSelected = false;
	cv::Point WindowHandler::initialMousePoint = cv::Point();
	cv::Point WindowHandler::currentMousePoint = cv::Point();

	WindowHandler::WindowHandler()
	{
		trackingObserver = nullptr;
		mainWindowName = Settings::Instance().StrGet("MAIN_WINDOW");
		debugWindowName = Settings::Instance().StrGet("DEBUG_WINDOW");
		cv::namedWindow(mainWindowName);
		cv::namedWindow(debugWindowName);
		cv::setMouseCallback(mainWindowName, ClickAndDragRectangle);
	}
}
