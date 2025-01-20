/******************************************************************************
 *
 * globals.h
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
 *      --> Global vars and struct for Thread exchange (Datatransfer)
******************************************************************************/


#ifndef GLOBALS_H
#define GLOBALS_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>


/*************************** local Defines ***********************************/


/************************** global Structure *********************************/
struct thread_share_s {
    std::mutex mutex;                       // mutex --> thread safe
    int score = 0;                          // 3 darts score
    int score_flag = 0;                     // recognize new 3 darts score
    std::string last_dart_str = "d";        // give last dart score as string to check double out
};


/************************* globals Variables *********************************/
/* control threads */
extern std::atomic<bool> running;


/************************** Function Declaration *****************************/

#endif 