/******************************************************************************
 *
 * main.cpp
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
 * author(s):   	Mika Paul Salewski <mika.paul.salewski@gmail.com>, 
 *                  Lukas Grose <lukas@grose.de>  
 * created on :     2025-01-06
 * last revision :  None
 *
 *
 *
 * Copyright (c) 2025, Mika Paul Salewski, Lukas Grose 
 * Version: 2025.01.06
 * License: CC BY-NC-SA 4.0,
 *      see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 *
 * 
 * Further information about this source-file:
 *      None
******************************************************************************/


/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/***************************** includes **************************************/
#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "ImageProc.h"
#include <thread>
#include <atomic>
#include <chrono>
#include "calibration.h"
#include "HoughLine.h"
#include "Sobel.h"
#include "dart_board.h"
#include "globals.h"
#include "cams.h"
#include <cstring>
#include "command_parser.h"


/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/
/* main program */
#define THREADING 1                 // use Camera Threads
#define SIMULATION 1                // use Simluation Cams Thread instead of 
                                    // real cams Thread
#define LOAD_STATIC_TEST_IMAGES 0   // use this macro for debugging and test







/************************** local Structure ***********************************/
/* score, exchange between threads */
struct thread_exchange_s t_e;   // GLOBAL!
struct thread_share_s t_s;      


/* default init in main() via pointer */
//static struct darts_s darts;


/************************* local Variables ***********************************/
/* control threads */
atomic<bool> running(true);     // GLOBAL!


/************************** Function Declaration *****************************/
void camThread(int threadId);
void static_test(void);






/****************************** main function ********************************/
int main() {

    //dart_board_create_scoreboard_gui(&game);
    
#if 0    
    /* create ptr p for strcuture handling */
    struct darts_s* p = &darts;

    /* default init struct */
    p->flags.diff_flag_top = 0;
    p->flags.diff_flag_right = 0;
    p->flags.diff_flag_left = 0;
    p->flags.diff_flag_raw = 0;


    p->count_throws = 0;
    
    p->t_line.line_top.r = 0;
    p->t_line.line_top.theta = 0;
    p->t_line.line_right.r = 0;
    p->t_line.line_right.theta = 0;
    p->t_line.line_left.r = 0;
    p->t_line.line_left.theta = 0;
#endif 


    /* always register the command line */
    thread command_line(commandLineThread, &t_s);

    cout << "type 'exit' to quit" << endl;


/* use threads */
#if !SIMULATION && THREADING

    /* create cams thread */
    thread cams(camsThread, &darts);

    /* create darts gui thread */
    thread guiThread(Dartsboard_GUI_Thread);

    this_thread::sleep_for(chrono::milliseconds(2000));

    /* kill all threads through the command line thread */
    while (running == 1) {
        this_thread::sleep_for(chrono::milliseconds(500));
    };

    /* clear threads */
    cams.join();
    guiThread.join();
    
#elif THREADING
    /***
     * run simualtion cams thread
    ***/
    //thread SIM_cams(SIMULATION_OF_camsThread, &darts);
    thread SIM_cams(SIMULATION_OF_camsThread, &t_s);
    
    /* create darts gui thread */
    thread guiThread(Dartsboard_GUI_Thread, &t_s);

    this_thread::sleep_for(chrono::milliseconds(2000));

    /* simulate darts */
    /*
    int fin = 0;
    fin = dart_board_update_scoreboard_gui(&game, 180, "dont care");
    waitKey(3000);
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 1, "dont care");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 76, "dont care");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 450, "dont care");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    fin = dart_board_update_scoreboard_gui(&game, 500, "Double 250");
    if (fin > 0) {
        std::cout << "finished by: " << game.p[fin - 1].p_name << endl;
        dart_board_finish_scoreboard_gui(&game, fin - 1);
        waitKey(3000);
    }
    */

    /* wait on enter to quit */
    //std::cout << "Press Enter to quit Threads...\n";
    //cin.get();

    /* kill threads */
    //running = false;
    this_thread::sleep_for(chrono::milliseconds(2000));

    /* kill all threads through the command line thread */
    while (running == 1) {
        this_thread::sleep_for(chrono::milliseconds(500));
    };

    /* clear threads */
    SIM_cams.join();
    guiThread.join();

#endif 


/* Do Static Test with loaded images and fixed points for fast debugging ans test */
#if LOAD_STATIC_TEST_IMAGES

    static_test();

#endif 


    /* clear command line thread */
    command_line.join();

    destroyAllWindows();

    return EXIT_SUCCESS;

}



/************************** Function Definitions *****************************/



void static_test(void) {


    /* load image from file */
    Mat top_image = cv::imread(TOP_1DARTS, cv::IMREAD_ANYCOLOR);
    Mat top_raw = cv::imread(TOP_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    Mat top_image2 = cv::imread(TOP_2DARTS, cv::IMREAD_ANYCOLOR);
    Mat top_image3 = cv::imread(TOP_3DARTS, cv::IMREAD_ANYCOLOR);

    /* load image from file */
    Mat right_image = cv::imread(RIGHT_1DARTS, cv::IMREAD_ANYCOLOR);
    Mat right_raw = cv::imread(RIGHT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    Mat right_image2 = cv::imread(RIGHT_2DARTS, cv::IMREAD_ANYCOLOR);
    Mat right_image3 = cv::imread(RIGHT_3DARTS, cv::IMREAD_ANYCOLOR);

    /* load image from file */
    Mat left_image = cv::imread(LEFT_1DARTS, cv::IMREAD_ANYCOLOR);
    Mat left_raw = cv::imread(LEFT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    Mat left_image2 = cv::imread(LEFT_2DARTS, cv::IMREAD_ANYCOLOR);
    Mat left_image3 = cv::imread(LEFT_3DARTS, cv::IMREAD_ANYCOLOR);



    //destroyAllWindows();
    struct line_s line;
    struct tripple_line_s t_line;
    line.r = 1;
    line.theta = 99;

    top_raw = cv::imread(TOP_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    right_raw = cv::imread(RIGHT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    left_raw = cv::imread(LEFT_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);

#if 0 
    image_proc_get_line(top_raw, top_image, TOP_CAM, &line, SHOW_IMG_LINE, "Top Static Test");
    image_proc_get_line(right_raw, right_image, RIGHT_CAM, &line, SHOW_SHORT_ANALYSIS, "Right Static Test");
    image_proc_get_line(left_raw, left_image, LEFT_CAM, &line, SHOW_IMG_LINE, "Left Static Test");
#endif 
#if 1 
    image_proc_get_line(top_image, top_image2, TOP_CAM, &t_line.line_top, SHOW_IMG_LINE, "Top Static Test");
    image_proc_get_line(right_image, right_image2, RIGHT_CAM, &t_line.line_right, SHOW_IMG_LINE, "Right Static Test");
    image_proc_get_line(left_image, left_image2, LEFT_CAM, &t_line.line_left, SHOW_SHORT_ANALYSIS, "Left Static Test");
#endif
#if 0
    image_proc_get_line(top_image2, top_image3, TOP_CAM, &t_line.line_top, SHOW_IMG_LINE, "Top Static Test");
    image_proc_get_line(right_image2, right_image3, RIGHT_CAM, &t_line.line_right, SHOW_IMG_LINE, "Right Static Test");
    image_proc_get_line(left_image2, left_image3, LEFT_CAM, &t_line.line_left, SHOW_SHORT_ANALYSIS, "Left Static Test");
#endif


    Point cross_point;
    calibration_get_img(top_raw, top_raw, TOP_CAM);
    cv::imwrite("top_raw_cal.jpg", top_raw);
    img_proc_cross_point(top_raw.size(), &t_line, cross_point);
    cout << "Raw Cal Size" << top_raw.size() << endl;

    /*
    // Beispiel-Daten für eine standardisierte Dartscheibe
    // Beispielpixel (ein Punkt auf der Scheibe)
    //cv::Point2f pixel(390, 139); // Tripple 18
    //cv::Point2f pixel(419, 371); // Single 2
    //cv::Point2f pixel(399, 411); // Double 17
    //cv::Point2f pixel(225, 161); // Tripple 9
    //cv::Point2f pixel(142, 204); // Single 14
    //cv::Point2f pixel(176, 365); // Double 16
    //cv::Point2f pixel(272, 347); // Tripple 19
    //std::string result = determineSector(pixel, board);
    std::string result = determineSector(cross_point, board);

    std::cout << "Wert: " << result << std::endl;
    */
    struct result_s result;
    dart_board_determineSector(cross_point, TOP_CAM, &result);

    std::cout << "Wert: " << result.str << std::endl;

    Point pixel(272, 347); // Tripple 19
    dart_board_determineSector(pixel, TOP_CAM, &result);

    std::cout << "Wert: " << result.str << std::endl;



    cv::waitKey(0);
}


#if 0
/* old implementation, one thread per cam, delete later */
void camThread(int threadId) {

    Mat lastFrame;
    Mat currentFrame;
    Mat currentCalFrame;
    Mat diffFrame;
    Scalar diffVal;

    /* open camera */
    VideoCapture camera(threadId);
    if (!camera.isOpened()) {
        std::cout << "[ERROR] cannot open Camera" << endl;
        return;
    }

    /* create camera window */
    ostringstream CamName;
    CamName << "Cam " << threadId << " [press Esc to quit]";
    string camWindowName = CamName.str();
    ostringstream CamNameDiff;
    CamNameDiff << "Diff Cam " << threadId << " [press Esc to quit]";
    string camWindowNameDiff = CamNameDiff.str();

    /* load raw init image from file */
    /*Mat lastImg = cv::imread(TOP_RAW_IMG_CAL, cv::IMREAD_ANYCOLOR);
    if (lastImg.empty()) {
        cout << "[ERROR] cannot open iamge: " << TOP_RAW_IMG_CAL << endl;
        return;
    }*/
    camera >> lastFrame;
    if (lastFrame.empty()) {
        return;
    }




    while (running) {

        /* quit on [Esc] */
        if ((cv::waitKey(WAIT_TIME_MS) == 27)) {
            break;
        }

        /* get next frame from cam (current image) */
        camera >> currentFrame;
        if (currentFrame.empty())
            break;

        /* create camera window */
        //imshow(camWindowName, currentFrame);

        /* Image Processing */

        /* get calibrated view */
        //cal_get_images(CamFrame, CalImg);


        //cvtColor(currentFrame, currentFrame, COLOR_BGR2GRAY);
        GaussianBlur(currentFrame, currentFrame, Size(3, 3), 0);



        /* check difference */
        absdiff(currentFrame, lastFrame, diffFrame);
        Mat diffFrame_gray;

        cvtColor(diffFrame, diffFrame_gray, COLOR_BGR2GRAY);
        Mat binaryEdges;
        double tau = 25; // initial threshold value
        threshold(diffFrame_gray, binaryEdges, tau, 255, THRESH_BINARY);
        imshow(camWindowNameDiff, binaryEdges);
        diffVal = sum(diffFrame_gray);
        if (diffVal[0] > DIFF_THRESH) {
            std::cout << camWindowNameDiff << "Diff Val detected : " << diffVal[0] << endl;
            this_thread::sleep_for(chrono::milliseconds(1000));
        }

        /* update last frame */
        lastFrame = currentFrame.clone();


        /* define sampling rate */
        this_thread::sleep_for(chrono::milliseconds(10));

    }

    /* free resoruces */
    camera.release();

    /* Thread finished */
    std::cout << "Thread " << threadId << " beendet.\n";
}


#endif








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