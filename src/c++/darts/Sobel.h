/******************************************************************************
 *
 * Sobel.h
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
 *      --> edge filter
******************************************************************************/


#ifndef IP_SOBEL_H
#define IP_SOBEL_H

 /* Include files */
#include <opencv2/opencv.hpp>

namespace ip
{
	void sobelFilter(const cv::Mat& image, cv::Mat& sobel);
}

#endif /* IP_SOBEL_H */


