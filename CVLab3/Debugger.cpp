#include "Debugger.h"
namespace Tracking
{
	void Debugger::AddImage(std::string label, cv::Mat image)
	{
		debugImages[label] = image.clone();
	}
	void Debugger::AddVariable(std::string label, int variable)
	{
		debugVariables[label] = std::to_string(variable);
	}
	void Debugger::AddVariable(std::string label, double variable)
	{
		debugVariables[label] = std::to_string(variable);
	}
	void Debugger::AddVariable(std::string label, bool variable)
	{
		debugVariables[label] = std::to_string(variable);
	}
	void Debugger::Show()
	{
		
	}
}