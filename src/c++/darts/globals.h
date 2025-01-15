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
#include <mutex>
#include <thread>
#include <condition_variable>


/*************************** local Defines ***********************************/


/************************** local Structure ***********************************/
struct thread_share_s {
    std::mutex mutex;                       // mutex --> thread safe
    int score = 0;                          // 3 darts score
    int score_flag = 0;                     // recognize new 3 darts score
    std::string last_dart_str = "d";        // give last dart score as string to check double out
};


#if 0
/* score, exchange between threads */
struct thread_exchange_s {

    int score = 0;
    int score_flag = 0;
    std::string last_dart_str = "d";

};
#endif

/************************* globals Variables *********************************/
//extern struct thread_exchange_s t_e;
/* control threads */
extern std::atomic<bool> running;


/************************** Function Declaration *****************************/

#endif 