/******************************************************************************
 *
 * calibration.c 
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
 * Author(s):   	Lukas Grose <lukas@grose.de>  
 *                  Mika Paul Salewski <mika.paul.salewski@gmail.com>
 *
 * Created on :     2025-01-06
 * Last revision :  None
 *
 *
 *
 * Copyright (c) 2025, Lukas Grose, Mika Paul Salewski
 * Version: 2025.01.06
 * License: CC BY-NC-SA 4.0,
 *      see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 *
 *
 * Further information about this source-file:
 *      --> warp perspective of 3 external cameras to get a consistent view
******************************************************************************/



/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()
/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "calibration.h"
#include "image_proc.h"
#include "Sobel.h"
#include "dart_board.h"
#include "globals.h"
#include "cams.h"

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
#define USE_DYNAMIC_COEFFS 1

#define TOP_CAM_CAL_TB  "Top Cam Calibration Trackbars"
#define RIGHT_CAM_CAL_TB  "Right Cam Calibration Trackbars"
#define LEFT_CAM_CAL_TB  "Left Cam Calibration Trackbars"


/************************** local structure **********************************/

struct src_points_s {
    Point2f twenty;
    Point2f six;
    Point2f three;
    Point2f eleven;

    //Point2f center;
};

struct warps_h_s {
    Mat H_top;
    Mat H_right;
    Mat H_left;

};

static struct cal_s {

    struct src_points_s top;
    struct src_points_s right;
    struct src_points_s left;

    vector<Point2f> src_points_top;
    vector<Point2f> src_points_right;
    vector<Point2f> src_points_left;

    vector<Point2f> dst_points;

    int cal_win;    // window to be calibrated 


    struct warps_h_s Homo;


}cal;


/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/
void calibration_init(void) {

    static struct cal_s* p = &cal;

    /* default init */

    p->top.twenty.x = 334;
    p->top.twenty.y = 81;
    p->top.six.x = 535;
    p->top.six.y = 271;
    p->top.three.x = 335;
    p->top.three.y = 364;
    p->top.eleven.x = 138;
    p->top.eleven.y = 260;

    //p->top.center.x = 320;
    //p->top.center.y = 262;

    p->src_points_top.push_back(p->top.twenty);  // 20
    p->src_points_top.push_back(p->top.six);     // 6
    p->src_points_top.push_back(p->top.three);   // 3
    p->src_points_top.push_back(p->top.eleven);  // 11
    //p->src_points_top.push_back(p->top.center);  // center


    p->right.twenty.x = 162;
    p->right.twenty.y = 339;
    p->right.six.x = 175;
    p->right.six.y = 168;
    p->right.three.x = 487;
    p->right.three.y = 234;
    p->right.eleven.x = 373;
    p->right.eleven.y = 370;

    //p->right.center.x = 308;
    //p->right.center.y = 297;

    p->src_points_right.push_back(p->right.twenty);  // 20
    p->src_points_right.push_back(p->right.six);     // 6
    p->src_points_right.push_back(p->right.three);   // 3
    p->src_points_right.push_back(p->right.eleven);  // 11
    //p->src_points_right.push_back(p->right.center);  // center


    p->left.twenty.x = 471;
    p->left.twenty.y = 329;
    p->left.six.x = 242;
    p->left.six.y = 355;
    p->left.three.x = 142;
    p->left.three.y = 200;
    p->left.eleven.x = 475;
    p->left.eleven.y = 144;

    //p->left.center.x = 340;
    //p->left.center.y = 258;

  

    p->src_points_left.push_back(p->left.twenty);  // 20
    p->src_points_left.push_back(p->left.six);     // 6
    p->src_points_left.push_back(p->left.three);   // 3
    p->src_points_left.push_back(p->left.eleven);  // 11
    //p->src_points_left.push_back(p->left.center);  // center

    /*points of the numbers 20,6,3,11 on the warped picture*/
    p->dst_points.push_back(Point2f(320, 40));  // 20
    p->dst_points.push_back(Point2f(520, 240)); // 6
    p->dst_points.push_back(Point2f(320, 440)); // 3
    p->dst_points.push_back(Point2f(120, 240)); // 11

    //p->dst_points.push_back(Point2f(420, 340)); // center





}





void calibration_match(cv::Mat img, cv::Mat ref, int CamId) {




    // Feature-Detektor (SIFT anstelle von ORB für bessere Ergebnisse)
    Ptr<SIFT> detector = cv::SIFT::create(2000);
    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Feature-Extraktion
    detector->detectAndCompute(img, Mat(), keypoints1, descriptors1);
    detector->detectAndCompute(ref, Mat(), keypoints2, descriptors2);

    // Matcher mit FLANN (Fast Library for Approximate Nearest Neighbors)
    FlannBasedMatcher flannMatcher(cv::makePtr<cv::flann::KDTreeIndexParams>(5)); // 5 statt Standard 4

    // Nur gute Matches beibehalten (Entfernung < 3*minDist oder maxDist)
    vector<DMatch> refined_matches;

    vector<vector<DMatch>> knn_matches;
    flannMatcher.knnMatch(descriptors1, descriptors2, knn_matches, 2);

    for (size_t i = 0; i < knn_matches.size(); i++) {
        if (knn_matches[i][0].distance < 0.7 * knn_matches[i][1].distance) {
            refined_matches.push_back(knn_matches[i][0]);
        }
    }

    // Sicherstellen, dass genügend gute Matches vorhanden sind
    if (refined_matches.size() < 4) {
        cerr << "not enough matches!" << endl;
    }

    // Punktpaare für die Homographie sammeln
    vector<Point2f> srcPoints, dstPoints;
    for (size_t i = 0; i < refined_matches.size(); i++) {
        srcPoints.push_back(keypoints1[refined_matches[i].queryIdx].pt);
        dstPoints.push_back(keypoints2[refined_matches[i].trainIdx].pt);
    }

    // Homographie mit RANSAC berechnen (Zufallsauswahl und Robustheit)
    Mat mask;
    Mat H = findHomography(srcPoints, dstPoints, RANSAC, 3.0, mask);    // 3.0

    if (H.empty()) {
        cerr << "homografics could not be computed!" << endl;
    }

    switch (CamId) {
    case TOP_CAM:
        cal.Homo.H_top = H;
        break;
    case RIGHT_CAM:
        cal.Homo.H_right = H;
        break;
    case LEFT_CAM:
        cal.Homo.H_left = H;
        break;

    default: break;
    }


}



void calibration_auto_cal(cv::Mat& top, cv::Mat& right, cv::Mat& left) {


    // --- 1. Bild laden und Vorverarbeiten ---

    //Mat img = imread(TOP_RAW_IMG_CAL, IMREAD_ANYCOLOR);
    //Mat img = imread(RIGHT_RAW_IMG_CAL, IMREAD_ANYCOLOR);
    // Mat img = imread(LEFT_RAW_IMG_CAL, IMREAD_ANYCOLOR);

    Mat top_ref = imread(TOP_REF, IMREAD_ANYCOLOR); // Perfekte Draufsicht
    Mat right_ref = imread(RIGHT_REF, IMREAD_ANYCOLOR); // Perfekte Draufsicht
    Mat left_ref = imread(LEFT_REF, IMREAD_ANYCOLOR); // Perfekte Draufsicht
   
    calibration_match(top, top_ref, TOP_CAM);
    calibration_match(right, right_ref, RIGHT_CAM);
    calibration_match(left, left_ref, LEFT_CAM);


    // Warping der Dartboard-Bilder in das ideale Zielbild
    Mat w_top, w_right, w_left;
    //warpPerspective(img, warped, H, ref.size());
    warpPerspective(top, w_top, cal.Homo.H_top, top_ref.size(), INTER_CUBIC, BORDER_REFLECT);
    warpPerspective(right, w_right, cal.Homo.H_right, right_ref.size(), INTER_CUBIC, BORDER_REFLECT);
    warpPerspective(left, w_left, cal.Homo.H_left, left_ref.size(), INTER_CUBIC, BORDER_REFLECT);


    dart_board_draw_sectors(w_top, TOP_CAM, 0, 0);
    dart_board_draw_sectors(w_right, RIGHT_CAM, 0, 0);
    dart_board_draw_sectors(w_left, LEFT_CAM, 0, 0);


    imshow("Top Auto Warp", w_top);
    imshow("Right Auto Warp", w_right);
    imshow("Left Auto Warp", w_left);

}




void calibration_ref_create(void) {

    
    /* define input image */
    Mat top_img = imread(TOP_TO_BE_REF, IMREAD_ANYCOLOR);
    Mat right_img = imread(RIGHT_TO_BE_REF, IMREAD_ANYCOLOR);
    Mat left_img = imread(LEFT_TO_BE_REF, IMREAD_ANYCOLOR);


    /*source points for top cam*/
    vector<Point2f> src_points_top;
    src_points_top.push_back(Point2f(470, 316)); // 20
    src_points_top.push_back(Point2f(243, 335)); // 6
    src_points_top.push_back(Point2f(149, 170)); // 3
    src_points_top.push_back(Point2f(490, 128)); // 11

    //src_points_top.push_back(Point2f(537, 238)); // 12-9-d 
    //src_points_top.push_back(Point2f(362, 350)); // 18-4-d
    //src_points_top.push_back(Point2f(154, 275)); // 15-2-d 
    //src_points_top.push_back(Point2f(296, 85)); // 16-7-d 
    //src_points_top.push_back(Point2f(326, 267)); // sbull-out->6 


    /*source points for right cam*/
    vector<Point2f> src_points_right;
    src_points_right.push_back(Point2f(470, 316)); // 20
    src_points_right.push_back(Point2f(243, 335)); // 6
    src_points_right.push_back(Point2f(149, 170)); // 3
    src_points_right.push_back(Point2f(490, 128)); // 11

    //src_points_right.push_back(Point2f(537, 238)); // 12-9-d 
    //src_points_right.push_back(Point2f(362, 350)); // 18-4-d
    //src_points_right.push_back(Point2f(154, 275)); // 15-2-d 
    //src_points_right.push_back(Point2f(296, 85)); // 16-7-d 
    //src_points_right.push_back(Point2f(326, 267)); // sbull-out->6 

    /*source points for left cam*/
    vector<Point2f> src_points_left;
    src_points_left.push_back(Point2f(470, 316)); // 20
    src_points_left.push_back(Point2f(243, 335)); // 6
    src_points_left.push_back(Point2f(149, 170)); // 3
    src_points_left.push_back(Point2f(490, 128)); // 11
    
    //src_points_left.push_back(Point2f(537, 238)); // 12-9-d 
    //src_points_left.push_back(Point2f(362, 350)); // 18-4-d
    //src_points_left.push_back(Point2f(154, 275)); // 15-2-d 
    //src_points_left.push_back(Point2f(296, 85)); // 16-7-d 
    //src_points_left.push_back(Point2f(326, 267)); // sbull-out->6 



    /* points of the numbers 20,6,3,11 on the warped picture */
    vector<Point2f> dst_points;
    dst_points.push_back(Point2f(320, 40)); // 20d
    dst_points.push_back(Point2f(520, 240));// 6d
    dst_points.push_back(Point2f(320, 440)); // 3d
    dst_points.push_back(Point2f(120, 240));// 11d
    /*
    // x = cos(theta)*r ; y = sin(theta)*r, r = 200, theta 45
    // 141x141 aon cricle --> 
    int x = (int)(left_img.cols / 2) - 141;
    int y = (int)(left_img.rows / 2) - 141;
    dst_points.push_back(Point2f(x, y)); // 12-9-d 
    x = (int)(left_img.cols / 2) + 141;
    y = (int)(left_img.rows / 2) - 141;
    dst_points.push_back(Point2f(x, y)); // 18-4-d
    x = (int)(left_img.cols / 2) + 141;
    y = (int)(left_img.rows / 2) + 141;
    dst_points.push_back(Point2f(x, y)); // 15-2-d 
    x = (int)(left_img.cols / 2) - 141;
    y = (int)(left_img.rows / 2) + 141;
    dst_points.push_back(Point2f(x, y)); // 16-7-d 

    //dst_points.push_back(Point2f(10, 240)); // sbull-out->6 
    */



    // Berechne die Homographie-Matrix
    Mat H_top = findHomography(src_points_top, dst_points, RANSAC, 3.0);
    Mat H_right = findHomography(src_points_right, dst_points, RANSAC, 3.0);
    Mat H_left = findHomography(src_points_left, dst_points, RANSAC, 3.0);

    // Transformiere das Bild
    Mat out_t, out_r, out_l;    

    warpPerspective(top_img, out_t, H_top, top_img.size(), INTER_CUBIC, BORDER_REFLECT);
    warpPerspective(right_img, out_r, H_right, right_img.size(), INTER_CUBIC, BORDER_REFLECT);
    warpPerspective(left_img, out_l, H_left, left_img.size(), INTER_CUBIC, BORDER_REFLECT);
    
    
    
    Mat write_t = out_t.clone();
    Mat write_r = out_r.clone();
    Mat write_l = out_l.clone();
    

    dart_board_draw_sectors(out_t, TOP_CAM, 0, 0);
    dart_board_draw_sectors(out_r, RIGHT_CAM, 0, 0);
    dart_board_draw_sectors(out_l, LEFT_CAM, 0, 0);
    // Ergebnis anzeigen
    imshow("Ref Image Top", out_t);
    imshow("Ref Image Right", out_r);
    imshow("Ref Image Left", out_l);
    waitKey(0);

    imwrite("top_ref.jpg", write_t);
    imwrite("right_ref.jpg", write_r);
    imwrite("left_ref.jpg", write_l);




}









void calibration_cal_src_points(cv::Mat& top, cv::Mat& right, cv::Mat& left) {

    static struct cal_s* p = &cal;




    /* create trackbars */
    Mat top_tb = Mat::zeros(100, 600, CV_8UC3);
    Mat right_tb = Mat::zeros(100, 600, CV_8UC3);
    Mat left_tb = Mat::zeros(100, 600, CV_8UC3);
    /* settings */
    top_tb.setTo(Scalar(50, 50, 50));  // background color gray 
    right_tb.setTo(Scalar(50, 50, 50));  // background color gray 
    left_tb.setTo(Scalar(50, 50, 50));  // background color gray 
    
    int image_width = top.cols;
    int image_height = top.rows;


    p->cal_win = TOP_CAM;

    imshow(TOP_CAM_CAL_TB, top_tb);



    createTrackbar("Twenty [x]", TOP_CAM_CAL_TB, NULL, image_width, on_trackbar_twenty_x, p);
    createTrackbar("Twenty [y]", TOP_CAM_CAL_TB, NULL, image_height, on_trackbar_twenty_y, p);
    createTrackbar("Six [x]", TOP_CAM_CAL_TB, NULL, image_width, on_trackbar_six_x, p);
    createTrackbar("Six [y]", TOP_CAM_CAL_TB, NULL, image_height, on_trackbar_six_y, p);
    createTrackbar("Three [x]", TOP_CAM_CAL_TB, NULL, image_width, on_trackbar_three_x, p);
    createTrackbar("Three [y]", TOP_CAM_CAL_TB, NULL, image_height, on_trackbar_three_y, p);
    createTrackbar("Eleven [x]", TOP_CAM_CAL_TB, NULL, image_width, on_trackbar_eleven_x, p);
    createTrackbar("Eleven [y]", TOP_CAM_CAL_TB, NULL, image_height, on_trackbar_eleven_y, p);

    //createTrackbar("Center [x]", TOP_CAM_CAL_TB, NULL, image_width, on_trackbar_center_x, p);
    //createTrackbar("Center [y]", TOP_CAM_CAL_TB, NULL, image_height, on_trackbar_center_y, p);


    setTrackbarPos("Twenty [x]", TOP_CAM_CAL_TB, p->top.twenty.x);
    setTrackbarPos("Twenty [y]", TOP_CAM_CAL_TB, p->top.twenty.y);
    setTrackbarPos("Six [x]", TOP_CAM_CAL_TB, p->top.six.x);
    setTrackbarPos("Six [y]", TOP_CAM_CAL_TB, p->top.six.y);
    setTrackbarPos("Three [x]", TOP_CAM_CAL_TB, p->top.three.x);
    setTrackbarPos("Three [y]", TOP_CAM_CAL_TB, p->top.three.y);
    setTrackbarPos("Eleven [x]", TOP_CAM_CAL_TB, p->top.eleven.x);
    setTrackbarPos("Eleven [y]", TOP_CAM_CAL_TB, p->top.eleven.y);

    //setTrackbarPos("Center [x]", TOP_CAM_CAL_TB, p->top.center.x);
    //setTrackbarPos("Center [y]", TOP_CAM_CAL_TB, p->top.center.y);


    cout << "calibrate top cam" << endl;
    Mat live_cal = top.clone();

    /* view diff, and wait for escape to quit img analysis */
    while ((waitKey(10) != 27) && running) {

        /* calibrate image */
        calibration_get_img(top, live_cal, TOP_CAM);

        dart_board_draw_sectors(live_cal, TOP_CAM, 0, 0);

        imshow("TOP CAM Live Calibration leave with [Esc]", live_cal);
       

    }

    /* calibrate image */
    calibration_get_img(top, live_cal, TOP_CAM);
    //imwrite("top_ref.jpg", live_cal);


    destroyWindow(TOP_CAM_CAL_TB);
    destroyWindow("TOP CAM Live Calibration leave with [Esc]");


    p->cal_win = RIGHT_CAM;

    imshow(RIGHT_CAM_CAL_TB, right_tb);

    createTrackbar("Twenty [x]", RIGHT_CAM_CAL_TB, NULL, image_width, on_trackbar_twenty_x, p);
    createTrackbar("Twenty [y]", RIGHT_CAM_CAL_TB, NULL, image_height, on_trackbar_twenty_y, p);
    createTrackbar("Six [x]", RIGHT_CAM_CAL_TB, NULL, image_width, on_trackbar_six_x, p);
    createTrackbar("Six [y]", RIGHT_CAM_CAL_TB, NULL, image_height, on_trackbar_six_y, p);
    createTrackbar("Three [x]", RIGHT_CAM_CAL_TB, NULL, image_width, on_trackbar_three_x, p);
    createTrackbar("Three [y]", RIGHT_CAM_CAL_TB, NULL, image_height, on_trackbar_three_y, p);
    createTrackbar("Eleven [x]", RIGHT_CAM_CAL_TB, NULL, image_width, on_trackbar_eleven_x, p);
    createTrackbar("Eleven [y]", RIGHT_CAM_CAL_TB, NULL, image_height, on_trackbar_eleven_y, p);

    //createTrackbar("Center [x]", RIGHT_CAM_CAL_TB, NULL, image_width, on_trackbar_center_x, p);
    //createTrackbar("Center [y]", RIGHT_CAM_CAL_TB, NULL, image_height, on_trackbar_center_y, p);

    setTrackbarPos("Twenty [x]", RIGHT_CAM_CAL_TB, p->right.twenty.x);
    setTrackbarPos("Twenty [y]", RIGHT_CAM_CAL_TB, p->right.twenty.y);
    setTrackbarPos("Six [x]", RIGHT_CAM_CAL_TB, p->right.six.x);
    setTrackbarPos("Six [y]", RIGHT_CAM_CAL_TB, p->right.six.y);
    setTrackbarPos("Three [x]", RIGHT_CAM_CAL_TB, p->right.three.x);
    setTrackbarPos("Three [y]", RIGHT_CAM_CAL_TB, p->right.three.y);
    setTrackbarPos("Eleven [x]", RIGHT_CAM_CAL_TB, p->right.eleven.x);
    setTrackbarPos("Eleven [y]", RIGHT_CAM_CAL_TB, p->right.eleven.y);

    //setTrackbarPos("Center [x]", RIGHT_CAM_CAL_TB, p->right.center.x);
    //setTrackbarPos("Center [y]", RIGHT_CAM_CAL_TB, p->right.center.y);


    cout << "calibrate right cam" << endl;
    live_cal = right.clone();

    /* view diff, and wait for escape to quit img analysis */
    while ((waitKey(10) != 27) && running) {

        /* calibrate image */
        calibration_get_img(right, live_cal, RIGHT_CAM);

        dart_board_draw_sectors(live_cal, RIGHT_CAM, 0, 0);

        imshow("RIGHT CAM Live Calibration leave with [Esc]", live_cal);


    }
    /* calibrate image */
    calibration_get_img(right, live_cal, RIGHT_CAM);
    //imwrite("right_ref.jpg", live_cal);

    destroyWindow(RIGHT_CAM_CAL_TB);
    destroyWindow("RIGHT CAM Live Calibration leave with [Esc]");

    
    p->cal_win = LEFT_CAM;

    imshow(LEFT_CAM_CAL_TB, left_tb);

    createTrackbar("Twenty [x]", LEFT_CAM_CAL_TB, NULL, image_width, on_trackbar_twenty_x, p);
    createTrackbar("Twenty [y]", LEFT_CAM_CAL_TB, NULL, image_height, on_trackbar_twenty_y, p);
    createTrackbar("Six [x]", LEFT_CAM_CAL_TB, NULL, image_width, on_trackbar_six_x, p);
    createTrackbar("Six [y]", LEFT_CAM_CAL_TB, NULL, image_height, on_trackbar_six_y, p);
    createTrackbar("Three [x]", LEFT_CAM_CAL_TB, NULL, image_width, on_trackbar_three_x, p);
    createTrackbar("Three [y]", LEFT_CAM_CAL_TB, NULL, image_height, on_trackbar_three_y, p);
    createTrackbar("Eleven [x]", LEFT_CAM_CAL_TB, NULL, image_width, on_trackbar_eleven_x, p);
    createTrackbar("Eleven [y]", LEFT_CAM_CAL_TB, NULL, image_height, on_trackbar_eleven_y, p);

    //createTrackbar("Center [x]", LEFT_CAM_CAL_TB, NULL, image_width, on_trackbar_center_x, p);
    //createTrackbar("Center [y]", LEFT_CAM_CAL_TB, NULL, image_height, on_trackbar_center_y, p);

    setTrackbarPos("Twenty [x]", LEFT_CAM_CAL_TB, p->left.twenty.x);
    setTrackbarPos("Twenty [y]", LEFT_CAM_CAL_TB, p->left.twenty.y);
    setTrackbarPos("Six [x]", LEFT_CAM_CAL_TB, p->left.six.x);
    setTrackbarPos("Six [y]", LEFT_CAM_CAL_TB, p->left.six.y);
    setTrackbarPos("Three [x]", LEFT_CAM_CAL_TB, p->left.three.x);
    setTrackbarPos("Three [y]", LEFT_CAM_CAL_TB, p->left.three.y);
    setTrackbarPos("Eleven [x]", LEFT_CAM_CAL_TB, p->left.eleven.x);
    setTrackbarPos("Eleven [y]", LEFT_CAM_CAL_TB, p->left.eleven.y);

    //setTrackbarPos("Center [x]", LEFT_CAM_CAL_TB, p->left.center.x);
    //setTrackbarPos("Center [y]", LEFT_CAM_CAL_TB, p->left.center.y);


    cout << "calibrate left cam" << endl;
    live_cal = left.clone();

    /* view diff, and wait for escape to quit img analysis */
    while ((waitKey(10) != 27) && running) {

        /* calibrate image */
        calibration_get_img(left, live_cal, LEFT_CAM);

        dart_board_draw_sectors(live_cal, LEFT_CAM, 0, 0);

        imshow("LEFT CAM Live Calibration leave with [Esc]", live_cal);


    }

    /* calibrate image */
    calibration_get_img(left, live_cal, LEFT_CAM);
    //imwrite("left_ref.jpg", live_cal);


    destroyWindow(LEFT_CAM_CAL_TB);
    destroyWindow("LEFT CAM Live Calibration leave with [Esc]");



    




}


void calibration_src_points_update(void) {



    cal.src_points_top[0] = cal.top.twenty;  // 20
    cal.src_points_top[1] = cal.top.six;     // 6
    cal.src_points_top[2] = cal.top.three;   // 3
    cal.src_points_top[3] = cal.top.eleven;  // 11

    //cal.src_points_top[4] = cal.top.center;  // center


    cal.src_points_right[0] = cal.right.twenty;  // 20
    cal.src_points_right[1] = cal.right.six;     // 6
    cal.src_points_right[2] = cal.right.three;   // 3
    cal.src_points_right[3] = cal.right.eleven;  // 11

    //cal.src_points_right[4] = cal.right.center;  // center


    cal.src_points_left[0] = cal.left.twenty;  // 20
    cal.src_points_left[1] = cal.left.six;     // 6
    cal.src_points_left[2] = cal.left.three;   // 3
    cal.src_points_left[3] = cal.left.eleven;  // 11


    //cal.src_points_left[4] = cal.left.center;  // center


}


/***
 *
 * calibration_get_img(void)
 *
 * Use test images to warp perspective of three different camera
 * angles
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
void calibration_get_img(void){

    /*using test images while not accessing the cameras*/
#if 0

    Mat img_top_ori = imread(TOP_RAW_IMG_CAL, IMREAD_ANYCOLOR);

    /*source points for top cam*/
    vector<Point2f> src_points_top;
    src_points_top.push_back(Point2f(334, 81));  // 20
    src_points_top.push_back(Point2f(535, 271)); // 6
    src_points_top.push_back(Point2f(335, 364)); // 3
    src_points_top.push_back(Point2f(138, 260)); // 11

    /*source points for left cam*/
    vector<Point2f> src_points_left;
    src_points_left.push_back(Point2f(471, 329));  // 20
    src_points_left.push_back(Point2f(242, 355)); // 6
    src_points_left.push_back(Point2f(142, 200)); // 3
    src_points_left.push_back(Point2f(475, 144)); // 11

    /*source points for right cam*/
    vector<Point2f> src_points_right;
    src_points_right.push_back(Point2f(162, 339));  // 20
    src_points_right.push_back(Point2f(175, 168)); // 6
    src_points_right.push_back(Point2f(487, 234)); // 3
    src_points_right.push_back(Point2f(373, 370)); // 11

    /*points of the numbers 20,6,3,11 on the warped picture*/
    vector<Point2f> dst_points;
    dst_points.push_back(Point2f(320, 40)); // 20
    dst_points.push_back(Point2f(520, 240));// 6
    dst_points.push_back(Point2f(320, 440)); // 3
    dst_points.push_back(Point2f(120, 240));// 11

    // Berechne die Homographie-Matrix
    Mat H = findHomography(src_points_top, dst_points, RANSAC);

    // Transformiere das Bild
    Mat output;
    warpPerspective(img_top_ori, output, H, Size(500, 500));

    // Ergebnis anzeigen
    imshow("Transformed Image", output);
    waitKey(0);


    /*source points for top cam*/
    vector<Point2f> src_points_top;
    src_points_top.push_back(Point2f(323, 80));  // 20
    src_points_top.push_back(Point2f(519, 271)); // 6
    src_points_top.push_back(Point2f(318, 358)); // 3
    src_points_top.push_back(Point2f(126, 254)); // 11

    /*source points for left cam*/
    vector<Point2f> src_points_left;
    src_points_left.push_back(Point2f(472, 317));  // 20
    src_points_left.push_back(Point2f(242, 336)); // 6
    src_points_left.push_back(Point2f(148, 173)); // 3
    src_points_left.push_back(Point2f(492, 130)); // 11

    /*source points for right cam*/
    vector<Point2f> src_points_right;
    src_points_right.push_back(Point2f(168, 339));  // 20
    src_points_right.push_back(Point2f(199, 169)); // 6
    src_points_right.push_back(Point2f(490, 243)); // 3
    src_points_right.push_back(Point2f(377, 372)); // 11

    /*points of the numbers 20,6,3,11 on the warped picture*/
    vector<Point2f> dst_points;
    dst_points.push_back(Point2f(320, 40)); // 20
    dst_points.push_back(Point2f(520, 240));// 6
    dst_points.push_back(Point2f(320, 440)); // 3
    dst_points.push_back(Point2f(120, 240));// 11

    /*tranform top*/
    Mat transform_matrix_top = getPerspectiveTransform(src_points_top, dst_points);
    Mat img_top_ori = imread("D:/darts/darts/images/test_img/top/top_1dart.jpg");
    Mat warped_img_top;
    warpPerspective(img_top_ori, warped_img_top, transform_matrix_top, img_top_ori.size());

    /*transform left*/
    Mat transform_matrix_left = getPerspectiveTransform(src_points_left, dst_points);
    Mat img_left_ori = imread("D:/darts/darts/images/test_img/left/left_1darts.jpg");
    Mat warped_img_left;
    warpPerspective(img_left_ori, warped_img_left, transform_matrix_left, img_left_ori.size());

    /*transform right*/
    Mat transform_matrix_right = getPerspectiveTransform(src_points_right, dst_points);
    Mat img_right_ori = imread("D:/darts/darts/images/test_img/right/right_1darts.jpg");
    Mat warped_img_right;
    warpPerspective(img_right_ori, warped_img_right, transform_matrix_right, img_right_ori.size());

    imshow("right", warped_img_right);
    imshow("Top", warped_img_top);
    imshow("left", warped_img_left);


    waitKey(0);

#endif 

}



/***
 *
 * calibration_get_img(cv::Mat& src, cv::Mat& dst, int ThreadId)
 * 
 * !Overloaded Function!
 *
 * Return calibrated / warped imaged based on given ThreadId
 *
 * @param:	cv::Mat& src --> raw image
 * @param:  cv::Mat& dst --> warped image 
 * @param   int ThreadId --> define camera perspective
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
void calibration_get_img(cv::Mat& src, cv::Mat& dst, int ThreadId) {

#if !USE_DYNAMIC_COEFFS
    /*source points for top cam*/
    vector<Point2f> src_points_top;
    src_points_top.push_back(Point2f(334, 81));  // 20
    src_points_top.push_back(Point2f(535, 271)); // 6
    src_points_top.push_back(Point2f(335, 364)); // 3
    src_points_top.push_back(Point2f(138, 260)); // 11

    /*source points for left cam*/
    vector<Point2f> src_points_left;
    src_points_left.push_back(Point2f(471, 329));  // 20
    src_points_left.push_back(Point2f(242, 355)); // 6
    src_points_left.push_back(Point2f(142, 200)); // 3
    src_points_left.push_back(Point2f(475, 144)); // 11

    /*source points for right cam*/
    vector<Point2f> src_points_right;
    src_points_right.push_back(Point2f(162, 339));  // 20
    src_points_right.push_back(Point2f(175, 168)); // 6
    src_points_right.push_back(Point2f(487, 234)); // 3
    src_points_right.push_back(Point2f(373, 370)); // 11

    /*points of the numbers 20,6,3,11 on the warped picture*/
    vector<Point2f> dst_points;
    dst_points.push_back(Point2f(320, 40)); // 20
    dst_points.push_back(Point2f(520, 240));// 6
    dst_points.push_back(Point2f(320, 440)); // 3
    dst_points.push_back(Point2f(120, 240));// 11

#else 

#if 0

    /* source points for top cam*/
    vector<Point2f> src_points_top = cal.src_points_top;

    /* source points for right cam*/
    vector<Point2f> src_points_right = cal.src_points_right;
    
    /* source points for left cam*/
    vector<Point2f> src_points_left = cal.src_points_left;

    /* points of the numbers 20,6,3,11 on the warped picture*/
    vector<Point2f> dst_points = cal.dst_points;
#endif 

#endif 

    if (ThreadId == TOP_CAM) {
        /*tranform top*/
        //Mat transform_matrix_top = getPerspectiveTransform(src_points_top, dst_points);
        //Mat H = findHomography(src_points_top, dst_points, RANSAC);
        //warpPerspective(src, dst, H, src.size());       
        warpPerspective(src, dst, cal.Homo.H_top, src.size());
    }
    else if (ThreadId == LEFT_CAM) {
        /*transform left*/
        //Mat transform_matrix_left = getPerspectiveTransform(src_points_left, dst_points);
        //Mat H = findHomography(src_points_left, dst_points, RANSAC);
        //warpPerspective(src, dst, H, src.size());
        warpPerspective(src, dst, cal.Homo.H_left, src.size());
    }
    else if (ThreadId == RIGHT_CAM) {
        /*transform right*/
        //Mat transform_matrix_right = getPerspectiveTransform(src_points_right, dst_points);
        //Mat H = findHomography(src_points_right, dst_points, RANSAC);
        //warpPerspective(src, dst, H, src.size());
        warpPerspective(src, dst, cal.Homo.H_right, src.size());
    }
    else {
        printf("error: unknown theradid");
    }

}



void on_trackbar_twenty_x(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
        case TOP_CAM:
            c->top.twenty.x = val;
            break;
        case RIGHT_CAM:
            c->right.twenty.x = val;
            break;
        case LEFT_CAM:
            c->left.twenty.x = val;
            break;

        default: break;
    }

    calibration_src_points_update();


}
void on_trackbar_twenty_y(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.twenty.y = val;
        break;
    case RIGHT_CAM:
        c->right.twenty.y = val;
        break;
    case LEFT_CAM:
        c->left.twenty.y = val;
        break;

    default: break;
    }


    calibration_src_points_update();
}
void on_trackbar_six_x(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.six.x = val;
        break;
    case RIGHT_CAM:
        c->right.six.x = val;
        break;
    case LEFT_CAM:
        c->left.six.x = val;
        break;

    default: break;
    }


    calibration_src_points_update();

}
void on_trackbar_six_y(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.six.y = val;
        break;
    case RIGHT_CAM:
        c->right.six.y = val;
        break;
    case LEFT_CAM:
        c->left.six.y = val;
        break;

    default: break;
    }

    calibration_src_points_update();

}
void on_trackbar_three_x(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.three.x = val;
        break;
    case RIGHT_CAM:
        c->right.three.x = val;
        break;
    case LEFT_CAM:
        c->left.three.x = val;
        break;

    default: break;
    }

    calibration_src_points_update();

}
void on_trackbar_three_y(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.three.y = val;
        break;
    case RIGHT_CAM:
        c->right.three.y = val;
        break;
    case LEFT_CAM:
        c->left.three.y = val;
        break;

    default: break;
    }

    calibration_src_points_update();

}
void on_trackbar_eleven_x(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.eleven.x = val;
        break;
    case RIGHT_CAM:
        c->right.eleven.x = val;
        break;
    case LEFT_CAM:
        c->left.eleven.x = val;
        break;

    default: break;
    }


    calibration_src_points_update();
}
void on_trackbar_eleven_y(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.eleven.y = val;
        break;
    case RIGHT_CAM:
        c->right.eleven.y = val;
        break;
    case LEFT_CAM:
        c->left.eleven.y = val;
        break;

    default: break;
    }

    calibration_src_points_update();

}

#if 0
void on_trackbar_center_x(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.center.x = val;
        break;
    case RIGHT_CAM:
        c->right.center.x = val;
        break;
    case LEFT_CAM:
        c->left.center.x = val;
        break;

    default: break;
    }

    calibration_src_points_update();
}


void on_trackbar_center_y(int val, void* arg) {

    struct cal_s* c = (struct cal_s*)(arg);

    switch (c->cal_win) {
    case TOP_CAM:
        c->top.center.y = val;
        break;
    case RIGHT_CAM:
        c->right.center.y = val;
        break;
    case LEFT_CAM:
        c->left.center.y = val;
        break;

    default: break;
    }

    calibration_src_points_update();
}
#endif 