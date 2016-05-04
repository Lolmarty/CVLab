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

		inline int IntGet(std::string keyword){ return intSettings[keyword]; }
		inline double DoubleGet(std::string keyword){ return doubleSettings[keyword]; }
		inline std::string StringGet(std::string keyword){ return stringSettings[keyword]; }

		void Update(int argc, char* argv[]);
	private:
		static std::string settingsPath;
		std::map<std::string, std::string> stringSettings;
		std::map<std::string, int> intSettings;
		std::map<std::string, double> doubleSettings;

		Settings();
		~Settings(){}
	};
}
