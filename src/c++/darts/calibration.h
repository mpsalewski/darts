/******************************************************************************
 *
 * calibration.c
 *
 *
 * Automated Dart Detection and Scoring System
 *
 *
 * This project was developed as part of the Digital Image / Video Processing
 * module at HAW Hamburg under Prof. Dr. Marc Hensel
 *
 *
 *
 * Author(s):   	Lukas Grose <lukas@grose.de>
 *                  Mika Paul Salewski <mika.paul.salewski@gmail.com>
 *
 * Created on :     2025-01-06
 * Last revision :  None
 *
 *
 *
 * Copyright (c) 2025, Lukas Grose, Mika Paul Salewski
 * Version: 2025.01.06
 * License: CC BY-NC-SA 4.0,
 *      see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 *
 *
 * Further information about this source-file:
 *      --> warp perspective of 3 external cameras to get a consistent view
******************************************************************************/


#ifndef CALIBRATION_H
#define CALIBRATION_H


#include <opencv2/opencv.hpp>

/************************** Function Declaration *****************************/

extern void calibration_get_img(void);

extern void calibration_get_img(cv::Mat& src, cv::Mat& dst, int ThreadId);


#endif 