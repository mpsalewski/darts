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
//#include "calibration.h"
#include "ImageProc.h"
#include <thread>
#include <atomic>
#include <chrono>


/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
#define WAIT_TIME_MS 100    // 10 Hz SamplingRate 
#define TOP_CAM     2
#define LEFT_CAM    1
#define RIGHT_CAM   3



/************************** local Structue ***********************************/




/************************* local Variables ***********************************/
/* control threads */
atomic<bool> running(true);


/************************** Function Declaration *****************************/
void camThread(int threadId);



/************************** Function Definitions *****************************/
// Beispiel-Funktion für einen Thread
void camThread(int threadId) {

    Mat CamFrame;
    /* open camera */
    VideoCapture camera(threadId);
    if (!camera.isOpened()) {
        cout << "[ERROR] cannot open Camera" << endl;
        return;
    }

    /* create camera window */ 
    ostringstream CamName;
    CamName << "Cam " << threadId << " [press Esc to quit]";
    string camWindowName = CamName.str();
   

    while (running){
        
        /* quit on [Esc] */
        if ((cv::waitKey(1) == 27)) {
            break;
        }

        /* get next frame from cam */
        camera >> CamFrame;
        if (CamFrame.empty())
            break;
        imshow(camWindowName, CamFrame);
        
        /* Image Processing */







        /* define sampling rate */
        this_thread::sleep_for(chrono::milliseconds(100)); 

    }

    /* free resoruces */
    camera.release();

    /* Thread finished */
    cout << "Thread " << threadId << " beendet.\n";
}

/* Main function */
int main(){
    
   
    /* open parallel cams */
    std::thread topCam(camThread, TOP_CAM);
    std::thread leftCam(camThread, LEFT_CAM);
    std::thread rightCam(camThread, RIGHT_CAM);



    /* wait on enter to quit */
    cout << "Press Enter to quit Threads...\n";
    cin.get(); 

    /* kill threads */ 
    running = false;


    /* clear threads */
    topCam.join();
    leftCam.join();
    rightCam.join();

    destroyAllWindows();

   
    cv::waitKey(0);

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






Mat frame;


/* open camera */
VideoCapture camera(1);
if (!camera.isOpened()) {
    cout << "[ERROR] cannot open Camera" << endl;
    return EXIT_FAILURE;
}

int imageCount = 1; // Zähler für die Bildnamen

/* loop through frames */
while (1) {
    /* get next frame from cam */
    camera >> frame;

    if (frame.empty())
        break;

    imshow("Camera [press any key to quit]", frame);

    //if (waitKey(WAIT_TIME_MS) >= 0)
        //break;
    if ((waitKey(WAIT_TIME_MS) == ' ')) { // Wenn die Leertaste gedrückt wird
        // Erzeuge den Dateinamen für das Bild
        ostringstream fileName;
        fileName << "bild" << imageCount << ".jpg";
        string outputImagePath = fileName.str();

        // Speichere das Bild
        imwrite(outputImagePath, frame);
        std::cout << "Bild gespeichert: " << outputImagePath << std::endl;

        // Zähler hochzählen
        imageCount++;
    }

    if ((waitKey(WAIT_TIME_MS) == 27)) {
        break;
    }

    }
/* free resoruces */
camera.release();
waitKey(0);




#endif  