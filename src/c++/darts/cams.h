/******************************************************************************
 *
 * cams.h
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
 * Author(s):   	Mika Paul Salewski <mika.paul.salewski@gmail.com>
 *
 * Created on :     2025-01-06
 * Last revision :  None
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
 *      --> control external cameras  
 *      --> call image processing
******************************************************************************/



#ifndef CAMS_H
#define CAMS_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>
#include "ImageProc.h"
#include "dart_board.h"


/*************************** global Defines **********************************/


/* camera identities */
#define TOP_CAM     2
#define LEFT_CAM    1
#define RIGHT_CAM   0
#define DIFF_THRESH 1e+6


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


/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/
void camsThread(void* arg);
void SIMULATION_OF_camsThread(void *arg);


#endif 
