/******************************************************************************
 *
 * image_proc.h
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
 * author(s):   	Mika Paul Salewski <mika.paul.salewski@gmail.com>
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
 *      This module is used for image processing of a dartboard to detect the
 *      position of the dart tip. Key elements include difference images,
 *      binary edge images, as well as contour and intersection calculations.
******************************************************************************/



#ifndef IMAGE_PROC_H
#define IMAGE_PROC_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>


/*************************** global Defines **********************************/
#define SHOW_NO_IMAGES 0
#define SHOW_ALL_IMAGES 1
#define SHOW_IMG_LINE 2
#define SHOW_EDGE_IMG 3
#define SHOW_EDGE_BIN 4
#define SHOW_SHARP_AFTER_DIFF 5
#define SHOW_SHORT_ANALYSIS -1

#define IMG_DIFFERENCE 1
#define IMG_NO_DIFFERENCE 0 

#define BIN_THRESH 31
#define DIFF_MIN_THRESH 1.6e+5  //1e+6; this worked wo gaussian

#define GAUSSIAN_BLUR_SIGMA 0.75

/* polar coordinates */
struct line_s {

	double r = 0;
	double theta= 0;

};

struct tripple_line_s {
	struct line_s line_top;
	struct line_s line_right;
	struct line_s line_left;
};

/* cartesian coordinates */
struct line_cart_s {
	cv::Point p0;
	cv::Point p1;
};

struct tri_line_cart_s {
	struct line_cart_s top;
	struct line_cart_s right;
	struct line_cart_s left;

};

/************************** Function Declaration *****************************/
extern int img_proc_get_line(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs = 0, std::string CamNameId = "Default");

extern void img_proc_sharpen_img(const cv::Mat& inputImage, cv::Mat& outputImage);


extern void img_proc_get_cross_points(const cv::Mat& image, std::vector<cv::Point>& maxLocations);
extern int img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p);


extern void img_proc_polar_to_cart(const cv::Mat& image, struct line_s l, struct line_cart_s& cart);
extern bool img_proc_find_intersection(const line_cart_s& line1, const line_cart_s& line2, cv::Point& intersection);
extern cv::Point img_proc_calculate_midpoint(cv::Point& p1, cv::Point& p2, cv::Point& p3);
extern int img_proc_cross_point_math(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p);


extern int img_proc_diff_check(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId);
extern int img_proc_diff_check_cal(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId, int* pixel_sum, bool show);


extern void img_proc_calibration(cv::Mat& raw_top, cv::Mat& raw_right, cv::Mat& raw_left, cv::Mat& dart_top, cv::Mat& dart_right, cv::Mat& dart_left);
extern void img_proc_auto_calibration();

extern void on_trackbar_bin_thresh(int thresh, void* arg);
extern void on_trackbar_diff_min_thresh(int thresh, void* arg);
extern void on_trackbar_aspect_ratio_max(int thresh, void* arg);
extern void on_trackbar_aspect_ratio_min(int thresh, void* arg);
extern void on_trackbar_area_min(int thresh, void* arg);
extern void on_trackbar_short_edge_max(int thresh, void* arg);

extern void img_proc_set_bin_thresh(int thresh);
extern void img_proc_set_diff_min_thresh(int thresh);
extern void img_proc_set_aspect_ratio_max(int aspect_ratio_max);
extern void img_proc_set_aspect_ratio_min(int aspect_ratio_min);
extern void img_proc_set_area_min(int area_min);
extern void img_proc_set_short_edge_max(int short_edge_min);




extern int img_proc_get_line_debug(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs = 0, std::string CamNameId = "Default");

#endif 
