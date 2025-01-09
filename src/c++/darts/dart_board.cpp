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
std::string dart_board_determineSector(const cv::Point& pixel, int ThreadId) {

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

        default:    return "error";
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
            return dart_board_getSectorValue(5 + 1, distance, board);
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
            return dart_board_getSectorValue(15 + 1, distance, board);
        }
    }



    // Bestimme den Sektor
    int sector = static_cast<int>(angle / 9) + 1;
    cout << sector << endl;

    // Erhalte den Wert (Single, Triple, Double) für den Sektor
    return dart_board_getSectorValue(sector, distance, board); // +1, da Sektoren 1 bis 20 gehen
}


// Funktion, um den richtigen Wert (Single, Double, Triple) und die Sektorzahl zu ermitteln
std::string dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board) {
    // Bestimme den Wert je nach Entfernung (Single, Double, Triple)
    if (distance <= board.Db_r.radiusBullseye) {
        return "Bullseye";
    }
    else if (distance <= board.Db_r.radiusSingleBull) {
        return "Single Bull";
    }
    else if (distance <= board.Db_r.radiusTripleInner) {
        return "Single " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else if (distance <= board.Db_r.radiusTripleOuter) {
        return "Triple " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else if (distance <= board.Db_r.radiusDoubleInner) {
        return "Single " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else if (distance <= board.Db_r.radiusDoubleOuter) {
        return "Double " + std::to_string(board.sectorNumbers[sector - 1]);
    }
    else {
        return "Out of Board";
    }
}





/* do the final sector decision based on multiple raw board sector results */
std::string dart_board_decide_sector(std::string sec_board_top, std::string sec_board_right, std::string sec_board_left) {


#if 1 

    /* all three are equal */
    if ((sec_board_top == sec_board_right)&& (sec_board_top == sec_board_left)) {
        return sec_board_top;
    }
    /* top and right are equal */
    else if (sec_board_top == sec_board_right) {
        return sec_board_top;
    }
    /* top and left are equal */
    else if (sec_board_top == sec_board_left) {
        return sec_board_top;
    }
    /* left and right are equal */
    else if (sec_board_left == sec_board_right) {
        return sec_board_left;
    }
    /* none of them are equal */
    else {
        printf("Warning: Three Different Sectors detected; return Top Detection as Default\n");
        return sec_board_top;
    }


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