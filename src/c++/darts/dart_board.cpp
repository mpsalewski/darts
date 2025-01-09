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

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/



/************************** local Structure ***********************************/

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

    string name_win;

}priv_gui;

static struct priv_gui_s* pg = &priv_gui;




static Point Dartboard_Center(320, 240);

static struct Dartboard_Radius_s Dartboard_Rad_top = { 
     7, 20, 117, 129, 187, 200 
};

static struct Dartboard_Sector_s Db_sec_top = {
    Dartboard_Center,
    Dartboard_Rad_top,
    { 20, 1,1 , 18, 18, 4,4, 13,13, 6,6, 10,10, 15,15, 2,2, 17,17, 3,3, 19,19, 7,7, 16,16, 8,8,11,11,14,14,9,9,12,12,5,5,20 }
};

/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/

// Funktion zur Bestimmung des Sektors und des entsprechenden Werts
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

    // Berechne Abstand und Winkel
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



    // Bestimme den Sektor
    int sector = static_cast<int>(angle / 9) + 1;
    //cout << sector << endl;

    // Erhalte den Wert (Single, Triple, Double) für den Sektor
    dart_board_getSectorValue(sector, distance, board, r); // +1, da Sektoren 1 bis 20 gehen
}


// Funktion, um den richtigen Wert (Single, Double, Triple) und die Sektorzahl zu ermitteln
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





/* do the final sector decision based on multiple raw board sector results */
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



void dart_board_create_scoreboard_gui(struct game_s* g, std::string name_win, int w, int h) {

    /* create empty gui frame */
    pg->gui = Mat::zeros(h, w, CV_8UC3);
    pg->h = h;
    pg->w = w;

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
    line(pg->gui, Point(pg->name_col, pg->row_offset), Point(pg->last_throw_col+ pg->last_throw_width, pg->row_offset), Scalar(255, 255, 255), 2);
    pg->row_offset += 30;

    /* fill tabular */
    for (const auto& p : g->p) {

        /* name */ 
        putText(pg->gui, p.p_name, Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        pg->text_w = getTextSize(to_string(g->set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
        putText(pg->gui, to_string(g->set), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        pg->text_w = getTextSize(to_string(g->leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
        putText(pg->gui, to_string(g->leg), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        pg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.score), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        pg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.last_throw), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        pg->row_offset += 50; // dist between players
    }

    pg->name_win = name_win;
   
    cv::imshow(pg->name_win, pg->gui);

    /* reset row offset to first player position */
    pg->row_offset = 150;


}



int dart_board_update_scoreboard_gui(struct game_s* g, int new_throw) {

    /* keep track which players turn it is */
    static int count_player = 0;

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

    /* fill tabular */
    for (const auto& p : g->p) {

        /* name */
        putText(pg->gui, p.p_name, Point(pg->name_col, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* set */
        pg->text_w = getTextSize(to_string(g->set), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->set_col + (int)((pg->set_width - pg->text_w) / 2);
        putText(pg->gui, to_string(g->set), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* leg */
        pg->text_w = getTextSize(to_string(g->leg), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->leg_col + (int)((pg->leg_width - pg->text_w) / 2);
        putText(pg->gui, to_string(g->leg), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* score */
        pg->text_w = getTextSize(to_string(p.score), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->score_col + (int)((pg->score_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.score), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        /* last throw */
        pg->text_w = getTextSize(to_string(p.last_throw), FONT_HERSHEY_SIMPLEX, 0.8, 1, nullptr).width;
        pg->text_pos = pg->last_throw_col + (int)((pg->last_throw_width - pg->text_w) / 2);
        putText(pg->gui, to_string(p.last_throw), Point(pg->text_pos, pg->row_offset), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

        pg->row_offset += 50; // dist between players
    }


    cv::imshow(pg->name_win, pg->gui);


    /* finished */
    if (g->p[count_player].score <= 0) {
        return (count_player + 1);
    }

    /* calc next player */
    count_player++;
    if (count_player == g->num_p) {
        count_player = 0;
    }

    /* reset row offset to first player position */
    pg->row_offset = 150;

    
    
    return 0;
    

}


void dart_board_finish_scoreboard_gui(struct game_s* g, int player) {


    string out = g->p[player].p_name + " wins!";


    /* create empty gui frame */
    pg->gui = Mat::zeros(pg->h, pg->w, CV_8UC3);

    /* settings */
    pg->gui.setTo(Scalar(50, 50, 50));  // background color gray 
    pg->text_w = getTextSize(out, FONT_HERSHEY_SIMPLEX, 2, 3, nullptr).width;
    pg->text_pos = 0 + (int)((pg->gui.cols - pg->text_w) / 2);
    putText(pg->gui, out, Point(pg->text_pos, (int)((pg->gui.rows - pg->text_w) / 2)), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 3); // titel

    cv::imshow(pg->name_win, pg->gui);




}