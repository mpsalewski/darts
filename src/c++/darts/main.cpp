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


/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
#define DATA_ROOT_PATH getenv("ImagingData")
#define INPUT_IMAGE "/image_data/images/misc/Docks.jpg"
#define OUTPUT_IMAGE "/image_data/images/misc/Docks_greyscale.jpg"
#define VIDEO_PATH "/image_data/videos/SoccerShot.mp4"
#define FPS 30
#define WAIT_TIME_MS (1000 / FPS)



/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/



/************************** Function Definitions *****************************/





/* Main function */
int main(){
    
#if 0
    /***
    * IMAGES BASICS
    ***/
    /* create input image file path with env var */
    string inputImagePath = string(DATA_ROOT_PATH).append(INPUT_IMAGE);
    /* create output file path with env var */
    string outputImagePath = string(DATA_ROOT_PATH).append(OUTPUT_IMAGE);

    /* load image from file */
    Mat image = cv::imread(inputImagePath, cv::IMREAD_ANYCOLOR);
    if (image.empty()) {
        cout << "[ERROR] cannot open iamge: " << inputImagePath << endl;
        return EXIT_FAILURE;
    }
    /* display image in named window */
    imshow("Image 01", image);

    /* wait for keypress and terminate */
    waitKey(0);

    /* load again as grayscale */
    image = imread(inputImagePath, IMREAD_GRAYSCALE);
    if (image.empty()) {
        cout << "[ERROR] cannot open iamge: " << inputImagePath << endl;
        return EXIT_FAILURE;
    }
    /* display image in named window */
    imshow("Image 02", image);

    /* wait for keypress and terminate */
    waitKey(0);

    /* save image to file */
    imwrite(outputImagePath, image);





    /***
    * VIDEO BASICS
    ***/
    /* create input video file path with env var */
    string inVidPath = string(DATA_ROOT_PATH).append(VIDEO_PATH);
    VideoCapture video(inVidPath);
    if (!video.isOpened()) {
        cout << "[ERROR] cannot open video: " << inVidPath << endl;
        return EXIT_FAILURE;
    }

    double fps = video.get(CAP_PROP_FPS);
    int waitTimeMs = (int)(1000.0 / fps);

    /* loop through frames */
    Mat frame;
    while (1) {
        /* get next frame from file */
        video >> frame;

        if (frame.empty())
            break;

        imshow("Video [press any key to quit]", frame);

        if (waitKey(waitTimeMs) >= 0)
            break;

    }
    /* free resoruces */
    video.release();
    waitKey(0);
#endif     

    Mat frame;


    /* open camera */
    VideoCapture camera(2);
    if (!camera.isOpened()) {
        cout << "[ERROR] cannot open Camera" << endl;
        return EXIT_FAILURE;
    }

    int imageCount = 1; // Z�hler f�r die Bildnamen

    /* loop through frames */
    while (1) {
        /* get next frame from cam */
        camera >> frame;

        if (frame.empty())
            break;

        imshow("Camera [press any key to quit]", frame);
        
        //if (waitKey(WAIT_TIME_MS) >= 0)
            //break;
        if (waitKey(1) == ' ') { // Wenn die Leertaste gedr�ckt wird
            // Erzeuge den Dateinamen f�r das Bild
            ostringstream fileName;
            fileName << "bild" << imageCount << ".jpg";
            string outputImagePath = fileName.str();
            
            // Speichere das Bild
            imwrite(outputImagePath, frame);
            std::cout << "Bild gespeichert: " << outputImagePath << std::endl;

            // Z�hler hochz�hlen
            imageCount++;
        }
        else if (waitKey(1) >= 0) {
            // Bricht die Schleife ab, wenn eine andere Taste als die Leertaste gedr�ckt wird
            break;
        }

    }
    /* free resoruces */
    camera.release();
    waitKey(0);

    return EXIT_SUCCESS;

}




// Programm ausf�hren: STRG+F5 oder Men�eintrag "Debuggen" > "Starten ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Men� "Debuggen starten"

// Tipps f�r den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzuf�gen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzuf�gen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzuf�gen", um dem Projekt vorhandene Codedateien hinzuzuf�gen.
//   6. Um dieses Projekt sp�ter erneut zu �ffnen, wechseln Sie zu "Datei" > "�ffnen" > "Projekt", und w�hlen Sie die SLN-Datei aus.
