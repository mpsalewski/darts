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


/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "ImageProc.h"
#include <opencv2/opencv.hpp>
#include "calibration.h"
#include "Sobel.h"
#include "HoughLine.h"
#include "dart_board.h"
#include "globals.h"
#include "cams.h"

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
/* do threshold calibration at program start */
#define CALIBRATION 0       // 1 = on; 0 = off

/* Timing */
#define FPS 15                      // defines Samplingrate in Cams Thread
#define WAIT_TIME_MS 1000/FPS

/* size for cross_point calc */
#define RAW_CAL_IMG_WIDTH 640       
#define RAW_CAL_IMG_HEIGHT 480

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

static struct darts_s darts;

/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/


/******************************* CAM THREADS **********************************/
/***
 * 
 * camsThread(void* arg) 
 *
 * About this function ...
 *
 * 
 * @param:	void* arg
 *          No parameters are required for this function
 *
 * 
 * @return: void
 *
 * 
 * @note:	None
 * 
 * 
 * Example usage: None 
 *
***/
void camsThread(void* arg) {

    /* assign void pointer */
    struct darts_s* xp = (struct darts_s*)(arg);

    /* cur = curent frames; last = last frames */
    Mat cur_frame_top, cur_frame_right, cur_frame_left;
    Mat last_frame_top, last_frame_right, last_frame_left;
    /* raw empty init board (just top view) */
    Mat raw_empty_init_frame;
    /* calm cams down in beginning, check diff with these frames */
    Mat f_top, f_right, f_left;

    /* open top camera */
    VideoCapture top_cam(TOP_CAM, CAP_ANY);
    if (!top_cam.isOpened()) {
        std::cout << "[ERROR] cannot open TOP Camera" << endl;
        return;
    }
    /* open right camera */
    VideoCapture right_cam(RIGHT_CAM, CAP_ANY);
    if (!right_cam.isOpened()) {
        std::cout << "[ERROR] cannot open RIGHT Camera" << endl;
        return;
    }
    /* open left camera */
    VideoCapture left_cam(LEFT_CAM, CAP_ANY);
    if (!left_cam.isOpened()) {
        std::cout << "[ERROR] cannot open LEFT Camera" << endl;
        return;
    }

    /* create camera windows */
    ostringstream CamName1;
    CamName1 << "Top Cam [press Esc to quit]";
    string top_cam_win = CamName1.str();
    ostringstream CamName2;
    CamName2 << "Right Cam [press Esc to quit]";
    string right_cam_win = CamName2.str();
    ostringstream CamName3;
    CamName3 << "Left Cam [press Esc to quit]";
    string left_cam_win = CamName3.str();

    /* short delay */
    this_thread::sleep_for(chrono::milliseconds(2000));

#if CALIBRATION
    /* calibration */
    /* init last frames */
    top_cam >> last_frame_top;
    right_cam >> last_frame_right;
    left_cam >> last_frame_left;
    if (last_frame_top.empty() || last_frame_right.empty() || last_frame_left.empty()) {
        std::cout << "Error: empty init frame 1\n" << last_frame_top.empty() << last_frame_right.empty() << last_frame_left.empty() << endl;
        return;
    }
    /* show frames */
    imshow(top_cam_win, last_frame_top);
    imshow(right_cam_win, last_frame_right);
    imshow(left_cam_win, last_frame_left);
    /**/
    std::cout << "throw a dart in the board --> then press 'c' to calibrate thresholds" << endl;
    while ((waitKey(10) != 'c') && running) {
        top_cam >> cur_frame_top;
        right_cam >> cur_frame_right;
        left_cam >> cur_frame_left;
        /* show frames */
        imshow(top_cam_win, cur_frame_top);
        imshow(right_cam_win, cur_frame_right);
        imshow(left_cam_win, cur_frame_left);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    img_proc_calibration(last_frame_top, last_frame_right, last_frame_left, cur_frame_top, cur_frame_right, cur_frame_left);
    std::cout << "calibration done" << endl;
#endif

    this_thread::sleep_for(chrono::milliseconds(500));

    /* init last frames */
    top_cam >> last_frame_top;
    right_cam >> last_frame_right;
    left_cam >> last_frame_left;
    if (last_frame_top.empty() || last_frame_right.empty() || last_frame_left.empty()) {
        std::cout << "Error: empty init frame 1\n" << last_frame_top.empty() << last_frame_right.empty() << last_frame_left.empty() << endl;
        return;
    }

    this_thread::sleep_for(chrono::milliseconds(500));


    /* init last frames */
    top_cam >> last_frame_top;
    right_cam >> last_frame_right;
    left_cam >> last_frame_left;
    if (last_frame_top.empty() || last_frame_right.empty() || last_frame_left.empty()) {
        std::cout << "Error: empty init frame 2\n" << last_frame_top.empty() << last_frame_right.empty() << last_frame_left.empty() << endl;
        return;
    }
    /* init frame */
    raw_empty_init_frame = last_frame_top.clone();


    /* loop */
    while (running == 1) {

        /* quit on [Esc] */
        if (cv::waitKey(10) == 27) {
            break;
        }

        /* get current frames from cams */
        top_cam >> cur_frame_top;
        right_cam >> cur_frame_right;
        left_cam >> cur_frame_left;

        /* check if frames are not empty */
        if (!cur_frame_top.empty() && !cur_frame_right.empty() && !cur_frame_left.empty()) {

            /* show frames */
            imshow(top_cam_win, cur_frame_top);
            imshow(right_cam_win, cur_frame_right);
            imshow(left_cam_win, cur_frame_left);

            /* check íf there are any differences */
            xp->flags.diff_flag_top = img_proc_diff_check(last_frame_top, cur_frame_top, TOP_CAM);
            xp->flags.diff_flag_right = img_proc_diff_check(last_frame_right, cur_frame_right, RIGHT_CAM);
            xp->flags.diff_flag_left = img_proc_diff_check(last_frame_left, cur_frame_left, LEFT_CAM);

            /* check detected difference && expecting throws (count_throws < 3) */
            if ((xp->flags.diff_flag_top || xp->flags.diff_flag_right || xp->flags.diff_flag_left) && (xp->count_throws < 3)) {
                /* clear flags */
                xp->flags.diff_flag_top = 0;
                xp->flags.diff_flag_top = 0;
                xp->flags.diff_flag_top = 0;

                /* count throws */
                xp->count_throws++;

                /* short delay to be sure dart is in board and was not on the fly */
                this_thread::sleep_for(chrono::milliseconds(300));

                /* get even newer frames, with darts which are definetly in the board */
                top_cam >> cur_frame_top;
                right_cam >> cur_frame_right;
                left_cam >> cur_frame_left;

                /* get líne polar coordinates */
                image_proc_get_line(last_frame_top, cur_frame_top, TOP_CAM, &xp->t_line.line_top, SHOW_EDGE_BIN , "Top"); //SHOW_SHORT_ANALYSIS
                image_proc_get_line(last_frame_right, cur_frame_right, RIGHT_CAM, &xp->t_line.line_right, SHOW_EDGE_BIN, "Right");
                image_proc_get_line(last_frame_left, cur_frame_left, LEFT_CAM, &xp->t_line.line_left, SHOW_EDGE_BIN, "Left");

                /* calculate cross point */
                img_proc_cross_point(Size(RAW_CAL_IMG_WIDTH, RAW_CAL_IMG_HEIGHT), &xp->t_line, xp->cross_point);

                /* check result on every raw board */
                dart_board_determineSector(xp->cross_point, TOP_CAM, &xp->r_top);
                dart_board_determineSector(xp->cross_point, RIGHT_CAM, &xp->r_right);
                dart_board_determineSector(xp->cross_point, LEFT_CAM, &xp->r_left);

                /* democratic result */
                dart_board_decide_sector(&xp->r_top, &xp->r_right, &xp->r_left, &xp->r_final);

                std::cout << "Dart is (String): " << xp->r_final.str << std::endl;
                std::cout << "Dart is (int Val): " << xp->r_final.val << std::endl;

                /* accumulate 3-dart score */
                t_e.score += xp->r_final.val;
                t_e.last_dart_str = xp->r_final.str;
            }
            /* 3 throws detected --> wait for Darts removed from Board */
            else if (xp->count_throws == 3) {

                /* recognize 3 darts */
                t_e.score_flag = 1;


                top_cam >> cur_frame_top;
                last_frame_top = cur_frame_top.clone();
                /* wait till darts board is back to raw and empty */
                xp->flags.diff_flag_raw = img_proc_diff_check(raw_empty_init_frame, cur_frame_top, TOP_CAM);
                //xp->flags.diff_flag_raw = img_proc_diff_check(last_frame_top, cur_frame_top, TOP_CAM);


                while (xp->flags.diff_flag_raw && (running == 1) && !(cv::waitKey(10) == 27) && running) {
                //while (!xp->flags.diff_flag_raw && !(cv::waitKey(10) == 27) && running) {
#if 0
                    /* get current frames from cams */
                    top_cam >> cur_frame_top;

                    xp->flags.diff_flag_raw = img_proc_diff_check(last_frame_top, cur_frame_top, TOP_CAM);
                    if (xp->flags.diff_flag_raw) {
                        break;
                    }
                    last_frame_top = cur_frame_top.clone();
                    this_thread::sleep_for(chrono::milliseconds(500));
#else 
                    /* get current frames from cams */
                    top_cam >> cur_frame_top;
                    xp->flags.diff_flag_raw = img_proc_diff_check(raw_empty_init_frame, cur_frame_top, TOP_CAM);
                    if (xp->flags.diff_flag_raw == IMG_NO_DIFFERENCE) {
                        break;
                    }
                    this_thread::sleep_for(chrono::milliseconds(WAIT_TIME_MS));
#endif 
                    
                    
                }



                /* init frame */
                raw_empty_init_frame = cur_frame_top.clone();

                /* removing throws */
                xp->count_throws = 0;

                /* wait for player left board */
                std::cout << "removing darts ..." << endl;
                this_thread::sleep_for(chrono::milliseconds(2000));
                std::cout << "ready ..." << endl;

                top_cam >> cur_frame_top;
                right_cam >> cur_frame_right;
                left_cam >> cur_frame_left;

            }

            /* update last frame */
            last_frame_top = cur_frame_top.clone();
            last_frame_right = cur_frame_right.clone();
            last_frame_left = cur_frame_left.clone();

        }
        else {
            /* recognized empty frame; short delay and try again */
            this_thread::sleep_for(chrono::milliseconds(250));
        }


        //this_thread::sleep_for(chrono::milliseconds(WAIT_TIME_MS));

    }

    /* thread finished */
    std::cout << "Cams Thread Finished\n";

    /* free resoruces */
    top_cam.release();
    right_cam.release();
    left_cam.release();

}



/***
 *
 * SIMULATION_OF_camsThread(void* arg)
 *
 * About this function ...
 *
 *
 * @param:	void* arg
 *          No parameters are required for this function
 *
 *
 * @return: void
 *
 *
 * @note:	None
 *
 *
 * Example usage: None
 *
***/
void SIMULATION_OF_camsThread(void) {

    /* assign void pointer */
    //struct darts_s* xp = (struct darts_s*)arg;
    struct darts_s* xp = &darts;

    Mat cur_frame_top, cur_frame_right, cur_frame_left;
    Mat last_frame_top, last_frame_right, last_frame_left;

    /* raw empty init board (just top view) */
    Mat raw_empty_init_frame;
#if 0
    /* open top camera */
    VideoCapture top_cam(TOP_CAM);
    if (!top_cam.isOpened()) {
        std::cout << "[ERROR] cannot open TOP Camera" << endl;
        return;
    }
    /* open right camera */
    VideoCapture right_cam(RIGHT_CAM);
    if (!right_cam.isOpened()) {
        std::cout << "[ERROR] cannot open RIGHT Camera" << endl;
        return;
    }
    /* open left camera */
    VideoCapture left_cam(LEFT_CAM);
    if (!left_cam.isOpened()) {
        std::cout << "[ERROR] cannot open LEFT Camera" << endl;
        return;
    }
#endif 
    /* create camera windows */
    ostringstream CamName1;
    CamName1 << "Top Cam [press Esc to quit]";
    string top_cam_win = CamName1.str();
    ostringstream CamName2;
    CamName2 << "Right Cam [press Esc to quit]";
    string right_cam_win = CamName2.str();
    ostringstream CamName3;
    CamName3 << "Left Cam [press Esc to quit]";
    string left_cam_win = CamName3.str();

    /* short delay */
    this_thread::sleep_for(chrono::milliseconds(500));

#if CALIBRATION
    /* calibration sim */
    last_frame_top = imread(TOP_2DARTS, IMREAD_ANYCOLOR);
    last_frame_right = imread(RIGHT_2DARTS, IMREAD_ANYCOLOR);
    last_frame_left = imread(LEFT_2DARTS, IMREAD_ANYCOLOR);
    /* show frames */
    imshow(top_cam_win, last_frame_top);
    imshow(right_cam_win, last_frame_right);
    imshow(left_cam_win, last_frame_left);
    /**/
    std::cout << "throw a dart in the board --> then press 'c' to calibrate thresholds" << endl;
    while ((waitKey(10) != 'c') && running) {
        cur_frame_top = imread(TOP_3DARTS, IMREAD_ANYCOLOR);
        cur_frame_right = imread(RIGHT_3DARTS, IMREAD_ANYCOLOR);
        cur_frame_left = imread(LEFT_3DARTS, IMREAD_ANYCOLOR);
        /* show frames */
        imshow(top_cam_win, cur_frame_top);
        imshow(right_cam_win, cur_frame_right);
        imshow(left_cam_win, cur_frame_left);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    //cur_frame_top = imread(TOP_3DARTS, IMREAD_ANYCOLOR);
    //cur_frame_right = imread(RIGHT_3DARTS, IMREAD_ANYCOLOR);
    //cur_frame_left = imread(LEFT_3DARTS, IMREAD_ANYCOLOR);
    img_proc_calibration(last_frame_top,last_frame_right,last_frame_left,cur_frame_top,cur_frame_right,cur_frame_left);
#endif

    /* init last frames */
    //top_cam >> last_frame_top;
    //right_cam >> last_frame_right;
    //left_cam >> last_frame_left;
    last_frame_top = imread(TOP_2DARTS, IMREAD_ANYCOLOR);
    last_frame_right = imread(RIGHT_2DARTS, IMREAD_ANYCOLOR);
    last_frame_left = imread(LEFT_2DARTS, IMREAD_ANYCOLOR);

    if (last_frame_top.empty() || last_frame_right.empty() || last_frame_left.empty()) {
        std::cout << "Error: empty init frame\n" << endl;
        return;
    }


    /* init frame */
    raw_empty_init_frame = last_frame_top.clone();


    /* loop */
    int once = 1;
    while ((running == 1) && once) {

        /* simulation --> close running after 1 loop */
        once = 0;

        /* quit on [Esc] */
        if (cv::waitKey(10) == 27) {
            break;
        }

        /* get current frames from cams */
        //top_cam >> cur_frame_top;
        //right_cam >> cur_frame_right;
        //left_cam >> cur_frame_left;
        cur_frame_top = imread(TOP_3DARTS, IMREAD_ANYCOLOR);
        cur_frame_right = imread(RIGHT_3DARTS, IMREAD_ANYCOLOR);
        cur_frame_left = imread(LEFT_3DARTS, IMREAD_ANYCOLOR);

        /* check if frames are not empty */
        if (!cur_frame_top.empty() && !cur_frame_right.empty() && !cur_frame_left.empty()) {

            /* show frames */
            imshow(top_cam_win, cur_frame_top);
            imshow(right_cam_win, cur_frame_right);
            imshow(left_cam_win, cur_frame_left);

            /* check íf there are any differences */
            xp->flags.diff_flag_top = img_proc_diff_check(last_frame_top, cur_frame_top, TOP_CAM);
            xp->flags.diff_flag_right = img_proc_diff_check(last_frame_right, cur_frame_right, RIGHT_CAM);
            xp->flags.diff_flag_left = img_proc_diff_check(last_frame_left, cur_frame_left, LEFT_CAM);

            /* check detected difference */
            if (xp->flags.diff_flag_top || xp->flags.diff_flag_right || xp->flags.diff_flag_left) {
                /* clear flags */
                xp->flags.diff_flag_top = 0;
                xp->flags.diff_flag_top = 0;
                xp->flags.diff_flag_top = 0;

                /* short delay to be sure dart is in board and was not on the fly */
                this_thread::sleep_for(chrono::milliseconds(20));

                /* get even newer frames, with darts which are definetly in the board */
                //top_cam >> cur_frame_top;
                //right_cam >> cur_frame_right;
                //left_cam >> cur_frame_left;

                /* get líne polar coordinates */
                image_proc_get_line(last_frame_top, cur_frame_top, TOP_CAM, &xp->t_line.line_top, SHOW_SHORT_ANALYSIS, "Top");
                xp->t_line.line_top.r = 0.000001;
                xp->t_line.line_top.theta = 2.3402;//180.00001/180.0* 3.14159265359;
                cout << xp->t_line.line_top.theta << endl;
                image_proc_get_line(last_frame_right, cur_frame_right, RIGHT_CAM, &xp->t_line.line_right, SHOW_SHORT_ANALYSIS, "Right");
                xp->t_line.line_right.r = 0;
                xp->t_line.line_right.theta = 2.3402;
                image_proc_get_line(last_frame_left, cur_frame_left, LEFT_CAM, &xp->t_line.line_left, SHOW_SHORT_ANALYSIS, "Left");

                /* calculate cross point */
                img_proc_cross_point(Size(RAW_CAL_IMG_WIDTH, RAW_CAL_IMG_HEIGHT), &xp->t_line, xp->cross_point);

                /* check result on every raw board */
                dart_board_determineSector(xp->cross_point, TOP_CAM, &xp->r_top);
                dart_board_determineSector(xp->cross_point, RIGHT_CAM, &xp->r_right);
                dart_board_determineSector(xp->cross_point, LEFT_CAM, &xp->r_left);

                /* democratic result */
                dart_board_decide_sector(&xp->r_top, &xp->r_right, &xp->r_left, &xp->r_final);

                std::cout << "Dart is (String): " << xp->r_final.str << std::endl;
                std::cout << "Dart is (int Val): " << xp->r_final.val << std::endl;

                /* accumulate 3-dart score */
                t_e.score += xp->r_final.val;
                t_e.last_dart_str = xp->r_final.str;

                /* THIS IS WRONG ! JUST FOR TESTING ! recognize 3 darts */
                //t_e.score = 501;
                //t_e.last_dart_str = "Double 250.5";
                t_e.score_flag = 1;

            }
            /* 3 throws detected --> wait for Darts removed from Board */
            else if (xp->count_throws == 3) {

                /* recognize 3 darts */
                t_e.score_flag = 1;

                /* wait till darts board is back to raw and empty */
                xp->flags.diff_flag_raw = img_proc_diff_check(raw_empty_init_frame, cur_frame_top, TOP_CAM);

                while (xp->flags.diff_flag_raw && (running == 1) && !(cv::waitKey(10) == 27) && running) {
                    /* get current frames from cams */
                    //top_cam >> cur_frame_top;
                    xp->flags.diff_flag_raw = img_proc_diff_check(raw_empty_init_frame, cur_frame_top, TOP_CAM);
                    if (xp->flags.diff_flag_raw == IMG_NO_DIFFERENCE) {
                        break;
                    }
                    this_thread::sleep_for(chrono::milliseconds(WAIT_TIME_MS));
                }

                /* removing throws */
                xp->count_throws = 0;


            }

            /* update last frame */
            last_frame_top = cur_frame_top.clone();
            last_frame_right = cur_frame_right.clone();
            last_frame_left = cur_frame_left.clone();

        }
        else {
            /* recognized empty frame; short delay and try again */
            this_thread::sleep_for(chrono::milliseconds(250));
        }

        this_thread::sleep_for(chrono::milliseconds(WAIT_TIME_MS));

    }

    /* thread finished */
    std::cout << "Cams Thread Finished; press [Esc] to finish thread and proceed program\n!!! waitKey(0); !!!\n";

    while (cv::waitKey(10) != 27 && running) {
        this_thread::sleep_for(chrono::milliseconds(250));
    }

    /* free resoruces */
    //top_cam.release();
    //right_cam.release();
    //left_cam.release();

}






/************************** Function Definitions *****************************/