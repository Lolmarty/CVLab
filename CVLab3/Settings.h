#include <string>
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

		std::string StrGet(std::string keyword){ return stringSettings[keyword]; }
	private:
		static std::string settingsPath;
		std::map<std::string, std::string> stringSettings;
		std::map<std::string, int> intSettings;
		std::map<std::string, double> doubleSettings;

		Settings();
		~Settings()
		{

		}
	};
}
