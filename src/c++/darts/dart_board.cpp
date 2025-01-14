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



/************************** local Structure ***********************************/
/***
 * create game
***/
/* players */
static std::vector<player_s> players = {
    {"Lukas", 501, 0, 0, 0},
    {"Mika", 501, 0, 0, 0},
    {"Marek", 501, 0, 0, 0},
};
/* game */
static struct game_s game = {
    players.size(),     // number of players
    players,            // players
};
/* create ptr for access and compatibility */
static struct game_s* g = &game;


/* create gui struct */
static struct priv_gui_s {

    /* creat empty gui frame */
    Mat gui;

    int row_offset = 150;               // dist to top
    int name_col = 50;                  // name column
    int name_width = 150;               // name column width
    int set_col = name_col + name_width;  // set column
    int set_width = 100;                // set column width
    int leg_col = set_col + set_width;  // leg column
    int leg_width = 100;                // leg column width
    int score_col = leg_col + leg_width;  // score column
    int score_width = 150;              // score column width
    int last_throw_col = score_col + score_width;   // last throw column
    int last_throw_width = 150;                     // last throw column width 
    int text_w, text_pos;
    int h, w;
    int dot_col = last_throw_col + last_throw_width;
    int dot_width = 100;
    int count_games = -1;


    string name_win;

}priv_gui;
/* create ptr for access and compatibility */
static struct priv_gui_s* pg = &priv_gui;



/* create DartsBoard */
static Point Dartboard_Center(320, 240);

static struct Dartboard_Radius_s Dartboard_Rad_top = { 
     10, 22, 117, 129, 187, 200 
};

static struct Dartboard_Sector_s Db_sec_top = {
    Dartboard_Center,
    Dartboard_Rad_top,
    { 20, 1,1 , 18, 18, 4,4, 13,13, 6,6, 10,10, 15,15, 2,2, 17,17, 3,3, 19,19, 7,7, 16,16, 8,8,11,11,14,14,9,9,12,12,5,5,20 }
};



/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/


/******************************* GUI THREAD **********************************/
/***
 *
 * Dartsboard_GUI_Thread(void)
 *
 * About this function ...
 *
 *
 * @param:	void
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
void Dartsboard_GUI_Thread(void) {

    /* unused void pointer assignment bc moved game structure in this module */
    //struct game_s* g = (struct game_s*)(arg);
    int fin = 0;
    int key = 0;

    /* init gui */
    dart_board_create_scoreboard_gui();
    // example2: dart_board_create_scoreboard_gui(g, "Board2", 900,900);
    std::cout << "press [Enter] to start the game" << endl;
    while (waitKey(10) != 13 && running) {
        this_thread::sleep_for(chrono::milliseconds(250));
    }

    while (running == 1) {


        /* quit on [Esc] */
        if (cv::waitKey(10) == 27) {
            break;
        }

        if (t_e.score_flag) {
            /* reset flag */
            t_e.score_flag = 0;

            /* update scoreboard and check for finish */
            fin = dart_board_update_scoreboard_gui(t_e.score, t_e.last_dart_str);
            if (fin > 0) {
                dart_board_finish_scoreboard_gui(fin - 1);
                std::cout << "finished by: " << g->p[fin - 1].p_name << endl;
                /* next game ? yes --> [Enter]; no --> [q] */
                while ((key != 13) && (key != 113) && running) {
                    key = waitKey(10);
                    this_thread::sleep_for(chrono::milliseconds(250));
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
            t_e.score = 0;
            this_thread::sleep_for(chrono::milliseconds(250));
        }
        this_thread::sleep_for(chrono::milliseconds(250));
    }

}



/************************** Function Definitions *****************************/
/***
 *
 * dart_board_determineSector(const cv::Point& pixel, int ThreadId, struct result_s* r)
 *
 * Compute the Darts result by given Pixel
 *
 *
 * @param:	void
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
void dart_board_determineSector(const cv::Point& pixel, int ThreadId, struct result_s* r) {

    struct Dartboard_Sector_s board;
    
    switch (ThreadId) {

        case TOP_CAM:
            board = Db_sec_top;
            break;

        case RIGHT_CAM:
            board = Db_sec_top;
            break;

        case LEFT_CAM:
            board = Db_sec_top;
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
}



/***
 *
 * dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board, struct result_s* r)
 *
 * Compute correct result by given sector and distance
 *
 *
 * @param:	void
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
void dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board, struct result_s* r) {
    // Bestimme den Wert je nach Entfernung (Single, Double, Triple)
    if (distance <= board.Db_r.radiusBullseye) {
        r->val = 50;
        r->str = "Bullseye";
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
 *
 *
 * @param:	void
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
void dart_board_decide_sector(struct result_s* sec_board_top, struct result_s* sec_board_right, struct result_s* sec_board_left, struct result_s* r) {


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

}



void dart_board_create_scoreboard_gui(std::string name_win, int w, int h) {


    int ply_track = 0;

    /* increment payled games */
    pg->count_games++;

    /* create empty gui frame */
    pg->gui = Mat::zeros(h, w, CV_8UC3);
    pg->h = h;
    pg->w = w;

    /* settings */
    pg->gui.setTo(Scalar(50, 50, 50));  // background color gray 
    pg->text_w = getTextSize(name_win, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, name_win, Point(pg->text_pos, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // titel


    /*** 
     * create tabular 
    ***/
    int end_vert_line = (pg->row_offset - 20) + 1 * 40 + ((g->p.size()) * 50);  // end vertical lines tabular

    /* vertical lines */
    line(pg->gui, Point(pg->set_col + 2, pg->row_offset - 20), Point(pg->set_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between name and set
    line(pg->gui, Point(pg->leg_col + 2, pg->row_offset - 20), Point(pg->leg_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between set and leg
    line(pg->gui, Point(pg->score_col + 2, pg->row_offset - 20), Point(pg->score_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between leg and score
    line(pg->gui, Point(pg->last_throw_col + 2, pg->row_offset - 20), Point(pg->last_throw_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between score and last throw

    /* create header */
    putText(pg->gui, "player", Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);  // player, left 
     
    pg->text_w = getTextSize("set", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
    putText(pg->gui, "set", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // set, center

    pg->text_w = getTextSize("leg", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
    putText(pg->gui, "leg", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // leg, center

    pg->text_w = getTextSize("score", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
    putText(pg->gui, "score", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);    // score, center

    pg->text_w = getTextSize("last throw", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
    putText(pg->gui, "last throw", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);   // last_throw, center

    /* horizontal line */
    pg->row_offset += 20;
    line(pg->gui, Point(pg->name_col, pg->row_offset), Point(pg->last_throw_col+ pg->last_throw_width, pg->row_offset), Scalar(255, 255, 255), 2);
    pg->row_offset += 30;

    /* fill tabular */
    for (auto& p : g->p) {

        /* name */ 
        putText(pg->gui, p.p_name, Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        pg->text_w = getTextSize(to_string(p.set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.set), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        pg->text_w = getTextSize(to_string(p.leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.leg), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        p.score = 501;
        pg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.score), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        p.last_throw = 0;
        pg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.last_throw), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);


        /* dot */  
        if ((pg->count_games % g->num_p) == ply_track) {
            pg->text_w = 10; // circle radius
            pg->text_pos = pg->dot_col + (int)((pg->dot_width - pg->text_w) / 2);
            cv::circle(pg->gui, Point(pg->text_pos, pg->row_offset - 7), 8, Scalar(0, 0, 255), -1);     // red dot, center
        }

        ply_track++;

        pg->row_offset += 50; // dist between players
    }


    /* info */
    string out = "press [Enter] to start the game";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, pg->gui.rows - 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    out = "Leg " + to_string(pg->count_games) + " and it's " + g->p[pg->count_games % g->num_p].p_name + " to throw first";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, pg->gui.rows - 150), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    out = "Game On!";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, pg->gui.rows - 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);


    pg->name_win = name_win;
   
    cv::imshow(pg->name_win, pg->gui);

    /* reset row offset to first player position */
    pg->row_offset = 150;

    

}



int dart_board_update_scoreboard_gui(int new_throw, std::string last_dart_str) {

    /* keep track which players turn it is */
    static int count_player = 0 + pg->count_games;
    int ply_track = 0;

    //pg->row_offset += count_player*50;


    /* update player */
    g->p[count_player].score -= new_throw;
    g->p[count_player].last_throw = new_throw;

    /* create empty gui frame */
    pg->gui = Mat::zeros(pg->h, pg->w, CV_8UC3);

    /* settings */
    pg->gui.setTo(Scalar(50, 50, 50));  // background color gray 
    pg->text_w = getTextSize(pg->name_win, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, pg->name_win, Point(pg->text_pos, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // titel


    /***
     * create tabular
    ***/
    int end_vert_line = (pg->row_offset - 20) + 1 * 40 + ((g->p.size()) * 50);  // end vertical lines tabular

    /* vertical lines */
    line(pg->gui, Point(pg->set_col + 2, pg->row_offset - 20), Point(pg->set_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between name and set
    line(pg->gui, Point(pg->leg_col + 2, pg->row_offset - 20), Point(pg->leg_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between set and leg
    line(pg->gui, Point(pg->score_col + 2, pg->row_offset - 20), Point(pg->score_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between leg and score
    line(pg->gui, Point(pg->last_throw_col + 2, pg->row_offset - 20), Point(pg->last_throw_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between score and last throw

    /* create header */
    putText(pg->gui, "player", Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);  // player, left 

    pg->text_w = getTextSize("set", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
    putText(pg->gui, "set", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // set, center

    pg->text_w = getTextSize("leg", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
    putText(pg->gui, "leg", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // leg, center

    pg->text_w = getTextSize("score", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
    putText(pg->gui, "score", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);    // score, center

    pg->text_w = getTextSize("last throw", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
    putText(pg->gui, "last throw", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);   // last_throw, center

    /* horizontal line */
    pg->row_offset += 20;
    line(pg->gui, Point(pg->name_col, pg->row_offset), Point(pg->last_throw_col + pg->last_throw_width, pg->row_offset), Scalar(255, 255, 255), 2);
    pg->row_offset += 30;




    /* busted */
    if (g->p[count_player].score < 0) {
        cout << "busted! no score: " << g->p[count_player].score << endl;
        g->p[count_player].score += new_throw;
        /* no score mesage in lower area of frame */
        pg->text_w = getTextSize("no score", FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
        pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
        putText(pg->gui, "no score", Point(pg->text_pos, (int)((pg->gui.rows - 20))), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }

    /* finished */
    if (g->p[count_player].score == 0) {
        /* check double checkout if defined */
        if (DOUBLE_CHECKOUT) {
            if (strncmp("Double", last_dart_str.c_str(), 6) == 0) {
                return (count_player + 1);
            }
            else {
                cout << "no double checkout! no score: " << g->p[count_player].score << endl;
                g->p[count_player].score += new_throw;
                /* no score message in lower area of frame */
                pg->text_w = getTextSize("no score", FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
                pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
                putText(pg->gui, "no score", Point(pg->text_pos, (int)((pg->gui.rows - 20))), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            }
        }
        else {
            return (count_player + 1);
        }

    }



    /* fill tabular */
    for (const auto& p : g->p) {

        /* name */
        putText(pg->gui, p.p_name, Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        pg->text_w = getTextSize(to_string(p.set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.set), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        pg->text_w = getTextSize(to_string(p.leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.leg), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        pg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.score), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        pg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.last_throw), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);


        /* dot */
        if ((ply_track % g->num_p) == ((count_player+1)%g->num_p)) {
            pg->text_w = 10; // circle radius
            pg->text_pos = pg->dot_col + (int)((pg->dot_width - pg->text_w) / 2);
            cv::circle(pg->gui, Point(pg->text_pos, pg->row_offset - 7), 8, Scalar(0, 0, 255), -1);     // red dot, center
        }

        ply_track++;

        pg->row_offset += 50; // dist between players
    }


    cv::imshow(pg->name_win, pg->gui);


    /* calc next player */
    count_player++;
    if (count_player == g->num_p) {
        count_player = 0;
    }

    /* reset row offset to first player position */
    pg->row_offset = 150;

    
    
    return 0;
    

}


void dart_board_finish_scoreboard_gui(int player) {


    string out;

    /* increment leg */
    g->p[player].leg++;
    /* set win */
    if (g->p[player].leg > 2) {
        g->p[player].set++;
        /* reset legs */
        for (auto& p : g->p) {
            p.leg = 0;
        }
    }


    


    /* create empty gui frame */
    pg->gui = Mat::zeros(pg->h, pg->w, CV_8UC3);

    /* settings */
    pg->gui.setTo(Scalar(50, 50, 50));  // background color gray 

    out = "Game Shot in the " + to_string(pg->count_games) + " Leg";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);


    out = g->p[player].p_name + " wins!";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, 250), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 3); 


    out = "press [Enter] to play the next";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, 400), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

    out = "press [q] to quit";
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, 450), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

    cv::imshow(pg->name_win, pg->gui);


    /* reset row offset to first player position */
    pg->row_offset = 150;



}




/***
 * Dartsboard manipulation 
***/

/* create a new game */
void dart_board_set_new_game(char* names[], int num_p) {

   vector<player_s> players;

    /* fill in new names */
    for (int i = 0; i < num_p; i++) {
        players.push_back({ names[i], 501, 0, 0, 0 }); 
    }

    /* re-initialize game structure */
    g->num_p = num_p;
    g->p = players;

    pg->count_games = -1;


    /* call create with new game */

    dart_board_create_scoreboard_gui();


}

/* set score of explicit player */
void dart_board_set_score(char* name, int score) {

    int ply_track = 0;
    int player = -1;

    /* find player;if there was no matching player just ignore */
    for (auto& p : g->p) {
        if (strcmp(p.p_name.c_str(), name) == 0) {
            int diff = 0;
            p.score = p.score + p.last_throw;
            p.last_throw = p.score - score;
            p.score = score;
        }
        player++;
    }

    /* create empty gui frame */
    pg->gui = Mat::zeros(pg->h, pg->w, CV_8UC3);

    /* settings */
    pg->gui.setTo(Scalar(50, 50, 50));  // background color gray    
    pg->text_w = getTextSize(pg->name_win, FONT_HERSHEY_SIMPLEX, 1, 2, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, pg->name_win, Point(pg->text_pos, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // titel


    /***
     * create tabular
    ***/
    int end_vert_line = (pg->row_offset - 20) + 1 * 40 + ((g->p.size()) * 50);  // end vertical lines tabular

    /* vertical lines */
    line(pg->gui, Point(pg->set_col + 2, pg->row_offset - 20), Point(pg->set_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between name and set
    line(pg->gui, Point(pg->leg_col + 2, pg->row_offset - 20), Point(pg->leg_col + 2, end_vert_line), Scalar(255, 255, 255), 2);        // line between set and leg
    line(pg->gui, Point(pg->score_col + 2, pg->row_offset - 20), Point(pg->score_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between leg and score
    line(pg->gui, Point(pg->last_throw_col + 2, pg->row_offset - 20), Point(pg->last_throw_col + 2, end_vert_line), Scalar(255, 255, 255), 2);    // line between score and last throw

    /* create header */
    putText(pg->gui, "player", Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);  // player, left 

    pg->text_w = getTextSize("set", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
    putText(pg->gui, "set", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // set, center

    pg->text_w = getTextSize("leg", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
    putText(pg->gui, "leg", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);      // leg, center

    pg->text_w = getTextSize("score", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
    putText(pg->gui, "score", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);    // score, center

    pg->text_w = getTextSize("last throw", FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
    pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
    putText(pg->gui, "last throw", Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);   // last_throw, center

    /* horizontal line */
    pg->row_offset += 20;
    line(pg->gui, Point(pg->name_col, pg->row_offset), Point(pg->last_throw_col + pg->last_throw_width, pg->row_offset), Scalar(255, 255, 255), 2);
    pg->row_offset += 30;




    /* busted; this means user set the score to <0, not supported at the moment */
    if (g->p[player].score < 0) {
        cout << "busted! no score: " << g->p[player].score << endl;
        g->p[player].score += g->p[player].last_throw;
        /* no score mesage in lower area of frame */
        pg->text_w = getTextSize("no score", FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
        pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
        putText(pg->gui, "no score", Point(pg->text_pos, (int)((pg->gui.rows - 20))), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }

    /* finished; this means user set the score to zero, not supported at the moment */
    if (g->p[player].score == 0) {
        //empty
    }



    /* fill tabular */
    for (const auto& p : g->p) {

        /* name */
        putText(pg->gui, p.p_name, Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        pg->text_w = getTextSize(to_string(p.set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.set), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        pg->text_w = getTextSize(to_string(p.leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.leg), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        pg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.score), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        pg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.last_throw), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);


        /* dot */
        if ((ply_track % g->num_p) == ((player + 1) % g->num_p)) {
            pg->text_w = 10; // circle radius
            pg->text_pos = pg->dot_col + (int)((pg->dot_width - pg->text_w) / 2);
            cv::circle(pg->gui, Point(pg->text_pos, pg->row_offset - 7), 8, Scalar(0, 0, 255), -1);     // red dot, center
        }

        ply_track++;

        pg->row_offset += 50; // dist between players
    }


    cv::imshow(pg->name_win, pg->gui);

    /* reset row offset to first player position */
    pg->row_offset = 150;



    return;




}