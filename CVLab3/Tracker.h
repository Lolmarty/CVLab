#pragma once
namespace Tracking
{
	class Tracker
	{
	private:
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