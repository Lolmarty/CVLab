#include <fstream>
#include <sstream>
#include <opencv2/core/core.hpp>

#include "Settings.h"
#include "Extensions.h"
#include "OptionParser.h"

namespace Tracking
{
	std::string Settings::settingsPath = "Settings.txt";

	Settings::Settings()
	{
		std::ifstream settingsStream(settingsPath);
		std::string line;
		while (std::getline(settingsStream, line))
		{
			std::istringstream streamLine(line);
			std::string setting;
			std::getline(streamLine, setting, '=');
			setting = trim(setting);
			std::string type;
			std::string value;
			std::getline(streamLine, type, ':');
			type = trim(type);
			std::getline(streamLine, value);
			value = trim(value);
			if (type == "int") intSettings[setting] = std::stoi(value);
			else if (type == "double") doubleSettings[setting] = std::stod(value);
			else if (type == "string") stringSettings[setting] = value;
			else
			{
				std::string exceptionText = cv::format("%s is an unknown type.", type);
				throw new std::exception(exceptionText.c_str());
			}
		}
	}

	void Settings::Update(int argc, char* argv[])
	{
		optparse::OptionParser optparse = optparse::OptionParser();
		optparse.add_option("--inputfile").help("Select the path to the video source file that will be used for tracking.");
		optparse.add_option("--logfile").help("Select the path to the log file that will be used for log output.");
		optparse.add_option("--debug").action("store_true").help("Include if it is needed to show debug windows."); //TODO: At all/from start?
		optparse::Values& options = optparse.parse_args(argc, argv);
		stringSettings["inputfile"] = options["inputfile"];
		stringSettings["logfile"] = options["logfile"];
		boolSettings["debug"] = options.is_set("debug");
	}
}
