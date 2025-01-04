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
 * author:          m. salewski
 * created on:
 * last revision:
 *
 ******************************************************************************/


/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()
/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>


/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
#define DATA_ROOT_PATH getenv("ImagingData")
#define INPUT_IMAGE "/image_data/images/misc/Parrot.JPG"
#define BINOMIAL_FILTER_SIZE 9



/************************* local Variables ***********************************/




/************************** Function Declaration *****************************/
void onTrackbarAlpha(int alphaPercent, void* imagePtr);
void onMouseSplitScreen(int event, int x, int y, int flags, void* userdata);
void processAndDisplay(Mat& image, double alphaPercent, int splitX = -1);
void unsharpMasking(const Mat& source, Mat& processed, double alpha);


/************************** Function Definitions *****************************/
/* Callback for trackbar to adjust alpha */
void onTrackbarAlpha(int alphaPercent, void* imagePtr) {
    Mat& image = *(Mat*)imagePtr;
    Mat processedImage;
    double alpha = alphaPercent / 100.0; // Convert percentage to alpha
    processAndDisplay(image, alpha);    // Update to use unified processing and display function
}

/* Callback for mouse events to handle split screen */
void onMouseSplitScreen(int event, int x, int y, int flags, void* userdata) {
    Mat& image = *(Mat*)userdata;
    static int splitX = -1;
    static int mouseTrack = 0;


    if (event == EVENT_LBUTTONDOWN) {
        splitX = x;
        processAndDisplay(image, -1, splitX); // alpha=-1 to use mem val
        mouseTrack = 1;
    }
    else if (event == EVENT_LBUTTONUP) {
        mouseTrack = 0;
        // empty 
    }
    else if ((event == EVENT_MOUSEMOVE) && mouseTrack) {
        //splitX = x;
        //processAndDisplay(image, -1, splitX); // alpha=-1 to use mem val
        // empty
    }
}

/* Apply unsharp masking to an image */
void unsharpMasking(const Mat& source, Mat& processed, double alpha) {
    // Define 9x9 Binomial Filter
    /*Mat hLP = (Mat_<float>(9, 9) <<
        1, 8, 28, 56, 70, 56, 28, 8, 1,
        8, 28, 70, 112, 140, 112, 70, 28, 8,
        28, 70, 140, 210, 252, 210, 140, 70, 28,
        56, 112, 210, 336, 420, 336, 210, 112, 56,
        70, 140, 252, 420, 504, 420, 252, 140, 70,
        56, 112, 210, 336, 420, 336, 210, 112, 56,
        28, 70, 140, 210, 252, 210, 140, 70, 28,
        8, 28, 70, 112, 140, 112, 70, 28, 8,
        1, 8, 28, 56, 70, 56, 28, 8, 1
        );*/
    //hLP /= 256.0; // Normalize filter

    Mat blurred;

    Mat kernelX = (Mat_<double>(1, BINOMIAL_FILTER_SIZE) << 1, 8, 28, 56, 70, 56, 28, 8, 1);
    Mat kernelY = (Mat_<double>(BINOMIAL_FILTER_SIZE, 1) << 1, 8, 28, 56, 70, 56, 28, 8, 1);
    kernelX /= sum(kernelX);
    kernelY /= sum(kernelY);

    sepFilter2D (source, blurred, CV_8U, kernelX, kernelY);
    //filter2D(source, blurred, CV_8U, kernelX);
    //filter2D(source, blurred, CV_8U, kernelY);
    //filter2D(source, blurred, CV_8U, hLP); // Apply filter

    processed = (1 + alpha) * source - alpha * blurred; // Unsharp masking formula

    // Ensure valid pixel values (0-255)
    processed.convertTo(processed, CV_8U);
}

/* Display image with optional split-screen effect */
void processAndDisplay(Mat& image, double alphaPercent, int splitX) {
    Mat processedImage;
    Vec3b color(0, 0, 255);
    static int splitX_mem = image.cols / 2;
    static double alpha_mem = 0.5;
 
    /* check if there is a new val to set */
    if (splitX >= 0) {
       splitX_mem = splitX;
    }
    if (alphaPercent >= 0) {
        alpha_mem = alphaPercent;
    }

    unsharpMasking(image, processedImage, alpha_mem);

    // Combine original and processed images with a split line
    Mat displayImage = image.clone();
    processedImage.colRange(0, splitX_mem).copyTo(displayImage.colRange(0, splitX_mem));

    // Draw red line to indicate split
    cvtColor(displayImage, displayImage, COLOR_GRAY2BGR);
    line(displayImage, Point(splitX_mem, 0), Point(splitX_mem, image.rows), color, 2);
    imshow("Unsharp Masking", displayImage);

}

/* Main function */
int main() {
    // Create input image file path
    string inputImagePath = string(DATA_ROOT_PATH).append(INPUT_IMAGE);

    // Load image from file
    Mat image = imread(inputImagePath, IMREAD_GRAYSCALE);
    if (image.empty()) {
        cout << "[ERROR] Cannot open image: " << inputImagePath << endl;
        return EXIT_FAILURE;
    }

    // Show the original image
    imshow("Original Image", image);

    // Show Unsharped Masked Image  
    imshow("Unsharp Masking", image);

    // Add trackbar for alpha adjustment
    createTrackbar("Alpha", "Unsharp Masking", nullptr, 100, onTrackbarAlpha, &image);

    // Set mouse callback for split screen
    setMouseCallback("Unsharp Masking", onMouseSplitScreen, &image);

    // Process and display initial image with default alpha = 0.5
    processAndDisplay(image, 1);

    // Wait for keypress and exit
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
