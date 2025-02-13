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



#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

/* Include files */
#include <limits.h>
#include <cstddef>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include <string>


/*************************** local Defines ***********************************/
/* define boundaries */
constexpr size_t MAX_COMMANDS = 32;
constexpr size_t MAX_COMMAND_ARGS = 8;
constexpr size_t MAX_COMMAND_NAME_LENGTH = 16;
constexpr size_t MAX_COMMAND_ARG_SIZE = 32;
constexpr size_t MAX_RESPONSE_SIZE = 200;
constexpr size_t MAX_HELP_LENGTH = 400;


/************************** local Structure ***********************************/



/************************* command parser class *******************************/

class CommandParser {
public:
    /* argument structure */
    union Argument {
        double asDouble;
        unsigned int asUInt;
        int asInt;
        char asString[MAX_COMMAND_ARG_SIZE + 1];
    };

    /* constructor */
    CommandParser();

    /* registers commands */
    bool registerCommand(const char* name, const char* argTypes, void (*callback)(Argument* args, size_t argCount, char* response), const char* help);
    
    /* process a command */
    bool processCommand(const char* command, char* response);

    /* process help command */
    void process_help_Command(Argument*args, size_t argCount, char* response);

private:
    /* command structure */
    struct Command {
        char name[MAX_COMMAND_NAME_LENGTH + 1];                             // command name
        char argTypes[MAX_COMMAND_ARGS + 1];                                // argument types
        void (*callback)(Argument* args, size_t argCount, char* response);  // callback func 
        char help[MAX_HELP_LENGTH + 1];                                     // help string
    };

    /* attributes */
    Command commandDefinitions[MAX_COMMANDS];   // hold all commands
    Argument commandArgs[MAX_COMMAND_ARGS];     // store the arguments parsed from the command input
    size_t numCommands;                         // keep track of the number of registered commands

    /* parse strings */
    size_t parseString(const char* buf, char* output);
};



/************************* local Variables ***********************************/



/************************** Function Declaration *****************************/
extern void commandLineThread(void* arg);
extern void command_parser_cmd_init(void);

extern void help_Cb(CommandParser::Argument* args, size_t argCount, char* response);
extern void helloCommandCallback(CommandParser::Argument* args, size_t argCount, char* response);
extern void welcomeCb(CommandParser::Argument* args, size_t argCount, char* response);

extern void set_new_game_Cb(CommandParser::Argument* args, size_t argCount, char* response);
extern void set_params(CommandParser::Argument* args, size_t argCount, char* response);

extern void pause(CommandParser::Argument* args, size_t argCount, char* response);
extern void auto_cal(CommandParser::Argument* args, size_t argCount, char* response);






#endif
