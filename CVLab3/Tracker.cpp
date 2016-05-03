#include <opencv/highgui.h>

#include "WindowHandler.h"
#include "Tracker.h"

namespace Tracking
{
	Tracker::Tracker()
	{
		WindowHandler::Instance().AttachTracking(this);
		showDebug = true;
		appIsRunning = true;
		paused = false;
	}
	Tracker::~Tracker()
	{
		
	}
	void Tracker::TogglePause()
	{
		paused = !paused;
	}
	void Tracker::ToggleDebug()
	{
		showDebug = !showDebug;
	}
	void Tracker::Stop()
	{
		appIsRunning = false;
	}

	void Tracker::PauseRoutine()
	{

	}

	void Tracker::TrackingRoutine()
	{

	}


	void Tracker::Main()
	{
		while (appIsRunning)
		{
			WindowHandler::Instance().ShowMain(cv::imread("../assets/sky_xl.jpg"));
		}
	};
}
