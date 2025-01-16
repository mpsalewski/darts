/******************************************************************************
 *
 * command_parser.h
 *
 ******************************************************************************
 *
 * Origin:
 *
 * CommandParser.h - Library for parsing commands of the form "COMMAND_NAME ARG1 ARG2 ARG3 ...".
 *
 * Copyright 2020 Anthony Zhang (Uberi) <me@anthonyz.ca>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *
 ******************************************************************************
 *
 *      !!! NOTE: !!!
 * This file is based on and extends the original work by Anthony Zhang (Uberi).
 * Original work: Copyright 2020 Anthony Zhang (Uberi) <me@anthonyz.ca>
 *
 * Modifications and extensions include significant restructuring, functionality
 * updates, and integration into the Automated Dart Detection and Scoring System.
 * Large Parts are deleted and / or rewritten.
 *
 *      --> Original functions have been restructured into .c/.h files.
 *      --> Added various new features
 *
 ******************************************************************************
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
 * Author(s):   	Mika Paul Salewski <mika.paul.salewski@gmail.com>
 *
 * Created on :     2025-01-06
 * Last revision :  None
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
 *      --> Command parser for processing text-based commands received
 *          through a command line interface.
 *      --> Supports parsing commands with arguments in the format:
 *          "COMMAND_NAME ARG1 ARG2 ARG3 ...".
 *      --> Provides an extensible interface for adding new commands
 *          and handling errors gracefully.
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
#include "globals.h"
#include "command_parser.h"
#include <cstring>
#include <cstdio>
#include <limits>

/* compiler settings */
#define _CRT_SECURE_NO_WARNINGS     // enable getenv()

/****************************** namespaces ***********************************/
using namespace cv;
using namespace std;



/*************************** local Defines ***********************************/


/************************** local Structure ***********************************/
/* create commadn parser instance */
static CommandParser parser;


/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/


/**************************** Command Line Thread ****************************/
/* Command Line thread */
void commandLineThread(void*arg) {

    string input;
    int cmd_return = 0;
    char response[MAX_RESPONSE_SIZE];

    /* assign void pointer */


    /* init registers all commands */
    command_parser_cmd_init();

    /* welcome */
    cout << "Command Line is active" << endl;
    parser.processCommand("welcome", response);
    cout << response << endl << endl;

    /* flush command line  */
    cin.sync();


    this_thread::sleep_for(chrono::milliseconds(500));
    /* loop */
    while (running == 1) {


        /* read command line */
        getline(cin, input);


        /* quit on exit command */
        if (input == "exit") {
            cout << "quit() Command Line thread..." << std::endl;
            break;
        }

        
        /* process command */
        cmd_return = parser.processCommand(input.c_str(), response);
        cout << response << endl;
        if (cmd_return) {
            //cout << "ans" << response << endl;
        }
        else {
            //cout << "error: invalid input" << std::endl;
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }

    /* kill all threads */
    running = false;

}


/********************* command parser class func def *************************/


/* constructor */
CommandParser::CommandParser() : numCommands(0) {
    
    /* init args */
    for (int i = 0; i < MAX_COMMAND_ARGS; ++i) {
        commandArgs[i].asString[0] = '\0';  
    }

    /* init cmds */
    for (int i = 0; i < MAX_COMMANDS; ++i) {
        /* reset name arr */
        commandDefinitions[i].name[0] = '\0'; 

        /* reset arg types */
        commandDefinitions[i].argTypes[0] = '\0';

        /* set func ptr NULL */
        commandDefinitions[i].callback = nullptr;
    }

}

/* string parsing function */
size_t CommandParser::parseString(const char* buf, char* output) {
    size_t readCount = 0;
    bool isQuoted = buf[0] == '"';
    if (isQuoted) readCount++;

    size_t i = 0;
    for (; i < MAX_COMMAND_ARG_SIZE && buf[readCount] != '\0'; i++) {
        if (isQuoted ? buf[readCount] == '"' : buf[readCount] == ' ') break;
        if (buf[readCount] == '\\') {
            readCount++;
            switch (buf[readCount]) {
            case 'n': output[i] = '\n'; readCount++; break;
            case 'r': output[i] = '\r'; readCount++; break;
            case 't': output[i] = '\t'; readCount++; break;
            case '"': output[i] = '"'; readCount++; break;
            case '\\': output[i] = '\\'; readCount++; break;
            case 'x': {
                readCount++;
                output[i] = 0;
                for (size_t j = 0; j < 2; j++, readCount++) {
                    if ('0' <= buf[readCount] && buf[readCount] <= '9') output[i] = output[i] * 16 + (buf[readCount] - '0');
                    else if ('a' <= buf[readCount] && buf[readCount] <= 'f') output[i] = output[i] * 16 + (buf[readCount] - 'a') + 10;
                    else if ('A' <= buf[readCount] && buf[readCount] <= 'F') output[i] = output[i] * 16 + (buf[readCount] - 'A') + 10;
                    else return 0;
                }
                break;
            }
            default: return 0;
            }
        }
        else {
            output[i] = buf[readCount];
            readCount++;
        }
    }
    if (isQuoted) {
        if (buf[readCount] != '"') return 0;
        readCount++;
    }

    output[i] = '\0';
    return readCount;
}

/* register commands */
bool CommandParser::registerCommand(const char* name, const char* argTypes, void (*callback)(Argument* args, size_t argCount,char* response), const char* help) {
    /* check if your allowed to register more commands */
    if (numCommands >= MAX_COMMANDS) {
        return false;
    }
    /* check command name length */
    if (strlen(name) > MAX_COMMAND_NAME_LENGTH) {
        return false;
    }
    /*check arg num */
    if (strlen(argTypes) > MAX_COMMAND_ARGS) {
        return false;
    }
    /* check if there is a valid callback */
    if (!callback) {
        return false;
    }
    /* check command name length */
    if (strlen(help) > MAX_HELP_LENGTH) {
        return false;
    }
    /* register in CommandParser Class */
    strncpy_s(commandDefinitions[numCommands].name, name, MAX_COMMAND_NAME_LENGTH);
    strncpy_s(commandDefinitions[numCommands].argTypes, argTypes, MAX_COMMAND_ARGS);
    commandDefinitions[numCommands].callback = callback;
    strncpy_s(commandDefinitions[numCommands].help, help, MAX_HELP_LENGTH);

    /* increment numer of cmds */
    numCommands++;
    
    return true;
}

/* process commands */
bool CommandParser::processCommand(const char* command, char* response) {
    char name[MAX_COMMAND_NAME_LENGTH + 1];
    size_t i = 0;

    /* extract command */
    while (*command != ' ' && *command != '\0' && i < MAX_COMMAND_NAME_LENGTH) {
        name[i++] = *command++;
    }
    name[i] = '\0';

    /* get cmd def */
    char* argTypes = nullptr;
    void (*callback)(Argument*, size_t argCount, char*) = nullptr;
    for (size_t i = 0; i < numCommands; i++) {
        if (strcmp(commandDefinitions[i].name, name) == 0) {
            argTypes = commandDefinitions[i].argTypes;
            callback = commandDefinitions[i].callback;
            break;
        }
    }
    /* check if there was matching command definition */
    if (argTypes == nullptr) {
        snprintf(response, MAX_RESPONSE_SIZE, "parse error: unknown command name %s", name);
        return false;
    }




    Argument commandArgs[MAX_COMMAND_ARGS] = { 0 };  // array var for args
    size_t argIndex = 0;

    /* command without args */
    if (argTypes[0] == ' ') {
        commandArgs[0].asString[0] = ' ';
        /* execute callbacks */
        if (callback) {
            callback(commandArgs, argIndex, response);
        }
        return true;
    }
    else {

        /* parse each arg */
        for (size_t i = 0; ((argTypes[i] != '\0') && (*command != '\0')); i++,argIndex++) {
            /* require and skip 1 or more whitespace characters */
            if (*command != ' ') {
                snprintf(response, MAX_RESPONSE_SIZE, "parse error: missing whitespace before arg %d", (int)(i + 1));
                return false;
            }
            /* get first arg */
            command++;
            while (*command == ' ') {
                command++;
            }
            //do { command++; } while (*command == ' ');

            /* recognize correct arg type */
            switch (argTypes[i]) {

                /* double argument */
                case 'd': { 
                    char* after;
                    commandArgs[i].asDouble = strtod(command, &after);
                    if (after == command || (*after != ' ' && *after != '\0')) {
                        snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid double for arg %d", (int)(i + 1));
                        return false;
                    }
                    command = after;
                    break;
                }
                /* uint64_t argument */
                case 'u': {
                    try {
                        size_t pos;
                        uint64_t value = std::stoull(command, &pos, 0); // Basis 0 erlaubt die Erkennung von Dezimal-, Hexadezimal- oder Oktalzahlen.

                        if (value > std::numeric_limits<uint64_t>::max()) {
                            snprintf(response, MAX_RESPONSE_SIZE, "parse error: value out of range for uint64_t for arg %d", (int)(i + 1));
                            return false;
                        }

                        if (command[pos] != ' ' && command[pos] != '\0') {
                            snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid uint64_t for arg %d", (int)(i + 1));
                            return false;
                        }

                        commandArgs[i].asUInt64 = value;
                        command += pos;
                        }
                    catch (const std::invalid_argument&) {
                        snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid uint64_t for arg %d", (int)(i + 1));
                        return false;
                    }
                    catch (const std::out_of_range&) {
                        snprintf(response, MAX_RESPONSE_SIZE, "parse error: value out of range for uint64_t for arg %d", (int)(i + 1));
                        return false;
                    }
                    break;
                }
                /* int64_t argument */
                case 'i': { 
                    try {
                        size_t pos;
                        int64_t value = std::stoll(command, &pos, 0); // Basis 0 erlaubt Erkennung von Dezimal-, Hexadezimal- und Oktalzahlen.

                        if (value < std::numeric_limits<int64_t>::min() || value > std::numeric_limits<int64_t>::max()) {
                            snprintf(response, MAX_RESPONSE_SIZE, "parse error: value out of range for int64_t for arg %d",(int)(i + 1));
                            return false;
                        }

                        if (command[pos] != ' ' && command[pos] != '\0') {
                            snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid int64_t for arg %d",(int)(i + 1));
                            return false;
                        }

                        commandArgs[i].asInt64 = value;
                        command += pos;
                    }
                    catch (const std::invalid_argument&) {
                        snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid int64_t for arg %d",(int)(i + 1));
                        return false;
                    }
                    catch (const std::out_of_range&) {
                        snprintf(response, MAX_RESPONSE_SIZE, "parse error: value out of range for int64_t for arg %d",(int)(i + 1));
                        return false;
                    }
                    break;
                }
                /* string argument */
                case 's': {
                    size_t readCount = parseString(command, commandArgs[i].asString);
                    if (readCount == 0) {
                        snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid string for arg %d",(int)(i + 1));
                        return false;
                    }
                    command += readCount;
                    break;
                }
                default:
                    snprintf(response, MAX_RESPONSE_SIZE, "parse error: invalid argtype %c for arg %d", argTypes[i],(int)(i + 1));
                    return false;
            }
        }
    }
    // skip whitespace
    while (*command == ' ') { command++; }

    // ensure that we're at the end of the command
    if (*command != '\0') {
        snprintf(response, MAX_RESPONSE_SIZE, "parse error: too many args (expected %d)", (int)strlen(argTypes));
        return false;
    }

    // set response to empty string
    response[0] = '\0';

    /* execute callbacks */
    if (callback) {
        callback(commandArgs, argIndex, response);
    }
    return true;

}


/* process help command */
void CommandParser::process_help_Command(Argument* args, size_t argCount, char* response){


    /* "help" call without args, show every help command */
    if (argCount == 0) {
        /* create a response */
        snprintf(response, MAX_RESPONSE_SIZE, "help menu done.\n");
        /* show every help string */
        for (size_t i = 0; i < numCommands; i++) {
            
            cout << commandDefinitions[i].name << "\t" << commandDefinitions[i].help << endl;
            
        }

        return;

    }

    bool matched_cmd = 0;
    /* show help of specific commnad passed by first arg */
    for (size_t i = 0; i < numCommands; i++) {
        if (strcmp(commandDefinitions[i].name, args[0].asString) == 0) {
            cout << commandDefinitions[i].name << "\t" << commandDefinitions[i].help << endl;
            matched_cmd = 1;
            break;
        }
    }

    /* no matching command */
    if (!matched_cmd) {
        snprintf(response, MAX_RESPONSE_SIZE, "parse error: unknown command name %s", args[0].asString);

        return;
    }


    /* create a response */
    snprintf(response, MAX_RESPONSE_SIZE, " ");

    return;

}



/************************** Function Definitions *****************************/
void command_parser_cmd_init(void){

    /***
     * REGISTER YOUR COMMANDS
    ***/

    /* help command */
    if (!parser.registerCommand("help", "s", help_Cb,
        "help menu: [type: 'help $CMD_NAME$' for specific command]\nexit\tshutdown program"
    )) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }

    /* "hello world" command */
    if (!parser.registerCommand("hello", "s", helloCommandCallback,
        "hello world of the command line. type 'hello max' to echo."
        )) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }

    /* welcome command */
    if (!parser.registerCommand("welcome", " ", welcomeCb,
        "just a welcome :)"
        )) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }

    /* new game command, max 4 players */
    if (!parser.registerCommand("new", "ssss", set_new_game_Cb,
        "create new darts game. example: new Lars Peter"
        )) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }

    /* set parametrrs; read everything as string to be more flexibel on the set command */
    if (!parser.registerCommand("set", "sss", set_params,
        "set parameters \
        \n\tset parameters for the ScoreBoard:\n\t\t-> set score $NAME$ $SCORE$\n\t\t-> set leg $NAME$ $NUM$ not defined atm \
        \n\tset parameters for image processing:\n\t\t-> set diff_min $intValue$ (set minimum difference value)\n\t\t-> set bin_thresh $intValue$ (set threshold value fir binarizing)"
        )) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }



}


/* help command */
void help_Cb(CommandParser::Argument* args, size_t argCount, char* response) {

    parser.process_help_Command(args, argCount, response);

}



/* hello world command */
void helloCommandCallback(CommandParser::Argument* args, size_t argCount, char* response) {

    // Check for missing argument, if so use the default name "DefaultName"
    if (args[0].asString[0] == '\0') {
        strncpy_s(args[0].asString, "DefaultName", MAX_COMMAND_ARG_SIZE);
    }
    // Null terminate the string to ensure no overflow
    args[0].asString[MAX_COMMAND_ARG_SIZE] = '\0';


    snprintf(response, MAX_RESPONSE_SIZE, "hello, %s!", args[0].asString);
}


/* welcome Callback command */
void welcomeCb(CommandParser::Argument* args, size_t argCount, char* response) {

    snprintf(response, MAX_RESPONSE_SIZE, "Welcome to the Darts Command Line");

}


/* create a new game */
void set_new_game_Cb(CommandParser::Argument* args, size_t argCount, char* response) {

    char *names[2];

    /* no players */
    if ((args[0].asString[0] == '\0') || (argCount == 0)) {
        snprintf(response, MAX_RESPONSE_SIZE,"err: not enough args");
        return;
    }

    snprintf(response, MAX_RESPONSE_SIZE, "new game with:");
    for (int i = 0; i < argCount; i++) {
        names[i]=args[i].asString;
        strncat_s(response, MAX_RESPONSE_SIZE, " ", MAX_RESPONSE_SIZE - strlen("    ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, args[i].asString, MAX_RESPONSE_SIZE - strlen(args[i].asString) - 1);
    }

    dart_board_set_new_game(names, argCount);

}


/* set params on Darts Scoreboard */
void set_params(CommandParser::Argument* args, size_t argCount, char* response) {

    /* no params */
    if ((args[0].asString[0] == '\0') || (argCount == 0)) {
        snprintf(response, MAX_RESPONSE_SIZE, "err: not enough args");
        return;
    }


    /* DarrtBoard Params */
    /* check whcih param should be set */
    if ( strcmp(args[0].asString, "score") == 0) {
        if (argCount < 3) {
            snprintf(response, MAX_RESPONSE_SIZE, "err: not enough args for param %s",args[0].asString);
            return;
        }
        char* name = args[1].asString;
        string score_str = args[2].asString;
        int score = stoi(score_str); 
        /* set response */
        strncat_s(response, MAX_RESPONSE_SIZE, "set score ", MAX_RESPONSE_SIZE - strlen("set score ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, name, MAX_RESPONSE_SIZE - strlen(name) - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, " ", MAX_RESPONSE_SIZE - strlen(" ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, score_str.c_str(), MAX_RESPONSE_SIZE - strlen(score_str.c_str()) - 1);
        /* call function */
        dart_board_set_score(name, score);
        return;
    }




    /* Image Processing Params */
    /* get param name */
    char* param = args[0].asString;


    /* check which param should be set */
    if (strcmp(param, "bin_thresh") == 0) {
        if (!(argCount == 2)) {
            snprintf(response, MAX_RESPONSE_SIZE, "err: not enough or two many args for param %s. argCount: %d", param, (int)argCount);
            return;
        }
        
        string bin_thresh_str = args[1].asString;
        int bin_thresh = stoi(bin_thresh_str);
        
        /* set response */
        strncat_s(response, MAX_RESPONSE_SIZE, "set ", MAX_RESPONSE_SIZE - strlen("set ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, param, MAX_RESPONSE_SIZE - strlen(param) - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, " ", MAX_RESPONSE_SIZE - strlen(" ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, bin_thresh_str.c_str(), MAX_RESPONSE_SIZE - strlen(bin_thresh_str.c_str()) - 1);
        /* call function */
        img_proc_set_bin_thresh(bin_thresh);
        return;
    }
    else if (strcmp(param, "diff_min") == 0) {
        if (!(argCount == 2)) {
            snprintf(response, MAX_RESPONSE_SIZE, "err: not enough or two many args for param %s. argCount: %d", param, (int)argCount);
            return;
        }
        string diff_min_str = args[1].asString;
        int diff_min = stoi(diff_min_str);
        
        /* set response */
        strncat_s(response, MAX_RESPONSE_SIZE, "set ", MAX_RESPONSE_SIZE - strlen("set ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, param, MAX_RESPONSE_SIZE - strlen(param) - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, " ", MAX_RESPONSE_SIZE - strlen(" ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, diff_min_str.c_str(), MAX_RESPONSE_SIZE - strlen(diff_min_str.c_str()) - 1);
        /* call function */
        img_proc_set_diff_min_thresh(diff_min);
        return;
    }
    
    
    /* never reached on correct on command */
    snprintf(response, MAX_RESPONSE_SIZE, "err: not a param");


}


