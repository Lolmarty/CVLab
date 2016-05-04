#include <opencv/highgui.h>

#pragma once
namespace Tracking
{
	class Tracker
	{
	private:
		cv::VideoCapture capture;
		cv::Mat currentFrame;
		bool showDebug;
		bool appIsRunning;
		bool paused;
	public:
		Tracker();
		~Tracker();
		void TogglePause();
		void ToggleDebug();
		void Stop();
		void PauseRoutine();
		void TrackingRoutine();
		void Main();
	};
}