#include "WindowHandler.h"

namespace Tracking
{
	bool WindowHandler::mouseIsDragging = false;
	bool WindowHandler::mouseIsMoving = false;
	bool WindowHandler::rectangleSelected = false;
	cv::Point WindowHandler::initialMousePoint = cv::Point();
	cv::Point WindowHandler::currentMousePoint = cv::Point();
}