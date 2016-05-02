#include <exception>
#include <list>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/contrib/contrib.hpp>
#include <opencv/highgui.h>

class WindowHandler
{
public:
	static WindowHandler* Instance()
	{
		if (WindowHandler::singletonWindowHandlerInstance == nullptr) new WindowHandler();
		return singletonWindowHandlerInstance;
	}

	void ShowMain(cv::Mat image)
	{
		
	}

private:
	//singleton stuff
	static WindowHandler * singletonWindowHandlerInstance;
	WindowHandler(WindowHandler const&) = delete;
	void operator=(WindowHandler const&) = delete;

	std::string mainWindowName;
	std::list<std::string> additionalWindowsNameList;

	WindowHandler()
	{
		if (WindowHandler::singletonWindowHandlerInstance != nullptr) std::exception("WindowHandler instance already exists");
		else
		{
			singletonWindowHandlerInstance = this;
			mainWindowName = "skullpoopl";
			additionalWindowsNameList = std::list<std::string> {"pop", "shsh"};
			cv::setMouseCallback(mainWindowName, ClickAndDragRectangle);
			cv::namedWindow(mainWindowName);
			for (std::list<std::string>::iterator iterator = additionalWindowsNameList.begin(); iterator != additionalWindowsNameList.end(); ++iterator)
			{
				cv::namedWindow(*iterator);
			}
		}
	}

	static bool mouseIsDragging;
	static bool mouseIsMoving;
	static bool rectangleSelected;
	static cv::Point initialMousePoint;
	static cv::Point currentMousePoint;
	static void ClickAndDragRectangle(int event, int x, int y, int flags, void* param)
	{
		//cv::Mat* imageDisplayed = (cv::Mat*)param;
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
				/*cv::rectangle(*imageDisplayed, cv::Rect(initialMousePoint, currentMousePoint),cv::Scalar());*/
				/*cv::Mat hsv_feed;
				cvtColor(*imageDisplayed, hsv_feed, CV_BGR2HSV);
				cv::Mat chunk(hsv_feed, cv::Rect(initialMousePoint, currentMousePoint));
				GetHSVBoundaries(chunk);*/
				break;
			}
		}
	}
};

WindowHandler * WindowHandler::singletonWindowHandlerInstance = nullptr;
bool WindowHandler::mouseIsDragging = false;
bool WindowHandler::mouseIsMoving = false;
bool WindowHandler::rectangleSelected = false;
cv::Point WindowHandler::initialMousePoint = cv::Point();
cv::Point WindowHandler::currentMousePoint = cv::Point();