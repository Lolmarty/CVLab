#include "WindowHandler.h"
#include "Tracker.h"
#include "Settings.h"

namespace Tracking
{
	Tracker::Tracker()
	{
		WindowHandler::Instance().AttachTracking(this);
		showDebug = Settings::Instance().BoolGet("debug");
		capture = cv::VideoCapture(Settings::Instance().StringGet("inputfile"));
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
		WindowHandler::Instance().ShowMain(currentFrame);
	}
	void Tracker::TrackingRoutine()
	{
		capture.read(currentFrame);
		WindowHandler::Instance().ShowMain(currentFrame);
	}
	void Tracker::Main()
	{
		while (appIsRunning)
		{
			if (paused) PauseRoutine();
			else TrackingRoutine();
		}
	};
}
