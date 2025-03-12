/******************************************************************************
 *
 * external_api.cpp
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
 * created on :     2025-13-02
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
 *
 *
******************************************************************************/




/***************************** includes **************************************/
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "image_proc.h"
#include <opencv2/opencv.hpp>
#include "calibration.h"
#include "Sobel.h"
#include "HoughLine.h"
#include "dart_board.h"
#include "cams.h"
#include "globals.h"
#include "command_parser.h"
#include <cstring>
#include <cstdio>
#include <limits>
#include "external_api.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>


/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/


/************************** local Structure ***********************************/
struct flags_s {
    int busted = 0;
};

static struct ext_api_s {
    struct flags_s flags;

    int score_val = 0;
    int score_factor = 0;

}ext_api;

/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/
using json = nlohmann::json;
void send_to_flask(const int& thr_val, int thr_mlt) {
    CURL* curl;
    CURLcode res;

    // JSON-Daten vorbereiten
    json j;
    j["Mode"] = 0;
    j["ThrVal"] = thr_val;
    j["ThrMlt"] = thr_mlt;

    // Flask API-URL
    const std::string url = "http://192.168.2.133:5000/api/api";

    // libcurl initialisieren
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Anfrage einrichten
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Setze den JSON-Body
        std::string json_data = j.dump();  // JSON als string
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_data.size());

        // Header hinzufügen (Content-Type für JSON)
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Anfrage ausführen
        res = curl_easy_perform(curl);

        // Überprüfen, ob es Fehler gab
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Bereinigung
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Funktion zum Abrufen des "Mode"-Werts von der Flask-API
int get_mode_from_flask() {
    CURL* curl;
    CURLcode res;
    std::string response_string;
    const std::string url = "http://192.168.2.133:5000/api/get_mode";  // API-Endpunkt für Mode-Wert

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Fehler bei curl_easy_perform(): " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    // JSON parsen und Mode-Wert zurückgeben
    try {
        json j = json::parse(response_string);
        if (j.contains("Mode")) {
            return j["Mode"];
        }
    }
    catch (json::parse_error& e) {
        std::cerr << "JSON-Fehler: " << e.what() << std::endl;
    }

    return -1;  // Fehlerfall
}


/**************************** External API Thread ****************************/
void external_api_thread(void* arg) {


    /* assign void pointer, thread safe exchange; */
    struct thread_share_s* t_s = (struct thread_share_s*)(arg);

    static struct ext_api_s* ea = &ext_api;


    this_thread::sleep_for(chrono::milliseconds(500));
    /* loop */
    while (running == 1) {

        /* do the event handling */
        /* check here from external api if you are busted and handle it */
        #if 0
        ea->flags.busted = get_mode_from_flask();

        if (ea->flags.busted == 1) {
            cams_external_bust();
        }
        #endif
        /* check if there was a new dart */
        t_s->mutex.lock();
        if (t_s->single_score_flag) {
            
            /* clear flag */
            t_s->single_score_flag = 0;

            /* send here the score to external api */
            //int example1 = t_s->single_score;
            //string example2 = t_s->single_score_str;
            external_api_split_val_fact(t_s->single_score, t_s->single_score_str, ea->score_val, ea->score_factor);
            // cout << ea->score_val << "\t" << ea->score_factor << endl;   // Debug 
            send_to_flask(ea->score_val, -ea->score_factor);
        }
        t_s->mutex.unlock();

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    /* kill all threads */
    running = false;

}


void external_api_split_val_fact(int score, std::string score_str, int& val, int&factor) {


    char c = score_str[0];
 
    /* invalid input */
    if (c == '\0')
        return;

    /* out of board */
    if (!score) {
        val = 0;
        factor = 0;
        return;
    }




    /* get factor */
    switch (c) {
        case 'S': 
            factor = 1;
            break;

        case 'D':
            factor = 2;
            break;

        case 'T':
            factor = 3;
            break;

        default: break;
    }


    val = score / factor; 


}
