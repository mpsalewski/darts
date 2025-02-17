/******************************************************************************
 *
 * external_api.cpp
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
 * created on :     2025-13-02
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
 *
 *
******************************************************************************/




/***************************** includes **************************************/
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "image_proc.h"
#include <opencv2/opencv.hpp>
#include "calibration.h"
#include "Sobel.h"
#include "HoughLine.h"
#include "dart_board.h"
#include "cams.h"
#include "globals.h"
#include "command_parser.h"
#include <cstring>
#include <cstdio>
#include <limits>
#include "external_api.h"

/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/


/************************** local Structure ***********************************/
struct flags_s {
    int busted = 0;
};

static struct ext_api_s {
    struct flags_s flags;

    int score_val = 0;
    int score_factor = 0;

}ext_api;

/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/


/**************************** Command Line Thread ****************************/

void external_api_thread(void* arg) {


    /* assign void pointer, thread safe exchange; */
    struct thread_share_s* t_s = (struct thread_share_s*)(arg);

    static struct ext_api_s* ea = &ext_api;


    this_thread::sleep_for(chrono::milliseconds(500));
    /* loop */
    while (running == 1) {

        /* do the event handling */
        /* check here from external api if you are busted and handle it */
        if (false) {
            cams_external_bust();
        }

        /* check if there was a new dart */
        t_s->mutex.lock();
        if (t_s->single_score_flag) {
            
            /* clear flag */
            t_s->single_score_flag = 0;

            /* send here the score to external api */
            //int example1 = t_s->single_score;
            //string example2 = t_s->single_score_str;
            external_api_split_val_fact(t_s->single_score, t_s->single_score_str, ea->score_val, ea->score_factor);
            // cout << ea->score_val << "\t" << ea->score_factor << endl;   // Debug 

        }
        t_s->mutex.unlock();

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    /* kill all threads */
    running = false;

}


void external_api_split_val_fact(int score, std::string score_str, int& val, int&factor) {


    char c = score_str[0];
 
    /* invalid input */
    if (c == '\0')
        return;

    /* out of board */
    if (!score) {
        val = 0;
        factor = 0;
    }



    /* get factor */
    switch (c) {
        case 'S': 
            factor = 1;
            break;

        case 'D':
            factor = 2;
            break;

        case 'T':
            factor = 3;
            break;

        default: break;
    }


    val = score / factor; 


}
