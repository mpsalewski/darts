/******************************************************************************
 *
 * HoughLine.h
 * 
 * 
 ******************************************************************************
 * 
 * Original work by: Marc Hensel, http://www.haw-hamburg.de/marc-hensel
 * 
 ******************************************************************************
 * Lecture sample code.
 ******************************************************************************
 * Author: Marc Hensel, http://www.haw-hamburg.de/marc-hensel
 * Project: https://github.com/MarcOnTheMoon/imaging_learners/
 * Copyright: 2023, Marc Hensel
 * Version: 2023.09.08
 * License: CC BY-NC-SA 4.0, see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 * 
 * 
 * 
 * 
 ******************************************************************************
 *
 *		! Modified:
 * 
 ******************************************************************************
 * 
 * Automated Dart Detection and Scoring System
 * 
 * 
 * This project was developed as part of the Digital Image / Video Processing 
 * module at HAW Hamburg under Prof. Dr. Marc Hensel
 * 
 *
 *
 * author(s):   	Mika Paul Salewski <mika.paul.salewski@gmail.com>, 
 *                  
 * created on :     2025-01-06
 * last revision :  None
 *
 *
 *
 * Copyright (c) 2025, Mika Paul Salewski
 * Version: 2025.01.06
 * License: CC BY-NC-SA 4.0,
 *      see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 *
 * 
 * Further information about this source-file:
 *      --> get polar coordinates of dart edges 
******************************************************************************/

#pragma once
#ifndef IP_HOUGH_LINE_H
#define IP_HOUGH_LINE_H

 /* Include files */
#include <opencv2/opencv.hpp>



namespace ip
{

	/* Prototypes */
	void houghTransform(const cv::Mat& edgeImage, cv::Mat& houghSpace, int height = 361, int width = 360);
	void houghSpaceToLine(cv::Size imgSize, cv::Size houghSize, int x, int y, double& r, double& theta);
	void drawLine(cv::Mat& image, double r, double theta);
	
	void drawLine_light_add(cv::Mat& image, double r, double theta);
	
	void drawHoughLineLabels(cv::Mat& houghSpace);
}

#endif /* IP_HOUGH_LINE_H */


