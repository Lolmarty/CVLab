/*
Copyright (c) 2011-2014, Mathieu Labbe - IntRoLab - Universite de Sherbrooke
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the Universite de Sherbrooke nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>



// OpenCV stuff
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp> // for homography

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/contrib/contrib.hpp>

#ifdef HAVE_OPENCV_NONFREE
#if CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION >=4
#include <opencv2/nonfree/gpu.hpp>
#include <opencv2/nonfree/features2d.hpp>
#endif
#endif
#ifdef HAVE_OPENCV_XFEATURES2D
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/cuda.hpp>
#endif


class transmorgifier{
	cv::VideoCapture capture;
	cv::Ptr<cv::FeatureDetector> detector;
	cv::Ptr<cv::DescriptorExtractor> extractor;
	std::vector<cv::KeyPoint> objectKeypoints;
	std::vector<cv::KeyPoint> sceneKeypoints;
	cv::Mat objectImage;
	cv::Mat objectDescriptors;
	cv::Mat sceneDescriptors;
	std::vector<cv::Point2f> objectCorners;
public:
	transmorgifier(char* file)
	{
		capture = cv::VideoCapture(file);

		// The detector can be any of (see OpenCV features2d.hpp):
		// detector = cv::Ptr<cv::FeatureDetector>(new cv::DenseFeatureDetector());
		// detector = cv::Ptr<cv::FeatureDetector>(new cv::FastFeatureDetector());
		//detector = cv::Ptr<cv::FeatureDetector>(new cv::GFTTDetector());
		// detector = cv::Ptr<cv::FeatureDetector>(new cv::MSER());
		// detector = cv::Ptr<cv::FeatureDetector>(new cv::ORB());
		detector = cv::Ptr<cv::FeatureDetector>(new cv::SIFT());
		// detector = cv::Ptr<cv::FeatureDetector>(new cv::StarFeatureDetector());
		//detector = cv::Ptr<cv::FeatureDetector>(new cv::SURF(600.0));
		 //detector = cv::Ptr<cv::FeatureDetector>(new cv::BRISK());

		// The extractor can be any of (see OpenCV features2d.hpp):
		// extractor = cv::Ptr<cv::DescriptorExtractor>(new cv::BriefDescriptorExtractor());
		// extractor = cv::Ptr<cv::DescriptorExtractor>(new cv::ORB());
		extractor = cv::Ptr<cv::DescriptorExtractor>(new cv::SIFT());
		//extractor = cv::Ptr<cv::DescriptorExtractor>(new cv::SURF(600.0));
		// extractor = cv::Ptr<cv::DescriptorExtractor>(new cv::BRISK());
		// extractor = cv::Ptr<cv::DescriptorExtractor>(new cv::FREAK());
		objectCorners = std::vector<cv::Point2f>(4);
	}

	void getInitialObject()
	{
		cv::Mat frame;
		capture.read(frame);
		cv::Rect roi(270, 160, 106, 60);
		cv::cvtColor(frame(roi), objectImage, CV_BGR2GRAY);
		detector->detect(objectImage, objectKeypoints);
		extractor->compute(objectImage, objectKeypoints, objectDescriptors);

		objectCorners[0] = cv::Point2f(0.0, 0.0);
		objectCorners[1] = cv::Point2f(objectImage.cols, 0.0);
		objectCorners[2] = cv::Point2f(objectImage.cols, objectImage.rows);
		objectCorners[3] = cv::Point2f(0.0, objectImage.rows);
	}

	int main()
	{
		getInitialObject();

		cv::Mat sceneImg;
		cv::Mat sceneImgColor;
		capture.read(sceneImgColor);

		do{
			cv::cvtColor(sceneImgColor, sceneImg, CV_BGR2GRAY);

			if (!sceneImg.empty())
			{
				////////////////////////////
				// EXTRACT KEYPOINTS
				////////////////////////////

				detector->detect(sceneImg, sceneKeypoints);

				////////////////////////////
				// EXTRACT DESCRIPTORS
				////////////////////////////

				extractor->compute(sceneImg, sceneKeypoints, sceneDescriptors);

				////////////////////////////
				// NEAREST NEIGHBOR MATCHING USING FLANN LIBRARY (included in OpenCV)
				////////////////////////////
				cv::Mat results;
				cv::Mat dists;
				std::vector<std::vector<cv::DMatch> > matches;
				int k = 2; // find the 2 nearest neighbors
				bool useBFMatcher = false; // SET TO TRUE TO USE BRUTE FORCE MATCHER
				if (objectDescriptors.type() == CV_8U&&sceneDescriptors.type() == CV_8U)
				{
					// Binary descriptors detected (from ORB, Brief, BRISK, FREAK)
					printf("Binary descriptors detected...\n");
					if (useBFMatcher)
					{
						cv::BFMatcher matcher(cv::NORM_HAMMING); // use cv::NORM_HAMMING2 for ORB descriptor with WTA_K == 3 or 4 (see ORB constructor)
						matcher.knnMatch(objectDescriptors, sceneDescriptors, matches, k);
					}
					else
					{
						// Create Flann LSH index
						cv::flann::Index flannIndex(sceneDescriptors, cv::flann::LshIndexParams(12, 20, 2), cvflann::FLANN_DIST_HAMMING);

						// search (nearest neighbor)
						flannIndex.knnSearch(objectDescriptors, results, dists, k, cv::flann::SearchParams());
					}
				}
				else
				{
					// assume it is CV_32F
					printf("Float descriptors detected...\n");
					if (useBFMatcher)
					{
						cv::BFMatcher matcher(cv::NORM_L2);
						matcher.knnMatch(objectDescriptors, sceneDescriptors, matches, k);
					}
					else
					{
						// Create Flann KDTree index
						cv::flann::Index flannIndex(sceneDescriptors, cv::flann::KDTreeIndexParams(), cvflann::FLANN_DIST_EUCLIDEAN);

						// search (nearest neighbor)
						flannIndex.knnSearch(objectDescriptors, results, dists, k, cv::flann::SearchParams());
					}
				}

				// Conversion to CV_32F if needed
				if (dists.type() == CV_32S)
				{
					cv::Mat temp;
					dists.convertTo(temp, CV_32F);
					dists = temp;
				}



				// Find correspondences by NNDR (Nearest Neighbor Distance Ratio)
				float nndrRatio = 0.8f;
				std::vector<cv::Point2f> mpts_1, mpts_2; // Used for homography
				std::vector<int> indexes_1, indexes_2; // Used for homography
				std::vector<uchar> outlier_mask;  // Used for homography
				// Check if this descriptor matches with those of the objects
				if (!useBFMatcher)
				{
					for (int i = 0; i < objectDescriptors.rows; ++i)
					{
						// Apply NNDR
						//printf("q=%d dist1=%f dist2=%f\n", i, dists.at<float>(i,0), dists.at<float>(i,1));
						if (results.at<int>(i, 0) >= 0 && results.at<int>(i, 1) >= 0 &&
							dists.at<float>(i, 0) <= nndrRatio * dists.at<float>(i, 1))
						{
							mpts_1.push_back(objectKeypoints.at(i).pt);
							indexes_1.push_back(i);

							mpts_2.push_back(sceneKeypoints.at(results.at<int>(i, 0)).pt);
							indexes_2.push_back(results.at<int>(i, 0));
						}
					}
				}
				else
				{
					for (unsigned int i = 0; i < matches.size(); ++i)
					{
						// Apply NNDR
						//printf("q=%d dist1=%f dist2=%f\n", matches.at(i).at(0).queryIdx, matches.at(i).at(0).distance, matches.at(i).at(1).distance);
						if (matches.at(i).size() == 2 &&
							matches.at(i).at(0).distance <= nndrRatio * matches.at(i).at(1).distance)
						{
							mpts_1.push_back(objectKeypoints.at(matches.at(i).at(0).queryIdx).pt);
							indexes_1.push_back(matches.at(i).at(0).queryIdx);

							mpts_2.push_back(sceneKeypoints.at(matches.at(i).at(0).trainIdx).pt);
							indexes_2.push_back(matches.at(i).at(0).trainIdx);
						}
					}
				}

				cv::Mat display;

				cv::drawKeypoints(sceneImg, sceneKeypoints, display, cv::Scalar(0, 0, 128));
				cv::drawKeypoints(display, objectKeypoints, display, cv::Scalar(0, 64, 196));
				
				// FIND HOMOGRAPHY
				unsigned int minInliers = 8;
				if (mpts_1.size() >= minInliers)
				{
					cv::Mat H = findHomography(mpts_1,
						mpts_2,
						cv::RANSAC,
						1.0,
						outlier_mask);
					int inliers = 0, outliers = 0;
					for (unsigned int k = 0; k < mpts_1.size(); ++k)
					{
						if (outlier_mask.at(k))
						{
							++inliers;
						}
						else
						{
							++outliers;
						}
					}
					std::vector<cv::Point2f> sceneCorners(4);

					cv::perspectiveTransform(objectCorners, sceneCorners, H);

					cv::line(display, sceneCorners[0], sceneCorners[1], cv::Scalar(0, 255, 0), 4);
					cv::line(display, sceneCorners[1], sceneCorners[2], cv::Scalar(0, 255, 0), 4);
					cv::line(display, sceneCorners[2], sceneCorners[3], cv::Scalar(0, 255, 0), 4);
					cv::line(display, sceneCorners[3], sceneCorners[0], cv::Scalar(0, 255, 0), 4);

					std::vector<cv::KeyPoint> warpedObjPoints;
					perspectiveTransformKeypoints(objectKeypoints, warpedObjPoints, H);
					cv::drawKeypoints(display, warpedObjPoints, display, cv::Scalar(0, 196, 64));
				}

				
				cv::imshow("moo", display);
				cv::waitKey(10);
			}
			else
			{
				printf("Images are not valid!\n");
			}
		} while (capture.read(sceneImgColor));


		return 1;
	}


	void perspectiveTransformKeypoints(std::vector<cv::KeyPoint> keypointsSrc, std::vector<cv::KeyPoint>& keypointsDst, cv::Mat Homography)
	{
		for (cv::KeyPoint keyPoint : keypointsSrc)
		{
			cv::Mat_<double> initialPoint(3, 1);
			initialPoint(0, 0) = keyPoint.pt.x;
			initialPoint(1, 0) = keyPoint.pt.y;
			initialPoint(2, 0) = 1;
			cv::Mat_<double> transformedPoint = Homography*initialPoint;
			cv::Point2f point(transformedPoint(0, 0), transformedPoint(1, 0));
			keypointsDst.push_back(cv::KeyPoint(point, keyPoint.size, keyPoint.angle, keyPoint.response, keyPoint.octave, keyPoint.class_id));
		}
	}



};



int main()
{
	transmorgifier("../assets/planes-cut.mp4").main();
}







