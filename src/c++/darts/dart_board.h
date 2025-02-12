/******************************************************************************
 *
 * dart_board.h
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


#ifndef DART_BOARD_H
#define DART_BOARD_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>


/*************************** global Defines **********************************/
#define SCOREBOARD_GUI_WIDTH 800
#define SCOREBOARD_GUI_HEIGHT 600

/************************* global Structure **********************************/
struct result_s {
    int val = 0;
    std::string str = "void";
};

/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/
extern void Dartsboard_GUI_Thread(void*arg);
extern void dart_board_init(void);

extern void dart_board_determineSector(const cv::Point& pixel, int ThreadId, struct result_s*r);
extern void dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board, struct result_s* r);
extern void dart_board_decide_sector(struct result_s* sec_board_top, struct result_s* sec_board_right, struct result_s* sec_board_left, struct result_s* r);
extern void dart_board_draw_sectors(cv::Mat& image, int ThreadId, int image_show = 1, int cal = 1);


extern void dart_board_create_scoreboard_gui(std::string name_win = "Darts Scoreboard", int w = SCOREBOARD_GUI_WIDTH, int h = SCOREBOARD_GUI_HEIGHT);
extern int dart_board_update_scoreboard_gui(int new_throw, std::string last_dart_str);
extern void dart_board_finish_scoreboard_gui(int player);
extern void dart_board_set_new_game(char* names[], int num_p);
extern void dart_board_set_score(char* name, int score);

extern int dart_board_get_cur_player_score(void);

#endif 