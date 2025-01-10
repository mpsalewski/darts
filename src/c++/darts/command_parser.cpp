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
void commandLineThread(void) {

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
bool CommandParser::registerCommand(const char* name, const char* argTypes, void (*callback)(Argument* args, char* response)) {
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
    /* register in CommandParser Class */
    strncpy_s(commandDefinitions[numCommands].name, name, MAX_COMMAND_NAME_LENGTH);
    strncpy_s(commandDefinitions[numCommands].argTypes, argTypes, MAX_COMMAND_ARGS);
    commandDefinitions[numCommands].callback = callback;

    /* increment numer of cmds */
    numCommands++;
    
    return true;
}

/* process commands */
bool CommandParser::processCommand(const char* command, char* response) {
    char name[MAX_COMMAND_NAME_LENGTH + 1];
    size_t i = 0;

    /* extract command */
    while (*command != ' ' && *command != '\0' && i < MAX_COMMAND_NAME_LENGTH) name[i++] = *command++;
    name[i] = '\0';

    const char* argTypes = nullptr;
    void (*callback)(Argument*, char*) = nullptr;

    /* get cmd def */
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

    /* get cmd args */
    char* args = (char*)command;
    while (*args == ' ') {
        ++args;             // skip spaces 
    }
    Argument commandArgs[MAX_COMMAND_ARGS] = {0};  // array var for args
    size_t argIndex = 0;
    size_t parsed = 0;


    /* process args */
    while (*args != '\0' && argIndex < MAX_COMMAND_ARGS) {
        /* extract args */
        /* arg is a string */
        if (argTypes[argIndex] == 's') {  
            parsed = parseString(args, commandArgs[argIndex].asString);  // get arg
            if (parsed == 0) {
                break;
            }
            args += parsed;     // get next arg
            argIndex++;
        }
        /* you might want to add other types(int, float etc.) */
        else 
            break;
    }

    /* execute callbacks */
    if (callback) {
        callback(commandArgs, response);
    }
    return true;
}


/************************** Function Definitions *****************************/
void command_parser_cmd_init(void){

    /***
     * REGISTER YOUR COMMANDS
    ***/

    /* "hello world" command */
    if (!parser.registerCommand("hello", "s", helloCommandCallback)) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }

    /* welcome command */
    if (!parser.registerCommand("welcome", " ", welcomeCb)) {
        std::cerr << "err: could not register command!" << std::endl;
        return;
    }





}


/* hello world command */
void helloCommandCallback(CommandParser::Argument* args, char* response) {

    // Check for missing argument, if so use the default name "DefaultName"
    if (args[0].asString[0] == '\0') {
        strncpy_s(args[0].asString, "DefaultName", MAX_COMMAND_ARG_SIZE);
    }
    // Null terminate the string to ensure no overflow
    args[0].asString[MAX_COMMAND_ARG_SIZE] = '\0';


    snprintf(response, MAX_RESPONSE_SIZE, "hello, %s!", args[0].asString);
}


/* welcome Callback command */
void welcomeCb(CommandParser::Argument* args, char* response) {

    snprintf(response, MAX_RESPONSE_SIZE, "Welcome to the Darts Command Line");

}


#if 0

size_t strToInt(const char* buf, void* value, bool isSigned, int64_t min_value, int64_t max_value) {
    size_t position = 0;
    bool isNegative = false;

    // Vorzeichen parsen, falls erlaubt (nur für vorzeichenbehaftete Typen)
    if (isSigned && min_value < 0 && (buf[position] == '+' || buf[position] == '-')) {
        isNegative = (buf[position] == '-');
        position++;
    }

    // Basis bestimmen (Binär, Oktal, Dezimal, Hexadezimal)
    int base = 10;
    if (buf[position] == '0' && buf[position + 1] == 'b') {
        base = 2;
        position += 2;
    }
    else if (buf[position] == '0' && buf[position + 1] == 'o') {
        base = 8;
        position += 2;
    }
    else if (buf[position] == '0' && buf[position + 1] == 'x') {
        base = 16;
        position += 2;
    }

    int digit = -1;
    if (isSigned) {
        int64_t* result = static_cast<int64_t*>(value);
        *result = 0;

        while (true) {
            // Ziffern ermitteln
            if (base >= 2 && buf[position] >= '0' && buf[position] <= '1') {
                digit = buf[position] - '0';
            }
            else if (base >= 8 && buf[position] >= '2' && buf[position] <= '7') {
                digit = buf[position] - '0';
            }
            else if (base >= 10 && buf[position] >= '8' && buf[position] <= '9') {
                digit = buf[position] - '0';
            }
            else if (base >= 16 && buf[position] >= 'a' && buf[position] <= 'f') {
                digit = buf[position] - 'a' + 10;
            }
            else if (base >= 16 && buf[position] >= 'A' && buf[position] <= 'F') {
                digit = buf[position] - 'A' + 10;
            }
            else {
                break;
            }

            // Über-/Unterlauf prüfen
            if (*result < min_value / base || *result > max_value / base) return 0;
            *result *= base;
            if (isNegative ? *result < min_value + digit : *result > max_value - digit) return 0;
            *result += digit;

            position++;
        }
    }
    else {
        uint64_t* result = static_cast<uint64_t*>(value);
        *result = 0;

        while (true) {
            // Ziffern ermitteln
            if (base >= 2 && buf[position] >= '0' && buf[position] <= '1') {
                digit = buf[position] - '0';
            }
            else if (base >= 8 && buf[position] >= '2' && buf[position] <= '7') {
                digit = buf[position] - '0';
            }
            else if (base >= 10 && buf[position] >= '8' && buf[position] <= '9') {
                digit = buf[position] - '0';
            }
            else if (base >= 16 && buf[position] >= 'a' && buf[position] <= 'f') {
                digit = buf[position] - 'a' + 10;
            }
            else if (base >= 16 && buf[position] >= 'A' && buf[position] <= 'F') {
                digit = buf[position] - 'A' + 10;
            }
            else {
                break;
            }

            // Überlauf prüfen
            if (*result > static_cast<uint64_t>(max_value) / base) return 0;
            *result *= base;
            if (*result > static_cast<uint64_t>(max_value) - digit) return 0;
            *result += digit;

            position++;
        }
    }

    return (digit == -1) ? 0 : position;
}

#endif 