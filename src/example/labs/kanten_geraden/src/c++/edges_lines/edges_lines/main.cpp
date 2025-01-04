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
#include "HoughLine.h"
#include "Sobel.h"

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;
using namespace ip;


/*************************** local Defines ***********************************/
#define DATA_ROOT_PATH getenv("ImagingData")
#define INPUT_IMAGE "/image_data/images/misc/Darts_dotted.jpg"
#define OUTPUT_IMAGE "/image_data/images/misc/lab2.jpg"
#define VIDEO_PATH "/image_data/videos/SoccerShot.mp4"
#define FPS 30
#define WAIT_TIME_MS (1000 / FPS)
#define SMOOTHING_KERNEL_SIZE 1
#define EDGE_IMAGE_THRESHOLD 25
#define WINDOW_NAME_THRESHOLD "Threshold"
#define TRACKBAR_NAME_THRESHOLD "Threshold"
#define INITIAL_THRESHOLD 127
#define WINDOW_NAME_ORIGINAL_IMG "Original Image"
#define WINDOW_NAME_EDGE_IMG "Edge Image"
#define WINDOW_NAME_HOUGH_TRANS "Hough Transform"

/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/
/* Callback for trackbar to adjust tau */
void onTrackbarThreshold(int thresh, void* imagePtr);
void onMouseHoughParam(int event, int x, int y, int flags, void* imagePtr);
void updateHough(int tau, int x, int y, void* imagePtr);

/************************** Function Definitions *****************************/
void onTrackbarThreshold(int thresh, void* imagePtr) {
    Mat& image = *(Mat*)imagePtr;
    Mat threshImage;
    threshold(image, threshImage, thresh, 255, THRESH_BINARY);
    
    // Calculate Hough transform
    Mat houghSpace;
    houghTransform(threshImage, houghSpace);

    // Find global maximum in Hough space ...
    //Point houghMaxLocation;
    GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);
    // Prepare Hough space image for display
    houghSpace = 255 - houghSpace;										// Invert
    drawHoughLineLabels(houghSpace);									// Axes
    imshow(WINDOW_NAME_HOUGH_TRANS, houghSpace);
    imshow(WINDOW_NAME_THRESHOLD, threshImage);
}

void onMouseHoughParam(int event, int x, int y, int flags, void* imagePtr) {
    Mat& image = *(Mat*)imagePtr;

    int tau = getTrackbarPos(TRACKBAR_NAME_THRESHOLD, WINDOW_NAME_THRESHOLD);

    if (event == EVENT_LBUTTONDOWN) {
        updateHough(tau, x, y, &image);
    }


}

void updateHough(int tau, int x, int y, void* imagePtr) {
    Mat& image = *(Mat*)imagePtr;
    Mat processImage = image.clone();
    Mat edgeImage, SobelImage;
    Mat grayImage;

    cvtColor(processImage, grayImage, cv::COLOR_BGR2GRAY);


    sobelFilter(grayImage, SobelImage);
    threshold(SobelImage, edgeImage, tau, 255, THRESH_BINARY);

    // Calculate Hough transform
    Mat houghSpace;
    houghTransform(edgeImage, houghSpace);

    // Find local maximum in Hough space ...
    // define region 
    int roiSize = 10;  
    Rect roi(max(0, x - roiSize), max(0, y - roiSize), 2 * roiSize, 2 * roiSize);

    // get region from houshpace
    Mat roiHoughSpace = houghSpace(roi);
    Point houghMaxLocation;
    GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);
    minMaxLoc(roiHoughSpace, NULL, NULL, NULL, &houghMaxLocation);
    // calculate maximum in region in original image
    houghMaxLocation.x += roi.x;
    houghMaxLocation.y += roi.y;


    // ... and draw corresponding line in original image
    double r, theta;
    houghSpaceToLine(
        Size(edgeImage.cols, edgeImage.rows),
        Size(houghSpace.cols, houghSpace.rows),
        //x, y, r, theta);
        houghMaxLocation.x, houghMaxLocation.y, r, theta);
    drawLine(processImage, r, theta);

    Point p(x, y);
    // Prepare Hough space image for display
    houghSpace = 255 - houghSpace;										// Invert
    drawHoughLineLabels(houghSpace);									// Axes
    //circle(houghSpace, p, 10, Scalar(0, 0, 255), 2);		// Global maximum
    circle(houghSpace, houghMaxLocation, 10, Scalar(0, 0, 255), 2);		// local maximum

    // Display image in named window
    imshow(WINDOW_NAME_ORIGINAL_IMG, processImage);
    imshow(WINDOW_NAME_EDGE_IMG, edgeImage);
    imshow(WINDOW_NAME_HOUGH_TRANS, houghSpace);
}




/* Main function */
int main(){
 


    /* create input image file path with env var */
    string inputImagePath = string(DATA_ROOT_PATH).append(INPUT_IMAGE);

    /* load image from file */
    Mat image = cv::imread(inputImagePath, cv::IMREAD_ANYCOLOR);

    if (image.empty()) {
        cout << "[ERROR] cannot open iamge: " << inputImagePath << endl;
        return EXIT_FAILURE;
    }

    /* display image in named window */
    imshow("Image BGR", image);
    Mat imageRaw = image.clone();


    /***
     * edge image 
    ***/

    /* 1. generate a grayscale image of the edge strengths using a method of your choice */
    Mat grayImage;
    cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    /* display image in named window */
    imshow("Image grayscale", grayImage);
    waitKey(0);
    destroyAllWindows();
    


    /* canny edge detection */
    Mat edgeImageCanny;
    Canny(grayImage, edgeImageCanny, 100, 150);
    imshow("edge image canny", edgeImageCanny);


    /* sobel filter */
    Mat gradX, gradY;
    Sobel(grayImage, gradX, CV_64F, 1, 0, 1); 
    Sobel(grayImage, gradY, CV_64F, 0, 1, 1);

    /* calc gradient magnitude */
    Mat gradMagnitude;
    magnitude(gradX, gradY, gradMagnitude);

    /* convert back top 8 bit */
    Mat edgeImageSobel;
    gradMagnitude.convertTo(edgeImageSobel, CV_8U);

    imshow("edge image sobel", edgeImageSobel);
    waitKey(0);
    destroyAllWindows();
   

    /***
     * 2. Create a binary edge image from the edge strengths using the threshold() method,
     * where edge pixels have a value of 255, and all other pixels have a value of 0.
     * Display the edge image in a window.
    ***/
    Mat binaryEdges;
    double tau = 75; // initial threshold value
    threshold(edgeImageCanny, binaryEdges, tau, 255, THRESH_BINARY);
    binaryEdges.convertTo(binaryEdges, CV_8U);

    imshow("edge image binary threshold canny", binaryEdges);

    threshold(edgeImageSobel, binaryEdges, tau, 255, THRESH_BINARY);
    binaryEdges.convertTo(binaryEdges, CV_8U);

    imshow("edge image binary threshold sobel", binaryEdges);
    waitKey(0);
    destroyAllWindows();
    

    /***
     * 3. 4. 5.
    ***/

    // Calculate edge image
    Mat edgeImage, SobelImage;
    sobelFilter(grayImage, SobelImage);
    //imshow("SOBEL", SobelImage);
    threshold(SobelImage, edgeImage, EDGE_IMAGE_THRESHOLD, 255, THRESH_BINARY);
    imshow(WINDOW_NAME_THRESHOLD, edgeImage);


    // Calculate Hough transform
    Mat houghSpace;
    houghTransform(edgeImage, houghSpace);

    // Find global maximum in Hough space ...
    Point houghMaxLocation;
    GaussianBlur(houghSpace, houghSpace, Size(SMOOTHING_KERNEL_SIZE, SMOOTHING_KERNEL_SIZE), 0.0);
    minMaxLoc(houghSpace, NULL, NULL, NULL, &houghMaxLocation);

    // ... and draw corresponding line in original image
    double r, theta;
    houghSpaceToLine(
        Size(edgeImage.cols, edgeImage.rows),
        Size(houghSpace.cols, houghSpace.rows),
        houghMaxLocation.x, houghMaxLocation.y, r, theta);
    drawLine(image, r, theta);

    // Prepare Hough space image for display
    houghSpace = 255 - houghSpace;										// Invert
    drawHoughLineLabels(houghSpace);									// Axes
    circle(houghSpace, houghMaxLocation, 10, Scalar(0, 0, 255), 2);		// Global maximum

    // Display image in named window
    imshow(WINDOW_NAME_ORIGINAL_IMG, image);
    imshow(WINDOW_NAME_EDGE_IMG, edgeImage);
    imshow(WINDOW_NAME_HOUGH_TRANS, houghSpace);

    waitKey(0);


    /***
     * 6. 7. 8.
    ***/
    /* add window sliders ("trackbars") */
    createTrackbar(TRACKBAR_NAME_THRESHOLD, WINDOW_NAME_THRESHOLD, NULL, 255, onTrackbarThreshold, &SobelImage);
    setTrackbarPos(TRACKBAR_NAME_THRESHOLD, WINDOW_NAME_THRESHOLD, INITIAL_THRESHOLD);

    /* set mouse callback for Hough Transform */
    setMouseCallback(WINDOW_NAME_HOUGH_TRANS, onMouseHoughParam, &imageRaw);

    waitKey(0);

    return EXIT_SUCCESS;

}




// Programm ausführen: STRG+F5 oder Menüeintrag "Debuggen" > "Starten ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Menü "Debuggen starten"

// Tipps für den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzufügen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzufügen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzufügen", um dem Projekt vorhandene Codedateien hinzuzufügen.
//   6. Um dieses Projekt später erneut zu öffnen, wechseln Sie zu "Datei" > "Öffnen" > "Projekt", und wählen Sie die SLN-Datei aus.
