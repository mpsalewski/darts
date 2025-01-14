/******************************************************************************
 *
 * $NAME.cpp
 *
 * Digital Image / Video Processing
 * HAW Hamburg, Prof. Dr. Marc Hensel
 *
 * TEMPLATE
 *
 *
 * author: 			m. salewski
 * created on:
 * last revision:
 *
 *
 *
 *
 *
******************************************************************************/



#ifndef CALIBRATION_H
#define CALIBRATION_H


#include <opencv2/opencv.hpp>

extern void calibration_get_img(void);

extern void calibration_get_img(cv::Mat& src, cv::Mat& dst, int ThreadId);


#endif 