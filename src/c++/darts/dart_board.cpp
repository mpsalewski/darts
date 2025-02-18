/******************************************************************************
 *
 * dart_board.cpp
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
 *      --> Darts Scoreboard ("GUI")
 *      --> Handle Darts Game (Logic)
 *          --> supported features:
 *              --> different number of players and names
 *              --> Tracking Score, Sets, Legs, Last 3-Dart-Throw and
 *                  who's next turn it is
 *              --> required double checkout and busted score check
 * 
 * 
 *      --> Connect discrete Pixel with Dartsscore (get Value from Pixel)
******************************************************************************/



/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "image_proc.h"
#include <opencv2/opencv.hpp>
#include "calibration.h"
#include "Sobel.h"
#include "HoughLine.h"
#include "dart_board.h"
#include "globals.h"
#include "cams.h"
#include <mutex>


/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;


/*************************** local Defines ***********************************/
#define DARTBOARD_SECTORS 40
#define DOUBLE_CHECKOUT 1



/************************** local Structure ***********************************/
/* player */
struct player_s {
    std::string p_name = "user";
    int score = 501;
    int last_throw = 0;
    int leg = 0;
    int set = 0;
};

/* game */
struct game_s {
    int num_p;
    std::vector<player_s> p;
};

/* GUI */ 
/* create gui struct */
struct darts_gui_s {

    /* creat empty gui frame */
    Mat gui;

    int row_offset = 150;                       // dist to top
    int name_col = 50;                          // name column
    int name_width = 150;                       // name column width
    int set_col = name_col + name_width;        // set column
    int set_width = 100;                        // set column width
    int leg_col = set_col + set_width;          // leg column
    int leg_width = 100;                        // leg column width
    int score_col = leg_col + leg_width;        // score column
    int score_width = 150;                      // score column width
    int last_throw_col = score_col + score_width;   // last throw column
    int last_throw_width = 150;                     // last throw column width 
    int text_w = 0;
    int text_pos = 0;
    int h = 0;
    int w = 0;
    int dot_col = last_throw_col + last_throw_width;    // red dot column
    int dot_width = 100;                                // red dot column width
    int count_games = -1;
    int count_player = 0;

    string name_win = "default";

};

/* dart board */
struct Dartboard_Radius_s {
    int radiusBullseye = 0;
    int radiusSingleBull = 0;
    int radiusTripleInner = 0;
    int radiusTripleOuter = 0;
    int radiusDoubleInner = 0;
    int radiusDoubleOuter = 0;

};

struct Dartboard_Sector_s {
    cv::Point center = cv::Point(0, 0);
    struct Dartboard_Radius_s Db_r = {};
    int sectorNumbers[DARTBOARD_SECTORS] = {};

};




/* summarize all structs into one */
struct dart_board_s {

    /* thread safe */
    mutex mtx;

    /* game */
    struct game_s* g = NULL;

    /* gui */
    struct darts_gui_s* dg = NULL;

    /* dart board */
    struct Dartboard_Sector_s *db = NULL;

};




/*****************************************************************************/
/***
 * summarize all structs into one struct
***/
static struct dart_board_s dart_board = {
    {},
   NULL,
   NULL,
   NULL,
};
/* create ptr for access and compatibility */
static struct dart_board_s* d = &dart_board;

/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/


/******************************* GUI THREAD **********************************/
/***
 *
 * Dartsboard_GUI_Thread(void)
 *
 * Hanles the Scorebboard GUI like updates, finished, new games etc.
 *
 *
 * @param:	void* arg --> called with thread_exchange_s struct for 
 *          Datatransfer
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
void Dartsboard_GUI_Thread(void*arg) {

    /* assign void pointer, thread safe exchange */
    struct thread_share_s* t_s = (struct thread_share_s*)(arg);

    int fin = 0;
    int key = 0;

    /* init struct */
    dart_board_init();

    /* init gui */
    dart_board_create_scoreboard_gui();
    // example2: dart_board_create_scoreboard_gui(d->g, "Board2", 900,900);
    //std::cout << "press [Enter] to start the game" << endl;
    while (waitKey(10) != 13 && running) {
        this_thread::sleep_for(chrono::milliseconds(50));
    }

    while (running == 1) {


        /* quit on [Esc] */
        if (cv::waitKey(10) == 27) {
            break;
        }

        /* thread safe */
        t_s->mutex.lock();

        if (t_s->score_flag) {
            /* reset flag */
            t_s->score_flag = 0;

            /* update scoreboard and check for finish */
            fin = dart_board_update_scoreboard_gui(t_s->score, t_s->last_dart_str);
            t_s->score = 0;
            t_s->mutex.unlock();
            if (fin > 0) {
                dart_board_finish_scoreboard_gui(fin - 1);
                d->mtx.lock();
                std::cout << "finished by: " << d->g->p[fin - 1].p_name << endl;
                d->mtx.unlock();
                /* next game ? yes --> [Enter]; no --> [q] */
                while ((key != 13) && (key != 113) && running) {
                    key = waitKey(10);
                    this_thread::sleep_for(chrono::milliseconds(50));
                }
                if (key == 13) {
                    std::cout << "pressed [Enter] --> next leg" << endl;
                    dart_board_create_scoreboard_gui();
                }
                else if(key == 113) {
                    std::cout << "pressed [q] --> quit and exit thread" << endl;
                    break;
                }
                else {
                    /* running == 0 --> thread kill */
                    //std::cout << "" << endl;
                    break;
                }
            }

            this_thread::sleep_for(chrono::milliseconds(50));
        }
        else{
            t_s->mutex.unlock();
        }
        

        this_thread::sleep_for(chrono::milliseconds(50));
    }

}



/************************** Function Definitions *****************************/
/***
 *
 * dart_board_init(void)
 *
 * Initialize Dartboard GUI and Hardware-Setup (Radius)
 *
 *
 * @param:	void
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
void dart_board_init(void) {

    d->mtx.lock();

    /***
     * create game
    ***/
    /* players */
    static vector<player_s> players = {
        {"Lukas", 501, 0, 0, 0},
        {"Mika", 501, 0, 0, 0},
        {"Marek", 501, 0, 0, 0},
    };
    /* game */
    static struct game_s game = {
        (int)players.size(),     // number of players
        players,            // players
    };
    /* create ptr for access and compatibility */
    //static struct game_s* d->g = &game;

    /***
     * create gui
    ***/
    static struct darts_gui_s darts_gui;
    /* create ptr for access and compatibility */
    //static struct darts_gui_s* d->dg = &darts_gui;


    /***
     * create DartBoard
    ***/
    static Point Dartboard_Center(320, 240);

    static struct Dartboard_Radius_s Dartboard_Rad_top = {
         10, 22, 117, 129, 187, 200
    };

    static struct Dartboard_Sector_s Db_sec_top = {
        Dartboard_Center,
        Dartboard_Rad_top,
        { 20, 1,1 , 18, 18, 4,4, 13,13, 6,6, 10,10, 15,15, 2,2, 17,17, 3,3, 19,19, 7,7, 16,16, 8,8,11,11,14,14,9,9,12,12,5,5,20 }
    };

    /* create ptr for access and compatibility */
    //static struct Dartboard_Sector_s* db = &Db_sec_top;


    d->g = &game;
    d->dg = &darts_gui;
    d->db = &Db_sec_top;

    d->mtx.unlock();

}




/******************************************************************************
 * Sector and Score calculations
******************************************************************************/
/***
 *
 * dart_board_determineSector(const cv::Point& pixel, int ThreadId, struct result_s* r)
 *
 * Compute the Darts result by given Pixel
 *
 *
 * @param:	const cv::Point& pixel --> Pixel
 * @param:  int ThreadId --> camera perspective
 * @param:  struct result_s* r --> Dartsscore
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
void dart_board_determineSector(const cv::Point& pixel, int ThreadId, struct result_s* r) {

    /* thread safe */
    d->mtx.lock();


    struct Dartboard_Sector_s board;
    
    switch (ThreadId) {

        case TOP_CAM:
            board = *d->db;
            break;

        case RIGHT_CAM:
            board = *d->db;
            break;

        case LEFT_CAM:
            board = *d->db;
            break;

        default:    r->str = "error";
                    r->val = 0;
                    break;
    }

    /* distance to center */
    float dx = pixel.x - board.center.x;
    if (dx == 0)
        dx += 1;
    float dy = pixel.y - board.center.y;
    if (dy == 0)
        dy += 1;
    float distance = std::sqrt(dx * dx + dy * dy);
    float angle = 0;
    int corner_factor = 0;

    if (dx > 0) {
        /* upper right corner */
        if (dy < 0) {
            angle = std::atan2(dx, dy) * 180.0 / CV_PI;
            angle = abs(angle - 180);
            corner_factor = 0;
        }
        /* lower right corner */
        else if (dy > 0) {
            angle = std::atan2(dy, dx) * 180.0 / CV_PI;
            angle = abs(angle) + 90;
            corner_factor = 1;
        }
        else {
            dart_board_getSectorValue(5 + 1, distance, board, r);
        }
    }
    else if (dx < 0) {
        /* upper left corner */
        if (dy < 0) {
            angle = std::atan2(dx, dy) * 180.0 / CV_PI;
            angle = abs(angle - 180);
            corner_factor = 2;
        }
        /* lower left corner */
        else if (dy > 0) {
            angle = std::atan2(dy, dx) * 180.0 / CV_PI;
            angle = abs(angle) + 90;
            corner_factor = 3;
        }
        else {
            dart_board_getSectorValue(15 + 1, distance, board, r);
        }
    }



    /* define sector; 360/40 =9 --> angle/9 */
    int sector = static_cast<int>(angle / 9) + 1;
    //cout << sector << endl;

    /* sector to value */
    dart_board_getSectorValue(sector, distance, board, r);

    d->mtx.unlock();
}



/***
 *
 * dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board, struct result_s* r)
 *
 * Compute correct result by given sector and distance
 *
 *
 * @param:	int sector --> Sector
 * @param:  float distance --> Distance to Center (Bullseye Center)
 * @param:  struct Dartboard_Sector_s& board --> given Dartboard Params
 *          (NOTE: at the moment there is only one board)
 * @param:  struct result_s* r --> Dartsscore
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
void dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board, struct result_s* r) {
    
    /* define value */
    if (distance <= board.Db_r.radiusBullseye) {
        r->val = 50;
        r->str = "Double Single Bull";  // change with Bullseye bc of Double Out Check by String
    }
    else if (distance <= board.Db_r.radiusSingleBull) {
        r->val = 25;
        r->str = "Single Bull";
    }
    else if (distance <= board.Db_r.radiusTripleInner) {
        r->val = 1 * board.sectorNumbers[sector - 1];
        r->str = "Single " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else if (distance <= board.Db_r.radiusTripleOuter) {
        r->val = 3 * board.sectorNumbers[sector - 1];
        r->str = "Triple " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else if (distance <= board.Db_r.radiusDoubleInner) {
        r->val = 1 * board.sectorNumbers[sector - 1];
        r->str = "Single " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else if (distance <= board.Db_r.radiusDoubleOuter) {
        r->val = 2 * board.sectorNumbers[sector - 1];
        r->str = "Double " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else {
        r->val = 0;
        r->str = "Out of Board";
    }
}




/***
 *
 * dart_board_decide_sector(struct result_s* sec_board_top, struct result_s* sec_board_right, struct result_s* sec_board_left, struct result_s* r)
 *
 * Do the final sector decision based on multiple raw board sector results
 * NOT SUPPORTED at the moment 
 *
 *
 * @param:	struct result_s* sec_board_top      --> Top Board
 * @param:  struct result_s* sec_board_right    --> Right Board 
 * @param:  struct result_s* sec_board_left     --> Left Board
 * @param:  struct result_s* r                  --> final result 
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
void dart_board_decide_sector(struct result_s* sec_board_top, struct result_s* sec_board_right, struct result_s* sec_board_left, struct result_s* r) {

    /* thread safe */
    d->mtx.lock();
#if 1 

    /* all three are equal */
    if ((sec_board_top->str == sec_board_right->str)&& (sec_board_top->str == sec_board_left->str)) {
        *r = *sec_board_top;
    }
    /* top and right are equal */
    else if (sec_board_top->str == sec_board_right->str) {
        *r = *sec_board_top;
    }
    /* top and left are equal */
    else if (sec_board_top->str == sec_board_left->str) {
        *r = *sec_board_top;
    }
    /* left and right are equal */
    else if (sec_board_left->str == sec_board_right->str) {
        *r = *sec_board_left;
    }
    /* none of them are equal */
    else {
        printf("Warning: Three Different Sectors detected; return Top Detection as Default\n");
        *r = *sec_board_top;
    }

    /* old, at the moment usage is not supported due to new result structure */
#else 

    /* all three are equal */
    if ( (!strcmp(sec_board_top.c_str(), sec_board_right.c_str())) && (!strcmp(sec_board_top.c_str(), sec_board_left.c_str()))) {
        return sec_board_top;
    }
    /* top and right are equal */
    else if ((!strcmp(sec_board_top.c_str(), sec_board_right.c_str()))) {
        return sec_board_top;
    }
    /* top and left are equal */
    else if ((!strcmp(sec_board_top.c_str(), sec_board_left.c_str()))) {
        return sec_board_top;
    }
    /* left and right are equal */
    else if ((!strcmp(sec_board_left.c_str(), sec_board_right.c_str()))) {
        return sec_board_left;
    }
    /* none of them are equal */
    else {
        printf("Warning: Three Different Sectors detected; return Top Detection as Default\n");
        return sec_board_top;
    }

#endif 

    d->mtx.unlock();

}

/* draw darts sectors */
void dart_board_draw_sectors(cv::Mat& image, int ThreadId, int image_show, int cal, int show_num) {

    string CamNameId = "TOP";
    struct Dartboard_Sector_s board;
    Mat cur = image.clone();
    /* calibrate images */
    if (cal) {
        calibration_get_img(cur, cur, ThreadId);
    }

    switch (ThreadId) {

        case TOP_CAM:
            CamNameId = "TOP";
            board = *d->db;
            break;

        case RIGHT_CAM:
            CamNameId = "RIGHT";
            board = *d->db;
            break;

        case LEFT_CAM:
            CamNameId = "LEFT";
            board = *d->db;
            break;

        default:    
            return;
            break;
    }

    /* center dart board */
    Point center = board.center;
    int radius = board.Db_r.radiusDoubleOuter;  // Äußerer Radius des Dartboards

    /* define colors */
    Scalar lineColor(255, 255, 255);  
    Scalar textColor(255, 255, 255); 
    Scalar circleColor(255, 255, 255);

    /* sector angles */
    float angleStep = 360.0 / 20;  
    int k = 0;
    for (int i = 0; i < 20; i++, k +=2) {
        /* minus for correct order */
        float angle1 = -((i * angleStep)) * CV_PI / 180.0;
        float angle2 = -(((i + 1) * angleStep)) * CV_PI / 180.0;
        /* shift in correct place */
        angle1 -= 279.0 * CV_PI / 180.0;
        angle2 -= 279.0 * CV_PI / 180.0;
        /* line endpoints */
        Point p1(center.x + radius * cos(angle1), center.y - radius * sin(angle1));
        Point p2(center.x + radius * cos(angle2), center.y - radius * sin(angle2));

        /* draw sector lines */
        line(cur, center, p1, lineColor, 1);
        line(cur, center, p2, lineColor, 1);

        /* write sector number */
        float midAngle = (angle1 + angle2) / 2;
        Point textPos(center.x + (radius * 1.125) * cos(midAngle), center.y - (radius * 1.125) * sin(midAngle));
       
        if (show_num) {
            /* actually 40 sectors */
            putText(cur, std::to_string(board.sectorNumbers[k + 1]), textPos, FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);
        }
     }

    /* circles */
    circle(cur, center, board.Db_r.radiusBullseye, circleColor, 1);       // bullseye
    circle(cur, center, board.Db_r.radiusSingleBull, circleColor, 1);     // single bull
    circle(cur, center, board.Db_r.radiusTripleInner, circleColor, 1);    // triple in
    circle(cur, center, board.Db_r.radiusTripleOuter, circleColor, 1);    // triple out
    circle(cur, center, board.Db_r.radiusDoubleInner, circleColor, 1);    // double in
    circle(cur, center, board.Db_r.radiusDoubleOuter, circleColor, 1);    // double out


    if (image_show) {
        string img_name = string("Raw Image with Sector (").append(CamNameId).append(" Cam)");
        imshow(img_name, cur);
    }
    else {
        image = cur.clone();
    }


}


/* draw darts color sectors */
void dart_board_color_sectors(cv::Mat& dst) {
    struct Dartboard_Sector_s board = *d->db;
    Point center = board.center;

    /* set background color */
    dst.setTo(Scalar(192, 192, 192));

    /* create darts board colors */
    Scalar darkRed(50, 50, 120);      
    Scalar darkGreen(20, 100, 20);   
    Scalar lightYellow(180, 220, 230); 
    Scalar softBlack(20, 20, 20);  

    /* set edge (out of board) */
    circle(dst, center, board.Db_r.radiusDoubleOuter+60, softBlack, -1);


    float angleStep = 360.0 / 20;
    int k = 0;

    for (int i = 0; i < 20; i++, k += 2) {
        float angle1 = -((i * angleStep)) * CV_PI / 180.0 - 279.0 * CV_PI / 180.0;
        float angle2 = -(((i + 1) * angleStep)) * CV_PI / 180.0 - 279.0 * CV_PI / 180.0;

        /* define sector corner points for different rings */

        vector<Point> singleInner = {
            Point(center.x + board.Db_r.radiusTripleInner * cos(angle1), center.y - board.Db_r.radiusTripleInner * sin(angle1)),
            Point(center.x + board.Db_r.radiusTripleInner * cos(angle2), center.y - board.Db_r.radiusTripleInner * sin(angle2)),
            Point(center.x + board.Db_r.radiusSingleBull * cos(angle2), center.y - board.Db_r.radiusSingleBull * sin(angle2)),
            Point(center.x + board.Db_r.radiusSingleBull * cos(angle1), center.y - board.Db_r.radiusSingleBull * sin(angle1))
        };
        

        vector<Point> tripleRing = {
            Point(center.x + board.Db_r.radiusTripleOuter * cos(angle1), center.y - board.Db_r.radiusTripleOuter * sin(angle1)),
            Point(center.x + board.Db_r.radiusTripleOuter * cos(angle2), center.y - board.Db_r.radiusTripleOuter * sin(angle2)),
            Point(center.x + board.Db_r.radiusTripleInner * cos(angle2), center.y - board.Db_r.radiusTripleInner * sin(angle2)),
            Point(center.x + board.Db_r.radiusTripleInner * cos(angle1), center.y - board.Db_r.radiusTripleInner * sin(angle1))
        };

        vector<Point> singleOuter = {
            Point(center.x + board.Db_r.radiusDoubleInner * cos(angle1), center.y - board.Db_r.radiusDoubleInner * sin(angle1)),
            Point(center.x + board.Db_r.radiusDoubleInner * cos(angle2), center.y - board.Db_r.radiusDoubleInner * sin(angle2)),
            Point(center.x + board.Db_r.radiusTripleOuter * cos(angle2), center.y - board.Db_r.radiusTripleOuter * sin(angle2)),
            Point(center.x + board.Db_r.radiusTripleOuter * cos(angle1), center.y - board.Db_r.radiusTripleOuter * sin(angle1))
        };


        vector<Point> doubleRing = {
            Point(center.x + board.Db_r.radiusDoubleOuter * cos(angle1), center.y - board.Db_r.radiusDoubleOuter * sin(angle1)),
            Point(center.x + board.Db_r.radiusDoubleOuter * cos(angle2), center.y - board.Db_r.radiusDoubleOuter * sin(angle2)),
            Point(center.x + board.Db_r.radiusDoubleInner * cos(angle2), center.y - board.Db_r.radiusDoubleInner * sin(angle2)),
            Point(center.x + board.Db_r.radiusDoubleInner * cos(angle1), center.y - board.Db_r.radiusDoubleInner * sin(angle1))
        };

        fillConvexPoly(dst, singleOuter, (i % 2 != 0) ? softBlack : lightYellow);
        fillConvexPoly(dst, tripleRing, (i % 2 != 0) ? darkRed : darkGreen);
        fillConvexPoly(dst, singleInner, (i % 2 != 0) ? softBlack : lightYellow);
        fillConvexPoly(dst, doubleRing, (i % 2 != 0) ? darkRed : darkGreen);
    }

    /* fill the bull and bullseye */
    circle(dst, center, board.Db_r.radiusSingleBull, darkGreen, -1);
    circle(dst, center, board.Db_r.radiusBullseye, darkRed, -1);
}


/******************************************************************************
 * Create GUI Frames
******************************************************************************/
/***
 *
 * dart_board_create_scoreboard_gui(std::string name_win, int w, int h)
 *
 * Create an empty Scoreboard based on pre-defined game struct and gui struct
 * 
 *
 *
 * @param:	std::string name_win --> Window Name
 *              --> default: "Darts Scoreboard"
 * @param:	int w --> Window Width (actually should not really be changed)
 *              --> default: "SCOREBOARD_GUI_WIDTH
 * @param:	int h --> Window Heiht (actually should not really be changed)
 *              --> default: "SCOREBOARD_GUI_HEIGHT
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
void dart_board_create_scoreboard_gui(std::string name_win, int w, int h) {

    /* thread safe */
    d->mtx.lock();

    int ply_track = 0;

    /* increment payled games */
    d->dg->count_games++;

    /* create empty gui frame */
    d->dg->gui = Mat::zeros(h, w, CV_8UC3);
    d->dg->h = h;
    d->dg->w = w;

    /* settings */
    d->dg->gui.setTo(Scalar(50, 50, 50));  // background color gray 
    d->dg->text_w = getTextSize(name_win, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, name_win, Point(d->dg->text_pos, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // titel


    /*** 
     * create tabular 
    ***/
    int end_vert_line = (d->dg->row_offset - 20) + 1 * 40 + ((d->g->p.size()) * 50);  // end vertical lines tabular

    /* vertical lines */
    line(d->dg->gui, Point(d->dg->set_col + 2, d->dg->row_offset - 20), Point(d->dg->set_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between name and set
    line(d->dg->gui, Point(d->dg->leg_col + 2, d->dg->row_offset - 20), Point(d->dg->leg_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between set and leg
    line(d->dg->gui, Point(d->dg->score_col + 2, d->dg->row_offset - 20), Point(d->dg->score_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between leg and score
    line(d->dg->gui, Point(d->dg->last_throw_col + 2, d->dg->row_offset - 20), Point(d->dg->last_throw_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between score and last throw

    /* create header */
    putText(d->dg->gui, "player", Point(d->dg->name_col, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);  // player, left 
     
    d->dg->text_w = getTextSize("set", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->set_col + (int)((d->dg->set_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "set", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // set, center

    d->dg->text_w = getTextSize("leg", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->leg_col + (int)((d->dg->leg_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "leg", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // leg, center

    d->dg->text_w = getTextSize("score", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->score_col + (int)((d->dg->score_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "score", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);    // score, center

    d->dg->text_w = getTextSize("last throw", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->last_throw_col + (int)((d->dg->last_throw_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "last throw", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);   // last_throw, center

    /* horizontal line */
    d->dg->row_offset += 20;
    line(d->dg->gui, Point(d->dg->name_col, d->dg->row_offset), Point(d->dg->last_throw_col+ d->dg->last_throw_width, d->dg->row_offset), Scalar(255, 255, 255), 2);
    d->dg->row_offset += 30;

    /* fill tabular */
    for (auto& p : d->g->p) {

        /* name */ 
        putText(d->dg->gui, p.p_name, Point(d->dg->name_col, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        d->dg->text_w = getTextSize(to_string(p.set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->set_col + (int)((d->dg->set_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.set), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        d->dg->text_w = getTextSize(to_string(p.leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->leg_col + (int)((d->dg->leg_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.leg), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        p.score = 501;
        d->dg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->score_col + (int)((d->dg->score_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.score), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        p.last_throw = 0;
        d->dg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->last_throw_col + (int)((d->dg->last_throw_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.last_throw), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);


        /* dot */  
        if ((d->dg->count_games % d->g->num_p) == ply_track) {
            d->dg->text_w = 10; // circle radius
            d->dg->text_pos = d->dg->dot_col + (int)((d->dg->dot_width - d->dg->text_w) / 2);
            cv::circle(d->dg->gui, Point(d->dg->text_pos, d->dg->row_offset - 7), 8, Scalar(0, 0, 255), -1);     // red dot, center
        }

        ply_track++;

        d->dg->row_offset += 50; // dist between players
    }


    /* info */
    string out = "press [Enter] to start the game";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, d->dg->gui.rows - 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    out = "Leg " + to_string(d->dg->count_games) + " and it's " + d->g->p[d->dg->count_games % d->g->num_p].p_name + " to throw first";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, d->dg->gui.rows - 150), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    out = "Game On!";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, d->dg->gui.rows - 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);


    d->dg->name_win = name_win;
   
    cv::imshow(d->dg->name_win, d->dg->gui);

    /* reset row offset to first player position */
    d->dg->row_offset = 150;

    
    
    d->mtx.unlock();

}



/***
 *
 * dart_board_update_scoreboard_gui(int new_throw, std::string last_dart_str)
 *
 * Update Scoreboard: Check Busted / double Checkout and might reset / won't
 * set your score if against Darts rules. 
 * Also keeps rack which player is next (red dot) and tracks set and legs 
 *
 *
 *
 * @param:	int new_throw --> new 3-Dart-Score
 * @param:  std::string last_dart_str --> Last 1-Dart-Score as String
 *              --> This is indispensable for double Checkout validation
 *
 * @return: int --> Status if any player won the leg
 *              --> 0:= no winner yet
 *              --> [1..Num_of_Players]:= indicates which Player won
 *
 *
 * @note:	None
 *
 *
 * Example usage: None
 *
***/
int dart_board_update_scoreboard_gui(int new_throw, std::string last_dart_str) {

    /* thread safe */
    d->mtx.lock();

    /* keep track which players turn it is */
    //d->dg->count_player = d->dg->count_games;
    int ply_track = 0;

    //d->dg->row_offset += d->dg->count_player*50;


    /* update player */
    d->g->p[d->dg->count_player].score -= new_throw;
    d->g->p[d->dg->count_player].last_throw = new_throw;

    /* create empty gui frame */
    d->dg->gui = Mat::zeros(d->dg->h, d->dg->w, CV_8UC3);

    /* settings */
    d->dg->gui.setTo(Scalar(50, 50, 50));  // background color gray 
    d->dg->text_w = getTextSize(d->dg->name_win, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, d->dg->name_win, Point(d->dg->text_pos, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // titel


    /***
     * create tabular
    ***/
    int end_vert_line = (d->dg->row_offset - 20) + 1 * 40 + ((d->g->p.size()) * 50);  // end vertical lines tabular

    /* vertical lines */
    line(d->dg->gui, Point(d->dg->set_col + 2, d->dg->row_offset - 20), Point(d->dg->set_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between name and set
    line(d->dg->gui, Point(d->dg->leg_col + 2, d->dg->row_offset - 20), Point(d->dg->leg_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between set and leg
    line(d->dg->gui, Point(d->dg->score_col + 2, d->dg->row_offset - 20), Point(d->dg->score_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between leg and score
    line(d->dg->gui, Point(d->dg->last_throw_col + 2, d->dg->row_offset - 20), Point(d->dg->last_throw_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between score and last throw

    /* create header */
    putText(d->dg->gui, "player", Point(d->dg->name_col, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);  // player, left 

    d->dg->text_w = getTextSize("set", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->set_col + (int)((d->dg->set_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "set", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // set, center

    d->dg->text_w = getTextSize("leg", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->leg_col + (int)((d->dg->leg_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "leg", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // leg, center

    d->dg->text_w = getTextSize("score", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->score_col + (int)((d->dg->score_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "score", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);    // score, center

    d->dg->text_w = getTextSize("last throw", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->last_throw_col + (int)((d->dg->last_throw_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "last throw", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);   // last_throw, center

    /* horizontal line */
    d->dg->row_offset += 20;
    line(d->dg->gui, Point(d->dg->name_col, d->dg->row_offset), Point(d->dg->last_throw_col + d->dg->last_throw_width, d->dg->row_offset), Scalar(255, 255, 255), 2);
    d->dg->row_offset += 30;





    /* finished */
    if (d->g->p[d->dg->count_player].score == 0) {
        /* check double checkout if defined */
        if (DOUBLE_CHECKOUT) {
            if (strncmp("Double", last_dart_str.c_str(), 6) == 0) {
                d->mtx.unlock();
                return (d->dg->count_player + 1);
            }
            else {
                cout << "no double checkout! no score: " << d->g->p[d->dg->count_player].score << endl;
                d->g->p[d->dg->count_player].score += new_throw;
                /* no score message in lower area of frame */
                d->dg->text_w = getTextSize("no score", FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
                d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
                putText(d->dg->gui, "no score", Point(d->dg->text_pos, (int)((d->dg->gui.rows - 20))), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            }
        }
        else {
            d->mtx.unlock();
            return (d->dg->count_player + 1);
        }

    }


    /* busted */
    if (d->g->p[d->dg->count_player].score < 2) {
        cout << "busted! no score: " << d->g->p[d->dg->count_player].score << endl;
        d->g->p[d->dg->count_player].score += new_throw;
        /* no score mesage in lower area of frame */
        d->dg->text_w = getTextSize("no score", FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
        d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
        putText(d->dg->gui, "no score", Point(d->dg->text_pos, (int)((d->dg->gui.rows - 20))), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }



    /* fill tabular */
    for (const auto& p : d->g->p) {

        /* name */
        putText(d->dg->gui, p.p_name, Point(d->dg->name_col, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        d->dg->text_w = getTextSize(to_string(p.set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->set_col + (int)((d->dg->set_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.set), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        d->dg->text_w = getTextSize(to_string(p.leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->leg_col + (int)((d->dg->leg_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.leg), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        d->dg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->score_col + (int)((d->dg->score_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.score), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        d->dg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->last_throw_col + (int)((d->dg->last_throw_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.last_throw), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);


        /* dot */
        if ((ply_track % d->g->num_p) == ((d->dg->count_player+1)%d->g->num_p)) {
            d->dg->text_w = 10; // circle radius
            d->dg->text_pos = d->dg->dot_col + (int)((d->dg->dot_width - d->dg->text_w) / 2);
            cv::circle(d->dg->gui, Point(d->dg->text_pos, d->dg->row_offset - 7), 8, Scalar(0, 0, 255), -1);     // red dot, center
        }

        ply_track++;

        d->dg->row_offset += 50; // dist between players
    }


    cv::imshow(d->dg->name_win, d->dg->gui);


    /* calc next player */
    d->dg->count_player++;
    if (d->dg->count_player == d->g->num_p) {
        d->dg->count_player = 0;
    }

    /* reset row offset to first player position */
    d->dg->row_offset = 150;

    d->mtx.unlock();
    
    return 0;
    

}


/***
 *
 * dart_board_finish_scoreboard_gui(int player)
 *
 * Create Finished Leg GUI Frame
 * Increment Leg / Set of Winner 
 * 
 *
 *
 * @param:	int player --> indicate which player won [0..-Num_of_player-1]
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
void dart_board_finish_scoreboard_gui(int player) {

    /* thread safe */
    d->mtx.lock();

    string out;

    /* increment leg */
    d->g->p[player].leg++;
    /* set win */
    if (d->g->p[player].leg > 2) {
        d->g->p[player].set++;
        /* reset legs */
        for (auto& p : d->g->p) {
            p.leg = 0;
        }
    }


    


    /* create empty gui frame */
    d->dg->gui = Mat::zeros(d->dg->h, d->dg->w, CV_8UC3);

    /* settings */
    d->dg->gui.setTo(Scalar(50, 50, 50));  // background color gray 

    out = "Game Shot in the " + to_string(d->dg->count_games) + " Leg";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);


    out = d->g->p[player].p_name + " wins!";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, 250), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 3); 


    out = "press [Enter] to play the next";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, 400), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

    out = "press [q] to quit";
    d->dg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, out, Point(d->dg->text_pos, 450), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

    cv::imshow(d->dg->name_win, d->dg->gui);


    /* reset row offset to first player position */
    d->dg->row_offset = 150;
    /* reset count_player */
    d->dg->count_player = 0;

    d->mtx.unlock();
}




/******************************************************************************
 * Dartsboard manipulation 
******************************************************************************/
/***
 *
 * dart_board_set_new_game(char* names[], int num_p)
 *
 * Create a new game
 * Called via Command Line 
 * Support for max. 4 Players at the moment
 *
 *
 * @param:	char* names[]   --> Names of the Players
 * @param:  int num_p       --> Number of Players
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
void dart_board_set_new_game(char* names[], int num_p) {

    /* thread safe */
    d->mtx.lock();

   vector<player_s> players;

    /* fill in new names */
    for (int i = 0; i < num_p; i++) {
        players.push_back({ names[i], 501, 0, 0, 0 }); 
    }

    /* re-initialize game structure */
    d->g->num_p = num_p;
    d->g->p = players;

    d->dg->count_games = -1;
    d->dg->count_player = 0;

    d->mtx.unlock();

    /* call create with new game */
    dart_board_create_scoreboard_gui();


}


/***
 *
 * dart_board_set_score(char* name, int score)
 *
 * set score of explicit player 
 * Called via Command Line
 * NOTE: there is no range check at the moment and no handling for setting 
 * the score to zero
 *
 *
 * @param:	char* name  --> Name of the Player
 * @param:  int score   --> Score to be left   
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
void dart_board_set_score(char* name, int score) {

    /* thread safe */
    d->mtx.lock();

    int ply_track = 0;
    int player = -1;

    /* find player;if there was no matching player just ignore */
    for (auto& p : d->g->p) {
        if (strcmp(p.p_name.c_str(), name) == 0) {
            int diff = 0;
            p.score = p.score + p.last_throw;
            p.last_throw = p.score - score;
            p.score = score;
        }
        player++;
    }

    /* create empty gui frame */
    d->dg->gui = Mat::zeros(d->dg->h, d->dg->w, CV_8UC3);

    /* settings */
    d->dg->gui.setTo(Scalar(50, 50, 50));  // background color gray    
    d->dg->text_w = getTextSize(d->dg->name_win, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
    putText(d->dg->gui, d->dg->name_win, Point(d->dg->text_pos, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // titel


    /***
     * create tabular
    ***/
    int end_vert_line = (d->dg->row_offset - 20) + 1 * 40 + ((d->g->p.size()) * 50);  // end vertical lines tabular

    /* vertical lines */
    line(d->dg->gui, Point(d->dg->set_col + 2, d->dg->row_offset - 20), Point(d->dg->set_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between name and set
    line(d->dg->gui, Point(d->dg->leg_col + 2, d->dg->row_offset - 20), Point(d->dg->leg_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between set and leg
    line(d->dg->gui, Point(d->dg->score_col + 2, d->dg->row_offset - 20), Point(d->dg->score_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between leg and score
    line(d->dg->gui, Point(d->dg->last_throw_col + 2, d->dg->row_offset - 20), Point(d->dg->last_throw_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between score and last throw

    /* create header */
    putText(d->dg->gui, "player", Point(d->dg->name_col, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);  // player, left 

    d->dg->text_w = getTextSize("set", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->set_col + (int)((d->dg->set_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "set", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // set, center

    d->dg->text_w = getTextSize("leg", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->leg_col + (int)((d->dg->leg_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "leg", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // leg, center

    d->dg->text_w = getTextSize("score", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->score_col + (int)((d->dg->score_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "score", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);    // score, center

    d->dg->text_w = getTextSize("last throw", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    d->dg->text_pos = d->dg->last_throw_col + (int)((d->dg->last_throw_width - d->dg->text_w) / 2);
    putText(d->dg->gui, "last throw", Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);   // last_throw, center

    /* horizontal line */
    d->dg->row_offset += 20;
    line(d->dg->gui, Point(d->dg->name_col, d->dg->row_offset), Point(d->dg->last_throw_col + d->dg->last_throw_width, d->dg->row_offset), Scalar(255, 255, 255), 2);
    d->dg->row_offset += 30;




    /* busted; this means user set the score to <0, not supported at the moment */
    if (d->g->p[player].score < 0) {
        cout << "busted! no score: " << d->g->p[player].score << endl;
        d->g->p[player].score += d->g->p[player].last_throw;
        /* no score mesage in lower area of frame */
        d->dg->text_w = getTextSize("no score", FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
        d->dg->text_pos = 0 + (int)((d->dg->gui.cols - d->dg->text_w) / 2);
        putText(d->dg->gui, "no score", Point(d->dg->text_pos, (int)((d->dg->gui.rows - 20))), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }

    /* finished; this means user set the score to zero, not supported at the moment */
    if (d->g->p[player].score == 0) {
        //empty
    }



    /* fill tabular */
    for (const auto& p : d->g->p) {

        /* name */
        putText(d->dg->gui, p.p_name, Point(d->dg->name_col, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        d->dg->text_w = getTextSize(to_string(p.set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->set_col + (int)((d->dg->set_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.set), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        d->dg->text_w = getTextSize(to_string(p.leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->leg_col + (int)((d->dg->leg_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.leg), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        d->dg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->score_col + (int)((d->dg->score_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.score), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        d->dg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        d->dg->text_pos = d->dg->last_throw_col + (int)((d->dg->last_throw_width - d->dg->text_w) / 2);
        putText(d->dg->gui, to_string(p.last_throw), Point(d->dg->text_pos, d->dg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);


        /* dot */
        if ((ply_track % d->g->num_p) == ((d->dg->count_player) % d->g->num_p)) {
            d->dg->text_w = 10; // circle radius
            d->dg->text_pos = d->dg->dot_col + (int)((d->dg->dot_width - d->dg->text_w) / 2);
            cv::circle(d->dg->gui, Point(d->dg->text_pos, d->dg->row_offset - 7), 8, Scalar(0, 0, 255), -1);     // red dot, center
        }

        ply_track++;

        d->dg->row_offset += 50; // dist between players
    }


    cv::imshow(d->dg->name_win, d->dg->gui);

    /* reset row offset to first player position */
    d->dg->row_offset = 150;


    d->mtx.unlock();


    return;




}


int dart_board_get_cur_player_score(void) {


    /* thread safe */
    d->mtx.lock();

    int score = d->g->p[d->dg->count_player].score;

    d->mtx.unlock();


    /* return score of current player */
    return score;

}