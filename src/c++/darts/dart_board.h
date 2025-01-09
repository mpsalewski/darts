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



#ifndef DART_BOARD_H
#define DART_BOARD_H

/* Include files */
#include <opencv2/opencv.hpp>
#include <string>


/*************************** local Defines ***********************************/
#define TOP_CAM     2
#define LEFT_CAM    1
#define RIGHT_CAM   3

#define DARTBOARD_SECTORS 40


//const int Dartboard_Sectors_right_round[DARTBOARD_SECTORS] = { 20, 1,1 , 18, 18, 4,4, 13,13, 6,6, 10,10, 15,15, 2,2, 17,17, 3,3, 19,19, 7,7, 16,16, 8,8,11,11,14,14,9,9,12,12,5,5,20 };

/************************** local Structure ***********************************/
struct Dartboard_Radius_s {
    float radiusBullseye, radiusSingleBull, radiusTripleInner, radiusTripleOuter, radiusDoubleInner, radiusDoubleOuter;

};


struct Dartboard_Sector_s {
    cv::Point center;
    struct Dartboard_Radius_s Db_r;
    int sectorNumbers[DARTBOARD_SECTORS];
    
};


struct result_s {
    int val;
    std::string str;
};

/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/
extern void dart_board_determineSector(const cv::Point& pixel, int ThreadId, struct result_s*r);
extern void dart_board_getSectorValue(int sector, float distance, struct Dartboard_Sector_s& board, struct result_s* r);
extern void dart_board_decide_sector(struct result_s* sec_board_top, struct result_s* sec_board_right, struct result_s* sec_board_left, struct result_s* r);


#endif 