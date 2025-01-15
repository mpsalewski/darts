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
 *      TBD
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

#define BIN_THRESH 32
#define DIFF_MIN_THRESH 1.75e+5  //1e+6; this worked wo gaussian

#define GAUSSIAN_BLUR_SIGMA 0.75

struct line_s {

	double r = 0;
	double theta= 0;

};

struct tripple_line_s {
	struct line_s line_top;
	struct line_s line_right;
	struct line_s line_left;
};


/************************** Function Declaration *****************************/
extern void img_proc_sharpen_img(const cv::Mat& inputImage, cv::Mat& outputImage);
extern void img_proc_get_cross_points(const cv::Mat& image, std::vector<cv::Point>& maxLocations);

extern int img_proc_get_line(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs = 0, std::string CamNameId = "Default");
extern int img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p);
extern int img_proc_cross_point_math(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p);
extern int img_proc_diff_check(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId);
extern int img_proc_diff_check_cal(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId, int* pixel_sum, bool show);


extern void img_proc_calibration(cv::Mat& raw_top, cv::Mat& raw_right, cv::Mat& raw_left, cv::Mat& dart_top, cv::Mat& dart_right, cv::Mat& dart_left);
extern void img_proc_auto_calibration();

extern void on_trackbar_bin_thresh(int thresh, void* arg);
extern void on_trackbar_diff_min_thresh(int thresh, void* arg);

extern void img_proc_set_bin_thresh(int thresh);
extern void img_proc_set_diff_min_thresh(int thresh);



extern int img_proc_get_line_debug(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs = 0, std::string CamNameId = "Default");

#endif 
