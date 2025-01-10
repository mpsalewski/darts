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



#ifndef GLOBALS_H
#define GLOBALS_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>
#include <atomic>


/*************************** local Defines ***********************************/


/************************** local Structure ***********************************/
/* score, exchange between threads */
struct thread_exchange_s {

    int score = 0;
    int score_flag = 0;
    std::string last_dart_str = "d";

};


/************************* globals Variables *********************************/
extern struct thread_exchange_s t_e;
/* control threads */
extern std::atomic<bool> running;


/************************** Function Declaration *****************************/

#endif 