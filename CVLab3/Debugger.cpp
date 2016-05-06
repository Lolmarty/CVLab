#include <opencv2/core/core_c.h>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Debugger.h"
#include "Settings.h"
#include "WindowHandler.h"

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
		std::map<std::string, cv::Mat>::iterator iterator = debugImages.begin();
		int itemCount = debugImages.size();
		int maxImagesHeight = itemCount / maxImagesWidth + ((itemCount % maxImagesWidth) ? 1 : 0);
		int contentHeight = 0;
		int contentWidth = 0;
		cv::Mat fullImage(1, 1, CV_8UC3);

		for (int yPosition = 0; yPosition < maxImagesHeight && iterator != debugImages.end(); yPosition++)
		{
			for (int xPosition = 0; xPosition < maxImagesWidth && iterator != debugImages.end(); xPosition++, ++iterator)
			{
				cv::Mat temp = iterator->second.clone();
				if (temp.channels() < 3) cv::cvtColor(temp, temp, CV_GRAY2BGR);
				cv::putText(temp, iterator->first, cv::Point(20, 20), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

				cv::Rect imagePlacementRect(contentWidth, contentHeight, temp.cols, temp.rows);
				cv::Rect fullImageRect(0, 0, fullImage.cols, fullImage.rows);
				bool isInside = (fullImageRect&imagePlacementRect) == imagePlacementRect;
				if (!isInside)
				{
					cv::Mat tempFullImage((fullImage.rows >(imagePlacementRect.y + imagePlacementRect.height)) ? fullImage.rows : imagePlacementRect.y + imagePlacementRect.height,
						(fullImage.cols > (imagePlacementRect.x + imagePlacementRect.width)) ? fullImage.cols : imagePlacementRect.x + imagePlacementRect.width, fullImage.type());
					fullImage.copyTo(tempFullImage(fullImageRect));
					fullImage = tempFullImage.clone();
				}
				temp.copyTo(fullImage(imagePlacementRect));
				contentWidth = fullImage.cols;
			}
			contentWidth = 0;
			contentHeight = fullImage.rows;
		}
		WindowHandler::Instance().ShowDebug(fullImage);
	}
}
