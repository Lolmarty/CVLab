#include <opencv2/core/core.hpp>
#include "Settings.h"

namespace Tracking
{
	class Debugger
	{
	public:
		static Debugger& Instance()
		{
			static Debugger singletonDebuggerInstance;
			return singletonDebuggerInstance;
		}
		void AddImage(std::string label, cv::Mat image);
		void AddVariable(std::string label, int variable);
		void AddVariable(std::string label, double variable);
		void AddVariable(std::string label, bool variable);
		void Show();
	private:
		//singleton stuff
		Debugger(Debugger const&) = delete;
		void operator=(Debugger const&) = delete;
		std::map<std::string, cv::Mat> debugImages;
		std::map<std::string, std::string> debugVariables;
		int maxImagesWidth;
		Debugger(){ maxImagesWidth = Settings::Instance().IntGet("MAX_IMAGES_WIDTH"); }
	};

}
