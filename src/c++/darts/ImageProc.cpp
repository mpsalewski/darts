/******************************************************************************
 *
 * $NAME.cpp
 *
 * Digital Image / Video Processing
 * HAW Hamburg, Prof. Dr. Marc Hensel
 *
 * TEMPLATE
 *
 *
 * author: 			m. salewski
 * created on:
 * last revision:
 *
 *
 *
 *
 *
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

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
#define SMOOTHING_KERNEL_SIZE 1
#define CROSS_POINT_INTENSITY_MIN 18
#define DIFF_MIN_THRESH 1e+6

/************************** local Structure ***********************************/


/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/

void sharpenImage(const cv::Mat& inputImage, cv::Mat& outputImage) {
    // Schärfungs-Kernel definieren
   /* Mat kernel = (Mat_<float>(3, 3) <<
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0);*/
    Mat kernel = (Mat_<float>(3, 3) <<
        0, -1, 0,
        -1, 11, -1,
        0, -1, 0);

    // Ausgabebild erstellen
    //Mat sharpenedImage;

    // Schärfung anwenden
    filter2D(inputImage, outputImage, -1, kernel);

    //return sharpenedImage;
}

void findAllMaxima(const cv::Mat& image, std::vector<cv::Point>& maxLocations) {
    // Schritt 1: Bestimme den maximalen Wert
    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(image, &minVal, &maxVal, &minLoc, &maxLoc);


    // Schritt 2: Durchlaufe das Bild und finde alle Pixel mit dem maximalen Wert
    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            // Falls der Pixel den maximalen Wert hat, speichere die Position
            if (image.at<uchar>(y, x) == maxVal) {
                maxLocations.push_back(Point(x, y));
            }
        }
    }
}

void findAllCrossPoints(const cv::Mat& image, std::vector<cv::Point>& maxLocations) {

    Mat image_gray;
    cvtColor(image, image_gray, COLOR_BGR2GRAY);

    // Schritt 1: Bestimme den maximalen Wert
    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(image_gray, &minVal, &maxVal, &minLoc, &maxLoc);
    //cout << maxVal;


    // Schritt 2: Durchlaufe das Bild und finde alle Pixel mit dem maximalen Wert
    for (int y = 0; y < image_gray.rows; ++y) {
        for (int x = 0; x < image_gray.cols; ++x) {
            // Falls der Pixel den maximalen Wert hat, speichere die Position
            if (image_gray.at<uchar>(y, x) > CROSS_POINT_INTENSITY_MIN) {
                maxLocations.push_back(Point(x, y));
            }
        }
    }
}


int image_proc_get_line(cv::Mat& lastImg, cv::Mat& currentImg, int ThreadId, struct line_s* line, int show_imgs, std::string CamNameId) {


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

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur_gray, COLOR_BGR2GRAY);
    cvtColor(last, last_gray, COLOR_BGR2GRAY);

    /* sharpen images */
    sharpenImage(cur, cur_sharp);
    sharpenImage(cur_gray, cur_sharp_gray);

    sharpenImage(last, last_sharp);
    sharpenImage(last_gray, last_sharp_gray);


    /* check difference */
    absdiff(last, cur, diff);
    absdiff(last_gray, cur_gray, diff_gray);
    absdiff(last_sharp, cur_sharp, diff_sharp);
    absdiff(last_sharp_gray, cur_sharp_gray, diff_sharp_gray);


    /* sharpen images after difference */
    sharpenImage(diff, sharp_after_diff);
    sharpenImage(diff_gray, sharp_after_diff_gray);

    /* edge image */
    //int thresh_top = 55;
    ip::sobelFilter(sharp_after_diff_gray, edge);
    threshold(edge, edge_bin, BIN_THRESH, 255, THRESH_BINARY);


    /* Calculate Hough transform */
    cur_line = cur.clone();
    ip::houghTransform(edge_bin, houghSpace);

    GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);




    /* find 3 gloabl maxima */
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

        // set max to zero
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -4; dy <= 4; ++dy) {
                int nx = houghMaxLocation.x + dx; // Nachbarpixel in x-Richtung
                int ny = houghMaxLocation.y + dy; // Nachbarpixel in y-Richtung

                // Überprüfen, ob der Nachbarpixel innerhalb der Bildgrenzen liegt
                if (nx >= 0 && nx < houghSpaceClone.cols && ny >= 0 && ny < houghSpaceClone.rows) {
                    houghSpaceClone.at<uchar>(ny, nx) = 0; // Setze den Pixel auf Null (nur für Grauwertbilder, CV_8U)
                }
            }
        }
        ip::drawLine(edge_bin, r, theta);   // Debug
        circle(houghSpace, houghMaxLocation, 5, Scalar(0, 0, 255), 2);		// Global maximum
        /* averaging */
        r_avg += r / (double)global_max;
        theta_avg += theta / (double)global_max;

    }
    //cout << r_avg << "\t" << theta_avg << endl;
    /* draw average line */
    ip::drawLine(cur_line, r_avg, theta_avg);

    /* return line values */
    line->r = r_avg;
    line->theta = theta_avg;


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



int img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p) {

    Mat frame = Mat::zeros(frameSize, CV_8UC3);


    ip::drawLine_light_add(frame, tri_line->line_top.r, tri_line->line_top.theta);
    ip::drawLine_light_add(frame, tri_line->line_right.r, tri_line->line_right.theta);
    ip::drawLine_light_add(frame, tri_line->line_left.r, tri_line->line_left.theta);



    imshow("Just Line", frame);


    // Vektor, um alle Positionen der maximalen Pixel zu speichern
    vector<Point> maxLocations;

    // Finde alle maximalen Positionen
    //findAllMaxima(frame_gray, maxLocations);
    findAllCrossPoints(frame, maxLocations);

    int sumX = 0, sumY = 0;

    // Ausgabe der maximalen Positionen
    //cout << "Maximale Positionen:" << endl;
    for (const Point& pt : maxLocations) {
        sumX += pt.x;
        sumY += pt.y;
        //cout << "x: " << pt.x << ", y: " << pt.y << endl;
    }

    if (maxLocations.size() == 0) {
        return EXIT_FAILURE;
    }

    Point centerOfMass(sumX / maxLocations.size(), sumY / maxLocations.size());

    // Ausgabe des Mittelpunkts
    //cout << "Mittelpunkt der maximalen Positionen: x: " << centerOfMass.x << ", y: " << centerOfMass.y << endl;

    cross_p.x = centerOfMass.x;
    cross_p.y = centerOfMass.y;

    return EXIT_SUCCESS;

}




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

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur, COLOR_BGR2GRAY);
    cvtColor(last, last, COLOR_BGR2GRAY);


    /* check difference */
    absdiff(last, cur, diff);

    /* sharpen images after difference */
    sharpenImage(diff, diff);

    /* edge image */
    ip::sobelFilter(diff, diff);
    threshold(diff, diff, BIN_THRESH, 255, THRESH_BINARY);

    imshow("01 Wait DIFF", diff);
    
    /* sum up all pixel */
    p_sum = sum(diff);
    //cout << "sum of pixel: " << p_sum[0] << endl;
    if (p_sum[0]>DIFF_MIN_THRESH) {
        return IMG_DIFFERENCE;
    }
    else {
        return IMG_NO_DIFFERENCE;
    }
      

}



int img_proc_diff_check_getback(cv::Mat& last_f, cv::Mat& cur_f, int ThreadId) {

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

    /* calibrate images */
    calibration_get_img(cur, cur, ThreadId);
    calibration_get_img(last, last, ThreadId);

    /* gray conversion */
    cvtColor(cur, cur, COLOR_BGR2GRAY);
    cvtColor(last, last, COLOR_BGR2GRAY);


    /* check difference */
    absdiff(last, cur, diff);

    /* sharpen images after difference */
    sharpenImage(diff, diff);

    /* edge image */
    //ip::sobelFilter(diff, diff);
    //threshold(diff, diff, BIN_THRESH, 255, THRESH_BINARY);

    imshow("01 Wait DIFF", diff);

    /* sum up all pixel */
    p_sum = sum(diff);
    cout << "sum of pixel: " << p_sum[0] << endl;
    if (p_sum[0] > DIFF_MIN_THRESH) {
        return IMG_DIFFERENCE;
    }
    else {
        return IMG_NO_DIFFERENCE;
    }


}


