/***************************** includes **************************************/
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <opencv2/opencv.hpp>
#include "ImageProc.h"
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
// CommandParser-Instanz erstellen
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
    // here unsued bc parser class only used in this module (static), so no 
    // pointer muss be actually passed to thread 
    // CommandParser* p = (CommandParser*)(arg);

    /* init registers all commands */
    command_parser_cmd_init();


    cout << "Command Line is active" << endl;
    
    /* read command line and ignore first input --> flush and welcome */
    
    getline(cin, input);
    parser.processCommand("welcome", response);
    cout << response << endl;

    
    //cin.ignore(std::numeric_limits<streamsize>::max(), '\n');  
    //cin.sync();

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

        /* mirror input */
        //cout << input << endl;  
        
        /* process command */
        cmd_return = parser.processCommand(input.c_str(), response);
        cout << response << endl;
        if (cmd_return) {
            //cout << "ans" << response << endl;
        }
        else {
            //cout << "error: invalid input" << std::endl;
        }

        this_thread::sleep_for(chrono::milliseconds(500));
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

#if 0
    /* get cmd def */
    const char* argTypes = nullptr;
    void (*callback)(Argument*, size_t argCount, char*) = nullptr;
    
    size_t l;
    for (l = 0; l < numCommands; l++) {
        if (std::strcmp(commandDefinitions[l].name, name) == 0) {
            argTypes = commandDefinitions[l].argTypes;
            callback = commandDefinitions[l].callback;
            break;
        }
    }
    /* check if there was matching command definition */
    if(l>=numCommands){
        snprintf(response, MAX_RESPONSE_SIZE, "Error: Unknown command '%s'", name);
        return false;
    }
#endif 
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
        "help menu: [type: 'help $CMD_NAME$' for specific command]"
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

    /* set scoreboard command */
    if (!parser.registerCommand("set", "ssi", set_dart_board_params,
        "set functions for the ScoreBoard. usage: set score $NAME$ $SCORE$; set leg $NAME$ $NUM$ not defined a.t.m."
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
void set_dart_board_params(CommandParser::Argument* args, size_t argCount, char* response) {

    /* no params */
    if ((args[0].asString[0] == '\0') || (argCount == 0)) {
        snprintf(response, MAX_RESPONSE_SIZE, "err: not enough args");
        return;
    }


    /* check whcih param should be set */
    if ( strcmp(args[0].asString, "score") == 0) {
        if (argCount < 3) {
            snprintf(response, MAX_RESPONSE_SIZE, "err: not enough args for param %s",args[0].asString);
            return;
        }
        char* name = args[1].asString;
        int score = args[2].asInt64;
        string score_str = to_string(score); 
        /* set response */
        strncat_s(response, MAX_RESPONSE_SIZE, "set score ", MAX_RESPONSE_SIZE - strlen("set score ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, name, MAX_RESPONSE_SIZE - strlen(name) - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, " ", MAX_RESPONSE_SIZE - strlen(" ") - 1);
        strncat_s(response, MAX_RESPONSE_SIZE, score_str.c_str(), MAX_RESPONSE_SIZE - strlen(score_str.c_str()) - 1);
        /* call function */
        dart_board_set_score(name, score);
        return;
    }
    
    
    /* never reached on correct on command */
    snprintf(response, MAX_RESPONSE_SIZE, "err: not a param");


}

