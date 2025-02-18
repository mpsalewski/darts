/******************************************************************************
 *
 * image_proc.cpp
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
 *      This module is used for image processing of a dartboard to detect the
 *      position of the dart tip. Key elements include difference images, 
 *      binary edge images, as well as contour and intersection calculations.
******************************************************************************/



/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()
/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/flann.hpp>
#include "image_proc.h"
#include <opencv2/opencv.hpp>
#include "calibration.h"
#include "Sobel.h"
#include "HoughLine.h"
#include "cams.h"
#include "dart_board.h"
#include "globals.h"

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
#define SMOOTHING_KERNEL_SIZE 1
#define CROSS_POINT_INTENSITY_MIN 18

#define DIFF_IMG "01 Wait DIFF"

#define WINDOW_NAME_THRESHOLD "Thresh Windows"
#define TRACKBAR_NAME_BIN_THRESH "Bin"
#define TRACKBAR_NAME_DIFF_MIN_THRESH "Diff Min"

/* size for cross_point calc */
#define RAW_CAL_IMG_WIDTH 640       
#define RAW_CAL_IMG_HEIGHT 480

/* decide beetwen cluster analysis [1] and classic obeject detection [0] */
#define DO_PCA  1

/************************** local Structure ***********************************/
struct roi_last_s {
    RotatedRect top;
    RotatedRect right;
    RotatedRect left;
};

static struct img_proc_s {
    int bin_thresh = 31;                // parameter BIN_THRESH 
    int diff_min_thresh = 1.5e+5;       // parameter DIFF_MIN_THRESH
    float aspect_ratio_max = 0.34;
    float aspect_ratio_min = 0.01;
    float area_min = 350;
    float short_edge_max = 22;
    struct roi_last_s roi_last;
}img_proc;




/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/
/***
 *
 * img_proc_get_line(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs, std::string CamNameId)
 *
 * Image Processing Main Function
 * --> processes current and last image and returns main line through the tip
 * of the dart
 *
 *
 * @param:	cv::Mat& lastImg --> last Image 
 * @param:	cv::Mat& currentImg --> current Image
 * @param:  int ThreadId --> defines camera perspective
 * @param:  struct line_s* line --> return single line in Polar Coordinates
 * @param:  int show_imgs --> defines Image to be displayed
 * @param   std::string CamNameId --> Name displayed Windows
 *
 *
 * @return: int status 
 *
 *
 * @note:   This function is the kernel compoment of the Darts detection 
 *          System. This function works based on difference images and
 *          Image processing steps to isolated the Dart and detect the barrel
 *          position and draw a line through the barrel and the tip of the 
 *          Dart. After this function a intersection function is called 
 *          to get the intersection of the three camera perspectives
 *
 *
 * Example usage: None
 *
***/
int img_proc_get_line(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs, std::string CamNameId) {


    /* declare images */
    Mat cur;
    Mat cur_gray;
    //Mat cur_sharp;
    //Mat cur_sharp_gray;

    Mat last;
    Mat last_gray;
    //Mat last_sharp;
    //Mat last_sharp_gray;

    //Mat diff;
    Mat diff_gray;
    //Mat diff_sharp;
    //Mat diff_sharp_gray;

    //Mat sharp_after_diff;
    Mat sharp_after_diff_gray;

    Mat edge;
    Mat edge_bin;

    Mat houghSpace;
    Mat cur_line;


    /* clone images */
    cur = currentImg.clone();
    if (cur.empty()) {
        std::cout << "[ERROR] Current Image is empty" << endl;
        return EXIT_FAILURE;
    }

    last = lastImg.clone();
    if (last.empty()) {
        std::cout << "[ERROR] Last Image is empty" << endl;
        return EXIT_FAILURE;
    }

    /* noise reduction */
    cv::GaussianBlur(cur, cur, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);
    cv::GaussianBlur(last, last, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur_gray, COLOR_BGR2GRAY);
    cvtColor(last, last_gray, COLOR_BGR2GRAY);

    /* sharpen images */
    //img_proc_sharpen_img(cur, cur_sharp);
    //img_proc_sharpen_img(cur_gray, cur_sharp_gray);

    //harpenImage(last, last_sharp);
    //img_proc_sharpen_img(last_gray, last_sharp_gray);


    /* check difference */
    //absdiff(last, cur, diff);
    absdiff(last_gray, cur_gray, diff_gray);
    //absdiff(last_sharp, cur_sharp, diff_sharp);
    //absdiff(last_sharp_gray, cur_sharp_gray, diff_sharp_gray);


    /* sharpen images after difference */
    //img_proc_sharpen_img(diff, sharp_after_diff);
    img_proc_sharpen_img(diff_gray, sharp_after_diff_gray);

    /* edge image */
    //int thresh_top = 55;
    ip::sobelFilter(sharp_after_diff_gray, edge);
    //threshold(edge, edge_bin, BIN_THRESH, 255, THRESH_BINARY);    // fixed macro
    cv::threshold(edge, edge_bin, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar



/********************** under construction ***********************************/


#if DO_PCA

    /***
     * recursive cluster analysis
     * 1. find main roi (Dart) in Image and find main axis through cluster 
     * 2. define a new roi tight around the main axis and find a new main axis in this roi
    ***/

    /*** 
     * create this image for imshow(), whats in earlier versions has been just the edge_bin image
     * the name edge_bin_cont is "historical" result for compatibility, dont worry about it :)
    ***/
    Mat edge_bin_cont = edge_bin.clone();

   
    Mat cluster_img = edge_bin.clone();
    /* heavy noise reduction, dont care if dart gets blurry */
    GaussianBlur(cluster_img, cluster_img, Size(29, 29), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);
    threshold(cluster_img, cluster_img, 190, 255, THRESH_BINARY);
    /* close open contours --> the goal is to get an even more symmetric cluster */
    morphologyEx(cluster_img, cluster_img, MORPH_CLOSE, Mat::ones(5, 5, CV_8U));
    
    /* define roi size in the first run */
    int imgWidth = cluster_img.cols;
    int imgHeight = cluster_img.rows;
    int roiWidth = (2 * imgWidth) / 3;
    int roiHeight = (2 * imgHeight) / 3;

    /* create niew overlapping rois */
    vector<Rect> rois;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int x = (i * imgWidth) / 3;
            int y = (j * imgHeight) / 3;
            rois.push_back(Rect(x, y, roiWidth, roiHeight) & Rect(0, 0, imgWidth, imgHeight));
        }
    }

    /* get the roi with the most withe pixels --> find dart */
    Rect bestRoi;
    int maxWhitePixels = 0;

    for (const auto& roi : rois) {
        Mat roiImage = cluster_img(roi);
        int whitePixels = countNonZero(roiImage);

        if (whitePixels > maxWhitePixels) {
            maxWhitePixels = whitePixels;
            bestRoi = roi;
        }
    }

    if (maxWhitePixels == 0) {
        cout << "err: black screen" << endl;
        return -1;
    }

    /* extract pxiels from best roi */
    vector<Point> points_roi;
    for (int y = bestRoi.y; y < bestRoi.y + bestRoi.height; y++) {
        for (int x = bestRoi.x; x < bestRoi.x + bestRoi.width; x++) {
            if (cluster_img.at<uchar>(y, x) == 255) {
                points_roi.push_back(Point(x, y));
            }
        }
    }


    /* cluster erase */
    // add this at a later project stage after other new features hab been validated 
    //cluster_erase(cluster_img, ThreadId);

    /* pca */
    Mat data_roi(points_roi.size(), 2, CV_32F);    
    for (size_t i = 0; i < points_roi.size(); i++) {
        data_roi.at<float>(i, 0) = points_roi[i].x;
        data_roi.at<float>(i, 1) = points_roi[i].y;
    }
    PCA pca_roi(data_roi, Mat(), PCA::DATA_AS_ROW);

    /* main axis of dart */
    Vec2f mainAxis_roi(pca_roi.eigenvectors.row(0));

    /* calc centroid of cluster */
    Point2f centroid_roi(pca_roi.mean.at<float>(0, 0), pca_roi.mean.at<float>(0, 1));


    /* creat image */
    edge_bin_cont = cluster_img.clone();
    
    cvtColor(edge_bin_cont, edge_bin_cont, COLOR_GRAY2BGR);
    
    /* draw best roi */
    rectangle(edge_bin_cont, bestRoi, Scalar(0, 255, 0), 2);

    /* draw cluster main axis */
    drawLine(edge_bin_cont, centroid_roi, mainAxis_roi, Scalar(0, 255, 0));

    /* draw cluster centroid */
    circle(edge_bin_cont, centroid_roi, 5, Scalar(0, 255, 0), -1);


    /*** 
     * do second cluster analysis with optimized roi 
    ***/
    cluster_img = edge_bin.clone();
    /* heavy noise reduction, dont care if dart gets blurry */
    GaussianBlur(cluster_img, cluster_img, Size(29, 29), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);
    threshold(cluster_img, cluster_img, 190, 255, THRESH_BINARY);
    /* close open contours --> the goal is to get an even more symmetric cluster */
    morphologyEx(cluster_img, cluster_img, MORPH_CLOSE, Mat::ones(5, 5, CV_8U));

    /* size of rotated rect */
    float roiWidth2 = 30;       // ±15 pixel around axis, should fit barrel and flight 
    /* length of new roi is length of diagonal of old roi */
    float roiHeight2 = std::sqrt(bestRoi.width * bestRoi.width + bestRoi.height * bestRoi.height); 

    /* roatetd rect with old main axis as angle */
    RotatedRect rotatedROI(centroid_roi, Size2f(roiHeight2, roiWidth2), atan2(mainAxis_roi[1], mainAxis_roi[0]) * 180.0 / CV_PI);

    /* mask for roatetd rect */
    Mat mask = Mat::zeros(cluster_img.size(), CV_8UC1);
    Point2f vertices[4];
    rotatedROI.points(vertices);
    vector<Point> roiContour = { vertices[0], vertices[1], vertices[2], vertices[3] };
    fillConvexPoly(mask, roiContour, Scalar(255));

    /* extract pixels from new roi */
    vector<Point> roiPoints;
    for (int y = 0; y < cluster_img.rows; y++) {
        for (int x = 0; x < cluster_img.cols; x++) {
            if (mask.at<uchar>(y, x) == 255 && cluster_img.at<uchar>(y, x) == 255) {
                roiPoints.push_back(Point(x, y));
            }
        }
    }

    /* second pca in rotated roi */
    Mat roiData(roiPoints.size(), 2, CV_32F);
    for (size_t i = 0; i < roiPoints.size(); i++) {
        roiData.at<float>(i, 0) = roiPoints[i].x;
        roiData.at<float>(i, 1) = roiPoints[i].y;
    }
    PCA pca2(roiData, Mat(), PCA::DATA_AS_ROW);
    Point2f centroid2(pca2.mean.at<float>(0, 0), pca2.mean.at<float>(0, 1));
    Vec2f mainAxis2(pca2.eigenvectors.row(0));


    /* draw rotated roi */
    drawRotatedRect(edge_bin_cont, rotatedROI, Scalar(255, 0, 255));

    /* draw ne main axis and new centroid */
    drawLine(edge_bin_cont, centroid2, mainAxis2, Scalar(255, 0, 255));
    circle(edge_bin_cont, centroid2, 5, Scalar(255, 0, 255), -1);


    /*** 
     * get polar coordinates from final main axis 
    ***/
    float theta = 0;
    float r = 0;

    calculatePolarCoordinates(centroid2, mainAxis2, edge_bin, r, theta);

    //Mat pca_img = Mat::zeros(edge_bin.size(), CV_8UC3);
    
    //drawLine(pca_img, centroid2, mainAxis2, Scalar(255, 255, 255));
    //pca_img = Mat::zeros(edge_bin.size(), CV_8UC3);
    
    //drawLine(pca_img, centroid2, mainAxis2, Scalar(255, 0, 255));
    //circle(pca_img, centroid2, 5, Scalar(0, 255, 255), -1);
    //Mat edge_bin_cont = pca_img.clone();
    //ip::drawLine(edge_bin_cont, r, theta);   // Debug
    

    cur_line = cur.clone();
    ip::drawLine(cur_line, r, theta);
    ip::drawLine(edge_bin_cont, r, theta);

    /* return line values */
    line->r = r;
    line->theta = theta;


    /* save roatetd rect with new main axis as angle */
    RotatedRect rotatedROI_final(centroid2, Size2f(roiHeight2-100, roiWidth2-16), atan2(mainAxis2[1], mainAxis2[0]) * 180.0 / CV_PI);
    /* set last roi default zero */
    switch (ThreadId) {
    case TOP_CAM:
        img_proc.roi_last.top = rotatedROI_final;
        break;
    case RIGHT_CAM:
        img_proc.roi_last.right = rotatedROI_final;
        break;
    case LEFT_CAM:
        img_proc.roi_last.left = rotatedROI_final;
        break;
    default:
        break;
    }

    

/*****************************************************************************/




#else

    /* barrel detction (added at later project stage) */
    /***
     * idea is to find barrel cotour and delete flight contour and then draw a 
     * close fitting rectangle around the barrel and do edge detection with 
     * this drawn rectangle
    ***/

    /* everything thats outcommented is for performance reasons, but are images for debug and analysis */

    /* use the original, if contours does not work */
    Mat edge_bin_original = edge_bin.clone();

    /* draw box, bc darts contours could be open if out of image */
    cv::line(edge_bin, Point(0, 0), Point(0, edge_bin.rows - 1), Scalar(255, 255, 255), 1);
    cv::line(edge_bin, Point(0, 0), Point(edge_bin.cols - 1, 0), Scalar(255, 255, 255), 1);
    cv::line(edge_bin, Point(edge_bin.cols - 1, 0), Point(edge_bin.cols - 1, edge_bin.rows - 1), Scalar(255, 255, 255), 2);
    cv::line(edge_bin, Point(0, edge_bin.rows - 1), Point(edge_bin.cols - 1, edge_bin.rows - 1), Scalar(255, 255, 255), 2);



    /* close open contours */
    cv::morphologyEx(edge_bin, edge_bin, cv::MORPH_CLOSE, cv::Mat::ones(2, 2, CV_8U));
    //GaussianBlur(edge_bin, edge_bin, Size(3, 3), 0.3, 0.3);

    /* create this image for imshow(), whats in earlier versions has been just the edge_bin image */
    Mat edge_bin_cont = edge_bin.clone();
    cvtColor(edge_bin_cont, edge_bin_cont, COLOR_GRAY2BGR);

    /* find contour of dart */
    vector<vector<Point>> cont;
    vector<Vec4i> hier;
    findContours(edge_bin, cont, hier, RETR_TREE, CHAIN_APPROX_SIMPLE);

    /* draw cont */
    //Mat contoursImg = Mat::zeros(edge_bin.size(), CV_8UC3);
    for (size_t i = 0; i < cont.size(); i++) {
        drawContours(edge_bin_cont, cont, (int)i, Scalar(255, 255, 0), 1, LINE_8, hier, 0);
    }

    //Mat result = Mat::zeros(edge_bin.size(), CV_8UC3);
    //Mat cont_rect = Mat::zeros(edge_bin.size(), CV_8UC3);

    /* image fitted final contour --> just the barrel as rectangle --> this will be used for Hough */
    Mat cont_rect_fitted = Mat::zeros(edge_bin.size(), CV_8UC3);
    
    /* store all endpoints */
    vector<Point> allPoints;

    
    /* look throug the contours */
    for (size_t i = 0; i < cont.size(); i++) {
        /* rotating bounding box */
        RotatedRect rotatedRect = minAreaRect(cont[i]);

        /* calculate properties */
        float width = rotatedRect.size.width;
        float height = rotatedRect.size.height;
        float aspectRatio = (width < height) ? width / height : height / width;
        double area = contourArea(cont[i]);

        float short_edge = (width < height) ? width : height;

        /* criteria: narrow and elongated */
        /***
         * aspect ratio defines form of rect around contour 
         * area > x helps to ignore long small artefacts
         * 
        ***/
        float ar_last = 1;
        //if ((aspectRatio < 0.2) && (aspectRatio > 0.01) && (area > 450) && (short_edge < 20 )) {
        //cout << short_edge;
        if ((aspectRatio < img_proc.aspect_ratio_max) && (aspectRatio > img_proc.aspect_ratio_min) && (area > img_proc.area_min) && (short_edge < img_proc.short_edge_max)) {
            /* only take the smallest aspect ratio */
            if ((aspectRatio < ar_last)) {
                ar_last = aspectRatio;
                /* reset contour */
                allPoints.clear();
                drawContours(edge_bin_cont, cont, (int)i, Scalar(0, 255, 255), 1);
                /* calculate corners of ratating rectangle */
                Point2f points[4];
                rotatedRect.points(points);
                
                /* draw rot rect */
                for (int j = 0; j < 4; j++) {
                    
                    allPoints.push_back(points[j]);
                    //cv::line(cont_rect_fitted, points[j], points[(j + 1) % 4], Scalar(0, 255, 255), 1);
                    //line(result, points[j], points[(j + 1) % 4], Scalar(255, 0, 0), 1);
                }
            }

        }
    }



    /* calaculate just one rot rect which fits all other rects, their might be more than bc of shaft and barrel might be divided through its haptic */
    if (!allPoints.empty()) {


        RotatedRect enclosingRect = minAreaRect(allPoints);
        Point2f points_enc[4];
        enclosingRect.points(points_enc);

        /* draw rect */
        for (int j = 0; j < 4; j++) {
            cv::line(edge_bin_cont, points_enc[j], points_enc[(j + 1) % 4], Scalar(255, 0, 0), 2);
            cv::line(cont_rect_fitted, points_enc[j], points_enc[(j + 1) % 4], Scalar(255, 0, 0), 1);
        }
        /* */
        cvtColor(cont_rect_fitted, edge_bin, COLOR_BGR2GRAY);
        cv::threshold(edge_bin, edge_bin, 10, 255, THRESH_BINARY);
    }
    /* if there were no conts, which fitted criteria do normal edge detection */
    else {
        edge_bin = edge_bin_original;
    }

    /* to be compatible with older versions go on with the img proc with 'edge_bin' */

    //imshow("narrow contours", result);
    //imshow("edges", edge_bin);
    //imshow("contoura", contoursImg);
    //imshow("Fitted all", cont_rect_fitted);

    /* Calculate Hough transform */
    cur_line = cur.clone();
    ip::houghTransform(edge_bin, houghSpace);

    cv::GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);


    /* find 2 gloabl maxima */
    Mat houghSpaceClone = houghSpace.clone();
    /* Prepare Hough space image for display */
    houghSpace = 255 - houghSpace;				// Invert
    ip::drawHoughLineLabels(houghSpace);		// Axes

    Point houghMaxLocation;
    double r, theta;
    double r_avg = 0;
    double theta_avg = 0;
    /* find global maxima */
    int global_max = 2;
    for (int i = 0; i < global_max; i++) {
        minMaxLoc(houghSpaceClone, NULL, NULL, NULL, &houghMaxLocation);
        ip::houghSpaceToLine(
            Size(edge_bin.cols, edge_bin.rows),
            Size(houghSpace.cols, houghSpace.rows),
            houghMaxLocation.x, houghMaxLocation.y, r, theta);
        //ip::drawLine(cur_line, r, theta);
        //cout << "Debug x: " << houghMaxLocation.x << "\ty: " << houghMaxLocation.y << endl;

        // set max to zero
        int delete_size = 3;
        for (int dx = -delete_size; dx <= delete_size; ++dx) {
            for (int dy = -delete_size; dy <= delete_size; ++dy) {
                int nx = houghMaxLocation.x + dx; // neighbor in x-dir
                int ny = houghMaxLocation.y + dy; // neighbor in y-dir


                if (nx >= 0 && nx < houghSpaceClone.cols && ny >= 0 && ny < houghSpaceClone.rows) {
                    houghSpaceClone.at<uchar>(ny, nx) = 0;
                }
            }
        }
        //cout << r << "\t" << theta << endl;
        /* shift the edges exactly on the edge to be more centered in averaging */
        int shift_val = 2;
        if (r > shift_val) {
            if (theta < CV_PI / 2) {
                r = r + shift_val;
            }
            else {
                r = r - shift_val;
            }
        }
        else if (r < -shift_val) {
            if (theta < CV_PI / 2){
                r = r - shift_val;
            }
            else {
                r = r + shift_val;
            }
        }
        //cout << r << endl;
        ip::drawLine(edge_bin_cont, r, theta);   // Debug
        cv::circle(houghSpace, houghMaxLocation, 5, Scalar(0, 0, 255), 2);		// Global maximum
        /* averaging */
        //out << "Debug r: " << r << "\ttheta" << theta << endl;
        /* !watch out when delta_theta > 90° */
        if ((i > 0) && (fabs(theta_avg - theta) > (CV_PI / 2))) {
            r_avg = r_avg + (-r);   // toggle sign
            if (theta > 0) {
                theta_avg = theta_avg + (CV_PI - theta);
            }
            else {
                theta_avg = theta_avg + (-CV_PI - theta);
            }
        }
        else {
            r_avg += r; // / (double)global_max;
            theta_avg += theta; /// (double)global_max;
        }
    }
    r_avg = r_avg / global_max;
    theta_avg = theta_avg / global_max;



    //cout << r_avg << "\t" << theta_avg << endl;
    /* draw average line */
    ip::drawLine(cur_line, r_avg, theta_avg);

    /* return line values */
    line->r = r_avg;
    line->theta = theta_avg;
    //cout << "Debug r_avg: " << r_avg << "\ttheta_avg" << theta_avg << endl;

#endif


    /* create windows */
    if (show_imgs == SHOW_NO_IMAGES) {
        return EXIT_SUCCESS;
    }
    else if (show_imgs == SHOW_ALL_IMAGES) {

        /* curent image plots */
        string image_basic = string("Current Image (").append(CamNameId).append(" Cam)");
        string image_gray = string("Current Image Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_basic, cur);
        cv::imshow(image_gray, cur_gray);

        /* sharpend images */
        //string image_sharp = string("Image Sharp (").append(CamNameId).append(" Cam)");
        //string image_sharp_gray = string("Image Sharp Gray (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_sharp, cur_sharp);
        //cv::imshow(image_sharp_gray, cur_sharp_gray);

        /* difference images */
        //string image_diff_basic = string("Image Diff (").append(CamNameId).append(" Cam)");
        string image_diff_gray = string("Image Diff Gray (").append(CamNameId).append(" Cam)");
        //string image_diff_sharp = string("Image Diff Sharp (").append(CamNameId).append(" Cam)");
        //string image_diff_sharp_gray = string("Image Diff Sharp Gray (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_diff_basic, diff);
        cv::imshow(image_diff_gray, diff_gray);
        //cv::imshow(image_diff_sharp, diff_sharp);
        //cv::imshow(image_diff_sharp_gray, diff_sharp_gray);

        /* sharpened images after diff */
        //string image_sharp_diff = string("Image Sharpened After Diff (").append(CamNameId).append(" Cam)");
        string image_sharp_diff_gray = string("Image Sharpened After Diff Gray (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_sharp_diff, sharp_after_diff);
        cv::imshow(image_sharp_diff_gray, sharp_after_diff_gray);


        /* edge image */
        string image_edge = string("Edge Image (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge, edge);
        /* edge binary image */
        string image_edge_bin = string("Image Edge Bin (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_edge_bin, edge_bin);
        cv::imshow(image_edge_bin, edge_bin_cont);

        /* Hough transform (line images) */
        string image_orig = string("Image Orig with line (").append(CamNameId).append(" Cam)");
        cv::imshow(image_orig, cur_line);
        // redundant string image_edge = string("Edge Image ").append(CamNameId);
        string image_hspace = string("HoughSpace (").append(CamNameId).append(" Cam)");
        cv::imshow(image_hspace, houghSpace);


        return EXIT_SUCCESS;

    }
    else if (show_imgs == SHOW_SHORT_ANALYSIS) {

        /* Hough transform (line images) */
        string image_orig = string("1 Image Orig with line (").append(CamNameId).append(" Cam)");
        cv::imshow(image_orig, cur_line);
        //string wimg_write = image_orig.append(".jpg");
        //cv::imwrite(image_orig, cur_line);
        /* edge image */
        //string image_edge = string("2 Edge Image (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_edge, edge);
        /* edge binary image */
        string image_edge_bin = string("3 Image Edge Bin (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_edge_bin, edge_bin);
        cv::imshow(image_edge_bin, edge_bin_cont);
        // string img_write = image_edge_bin.append(".jpg");
        //cv::imwrite(img_write, edge_bin_cont);
        /* sharpened images after diff */
        //string image_sharp_diff = string("Image Sharpened After Diff (").append(CamNameId).append(" Cam)");
        //string image_sharp_diff_gray = string("4 Image Sharpened After Diff Gray (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_sharp_diff, sharp_after_diff);
        //cv::imshow(image_sharp_diff_gray, sharp_after_diff_gray);
        return EXIT_SUCCESS;
    }


    if (show_imgs == SHOW_IMG_LINE) {
        /* Hough transform (line images) */
        string image_orig = string("Image Orig with line (").append(CamNameId).append(" Cam)");
        cv::imshow(image_orig, cur_line);
        //cv::imwrite(image_orig, cur_line);
    }
    if (show_imgs == SHOW_EDGE_IMG) {
        /* edge image */
        string image_edge = string("Edge Image (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge, edge);
    }
    if (show_imgs == SHOW_EDGE_BIN) {
        /* edge binary image */
        string image_edge_bin = string("Image Edge Bin (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_edge_bin, edge_bin);
        cv::imshow(image_edge_bin, edge_bin_cont);
    }
    if (show_imgs == SHOW_SHARP_AFTER_DIFF) {
        /* sharpened images after diff */
        //string image_sharp_diff = string("Image Sharpened After Diff (").append(CamNameId).append(" Cam)");
        string image_sharp_diff_gray = string("Image Sharpened After Diff Gray (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_sharp_diff, sharp_after_diff);
        cv::imshow(image_sharp_diff_gray, sharp_after_diff_gray);
    }


    return EXIT_SUCCESS;

}



/******************************************************************************
 * Image Processing Help Functions
 ******************************************************************************/
 /***
  *
  * img_proc_sharpen_img(const cv::Mat& inputImage, cv::Mat& outputImage)
  *
  * Image Sharpening
  *
  *
  * @param: const cv::Mat& inputImage --> Image to be sharpened
  * @param: cv::Mat& outputImage --> sharpened Image 
  *          
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
void img_proc_sharpen_img(const cv::Mat& inputImage, cv::Mat& outputImage) {
    /* define sharpening kernel */
    /* Mat kernel = (Mat_<float>(3, 3) <<
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0);*/
    Mat kernel = (Mat_<float>(3, 3) <<
        0, -1, 0,
        -1, 11, -1,
        0, -1, 0);

    /* sharpen */
    filter2D(inputImage, outputImage, -1, kernel);
}



/* draw main axis */
void drawLine(cv::Mat& img, cv::Point p, cv::Vec2f dir, cv::Scalar color, int length) {
    cv::Point p1 = p + cv::Point(dir[0] * length, dir[1] * length);
    cv::Point p2 = p - cv::Point(dir[0] * length, dir[1] * length);
    cv::line(img, p1, p2, color, 2);
}

/* get polar coordinates (r, theta) from main axis (line) */
void calculatePolarCoordinates(cv::Point p, cv::Vec2f dir, cv::Mat& img, float& r, float& theta) {

    /* image center */
    cv::Point center(img.cols / 2, img.rows / 2);

    /* normal vector of line */
    cv::Vec2f normal(-dir[1], dir[0]);

    /* get angle theta from normal vector[-pi / 2 und pi / 2] */
    theta = std::atan2(normal[1], normal[0]);

    /* get distance r of line to image center */
    r = (p.x - center.x) * normal[0] + (p.y - center.y) * normal[1];

    /* shift angle [0, pi] */
    if (theta < 0) {
        theta += CV_PI;
        r = -r;
    }

}

/* extract roatetd rect from center and main axis */
cv::Mat getRotatedROI(const cv::Mat& img, cv::Point2f center, cv::Vec2f axis, int width, int height) {
    /* calc rot matrix */
    double angle = atan2(axis[1], axis[0]) * 180.0 / CV_PI;
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);

    /* rotate original image */
    cv::Mat rotatedImg;
    cv::warpAffine(img, rotatedImg, rotationMatrix, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));

    /* define new roi window */
    cv::Rect roiRect(center.x - width / 2, center.y - height / 2, width, height);

    /* boundaries */
    roiRect &= cv::Rect(0, 0, img.cols, img.rows);

    return rotatedImg(roiRect);
}


/* draw rotated rect */
void drawRotatedRect(cv::Mat& img, cv::RotatedRect rRect, cv::Scalar color) {
    cv::Point2f vertices[4];
    rRect.points(vertices);
    for (int i = 0; i < 4; i++) {
        cv::line(img, vertices[i], vertices[(i + 1) % 4], color, 2);
    }
}


/* 
 * function to erase double darts when following dart touched the dart before
 * and to detect and cluster contours based on PCA orientation 
 */
void cluster_erase(cv::Mat& image, int ThreadId) {

    RotatedRect roi;

    /* set last roi default zero */
    switch (ThreadId) {
    case TOP_CAM:
        roi = img_proc.roi_last.top;
        break;
    case RIGHT_CAM:
        roi = img_proc.roi_last.right;
        break;
    case LEFT_CAM:
        roi = img_proc.roi_last.left;
        break;
    default:
        break;
    }

    /* ro rect corners */
    cv::Point2f vertices[4];
    roi.points(vertices);


    std::vector<cv::Point> polygon;
    for (int i = 0; i < 4; i++) {
        polygon.push_back(vertices[i]);
    }

    /* reset last rot rect roi */
    fillConvexPoly(image, polygon, cv::Scalar(0, 0, 0));

    return;

#if 0
    /* clone input image to avoid modifying original */
    Mat cluster_img = image.clone();

    /* apply morphological closing to connect nearby pixels */
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(10, 10));
    morphologyEx(image, cluster_img, MORPH_CLOSE, kernel);

    /* find external contours */
    vector<vector<Point>> contours;
    findContours(cluster_img, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    /* filter contours based on area */
    vector<vector<Point>> filteredContours;
    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area > 300) {
            filteredContours.push_back(contour);
        }
    }

    cout << "num clusters: " << filteredContours.size() << endl;

    /* create an empty result image */
    Mat result = Mat::zeros(cluster_img.size(), CV_8UC3);

    /* calculate and draw PCA axes for each contour */
    for (const auto& contour : filteredContours) {
        if (contour.size() >= 5) { // at least 5 points needed for PCA
            Mat dataPts(contour.size(), 2, CV_32F);
            for (size_t i = 0; i < contour.size(); i++) {
                dataPts.at<float>(i, 0) = contour[i].x;
                dataPts.at<float>(i, 1) = contour[i].y;
            }

            /* perform PCA to find the main orientation */
            PCA pca(dataPts, Mat(), PCA::DATA_AS_ROW);
            //Point2f mean(pca.mean.at<float>(0, 0), pca.mean.at<float>(0, 1));
            //Point2f direction(pca.eigenvectors.at<float>(0, 0), pca.eigenvectors.at<float>(0, 1));
            Vec2f mainAxis(pca.eigenvectors.row(0));
            Point2f centroid(pca.mean.at<float>(0, 0), pca.mean.at<float>(0, 1));

            /* draw principal axis */
            drawLine(result, centroid, mainAxis, Scalar(255, 0, 255));

        }
    }



    /* display the result */
    imshow("Contour Axes", result);
    waitKey(0);
#endif 

}


/******************************************************************************
 * find cross crosspoints with grafic method 
******************************************************************************/
/***
  *
  * img_proc_get_cross_points(const cv::Mat& image, std::vector<cv::Point>& maxLocations)
  *
  * Grafic Method o find all intersection points in an image. Based  on adding
  * up pixels. Every Pixel value above pre-defined Thresh is an Intersection
  * 
  *
  *
  * @param: const cv::Mat& image --> Image with intersection
  * @param: std::vector<cv::Point>& maxLocations --> Intersections
  *
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
void img_proc_get_cross_points(const cv::Mat& image, std::vector<cv::Point>& maxLocations) {

    Mat image_gray;
    cvtColor(image, image_gray, COLOR_BGR2GRAY);

    /* look up cross points */
    for (int y = 0; y < image.rows; y++) {
        uchar* row = image_gray.ptr<uchar>(y);
        for (int x = 0; x < image_gray.cols; x++) {
            if (image_gray.at<uchar>(y, x) > CROSS_POINT_INTENSITY_MIN) {
                maxLocations.push_back(Point(x, y));
            }
        }
    }

}



/***
  *
  * img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p) 
  *
  * Grafic Method to find the midpoint of multiple intersections of three lines.
  *
  *
  *
  * @param: cv::Size frameSize --> Size of calibrated Images
  * @param: struct tripple_line_s* tri_line --> Input three lines in Polar 
  *         Coordinates
  * @param: cv::Point& cross_p --> return midpoint f intersections --> final 
  *         detection poont 
  *
  *
  *
  * @return: int status
  *
  *
  * @note:	None
  *
  *
  * Example usage: None
  *
 ***/
int img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p) {

    Mat frame = Mat::zeros(frameSize, CV_8UC3);


    ip::drawLine_light_add(frame, tri_line->line_top.r, tri_line->line_top.theta);
    ip::drawLine_light_add(frame, tri_line->line_right.r, tri_line->line_right.theta);
    ip::drawLine_light_add(frame, tri_line->line_left.r, tri_line->line_left.theta);



    imshow("Z Cross Line", frame);


    /* vector to store all cross points */
    vector<Point> maxLocations;

    /* find cross points */
    img_proc_get_cross_points(frame, maxLocations);

    int sumX = 0, sumY = 0;

    for (const Point& pt : maxLocations) {
        sumX += pt.x;
        sumY += pt.y;
        //cout << "x: " << pt.x << ", y: " << pt.y << endl;
    }

    if (maxLocations.size() == 0) {
        return EXIT_FAILURE;
    }

    /* midpoints */
    Point centerOfMass(sumX / maxLocations.size(), sumY / maxLocations.size());

    // Debug
    //cout << "midpoint: x: " << centerOfMass.x << ", y: " << centerOfMass.y << endl;

    cross_p.x = centerOfMass.x;
    cross_p.y = centerOfMass.y;

    return EXIT_SUCCESS;

}



/******************************************************************************
 * find cross crosspoints with math method
******************************************************************************/
/***
  *
  * img_proc_polar_to_cart(const cv::Mat& image, struct line_s l, struct line_cart_s& cart)
  *
  * transform Polar - Coordinates(r, theta) to Cartesian - Coordinates 
  *
  *
  * @param: const cv::Mat& image --> refered image
  * @param: struct line_s l --> Input line in Polar Coordinates
  * @param: struct line_cart_s& cart --> return line Cartesian Coordinates
  *
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
void img_proc_polar_to_cart(const cv::Mat& image, struct line_s l, struct line_cart_s& cart) {
    
    /* Transformation is copied from HoughLine.cpp */

    Point imgCenter(image.cols / 2, image.rows / 2);
    
    double cosine = cos(l.theta);
    double sine = sin(l.theta);
    // Line end points for "almost horizontal" lines
    if ((l.theta > 1.0 / 4.0 * CV_PI) && (l.theta < 3.0 / 4.0 * CV_PI)) {
        int x0 = 0;
        int x1 = image.cols - 1;
        int xc0 = x0 - image.cols / 2;
        int xc1 = x1 - image.cols / 2;
        int yc0 = (int)((l.r - xc0 * cosine) / sine);
        int yc1 = (int)((l.r - xc1 * cosine) / sine);

        cart.p0 = Point(x0, yc0 + imgCenter.y);
        cart.p1 = Point(x1, yc1 + imgCenter.y);
    }
    // Line end points for "almost vertical" lines
    else {
        int y0 = 0;
        int y1 = image.rows - 1;
        int yc0 = y0 - image.rows / 2;
        int yc1 = y1 - image.rows / 2;
        int xc0 = (int)((l.r - yc0 * sine) / cosine);
        int xc1 = (int)((l.r - yc1 * sine) / cosine);

        cart.p0 = Point(xc0 + imgCenter.x, y0);
        cart.p1 = Point(xc1 + imgCenter.x, y1);
    }
    return;
}


/***
  *
  * img_proc_find_intersection(const line_cart_s& line1, const line_cart_s& line2, cv::Point& intersection)
  *
  * get math intersection of two lines (cartesian coordinates)
  *
  *
  * @param: const line_cart_s& line1 --> input line 01
  * @param: const line_cart_s& line2 --> input line 02
  * @param: cv::Point& intersection --> calculated intersection; set to 
  *         (-66666,-66666) on error    
  *
  *
  *
  * @return: bool status
  *
  *
  * @note:	None
  *
  *
  * Example usage: None
  *
 ***/
bool img_proc_find_intersection(const line_cart_s& line1, const line_cart_s& line2, cv::Point& intersection) {
    
    /* first line */
    float x0_1 = line1.p0.x, y0_1 = line1.p0.y;
    float x1_1 = line1.p1.x, y1_1 = line1.p1.y;
    
    /* second line */
    float x0_2 = line2.p0.x, y0_2 = line2.p0.y;
    float x1_2 = line2.p1.x, y1_2 = line2.p1.y;

    /* calc params for equation */
    float A1 = y1_1 - y0_1;
    float B1 = x0_1 - x1_1;
    float C1 = x1_1 * y0_1 - x0_1 * y1_1;

    float A2 = y1_2 - y0_2;
    float B2 = x0_2 - x1_2;
    float C2 = x1_2 * y0_2 - x0_2 * y1_2;

    /* calc determinant */
    float det = A1 * B2 - A2 * B1;

    /* if determinant == 0 --> no cross point; set unrealistic point */
    if (det == 0) {
        intersection.x = -66666;
        intersection.y = -66666;
        return false;
    }
    /* Note: usage of this macro at this point makes it easy but not generic anymore for other frame sizes */
    if ((intersection.x < 0) || (intersection.x > RAW_CAL_IMG_WIDTH)) {
        intersection.x = -66666;
        intersection.y = -66666;
        return false;
    }
    if ((intersection.y < 0) || (intersection.y > RAW_CAL_IMG_HEIGHT)) {
        intersection.x = -66666;
        intersection.y = -66666;
        return false;
    }

    /* intersectio coordinates */
    intersection.x = (int)((B1 * C2 - B2 * C1) / det);
    intersection.y = (int)((A2 * C1 - A1 * C2) / det);

    return true;
}



/***
  *
  * img_proc_calculate_midpoint(const cv::Point& p1, const cv::Point& p2, const cv::Point& p3)
  *
  * get math center of three intersection points
  *
  *
  * @param: const cv::Point& p1 --> intersection point
  * @param: const cv::Point& p2 --> intersection point
  * @param: const cv::Point& p3 --> intersection point
  *
  *
  * @return: cv::Point --> midpoint of 3 intersection
  *
  *
  * @note:	None
  *
  *
  * Example usage: None
  *
 ***/
cv::Point img_proc_calculate_midpoint(cv::Point& p1, cv::Point& p2, cv::Point& p3){
    Point midpoint;

    /***
     * calculate distance between each point; 
     * if there were almost parallel lines due to unlucky cam angle, one point might be far away
     * two avoid other errors this is done max. ones (if, else if,...)
    ***/
    double eps = 120.0;
    if ((norm(p1 - p2) > eps) && (norm(p1 - p3) > eps)) {
        p1.x = -66666;
        p1.y = -66666;
    }
    else if ((norm(p2 - p1) > eps) && (norm(p2 - p3) > eps)) {
        p2.x = -66666;
        p2.y = -66666;
    }
    else if ((norm(p3 - p1) > eps) && (norm(p3 - p2) > eps)) {
        p3.x = -66666;
        p3.y = -66666;
    }

    /* all points are valid */
    if (!(p1 == Point(-66666, -66666)) && !(p2 == Point(-66666, -66666)) && !(p3 == Point(-66666, -66666))) {
        midpoint.x = (p1.x + p2.x + p3.x) / 3;
        midpoint.y = (p1.y + p2.y + p3.y) / 3;
        return midpoint;
    }


    /* error handling */
    /* all points are invalid */
    if ((p1 == Point(-66666, -66666)) && (p2 == Point(-66666, -66666)) && (p3 == Point(-66666, -66666))) {
        /* just return invalid point */
        return p1;
    }
    /* two points are invalid; return only valid point */
    else if ((p1 == Point(-66666, -66666)) && (p2 == Point(-66666, -66666))) {
        return p3;
    }
    else if ((p1 == Point(-66666, -66666)) && (p3 == Point(-66666, -66666))) {
        return p2;
    }
    else if ((p2 == Point(-66666, -66666)) && (p3 == Point(-66666, -66666))) {
        return p1;
    }
    /* one point is invalid; return intersection between the two valid points */
    else if (p1 == Point(-66666, -66666)) {
        midpoint.x = (p2.x + p3.x) / 2;
        midpoint.y = (p2.y + p3.y) / 2;
        return midpoint;
    }
    else if (p2== Point(-66666, -66666)) {
        midpoint.x = (p1.x + p3.x) / 2;
        midpoint.y = (p1.y + p3.y) / 2;
        return midpoint;
    }
    else if (p3 == Point(-66666, -66666)) {
        midpoint.x = (p1.x + p2.x) / 2;
        midpoint.y = (p1.y + p2.y) / 2;
        return midpoint;
    }
}


/***
  *
  * img_proc_calculate_midpoint(const cv::Point& p1, const cv::Point& p2, const cv::Point& p3)
  *
  * 
  * Call this function with Polar Coordinates and do math computation of
  * intersections of three lines.
  *
  *
  * @param: cv::Size frameSize --> refered image size
  * @param: struct tripple_line_s* --> 3 lines in Polar Coordinates
  * @param: cv::Point& cross_p --> final intersection point 
  *
  *
  * @return: int status
  *
  *
  * @note:  Displays Intersection Image as "Z Line Intersection"
  *
  *
  * Example usage: None
  *
 ***/
int img_proc_cross_point_math(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p) {

    /* cartesian tripple lines */
    struct tri_line_cart_s tlk;
    /* intersection points */
    Point intersection1, intersection2, intersection3;

    /* show intersection images */
    Mat frame = Mat::zeros(frameSize, CV_8UC3);

    /* visualize; has nothig to do with intersection computation */
    ip::drawLine_light_add(frame, tri_line->line_top.r, tri_line->line_top.theta);
    ip::drawLine_light_add(frame, tri_line->line_right.r, tri_line->line_right.theta);
    ip::drawLine_light_add(frame, tri_line->line_left.r, tri_line->line_left.theta);

    
    /* transform coordinates */
    img_proc_polar_to_cart(frame, tri_line->line_top, tlk.top);
    img_proc_polar_to_cart(frame, tri_line->line_right, tlk.right);
    img_proc_polar_to_cart(frame, tri_line->line_left, tlk.left);

    /* get intersections */
    img_proc_find_intersection(tlk.top, tlk.right, intersection1);
    img_proc_find_intersection(tlk.top, tlk.left, intersection2);
    img_proc_find_intersection(tlk.left, tlk.right, intersection3);

    /* get midpoint */
    cross_p = img_proc_calculate_midpoint(intersection1, intersection2, intersection3);

    /* draw interections */
    //circle(image, intersection1, 5, Scalar(0, 0, 255), 1);
    //circle(image, intersection2, 5, Scalar(0, 255, 0), 1);
    //circle(image, intersection3, 5, Scalar(255, 0, 0), 1);
    
    /* draw midpoint*/
    cv::circle(frame, cross_p, 8, Scalar(255, 255, 0), 1.5); 

    // Debug 
    //cout << "midpoint: x: " << centerOfMass.x << ", y: " << centerOfMass.y << endl;
    dart_board_draw_sectors(frame, TOP_CAM, 0, 0);
    imshow("Z Line Intersection", frame);
    //cv::imwrite("lines_intersection_math.jpg", frame);
    
    return EXIT_SUCCESS;

}


/******************************************************************************
 * Difference Functions 
******************************************************************************/
/***
  *
  * img_proc_diff_check(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId)
  *
  *
  * Return status if there is a significant difference between two images
  *
  *
  * @param: cv::Mat& last_f --> last Image
  * @param: cv::Mat& cur_f --> current Image
  * @param: int ThreadId --> define camera perspective
  *
  *
  * @return: int status
  *
  *
  * @note:  Displays Intersection Image as "Z Line Intersection"
  *
  *
  * Example usage: None
  *
 ***/
int img_proc_diff_check(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId) {

    /* bin img threshold */
    //int thresh = 55;

    /* pixel sum*/
    Scalar p_sum;

    /* declare images */
    Mat cur, last, diff;

    /* clone images */
    cur = cur_f.clone();
    if (cur.empty()) {
        std::cout << "[ERROR] Current Image is empty" << endl;
        return EXIT_FAILURE;
    }

    last = last_f.clone();
    if (last.empty()) {
        std::cout << "[ERROR] Last Image is empty" << endl;
        return EXIT_FAILURE;
    }

    /* noise reduction */
    cv::GaussianBlur(cur, cur, Size(9, 9), 1.25, 1.25);
    cv::GaussianBlur(last, last, Size(9, 9), 1.25, 1.25);

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur, COLOR_BGR2GRAY);
    cvtColor(last, last, COLOR_BGR2GRAY);


    /* check difference */
    absdiff(last, cur, diff);

    /* sharpen images after difference */
    img_proc_sharpen_img(diff, diff);

    /* edge image */
    ip::sobelFilter(diff, diff);
    //threshold(diff, diff, BIN_THRESH, 255, THRESH_BINARY);    // fixed macro
    cv::threshold(diff, diff, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

    imshow(DIFF_IMG, diff);
    
    /* sum up all pixel */
    p_sum = sum(diff);
    //cout << "sum of pixel: " << p_sum[0] << endl;
    //if (p_sum[0]>DIFF_MIN_THRESH) { // fixed macro
    if (p_sum[0]>img_proc.diff_min_thresh) { 
        return IMG_DIFFERENCE;
    }
    else {
        return IMG_NO_DIFFERENCE;
    }
      

}


/******************************************************************************
 * Calibration of Parameters
******************************************************************************/
/***
  *
  * img_proc_diff_check_cal(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId, int* pixel_sum, bool show)
  *
  *
  * Calibrate Difference Image Parameters
  *
  *
  * @param: cv::Mat& last_f --> last Image
  * @param: cv::Mat& cur_f --> current Image
  * @param: int ThreadId --> define camera perspective
  * @param: bool show --> debug ouput
  *
  *
  * @return: int status
  *
  *
  * @note:  
  *
  *
  * Example usage: None
  *
 ***/
int img_proc_diff_check_cal(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId, int* pixel_sum, bool show) {

    /* bin img threshold */
    //int thresh = 55;

    /* pixel sum*/
    Scalar p_sum;

    /* declare images */
    Mat cur, last, diff;

    /* clone images */
    cur = cur_f.clone();
    if (cur.empty()) {
        std::cout << "[ERROR] Current Image is empty" << endl;
        return EXIT_FAILURE;
    }

    last = last_f.clone();
    if (last.empty()) {
        std::cout << "[ERROR] Last Image is empty" << endl;
        return EXIT_FAILURE;
    }

    /* noise reduction */
    cv::GaussianBlur(cur, cur, Size(9, 9), 1.25, 1.25);
    cv::GaussianBlur(last, last, Size(9, 9), 1.25, 1.25);

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur, COLOR_BGR2GRAY);
    cvtColor(last, last, COLOR_BGR2GRAY);


    /* check difference */
    absdiff(last, cur, diff);

    /* sharpen images after difference */
    img_proc_sharpen_img(diff, diff);

    /* edge image */
    ip::sobelFilter(diff, diff);
    //threshold(diff, diff, BIN_THRESH, 255, THRESH_BINARY);    // fixed macro
    cv::threshold(diff, diff, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

    imshow(DIFF_IMG, diff);

    /* sum up all pixel */
    p_sum = sum(diff);
    if (show)
        cout << "sum of pixel: " << p_sum[0] << endl;
    
    *pixel_sum = (int)(p_sum[0]);
    //if (p_sum[0]>DIFF_MIN_THRESH) { // fixed macro
    if (p_sum[0] > img_proc.diff_min_thresh) { 
        return IMG_DIFFERENCE;
    }
    else {
        return IMG_NO_DIFFERENCE;
    }


}

/* Function to compute and visualize the correlation between two images */
void computeAndShowCorrelation(const cv::Mat& img1, const cv::Mat& img2) {
    /* Check if images are loaded properly */
    if (img1.empty() || img2.empty()) {
        std::cerr << "Error: One or both images could not be loaded!" << std::endl;
        return;
    }

    /* Convert images to grayscale if they are not already */
    cv::Mat gray1, gray2;
    if (img1.channels() == 3) {
        cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
    }
    else {
        gray1 = img1.clone();
    }
    if (img2.channels() == 3) {
        cv::cvtColor(img2, gray2, cv::COLOR_BGR2GRAY);
    }
    else {
        gray2 = img2.clone();
    }

    /* Resize second image to match the first image's size */
    cv::resize(gray2, gray2, gray1.size());

    /* Convert to double precision for calculations */
    cv::Mat img1_double, img2_double;
    gray1.convertTo(img1_double, CV_64F);
    gray2.convertTo(img2_double, CV_64F);

    /* Calculate mean and standard deviation */
    cv::Scalar mean1, mean2, stddev1, stddev2;
    cv::meanStdDev(img1_double, mean1, stddev1);
    cv::meanStdDev(img2_double, mean2, stddev2);

    /* Compute correlation matrix */
    cv::Mat diff1 = img1_double - mean1[0];
    cv::Mat diff2 = img2_double - mean2[0];
    cv::Mat correlationMap = diff1.mul(diff2);

    /* Normalize the correlation map for visualization */
    double minVal, maxVal;
    cv::minMaxLoc(correlationMap, &minVal, &maxVal);
    cv::Mat normalizedCorrelationMap;
    correlationMap.convertTo(normalizedCorrelationMap, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));

    /* Apply a color map for better visualization */
    cv::Mat heatmap;
    cv::applyColorMap(normalizedCorrelationMap, heatmap, cv::COLORMAP_JET);

    /* Define scanning parameters */
    int rectWidth = 50;  // Fixed width
    int rectMaxLength = img1.cols / 2; // Maximum length is half of image width
    int stepSize = 20;  // Step size for moving the rectangle
    int angleStep = 10; // Step size for angles

    cv::RotatedRect bestRect;
    double minCorrelation = std::numeric_limits<double>::max();

    /* Scan the image with rotated rectangles */
    for (int y = 0; y < img1.rows; y += stepSize) {
        for (int x = 0; x < img1.cols; x += stepSize) {
            for (int angle = 0; angle < 180; angle += angleStep) {
                /* Define rotated rectangle */
                cv::RotatedRect rect(cv::Point2f(x, y), cv::Size2f(rectMaxLength, rectWidth), angle);

                /* Get bounding box to extract ROI */
                cv::Rect boundingBox = rect.boundingRect();

                /* Ensure bounding box is within image limits */
                if (boundingBox.x >= 0 && boundingBox.y >= 0 &&
                    boundingBox.x + boundingBox.width < img1.cols &&
                    boundingBox.y + boundingBox.height < img1.rows) {

                    /* Extract ROI */
                    cv::Mat roi1 = img1_double(boundingBox);
                    cv::Mat roi2 = img2_double(boundingBox);

                    /* Compute mean and standard deviation in ROI */
                    cv::Scalar meanRoi1, meanRoi2, stddevRoi1, stddevRoi2;
                    cv::meanStdDev(roi1, meanRoi1, stddevRoi1);
                    cv::meanStdDev(roi2, meanRoi2, stddevRoi2);

                    /* Compute covariance */
                    cv::Mat diffRoi1 = roi1 - meanRoi1[0];
                    cv::Mat diffRoi2 = roi2 - meanRoi2[0];
                    double covariance = cv::sum(diffRoi1.mul(diffRoi2))[0] / (roi1.total() - 1);

                    /* Compute correlation */
                    double correlation = covariance / (stddevRoi1[0] * stddevRoi2[0]);

                    /* Update best rectangle if correlation is lower */
                    if (correlation < minCorrelation) {
                        minCorrelation = correlation;
                        bestRect = rect;
                    }
                }
            }
        }
    }

    /* Draw the best-matching rotated rectangle */
    if (minCorrelation < std::numeric_limits<double>::max()) {
        cv::Point2f rectPoints[4];
        bestRect.points(rectPoints);
        for (int i = 0; i < 4; i++) {
            cv::line(heatmap, rectPoints[i], rectPoints[(i + 1) % 4], cv::Scalar(0, 255, 0), 3);
        }
        std::cout << "Lowest Correlation Value: " << minCorrelation << std::endl;
    }
    else {
        std::cout << "No valid low-correlation region found!" << std::endl;
    }

    /* Compute overall correlation coefficient */
    double covariance = cv::sum(correlationMap)[0] / (img1_double.total() - 1);
    double overallCorrelation = covariance / (stddev1[0] * stddev2[0]);

    /* Display results */
    std::cout << "Overall Correlation Coefficient: " << overallCorrelation << std::endl;
    cv::imshow("Image 1", gray1);
    cv::imshow("Image 2", gray2);
    cv::imshow("Correlation Heatmap with ROI", heatmap);

    /* Wait for user interaction */
    cv::waitKey(0);
}



/***
  *
  * img_proc_calibration(cv::Mat& raw_top, cv::Mat& raw_right, cv::Mat& raw_left, cv::Mat& dart_top, cv::Mat& dart_right, cv::Mat& dart_left)
  *
  *
  * Calibrate Difference Image Parameters
  *
  *
  * @param: cv::Mat& raw_top --> raw Dart Board Image (Top)
  * @param: cv::Mat& raw_right --> raw Dart Board Image (Right)
  * @param: cv::Mat& raw_left --> raw Dart Board Image (Left)
  * @param: cv::Mat& dart_top --> Dart Board Image with Dart for Cal (Top)
  * @param: cv::Mat& dart_top --> Dart Board Image with Dart for Cal (Right)
  * @param: cv::Mat& dart_top --> Dart Board Image with Dart for Cal (Left)
  *
  *
  * @return: void 
  *
  *
  * @note:
  *
  *
  * Example usage: None
  *
 ***/
void img_proc_calibration(cv::Mat& raw_top, cv::Mat& raw_right, cv::Mat& raw_left, cv::Mat& dart_top, cv::Mat& dart_right, cv::Mat& dart_left) {

    Mat frame;
    int key = 0;
    int p_sum_last = 10e+6;
    int p_sum = 0;
    string input;

    frame = Mat::zeros(200, 600, CV_8UC3);
    /* settings */
    frame.setTo(Scalar(50, 50, 50));  // background color gray 
    imshow(WINDOW_NAME_THRESHOLD, frame);

    createTrackbar(TRACKBAR_NAME_BIN_THRESH, WINDOW_NAME_THRESHOLD, NULL, 255, on_trackbar_bin_thresh, &img_proc);
    createTrackbar(TRACKBAR_NAME_DIFF_MIN_THRESH, WINDOW_NAME_THRESHOLD, NULL, 200 , on_trackbar_diff_min_thresh, &img_proc);
    createTrackbar("AR_MAX", WINDOW_NAME_THRESHOLD, NULL, 255, on_trackbar_aspect_ratio_max, &img_proc);
    createTrackbar("AR_MIN", WINDOW_NAME_THRESHOLD, NULL, 255, on_trackbar_aspect_ratio_min, &img_proc);
    createTrackbar("AREA_MIN", WINDOW_NAME_THRESHOLD, NULL, 255, on_trackbar_area_min, &img_proc);
    createTrackbar("W_MAX", WINDOW_NAME_THRESHOLD, NULL, 200, on_trackbar_short_edge_max, &img_proc);


    setTrackbarPos(TRACKBAR_NAME_BIN_THRESH, WINDOW_NAME_THRESHOLD, BIN_THRESH);
    setTrackbarPos(TRACKBAR_NAME_DIFF_MIN_THRESH, WINDOW_NAME_THRESHOLD, (int)(DIFF_MIN_THRESH/1e+4));
    setTrackbarPos("AR_MAX", WINDOW_NAME_THRESHOLD, 34);
    setTrackbarPos("AR_MIN", WINDOW_NAME_THRESHOLD, 1);
    setTrackbarPos("AREA_MIN", WINDOW_NAME_THRESHOLD, 35);
    setTrackbarPos("W_MAX", WINDOW_NAME_THRESHOLD,22);

    
    cout << "go through cams with enter; adjust bin thresh with trackbar; leave with [Esc]" << endl;
    /* view diff, and wait for escape to quit img analysis */
    while ((waitKey(10) != 27) && running){

        while ((waitKey(10) != 13) && running){
            img_proc_diff_check_cal(raw_top, dart_top, TOP_CAM, &p_sum, false);
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        
        while ((waitKey(10) != 13) && running) {
            img_proc_diff_check_cal(raw_right, dart_right, RIGHT_CAM, &p_sum, false);
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        while ((waitKey(10) != 13) && running) {
            img_proc_diff_check_cal(raw_left, dart_left, LEFT_CAM, &p_sum, false);
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        /* hit enter to view images again and [Esc] to leave calibration */
        while ((key != 13) && (key != 27) && running) {
            key = waitKey(10);
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        if ((key == 27))
            break;
        else
            key = 0;

    }

    if (!running)
        return;

    /* now adjust diff_min_thresh (depends directly on bin_thresh) */
    img_proc_diff_check_cal(raw_top, dart_top, TOP_CAM, &p_sum, true);
    if (p_sum < p_sum_last)
        p_sum_last = p_sum;

    img_proc_diff_check_cal(raw_right, dart_right, RIGHT_CAM, &p_sum, true);
    if (p_sum < p_sum_last)
        p_sum_last = p_sum;

    img_proc_diff_check_cal(raw_left, dart_left, LEFT_CAM, &p_sum, true);
    if (p_sum < p_sum_last)
        p_sum_last = p_sum;

    /* p_sum_last is smallest sum val */
    cout << "smallest sum: " << p_sum_last << "\tsuggested diff min thresh: " << p_sum_last * 0.6 << " (Trackbar: " << p_sum_last * 0.6 / 1e+4 << ")" << endl;
    cout << "adjust diff min thresh with trackbar [0...200 * 1e+4]; remove dart and finish calibration with [Esc]" << endl;
    
    while ((waitKey(10) != 27) && running) 
        this_thread::sleep_for(chrono::milliseconds(10));

    return;

}

/******************************************************************************
 * Calibration related trackbar callbacks
/*****************************************************************************/
/* set Binary Threshold via Trackbar */
void on_trackbar_bin_thresh(int thresh, void* arg) {
    
    struct img_proc_s* iproc = (struct img_proc_s*)(arg);  
    
    /* update value */
    iproc->bin_thresh = thresh;


}

/* set Minimum Difference Threshold via Trackbar */
void on_trackbar_diff_min_thresh(int thresh, void* arg) {

    struct img_proc_s* iproc = (struct img_proc_s*)(arg);

    /* update value */
    iproc->diff_min_thresh = thresh*1e+4;


}


/* set  */
void on_trackbar_aspect_ratio_max(int aspect_ratio_max, void* arg) {

    struct img_proc_s* iproc = (struct img_proc_s*)(arg);

    /* update value */
    iproc->aspect_ratio_max = (float)(aspect_ratio_max)/100.0f;
    cout << iproc->aspect_ratio_max << endl;

}



/* set  */
void on_trackbar_aspect_ratio_min(int aspect_ratio_min, void* arg) {

    struct img_proc_s* iproc = (struct img_proc_s*)(arg);

    /* update value */
    iproc->aspect_ratio_min = (float)(aspect_ratio_min)/100.0f;
    cout << iproc->aspect_ratio_min << endl;

}

/* set  */
void on_trackbar_area_min(int area_min, void* arg) {

    struct img_proc_s* iproc = (struct img_proc_s*)(arg);

    /* update value */
    iproc->area_min = (float)(area_min)*10;
    cout << iproc->area_min << endl;

}

/* set  */
void on_trackbar_short_edge_max(int short_edge_max, void* arg) {

    struct img_proc_s* iproc = (struct img_proc_s*)(arg);

    /* update value */
    iproc->short_edge_max = (float)(short_edge_max);
    cout << iproc->short_edge_max << endl;

}







/******************************************************************************
 * Command Line Support Function
/*****************************************************************************/
/* set Binary Threshold via Command Line */
void img_proc_set_bin_thresh(int thresh) {

    /* update value */
    img_proc.bin_thresh = thresh;

}


/* set Minimum Difference Threshold via Command Line */
void img_proc_set_diff_min_thresh(int thresh) {

    /* update value */
    img_proc.bin_thresh = thresh;

}


/* set */
void img_proc_set_aspect_ratio_max(int aspect_ratio_max) {

    /* update value */
    img_proc.bin_thresh = aspect_ratio_max;

}

/* set */
void img_proc_set_aspect_ratio_min(int aspect_ratio_min) {

    /* update value */
    img_proc.aspect_ratio_min = aspect_ratio_min;

}

/* set */
void img_proc_set_area_min(int area_min) {

    /* update value */
    img_proc.area_min = area_min;

}

/* set */
void img_proc_set_short_edge_max(int short_edge_max) {

    /* update value */
    img_proc.short_edge_max = short_edge_max;

}




/******************************************************************************
 * Imange Processing Debug and tryout Function
/*****************************************************************************/
/***
 * Large Debug Copy for tryouts and various image display,
 * that's why this def is at the bottom. 
 * For Documentation see img_proc_get_line() at the top of the module 
***/
int img_proc_get_line_debug(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs, std::string CamNameId) {


    /* declare images */
    Mat cur;
    Mat cur_gray;
    Mat cur_sharp;
    Mat cur_sharp_gray;

    Mat last;
    Mat last_gray;
    Mat last_sharp;
    Mat last_sharp_gray;

    Mat diff;
    Mat diff_gray;
    Mat diff_sharp;
    Mat diff_sharp_gray;

    Mat sharp_after_diff;
    Mat sharp_after_diff_gray;

    Mat edge;
    Mat edge_bin;

    Mat houghSpace;
    Mat cur_line;


    /* clone images */
    cur = currentImg.clone();
    if (cur.empty()) {
        std::cout << "[ERROR] Current Image is empty" << endl;
        return EXIT_FAILURE;
    }

    last = lastImg.clone();
    if (last.empty()) {
        std::cout << "[ERROR] Last Image is empty" << endl;
        return EXIT_FAILURE;
    }

    /* noise reduction */
    cv::GaussianBlur(cur, cur, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);
    cv::GaussianBlur(last, last, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur_gray, COLOR_BGR2GRAY);
    cvtColor(last, last_gray, COLOR_BGR2GRAY);

    /* sharpen images */
    img_proc_sharpen_img(cur, cur_sharp);
    img_proc_sharpen_img(cur_gray, cur_sharp_gray);

    img_proc_sharpen_img(last, last_sharp);
    img_proc_sharpen_img(last_gray, last_sharp_gray);


    /* check difference */
    absdiff(last, cur, diff);
    absdiff(last_gray, cur_gray, diff_gray);
    absdiff(last_sharp, cur_sharp, diff_sharp);
    absdiff(last_sharp_gray, cur_sharp_gray, diff_sharp_gray);


    /* sharpen images after difference */
    img_proc_sharpen_img(diff, sharp_after_diff);
    img_proc_sharpen_img(diff_gray, sharp_after_diff_gray);

    /* edge image */
    //int thresh_top = 55;
    ip::sobelFilter(sharp_after_diff_gray, edge);
    //threshold(edge, edge_bin, BIN_THRESH, 255, THRESH_BINARY);    // fixed macro
    cv::threshold(edge, edge_bin, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

#if 1
    /*********************************************** under construction *****************************************************/
    /* everything thats outcommented is for performance reasons, but are images for debug and analysis */

    /* use the original if contours does not work */
    Mat edge_bin_original = edge_bin.clone();

    /* draw box, bc darts contours could be open if out of image */
    cv::line(edge_bin, Point(0, 0), Point(0, edge_bin.rows - 1), Scalar(255, 255, 255), 1);
    cv::line(edge_bin, Point(0, 0), Point(edge_bin.cols - 1, 0), Scalar(255, 255, 255), 1);
    cv::line(edge_bin, Point(edge_bin.cols - 1, 0), Point(edge_bin.cols - 1, edge_bin.rows - 1), Scalar(255, 255, 255), 2);
    cv::line(edge_bin, Point(0, edge_bin.rows - 1), Point(edge_bin.cols - 1, edge_bin.rows - 1), Scalar(255, 255, 255), 2);



    /* close open contours */
    cv::morphologyEx(edge_bin, edge_bin, cv::MORPH_CLOSE, cv::Mat::ones(2, 2, CV_8U));
    //GaussianBlur(edge_bin, edge_bin, Size(3, 3), 0.3, 0.3);


    Mat edge_bin_cont = edge_bin.clone();
    cvtColor(edge_bin_cont, edge_bin_cont, COLOR_GRAY2BGR);
    /* find contour of dart */
    vector<vector<Point>> cont;
    vector<Vec4i> hier;
    findContours(edge_bin, cont, hier, RETR_TREE, CHAIN_APPROX_SIMPLE);

    /* draw cont */

    //Mat contoursImg = Mat::zeros(edge_bin.size(), CV_8UC3);
    for (size_t i = 0; i < cont.size(); i++) {
        drawContours(edge_bin_cont, cont, (int)i, Scalar(255, 255, 0), 1, LINE_8, hier, 0);
    }



    //Mat result = Mat::zeros(edge_bin.size(), CV_8UC3);
    //Mat cont_rect = Mat::zeros(edge_bin.size(), CV_8UC3);

    /* fitted final contour --> just the barrel as rectangle */
    Mat cont_rect_fitted = Mat::zeros(edge_bin.size(), CV_8UC3);
    /* store all endpoints */
    vector<Point> allPoints;

    for (size_t i = 0; i < cont.size(); i++) {
        /* rotating bounding box */
        RotatedRect rotatedRect = minAreaRect(cont[i]);

        /* calculate properties */
        float width = rotatedRect.size.width;
        float height = rotatedRect.size.height;

        float aspectRatio = (width < height) ? width / height : height / width;

        double area = contourArea(cont[i]);


        /* criteria: narrow and elongated */
        if ((aspectRatio < 0.2) && (aspectRatio > 0.01) && (area > 450)) {
            drawContours(edge_bin_cont, cont, (int)i, Scalar(0, 255, 255), 1);
            /* calculate corners of ratating rectangle */
            Point2f points[4];
            rotatedRect.points(points);

            /* draw rot rect */
            for (int j = 0; j < 4; j++) {
                allPoints.push_back(points[j]);
                //cv::line(cont_rect_fitted, points[j], points[(j + 1) % 4], Scalar(0, 255, 255), 1);
                //line(result, points[j], points[(j + 1) % 4], Scalar(255, 0, 0), 1);
            }

        }
    }



    /* calaculate just one rot rect which fits all other rects, their might be more than bc of shaft and barrel might be divided through its haptic */
    if (!allPoints.empty()) {
        RotatedRect enclosingRect = minAreaRect(allPoints);
        Point2f points_enc[4];
        enclosingRect.points(points_enc);

        /* draw rect */
        for (int j = 0; j < 4; j++) {
            cv::line(edge_bin_cont, points_enc[j], points_enc[(j + 1) % 4], Scalar(255, 0, 0), 2);
            cv::line(cont_rect_fitted, points_enc[j], points_enc[(j + 1) % 4], Scalar(255, 0, 0), 1);
        }
        /**/
        cvtColor(cont_rect_fitted, edge_bin, COLOR_BGR2GRAY);
        cv::threshold(edge_bin, edge_bin, 10, 255, THRESH_BINARY);
    }
    /* if there were no conts, which fitted criteria do normal edge detection */
    else {
        edge_bin = edge_bin_original;
    }



    //imshow("narrow contours", result);
    //imshow("edges", edge_bin);
    //imshow("contoura", contoursImg);
    //imshow("Fitted all", cont_rect_fitted);

    /*********************************************** under construction end **************************************************/
#endif 


    /* Calculate Hough transform */
    cur_line = cur.clone();
    ip::houghTransform(edge_bin, houghSpace);

    cv::GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);




    /* find 2 gloabl maxima */
    Mat houghSpaceClone = houghSpace.clone();
    /* Prepare Hough space image for display */
    houghSpace = 255 - houghSpace;				// Invert
    ip::drawHoughLineLabels(houghSpace);		// Axes

    Point houghMaxLocation;
    double r, theta;
    double r_avg = 0;
    double theta_avg = 0;
    /* find global maxima */
    int global_max = 2;
    for (int i = 0; i < global_max; i++) {
        minMaxLoc(houghSpaceClone, NULL, NULL, NULL, &houghMaxLocation);
        ip::houghSpaceToLine(
            Size(edge_bin.cols, edge_bin.rows),
            Size(houghSpace.cols, houghSpace.rows),
            houghMaxLocation.x, houghMaxLocation.y, r, theta);
        //ip::drawLine(cur_line, r, theta);
        //cout << "Debug x: " << houghMaxLocation.x << "\ty: " << houghMaxLocation.y << endl;

        // set max to zero
        int delete_size = 3;
        for (int dx = -delete_size; dx <= delete_size; ++dx) {
            for (int dy = -delete_size; dy <= delete_size; ++dy) {
                int nx = houghMaxLocation.x + dx; // neighbor in x-dir
                int ny = houghMaxLocation.y + dy; // neighbor in y-dir


                if (nx >= 0 && nx < houghSpaceClone.cols && ny >= 0 && ny < houghSpaceClone.rows) {
                    houghSpaceClone.at<uchar>(ny, nx) = 0;
                }
            }
        }

        /* shift the edges exactly on the edge to be more centered in averaging */
        int shift_val = 2;
        if (r > shift_val) {
            r = r + shift_val;
        }
        else if (r < -shift_val) {
            r = r - shift_val;
        }

        ip::drawLine(edge_bin_cont, r, theta);   // Debug
        cv::circle(houghSpace, houghMaxLocation, 5, Scalar(0, 0, 255), 2);		// Global maximum
        /* averaging */
        //out << "Debug r: " << r << "\ttheta" << theta << endl;
        /* !watch out when delta_theta > 90° */
        if ((i > 0) && (fabs(theta_avg - theta) > (CV_PI / 2))) {
            r_avg = r_avg + (-r);   // toggle sign
            if (theta > 0) {
                theta_avg = theta_avg + (CV_PI - theta);
            }
            else {
                theta_avg = theta_avg + (-CV_PI - theta);
            }
        }
        else {
            r_avg += r; // / (double)global_max;
            theta_avg += theta; /// (double)global_max;
        }
    }
    r_avg = r_avg / global_max;
    theta_avg = theta_avg / global_max;

    //cout << r_avg << "\t" << theta_avg << endl;
    /* draw average line */
    ip::drawLine(cur_line, r_avg, theta_avg);

    /* return line values */
    line->r = r_avg;
    line->theta = theta_avg;
    //cout << "Debug r_avg: " << r_avg << "\ttheta_avg" << theta_avg << endl;

    /* create windows */
    if (show_imgs == SHOW_NO_IMAGES) {
        return EXIT_SUCCESS;
    }
    else if (show_imgs == SHOW_ALL_IMAGES) {

        /* curent image plots */
        string image_basic = string("Current Image (").append(CamNameId).append(" Cam)");
        string image_gray = string("Current Image Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_basic, cur);
        cv::imshow(image_gray, cur_gray);

        /* sharpend images */
        string image_sharp = string("Image Sharp (").append(CamNameId).append(" Cam)");
        string image_sharp_gray = string("Image Sharp Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_sharp, cur_sharp);
        cv::imshow(image_sharp_gray, cur_sharp_gray);

        /* difference images */
        string image_diff_basic = string("Image Diff (").append(CamNameId).append(" Cam)");
        string image_diff_gray = string("Image Diff Gray (").append(CamNameId).append(" Cam)");
        string image_diff_sharp = string("Image Diff Sharp (").append(CamNameId).append(" Cam)");
        string image_diff_sharp_gray = string("Image Diff Sharp Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_diff_basic, diff);
        cv::imshow(image_diff_gray, diff_gray);
        cv::imshow(image_diff_sharp, diff_sharp);
        cv::imshow(image_diff_sharp_gray, diff_sharp_gray);

        /* sharpened images after diff */
        string image_sharp_diff = string("Image Sharpened After Diff (").append(CamNameId).append(" Cam)");
        string image_sharp_diff_gray = string("Image Sharpened After Diff Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_sharp_diff, sharp_after_diff);
        cv::imshow(image_sharp_diff_gray, sharp_after_diff_gray);


        /* edge image */
        string image_edge = string("Edge Image (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge, edge);
        /* edge binary image */
        string image_edge_bin = string("Image Edge Bin (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge_bin, edge_bin);

        /* Hough transform (line images) */
        string image_orig = string("Image Orig with line (").append(CamNameId).append(" Cam)");
        cv::imshow(image_orig, cur_line);
        // redundant string image_edge = string("Edge Image ").append(CamNameId);
        string image_hspace = string("HoughSpace (").append(CamNameId).append(" Cam)");
        cv::imshow(image_hspace, houghSpace);


        return EXIT_SUCCESS;

    }
    else if (show_imgs == SHOW_SHORT_ANALYSIS) {

        /* Hough transform (line images) */
        string image_orig = string("Image Orig with line (").append(CamNameId).append(" Cam)");
        cv::imshow(image_orig, cur_line);
        /* edge image */
        string image_edge = string("Edge Image (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge, edge);
        /* edge binary image */
        string image_edge_bin = string("Image Edge Bin (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge_bin, edge_bin);
        /* sharpened images after diff */
        string image_sharp_diff = string("Image Sharpened After Diff (").append(CamNameId).append(" Cam)");
        string image_sharp_diff_gray = string("Image Sharpened After Diff Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_sharp_diff, sharp_after_diff);
        cv::imshow(image_sharp_diff_gray, sharp_after_diff_gray);
        return EXIT_SUCCESS;
    }


    if (show_imgs == SHOW_IMG_LINE) {
        /* Hough transform (line images) */
        string image_orig = string("Image Orig with line (").append(CamNameId).append(" Cam)");
        cv::imshow(image_orig, cur_line);
    }
    if (show_imgs == SHOW_EDGE_IMG) {
        /* edge image */
        string image_edge = string("Edge Image (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge, edge);
    }
    if (show_imgs == SHOW_EDGE_BIN) {
        /* edge binary image */
        string image_edge_bin = string("Image Edge Bin (").append(CamNameId).append(" Cam)");
        cv::imshow(image_edge_bin, edge_bin);
    }
    if (show_imgs == SHOW_SHARP_AFTER_DIFF) {
        /* sharpened images after diff */
        string image_sharp_diff = string("Image Sharpened After Diff (").append(CamNameId).append(" Cam)");
        string image_sharp_diff_gray = string("Image Sharpened After Diff Gray (").append(CamNameId).append(" Cam)");
        cv::imshow(image_sharp_diff, sharp_after_diff);
        cv::imshow(image_sharp_diff_gray, sharp_after_diff_gray);
    }


    return EXIT_SUCCESS;

}

