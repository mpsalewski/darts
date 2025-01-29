/******************************************************************************
 *
 * main.cpp
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
 * author(s):   	Mika Paul Salewski <mika.paul.salewski@gmail.com>, 
 *                  Lukas Grose <lukas@grose.de>  
 * created on :     2025-01-06
 * last revision :  None
 *
 *
 *
 * Copyright (c) 2025, Mika Paul Salewski, Lukas Grose 
 * Version: 2025.01.06
 * License: CC BY-NC-SA 4.0,
 *      see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 *
 * 
 * Further information about this source-file:
 *      This project is divided into three main components (Threads):
 *          --> Dart Scoreboard
 *          --> Image Processing
 *          --> Command Line Interface (CLI)
 * 
 * for detailed information read submodules description 
 * or see README.md in github:
 *      --> https://github.com/mpsalewski/
******************************************************************************/


/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "image_proc.h"
#include <thread>
#include <atomic>
#include <chrono>
#include "calibration.h"
#include "HoughLine.h"
#include "Sobel.h"
#include "dart_board.h"
#include "globals.h"
#include "cams.h"
#include <cstring>
#include "command_parser.h"


/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
/* main program */
#define THREADING 1                 // use Camera Threads
#define SIMULATION 1                // use Simluation Cams Thread instead of 
                                    // real cams Thread
#define LOAD_STATIC_TEST_IMAGES 0   // use this macro for debugging and test







/************************** local Structure ***********************************/
/* score, exchange between threads */
static struct thread_share_s t_s;      


/************************* local Variables ***********************************/
/* control threads */
atomic<bool> running(true);     // GLOBAL!


/************************** Function Declaration *****************************/
void static_test(void);



/****************************** main function ********************************/
int main() {


    /* always register the command line */
    thread command_line(commandLineThread, &t_s);

    cout << "type 'exit' to quit" << endl;


/* use threads */
#if !SIMULATION && THREADING

    /* create cams thread */
    thread cams(camsThread, &t_s);

    /* create darts gui thread */
    thread guiThread(Dartsboard_GUI_Thread, &t_s);

    this_thread::sleep_for(chrono::milliseconds(2000));

    /* kill all threads through the command line thread */
    while (running == 1) {
        this_thread::sleep_for(chrono::milliseconds(500));
    };

    /* clear threads */
    cams.join();
    guiThread.join();
    
#elif THREADING
    /***
     * run simualtion cams thread
    ***/
    //thread SIM_cams(SIMULATION_OF_camsThread, &darts);
    thread SIM_cams(SIMULATION_OF_camsThread, &t_s);
    
    /* create darts gui thread */
    thread guiThread(Dartsboard_GUI_Thread, &t_s);

    this_thread::sleep_for(chrono::milliseconds(2000));

    /* simulate darts */
    /*
    int fin = 0;
    fin = dart_board_update_scoreboard_gui(&game, 180, "dont care");
    waitKey(3000);
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 1, "dont care");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 76, "dont care");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 450, "dont care");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 500, "Double 250");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    */

    /* wait on enter to quit */
    //std::cout << "Press Enter to quit Threads...\n";
    //cin.get();

    /* kill threads */
    //running = false;
    this_thread::sleep_for(chrono::milliseconds(2000));

    /* kill all threads through the command line thread */
    while (running == 1) {
        this_thread::sleep_for(chrono::milliseconds(500));
    };

    /* clear threads */
    SIM_cams.join();
    guiThread.join();

#endif 


/* Do Static Test with loaded images and fixed points for fast debugging ans test */
#if LOAD_STATIC_TEST_IMAGES

    static_test();

#endif 


    /* clear command line thread */
    command_line.join();

    destroyAllWindows();

    return EXIT_SUCCESS;

}



/************************** Function Definitions *****************************/


/***
 * This is a test debug and tryout and function which can be called in the 
 * main and allows a quick first view on the beheavior of new features
***/
void static_test(void) {


    /* load image from file */
    Mat top_image = cv::imread(TOP_1DARTS, cv::IMREAD_ANYCOLOR);
    Mat top_raw = cv::imread(TOP_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    Mat top_image2 = cv::imread(TOP_2DARTS, cv::IMREAD_ANYCOLOR);
    Mat top_image3 = cv::imread(TOP_3DARTS, cv::IMREAD_ANYCOLOR);

    /* load image from file */
    Mat right_image = cv::imread(RIGHT_1DARTS, cv::IMREAD_ANYCOLOR);
    Mat right_raw = cv::imread(RIGHT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    Mat right_image2 = cv::imread(RIGHT_2DARTS, cv::IMREAD_ANYCOLOR);
    Mat right_image3 = cv::imread(RIGHT_3DARTS, cv::IMREAD_ANYCOLOR);

    /* load image from file */
    Mat left_image = cv::imread(LEFT_1DARTS, cv::IMREAD_ANYCOLOR);
    Mat left_raw = cv::imread(LEFT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    Mat left_image2 = cv::imread(LEFT_2DARTS, cv::IMREAD_ANYCOLOR);
    Mat left_image3 = cv::imread(LEFT_3DARTS, cv::IMREAD_ANYCOLOR);



    //destroyAllWindows();
    struct line_s line;
    struct tripple_line_s t_line;
    line.r = 1;
    line.theta = 99;

    top_raw = cv::imread(TOP_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    right_raw = cv::imread(RIGHT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    left_raw = cv::imread(LEFT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);

#if 1
    img_proc_get_line_debug(top_raw, top_image, TOP_CAM, &t_line.line_top, SHOW_SHORT_ANALYSIS, "Top Static Test");
    waitKey(0);
    img_proc_get_line_debug(right_raw, right_image, RIGHT_CAM, &t_line.line_right, SHOW_SHORT_ANALYSIS, "Right Static Test");
    waitKey(0);
    img_proc_get_line_debug(left_raw, left_image, LEFT_CAM, &t_line.line_left, SHOW_SHORT_ANALYSIS, "Left Static Test");
    waitKey(0);
#endif 
#if 0
    img_proc_get_line(top_image, top_image2, TOP_CAM, &t_line.line_top, SHOW_SHORT_ANALYSIS, "Top Static Test");
    img_proc_get_line(right_image, right_image2, RIGHT_CAM, &t_line.line_right, SHOW_SHORT_ANALYSIS, "Right Static Test");
    img_proc_get_line(left_image, left_image2, LEFT_CAM, &t_line.line_left, SHOW_SHORT_ANALYSIS, "Left Static Test");
#endif
#if 0
    img_proc_get_line(top_image2, top_image3, TOP_CAM, &t_line.line_top, SHOW_IMG_LINE, "Top Static Test");
    img_proc_get_line(right_image2, right_image3, RIGHT_CAM, &t_line.line_right, SHOW_IMG_LINE, "Right Static Test");
    img_proc_get_line(left_image2, left_image3, LEFT_CAM, &t_line.line_left, SHOW_SHORT_ANALYSIS, "Left Static Test");
#endif

    destroyAllWindows();
    Point cross_point;
    calibration_get_img(top_raw, top_raw, TOP_CAM);
    cv::imwrite("top_raw_cal.jpg", top_raw);
    img_proc_cross_point(top_raw.size(), &t_line, cross_point);
    img_proc_cross_point_math(top_raw.size(), &t_line, cross_point);
    cout << "Raw Cal Size" << top_raw.size() << endl;

    /*
    // Beispiel-Daten für eine standardisierte Dartscheibe
    // Beispielpixel (ein Punkt auf der Scheibe)
    //cv::Point2f pixel(390, 139); // Tripple 18
    //cv::Point2f pixel(419, 371); // Single 2
    //cv::Point2f pixel(399, 411); // Double 17
    //cv::Point2f pixel(225, 161); // Tripple 9
    //cv::Point2f pixel(142, 204); // Single 14
    //cv::Point2f pixel(176, 365); // Double 16
    //cv::Point2f pixel(272, 347); // Tripple 19
    //std::string result = determineSector(pixel, board);
    std::string result = determineSector(cross_point, board);

    std::cout << "Wert: " << result << std::endl;
    */
    struct result_s result;
    dart_board_determineSector(cross_point, TOP_CAM, &result);

    std::cout << "Wert: " << result.str << std::endl;

    Point pixel(272, 347); // Tripple 19
    dart_board_determineSector(pixel, TOP_CAM, &result);

    std::cout << "Wert: " << result.str << std::endl;



    cv::waitKey(0);
}


