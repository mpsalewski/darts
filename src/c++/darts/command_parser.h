/******************************************************************************
 *
 * $NAME.cpp
 *
 * Digital Image / Video Processing
 * HAW Hamburg, Prof.Dr.Marc Hensel
 *
 * TEMPLATE
 *
 *
 * author: 			m.salewski
 * created on :
 * last revision :
 *
 *
 * 
 * 
 * 
 * 
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
 * 
 * 
 * NOTE:
 * The refered software from above ist completely restructured and in many parts modified by m. salewski 
 * 
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
constexpr size_t MAX_RESPONSE_SIZE = 64;

/************************** local Structure ***********************************/



/************************* command parser class *******************************/

class CommandParser {
public:
    /* argument structure */
    union Argument {
        double asDouble;
        uint64_t asUInt64;
        int64_t asInt64;
        char asString[MAX_COMMAND_ARG_SIZE + 1];
    };

    /* constructor */
    CommandParser();

    /* registers commands */
    bool registerCommand(const char* name, const char* argTypes, void (*callback)(Argument* args, size_t argCount, char* response));
    
    /* process a command */
    bool processCommand(const char* command, char* response);

private:
    /* command structure */
    struct Command {
        char name[MAX_COMMAND_NAME_LENGTH + 1];
        char argTypes[MAX_COMMAND_ARGS + 1];
        void (*callback)(Argument* args, size_t argCount, char* response);
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
extern void helloCommandCallback(CommandParser::Argument* args, size_t argCount, char* response);
extern void welcomeCb(CommandParser::Argument* args, size_t argCount, char* response);
extern void set_new_game_Cb(CommandParser::Argument* args, size_t argCount, char* response);
extern void set_dart_board_params(CommandParser::Argument* args, size_t argCount, char* response);
size_t strToInt(const char* buf, void* value, bool isSigned, int64_t min_value, int64_t max_value);





#endif
