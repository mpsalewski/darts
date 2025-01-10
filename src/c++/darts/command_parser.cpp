/***************************** includes **************************************/
#include <iostream>
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
#include <cstdlib>
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


/****************************** command THREAD *******************************/
// Konsolen-Thread-Funktion
void consoleThread(void) {

    /* assign void pointer */
    // here unsued bc parser class only used in this module (static), so no 
    // pointer muss be actually passed to thread 
    // CommandParser* p = (CommandParser*)(arg);

    command_parser_cmd_init();


    std::cout << "Warte auf Eingabe von Befehlen..." << std::endl;

    char response[MAX_RESPONSE_SIZE];
    while (true) {
        std::string input;
        std::getline(std::cin, input); // Benutzer-Eingabe

        if (input == "exit") {
            std::cout << "Beenden..." << std::endl;
            break;
        }

        // Verarbeitung mit dem CommandParser
        std::cout << input << std::endl;  
        if (parser.processCommand(input.c_str(), response)) {
            std::cout << "Antwort: " << response << std::endl;
        }
        else {
            std::cout << "Fehler: Eingabe konnte nicht verarbeitet werden." << std::endl;
        }
    }

}


/********************* command parser class func def *************************/


// Konstruktor
CommandParser::CommandParser() : numCommands(0) {
    /*
    // Initialisiere jedes Argument im Array mit einem Standardwert (in diesem Fall den leeren String)
    for (int i = 0; i < MAX_COMMAND_ARGS; ++i) {
        commandArgs[i].asString[0] = '\0';  // Setze den String-Teil auf einen leeren String
    }

    // Initialisiere jedes `Command` im Array
    for (int i = 0; i < MAX_COMMANDS; ++i) {
        // Setze den `name`-Array auf eine leere Zeichenkette
        commandDefinitions[i].name[0] = '\0'; // Name auf leeren String setzen

        // Setze `argTypes` auf leere Zeichenkette
        commandDefinitions[i].argTypes[0] = '\0'; // Arg-Typ auf leeren String setzen

        // Setze den Funktionszeiger auf `nullptr` (für den Fall, dass es keinen gültigen Callback gibt)
        commandDefinitions[i].callback = nullptr; // Keine Callback-Funktion zugewiesen
    }*/

}

// String-Parsing-Funktion
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

// Befehl registrieren
bool CommandParser::registerCommand(const char* name, const char* argTypes, void (*callback)(Argument* args, char* response)) {
    if (numCommands >= MAX_COMMANDS) return false;
    if (std::strlen(name) > MAX_COMMAND_NAME_LENGTH) return false;
    if (std::strlen(argTypes) > MAX_COMMAND_ARGS) return false;
    if (!callback) return false;

    strncpy_s(commandDefinitions[numCommands].name, name, MAX_COMMAND_NAME_LENGTH);
    strncpy_s(commandDefinitions[numCommands].argTypes, argTypes, MAX_COMMAND_ARGS);
    commandDefinitions[numCommands].callback = callback;
    numCommands++;
    return true;
}

// Befehl verarbeiten
bool CommandParser::processCommand(const char* command, char* response) {
    char name[MAX_COMMAND_NAME_LENGTH + 1];
    size_t i = 0;

    // Kommando extrahieren (Bis zum ersten Leerzeichen)
    while (*command != ' ' && *command != '\0' && i < MAX_COMMAND_NAME_LENGTH) name[i++] = *command++;
    name[i] = '\0';

    const char* argTypes = nullptr;
    void (*callback)(Argument*, char*) = nullptr;

    // Finden der passenden Befehldefinition
    for (size_t i = 0; i < numCommands; i++) {
        if (std::strcmp(commandDefinitions[i].name, name) == 0) {
            argTypes = commandDefinitions[i].argTypes;
            callback = commandDefinitions[i].callback;
            break;
        }
    }

    if (!argTypes) {
        std::snprintf(response, MAX_RESPONSE_SIZE, "Error: Unknown command '%s'", name);
        return false;
    }

    // Argumente extrahieren (Nach dem Befehlstrennzeichen ' ')
    char* args = (char*)command;
    while (*args == ' ') ++args;  // Leerzeichen überspringen

    Argument commandArgs[MAX_COMMAND_ARGS];  // Eine Array-Variable für die Argumente
    size_t argIndex = 0;
    size_t parsed = 0;

    // Argumente verarbeiten
    while (*args != '\0' && argIndex < MAX_COMMAND_ARGS) {
        // Argument extrahieren
        if (argTypes[argIndex] == 's') {  // Beispiel, wenn das Argument ein String ist
            parsed = parseString(args, commandArgs[argIndex].asString);  // Rufe die String-Parsing-Funktion auf
            if (parsed == 0) break;
            args += parsed; // Weiter zur nächsten Zeichenkette
            argIndex++;
        }
        // Weitere Typen hier hinzufügen (int, float etc.)
        else break;
    }

    // Callbacks ausführen
    if (callback) callback(commandArgs, response);
    return true;
}


/************************** Function Definitions *****************************/
void command_parser_cmd_init(void){


    // Befehl "hallo" registrieren, er erwartet ein String-Argument
    if (!parser.registerCommand("hallo", "s", helloCommandCallback)) {
        std::cerr << "Fehler: Der Befehl konnte nicht registriert werden!" << std::endl;
        return;
    }




}


// Callback-Funktion für den "hallo"-Befehl
void helloCommandCallback(CommandParser::Argument* args, char* response) {
    if (args[0].asString[0] != '\0') {
        std::snprintf(response, MAX_RESPONSE_SIZE, "Hallo, %s!", args[0].asString);
    }
    else {
        // Verwende strncpy_s anstelle von strncpy
        strncpy_s(response, MAX_RESPONSE_SIZE, "Hallo!", MAX_RESPONSE_SIZE - 1);
    }
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