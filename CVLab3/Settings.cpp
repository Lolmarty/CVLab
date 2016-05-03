#include <fstream>
#include <sstream>

#include "Settings.h"
#include "Extensions.h"

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
				std::string exceptionTemplate = "{0}";
				throw new std::exception();
			}
		}
	}
}