/******************************************************************************
 *
 * $NAME.cpp
 *
 * Digital Image / Video Processing
 * HAW Hamburg, Prof.Dr.Marc Hensel
 *
 * TEMPLATE
 *
 *
 * author: 			m.salewski
 * created on :
 *last revision :
 *
 *
 *
 *
 *
******************************************************************************/



#ifndef CAMS_H
#define CAMS_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>


/*************************** global Defines **********************************/
/* Timing */
#define FPS 15                      // defines Samplingrate in Cams Thread
#define WAIT_TIME_MS 1000/FPS

/* camera identities */
#define TOP_CAM     2
#define LEFT_CAM    1
#define RIGHT_CAM   3
#define DIFF_THRESH 1e+6


/* size for cross_point calc */
#define RAW_CAL_IMG_WIDTH 640       
#define RAW_CAL_IMG_HEIGHT 480




/* Image Paths; mostly for test and sims */
#define TOP_RAW_IMG_CAL "images/test_img/top/top_raw.jpg"
#define RIGHT_RAW_IMG_CAL "images/test_img/right/right_raw.jpg"
#define LEFT_RAW_IMG_CAL "images/test_img/left/left_raw.jpg"
#define TOP_1DARTS "images/test_img/top/top_1dart.jpg"
#define LEFT_1DARTS "images/test_img/left/left_1darts.jpg"
#define RIGHT_1DARTS "images/test_img/right/right_1darts.jpg"
#define TOP_2DARTS "images/test_img/top/top_2dart.jpg"
#define LEFT_2DARTS "images/test_img/left/left_2darts.jpg"
#define RIGHT_2DARTS "images/test_img/right/right_2darts.jpg"
#define TOP_3DARTS "images/test_img/top/top_3dart.jpg"
#define LEFT_3DARTS "images/test_img/left/left_3darts.jpg"
#define RIGHT_3DARTS "images/test_img/right/right_3darts.jpg"

/************************** local Structure ***********************************/
/***
 * local sub structs
***/
/* flags */
struct flags_s {
    int diff_flag_top = 0;
    int diff_flag_right = 0;
    int diff_flag_left = 0;
    int diff_flag_raw = 0;
};


/* cam structure for data exchange */
struct darts_s {

    /* flags */
    struct flags_s flags;

    /* count throws [0..3] */
    int count_throws = 0;

    /* dart position */
    struct tripple_line_s t_line;
    cv::Point cross_point;

    /* results */
    struct result_s r_top;
    struct result_s r_right;
    struct result_s r_left;
    struct result_s r_final;

};

/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/
void camsThread(void* arg);
void SIMULATION_OF_camsThread(void* arg);


#endif 
