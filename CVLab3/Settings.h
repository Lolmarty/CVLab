#ifndef SETTINGS_HEADER
#define SETTINGS_HEADER
#include <map>

namespace Tracking
{
	class Settings
	{
	public:
		static Settings&  Instance()
		{
			static Settings singletonSettingsInstance;
			return singletonSettingsInstance;
		}

		int IntGet(std::string keyword){ return intSettings[keyword]; }
		double DoubleGet(std::string keyword){ return doubleSettings[keyword]; }
		std::string StringGet(std::string keyword){ return stringSettings[keyword]; }
		bool BoolGet(std::string keyword){ return boolSettings[keyword]; }

		void Update(int argc, char* argv[]);
	private:
		//singleton stuff
		Settings(Settings const&) = delete;
		void operator=(Settings const&) = delete;
		static std::string settingsPath;
		std::map<std::string, std::string> stringSettings;
		std::map<std::string, int> intSettings;
		std::map<std::string, double> doubleSettings;
		std::map<std::string, bool> boolSettings;

		Settings();
		~Settings(){}
	};
}
#endif