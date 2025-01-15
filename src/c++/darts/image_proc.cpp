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
 *      TBD
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


/************************** local Structure ***********************************/
static struct img_proc_s {
    int bin_thresh = 41;
    int diff_min_thresh = 1.5e+5;
}img_proc;

/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/

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
    GaussianBlur(cur,cur, Size(3,3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);
    GaussianBlur(last, last, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);

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
    threshold(edge, edge_bin, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

#if 1
/*********************************************** under construction *****************************************************/
    /* everything thats outcommented for performance, but are images for debug and analysis reasons */

    cv::morphologyEx(edge_bin, edge_bin, cv::MORPH_CLOSE, cv::Mat::ones(1, 1, CV_8U));

    Mat edge_bin_cont = edge_bin.clone();
    cvtColor(edge_bin_cont, edge_bin_cont, COLOR_GRAY2BGR);
    /* find contour of dart */
    vector<vector<Point>> cont;
    vector<Vec4i> hier;
    findContours(edge_bin, cont, hier, RETR_TREE, CHAIN_APPROX_SIMPLE);

    /* draw cont */
    
    //Mat contoursImg = Mat::zeros(edge_bin.size(), CV_8UC3);
    for (size_t i = 0; i < cont.size(); i++) {
        drawContours(edge_bin_cont, cont, (int)i, Scalar(0, 255, 0), 1, LINE_8, hier, 0);
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
        if ((aspectRatio < 0.2 )&& (area > 50 )) { // Passen Sie die Werte f�r Ihre Bed�rfnisse an
            drawContours(edge_bin_cont, cont, (int)i, Scalar(255, 0, 0), 1);
            /* calculate corners of ratating rectangle */
            Point2f points[4];
            rotatedRect.points(points);

            /* draw rot rect */
            for (int j = 0; j < 4; j++) {
                allPoints.push_back(points[j]);
                //line(cont_rect, points[j], points[(j + 1) % 4], Scalar(255, 0, 0), 1);
                //line(result, points[j], points[(j + 1) % 4], Scalar(255, 0, 0), 1);
            }

        }
    }
    /* calaculate just one rot rect which fits all other rect, ther might be more than bc of shaft and barrel might be divided through its haptic */
    if (!allPoints.empty()) {
        RotatedRect enclosingRect = minAreaRect(allPoints); 
        Point2f points_enc[4];
        enclosingRect.points(points_enc);

        /* draw rect */
        for (int j = 0; j < 4; j++) {
            cv::line(edge_bin_cont, points_enc[j], points_enc[(j + 1) % 4], Scalar(255, 0, 0), 1);
            cv::line(cont_rect_fitted, points_enc[j], points_enc[(j + 1) % 4], Scalar(255, 0, 0), 1);  
        }

    }



    //imshow("narrow contours", result);
    //imshow("edges", edge_bin);
    //imshow("contoura", contoursImg);
    //imshow("Fitted all", cont_rect_fitted);
    /**/
    cvtColor(cont_rect_fitted, edge_bin, COLOR_BGR2GRAY);
    threshold(edge_bin, edge_bin, 10, 255, THRESH_BINARY);
    /*********************************************** under construction end **************************************************/
#endif 


    /* Calculate Hough transform */
    cur_line = cur.clone();
    ip::houghTransform(edge_bin, houghSpace);

    GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);




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
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dy = -3; dy <= 3; ++dy) {
                int nx = houghMaxLocation.x + dx; // Nachbarpixel in x-Richtung
                int ny = houghMaxLocation.y + dy; // Nachbarpixel in y-Richtung

                // �berpr�fen, ob der Nachbarpixel innerhalb der Bildgrenzen liegt
                if (nx >= 0 && nx < houghSpaceClone.cols && ny >= 0 && ny < houghSpaceClone.rows) {
                    houghSpaceClone.at<uchar>(ny, nx) = 0; // Setze den Pixel auf Null (nur f�r Grauwertbilder, CV_8U)
                }
            }
        }
        ip::drawLine(edge_bin_cont, r, theta);   // Debug
        circle(houghSpace, houghMaxLocation, 5, Scalar(0, 0, 255), 2);		// Global maximum
        /* averaging */
        //out << "Debug r: " << r << "\ttheta" << theta << endl;
        /* !watch out when delta_theta > 90� */
        if ((i > 0) && (fabs(theta_avg - theta) > (CV_PI / 2))) {
            r_avg = r_avg + (-r);   // toggle sign
            if (theta > 0) {
                theta_avg = theta_avg + (CV_PI-theta);
            }
            else {
                theta_avg = theta_avg + (-  CV_PI - theta);
            }
        }
        else {
            r_avg += r; // / (double)global_max;
            theta_avg += theta; /// (double)global_max;
        }
    }
    r_avg = r_avg / global_max;
    theta_avg = theta_avg / global_max;
    /*if (fabs(r_avg) < 0.000001) {
        r_avg = 1;
    }*/
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
        /* edge image */
        //string image_edge = string("2 Edge Image (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_edge, edge);
        /* edge binary image */
        string image_edge_bin = string("3 Image Edge Bin (").append(CamNameId).append(" Cam)");
        //cv::imshow(image_edge_bin, edge_bin);
        cv::imshow(image_edge_bin, edge_bin_cont);
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



int img_proc_cross_point(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p) {

    Mat frame = Mat::zeros(frameSize, CV_8UC3);


    ip::drawLine_light_add(frame, tri_line->line_top.r, tri_line->line_top.theta);
    ip::drawLine_light_add(frame, tri_line->line_right.r, tri_line->line_right.theta);
    ip::drawLine_light_add(frame, tri_line->line_left.r, tri_line->line_left.theta);



    imshow("Z Cross Line", frame);


    // Vektor, um alle Positionen der maximalen Pixel zu speichern
    vector<Point> maxLocations;

    /* find cross points */
    img_proc_get_cross_points(frame, maxLocations);

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

struct line_kart_s {
    Point p0;
    Point p1;
};

struct tri_line_kart_s {
    struct line_kart_s top;
    struct line_kart_s right;
    struct line_kart_s left;

};

// Umrechnung von Polar-Koordinaten (r, theta) in kartesische Koordinaten
void polarToCartesian(const cv::Mat& image, struct line_s l, struct line_kart_s& kart) {
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

        kart.p0 = Point(x0, yc0 + imgCenter.y);
        kart.p1 = Point(x1, yc1 + imgCenter.y);
    }
    // Line end points for "almost vertical" lines
    else {
        int y0 = 0;
        int y1 = image.rows - 1;
        int yc0 = y0 - image.rows / 2;
        int yc1 = y1 - image.rows / 2;
        int xc0 = (int)((l.r - yc0 * sine) / cosine);
        int xc1 = (int)((l.r - yc1 * sine) / cosine);

        kart.p0 = Point(xc0 + imgCenter.x, y0);
        kart.p1 = Point(xc1 + imgCenter.x, y1);
    }
    return;
}

// Berechnung des Schnittpunkts zweier Linien
bool find_intersection(const line_kart_s& line1, const line_kart_s& line2, Point& intersection) {
    // Koordinaten der ersten Linie
    int x0_1 = line1.p0.x, y0_1 = line1.p0.y;
    int x1_1 = line1.p1.x, y1_1 = line1.p1.y;
    
    // Koordinaten der zweiten Linie
    int x0_2 = line2.p0.x, y0_2 = line2.p0.y;
    int x1_2 = line2.p1.x, y1_2 = line2.p1.y;

    // Berechnung der Parameter f�r die Gleichungen der beiden Linien
    int A1 = y1_1 - y0_1;
    int B1 = x0_1 - x1_1;
    int C1 = x1_1 * y0_1 - x0_1 * y1_1;

    int A2 = y1_2 - y0_2;
    int B2 = x0_2 - x1_2;
    int C2 = x1_2 * y0_2 - x0_2 * y1_2;

    // Berechnung des Determinanten
    int det = A1 * B2 - A2 * B1;

    // Wenn der Determinant null ist, sind die Linien parallel und schneiden sich nicht
    if (det == 0) {
        intersection.x = -66666;
        intersection.y = -66666;
        return false;
    }

    // Berechnung der Schnittpunkt-Koordinaten
    intersection.x = (B1 * C2 - B2 * C1) / det;
    intersection.y = (A2 * C1 - A1 * C2) / det;

    return true;
}


// Berechnung des Mittelpunkts der Schnittpunkte
Point calculate_midpoint(const Point& p1, const Point& p2, const Point& p3) {
    Point midpoint;
    midpoint.x = (p1.x + p2.x + p3.x) / 3;
    midpoint.y = (p1.y + p2.y + p3.y) / 3;
    return midpoint;
}
int img_proc_cross_point_math(cv::Size frameSize, struct tripple_line_s* tri_line, cv::Point& cross_p) {

    Mat frame = Mat::zeros(frameSize, CV_8UC3);


    ip::drawLine_light_add(frame, tri_line->line_top.r, tri_line->line_top.theta);
    ip::drawLine_light_add(frame, tri_line->line_right.r, tri_line->line_right.theta);
    ip::drawLine_light_add(frame, tri_line->line_left.r, tri_line->line_left.theta);

    struct tri_line_kart_s tlk;
    // Umwandlung der Polar-Koordinaten der Linien in kartesische Koordinaten
    // Berechnung von Punkten f�r jede Linie auf dem Bild
    polarToCartesian(frame, tri_line->line_top, tlk.top);
    polarToCartesian(frame, tri_line->line_right, tlk.right);
    polarToCartesian(frame, tri_line->line_left, tlk.left);

    // Berechnung der Schnittpunkte der Linien
    Point intersection1, intersection2, intersection3;
    find_intersection(tlk.top, tlk.right, intersection1);
    find_intersection(tlk.top, tlk.left, intersection2);
    find_intersection(tlk.left, tlk.right, intersection3);

    // Berechnung des Mittelpunkts der Schnittpunkte
    Point midpoint = calculate_midpoint(intersection1, intersection2, intersection3);


    

    // Zeichnen der Schnittpunkte und des Mittelpunkts
    //circle(image, intersection1, 5, Scalar(0, 0, 255), 1); // Schnittpunkt Linie 1 und 2 in Rot
    //circle(image, intersection2, 5, Scalar(0, 255, 0), 1); // Schnittpunkt Linie 2 und 3 in Gr�n
    //circle(image, intersection3, 5, Scalar(255, 0, 0), 1); // Schnittpunkt Linie 3 und 1 in Blau
    circle(frame, midpoint, 8, Scalar(255, 255, 0), 1.5); // Mittelpunkt in Gelb

    // Ausgabe des Mittelpunkts
    //cout << "Mittelpunkt der maximalen Positionen: x: " << centerOfMass.x << ", y: " << centerOfMass.y << endl;

    cross_p.x = midpoint.x;
    cross_p.y = midpoint.y;


    imshow("Z Cross Line", frame);
    waitKey(0); // Warten, bis eine Taste gedr�ckt wird

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

    /* noise reduction */
    GaussianBlur(cur, cur, Size(9, 9), 1.25, 1.25);
    GaussianBlur(last, last, Size(9, 9), 1.25, 1.25);

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
    threshold(diff, diff, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

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
    GaussianBlur(cur, cur, Size(9, 9), 1.25, 1.25);
    GaussianBlur(last, last, Size(9, 9), 1.25, 1.25);

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
    threshold(diff, diff, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

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

    setTrackbarPos(TRACKBAR_NAME_BIN_THRESH, WINDOW_NAME_THRESHOLD, BIN_THRESH);
    setTrackbarPos(TRACKBAR_NAME_DIFF_MIN_THRESH, WINDOW_NAME_THRESHOLD, (int)(DIFF_MIN_THRESH/1e+4));

    
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


void on_trackbar_bin_thresh(int thresh, void* arg) {
    
    struct img_proc_s* iproc = (struct img_proc_s*)(arg);  
    
    /* update value */
    iproc->bin_thresh = thresh;


}



void on_trackbar_diff_min_thresh(int thresh, void* arg) {

    struct img_proc_s* iproc = (struct img_proc_s*)(arg);

    /* update value */
    iproc->diff_min_thresh = thresh*1e+4;


}



void img_proc_set_bin_thresh(int thresh) {

    /* update value */
    img_proc.bin_thresh = thresh;

}

void img_proc_set_diff_min_thresh(int thresh) {

    /* update value */
    img_proc.bin_thresh = thresh;

}



/* Large Debug Copy, that's why this def is at the bottom */
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
    GaussianBlur(cur, cur, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);
    GaussianBlur(last, last, Size(3, 3), GAUSSIAN_BLUR_SIGMA, GAUSSIAN_BLUR_SIGMA);

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
    threshold(edge, edge_bin, img_proc.bin_thresh, 255, THRESH_BINARY);      // set by trackbar

#if 1
/*********************************************** under construction *****************************************************/
    //imshow("darts", edge_bin);
    //waitKey(0);
    //destroyWindow("darts");
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(edge_bin, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Konturen auf einem neuen Bild zeichnen
    cv::Mat contoursImg = cv::Mat::zeros(edge_bin.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++) {
        cv::drawContours(contoursImg, contours, (int)i, cv::Scalar(0, 255, 0), 1, cv::LINE_8, hierarchy, 0);
    }
    // Neue Bildmaske f�r schmale Konturen
    cv::Mat result = cv::Mat::zeros(edge_bin.size(), CV_8UC3);
    cv::Mat cont_rect = cv::Mat::zeros(edge_bin.size(), CV_8UC3);
    cv::Mat cont_rect_fitted = cv::Mat::zeros(edge_bin.size(), CV_8UC3);
    std::vector<cv::Point> allPoints; // Hier speichern wir alle Eckpunkte

    for (size_t i = 0; i < contours.size(); i++) {
        // Rotierende Begrenzungsbox
        cv::RotatedRect rotatedRect = cv::minAreaRect(contours[i]);

        // Eigenschaften berechnen
        float width = rotatedRect.size.width;
        float height = rotatedRect.size.height;
        float aspectRatio = (width < height) ? width / height : height / width; // Aspektverh�ltnis (immer <= 1)
        double area = cv::contourArea(contours[i]);

        // Filterkriterien: Schmal und l�nglich
        if (aspectRatio < 0.2 && area > 40) { // Passen Sie die Werte f�r Ihre Bed�rfnisse an
            cv::drawContours(result, contours, (int)i, cv::Scalar(0, 255, 0), 1);
            // Eckpunkte des rotierenden Rechtecks berechnen
            cv::Point2f points[4];
            rotatedRect.points(points);

            // Rotierendes Rechteck zeichnen
            for (int j = 0; j < 4; j++) {
                allPoints.push_back(points[j]);
                cv::line(cont_rect, points[j], points[(j + 1) % 4], cv::Scalar(255, 0, 0), 1);
                cv::line(result, points[j], points[(j + 1) % 4], cv::Scalar(255, 0, 0), 1);
            }
           
        }
    }
    // Berechne das gr��te rotierte Rechteck, das alle gesammelten Punkte umschlie�t
    if (!allPoints.empty()) {
        cv::RotatedRect enclosingRect = cv::minAreaRect(allPoints); // Berechne das RotatedRect f�r alle Eckpunkte

        // Berechne die Eckpunkte des gro�en rotierenden Rechtecks
        cv::Point2f points[4];
        enclosingRect.points(points);

        // Zeichne das gr��te rotierte Rechteck
        for (int j = 0; j < 4; j++) {
            //cv::line(result, points[j], points[(j + 1) % 4], cv::Scalar(0, 0, 255), 2);  // Rotes Rechteck
            cv::line(cont_rect_fitted, points[j], points[(j + 1) % 4], cv::Scalar(0, 0, 255), 1);  // Rotes Rechteck
        }
  
    }



    cv::imshow("Schmale Konturen", result);
    cv::imshow("Kanten", edge_bin);
    cv::imshow("Konturen", contoursImg);
    cv::imshow("Fitted all", cont_rect_fitted);
    cvtColor(cont_rect_fitted, cont_rect_fitted, COLOR_BGR2GRAY);
    threshold(cont_rect_fitted, edge_bin, 10, 255, THRESH_BINARY);
/*********************************************** under construction end **************************************************/
#endif 
    /* Calculate Hough transform */
    cur_line = cur.clone();
    ip::houghTransform(edge_bin, houghSpace);

    GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);




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
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -4; dy <= 4; ++dy) {
                int nx = houghMaxLocation.x + dx; // Nachbarpixel in x-Richtung
                int ny = houghMaxLocation.y + dy; // Nachbarpixel in y-Richtung

                // �berpr�fen, ob der Nachbarpixel innerhalb der Bildgrenzen liegt
                if (nx >= 0 && nx < houghSpaceClone.cols && ny >= 0 && ny < houghSpaceClone.rows) {
                    houghSpaceClone.at<uchar>(ny, nx) = 0; // Setze den Pixel auf Null (nur f�r Grauwertbilder, CV_8U)
                }
            }
        }
        ip::drawLine(edge_bin, r, theta);   // Debug
        circle(houghSpace, houghMaxLocation, 5, Scalar(0, 0, 255), 2);		// Global maximum
        /* averaging */
        //out << "Debug r: " << r << "\ttheta" << theta << endl;
        /* !watch out when delta_theta > 90� */
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
    /*if (fabs(r_avg) < 0.000001) {
        r_avg = 1;
    }*/
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

