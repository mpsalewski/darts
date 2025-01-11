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

#define BIN_THRESH 45

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
extern int ImgP_newDart(cv::Mat& lastImg, cv::Mat& newImg);
//void diff(cv::Mat& image, cv::Mat& sobel);

extern void findAllMaxima(const cv::Mat& image, std::vector<cv::Point>& maxLocations);
extern int image_proc_get_line(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs = 0, std::string CamNameId = "Default");
extern int img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p);


extern int img_proc_diff_check(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId);
extern int img_proc_diff_check_getback(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId);


#endif 
