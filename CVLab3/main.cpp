#include "Tracker.h"
#include "Settings.h"

int main(int argc, char* argv[])
{
	Tracking::Settings::Instance().Update(argc, argv);
	Tracking::Tracker tracker;
	tracker.Main();
}
