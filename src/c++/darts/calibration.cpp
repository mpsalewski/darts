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
#include "dart_board.h"
#include "cams.h"

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/



/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/
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

    if (ThreadId == TOP_CAM) {
        /*tranform top*/
        Mat transform_matrix_top = getPerspectiveTransform(src_points_top, dst_points);
        //Mat img_top_ori = imread("D:/darts/darts/images/test_img/top/top_1dart.jpg");
        //Mat warped_img_top;
        warpPerspective(src, dst, transform_matrix_top, src.size());
    }
    else if (ThreadId == LEFT_CAM) {
        /*transform left*/
        Mat transform_matrix_left = getPerspectiveTransform(src_points_left, dst_points);
        //Mat img_left_ori = imread("D:/darts/darts/images/test_img/left/left_1darts.jpg");
        Mat warped_img_left;
        warpPerspective(src, dst, transform_matrix_left, src.size());
    }
    else if (ThreadId == RIGHT_CAM) {
        /*transform right*/
        Mat transform_matrix_right = getPerspectiveTransform(src_points_right, dst_points);
        //Mat img_right_ori = imread("D:/darts/darts/images/test_img/right/right_1darts.jpg");
        Mat warped_img_right;
        warpPerspective(src, dst, transform_matrix_right, src.size());
    }
    else {
        printf("error: unknown theradid");
    }

}



