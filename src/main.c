#include <stdio.h>
#include <string.h> 
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>

// preparation for coloring
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

// table characters
#define V_LINE 0xB3
#define H_LINE 0xC4
#define TL_CORNER 0xDA
#define TR_CORNER 0xBF
#define BL_CORNER 0xC0
#define BR_CORNER 0xD9

// error enum
enum response
{
    OK = 0,         // ALL OK
    NOT_OK = 1,     // GENERAL ERROR
};

// struct with data received from call parameters
struct data
{
    uint8_t filepath[256];
    bool hasStart;
    uint64_t startAddres;
    bool hasEnd;
    uint64_t endAddres;
};

// i was supposed to use this somewhere, but I did not... ¯\_(ツ)_/¯
bool file_exists(const uint8_t *filename)
{
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? true : false;
}

// parse args
uint8_t parseArguments(int argc, uint8_t **argv, struct data *currentData)
{
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        printf("TODO PRINT HELP\n");
        return NOT_OK;
    }

    if (argc == 1)
    {
        printf("No arguments provided. Try -h for help.\n");
        return NOT_OK;
    }

    if (argc % 2 == 0)
    {
        printf("Wrong number of arguments. Try -h for help.\n");
        return NOT_OK;
    }

    for (uint8_t i = 1; i < argc; i=i+2 )
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            strcpy(currentData->filepath, argv[i+1]);
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            printf("Start addres: %s \n", argv[i+1]);

            currentData->hasStart = true;
            currentData->startAddres = strtol(argv[i+1], NULL, 16);
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            printf("End addres: %s \n", argv[i+1]);

            currentData->hasEnd = true;
            currentData->endAddres = strtol(argv[i+1], NULL, 16);
        }
        else
        {
            printf("Unknown argument. Try -h for help.\n");
            return NOT_OK;
        }
    } 
    
    return OK;
}


void setTerminalColor(uint8_t character)
{
    // color for numbers
    if (character >= 0x30 && character <= 0x39)
    {
        printf(BLU);
    }
    // color for alpha
    else if ((character >= 0x41 && character <= 0x5A) || (character >= 0x61 && character <= 0x7A))
    {
        printf(MAG);
    }
    else
    {
        printf(CYN);
    }
}

void resetTerminalColor()
{
    printf(RESET);
}

// display the line of hex, but make it nice
uint8_t displayLine(uint8_t *line)
{
    uint8_t strByte[2];
    uint8_t DataLen = 0;

    printf("%c ", V_LINE);

    printf(YEL);
    // byte count
    memcpy(strByte, &line[1], 2);
    DataLen = strtol(strByte, NULL, 16);
    printf("%02X", DataLen);

    // address
    memcpy(strByte, &line[3], 2);
    printf(" %02X", strtol(strByte, NULL, 16));
    memcpy(strByte, &line[5], 2);
    printf("%02X", strtol(strByte, NULL, 16));

    // record type
    memcpy(strByte, &line[7], 2);
    printf(" %02X ", strtol(strByte, NULL, 16));

    resetTerminalColor();
    printf("%c ", V_LINE);

    // data
    for (uint8_t i = 9; i < (DataLen * 2) + 8; i = i + 2)
    {
        memcpy(strByte, &line[i], 2);
        uint8_t character = 0;
        character = strtol(strByte, NULL, 16);

        setTerminalColor(character);

        printf("%02X ", character);

        resetTerminalColor();
    }

    // fill the rest data empty
    for (uint8_t i = (DataLen * 2) + 8; i < 40; i = i + 2)
    {
        memcpy(strByte, &line[i], 2);
        printf("   ", strtol(strByte, NULL, 16));
    }

    // CS
    memcpy(strByte, &line[41], 2);

    // right now make checksum green. But later it should be Green or RED (depends if the checksum is correct)
    printf("%c ", V_LINE);

    printf(GRN);
    printf("%02X", strtol(strByte, NULL, 16));
    resetTerminalColor();

    printf(" %c ", V_LINE);

    for (uint8_t i = 9; i < (DataLen * 2) + 8; i = i + 2)
    {
        memcpy(strByte, &line[i], 2);
        uint8_t character = 0;
        character = strtol(strByte, NULL, 16);

        setTerminalColor(character);

        //skip problematic chars
        if ((character >= 0x00 && character <= 0x20) || character == 0x7F || character == 0xFF || character == 0xF0)
        {
            printf("  ");
        }
        else
        {
            printf("%c ", character);
        }

        resetTerminalColor();
    }

    // fill the rest cahracters empty
    for (uint8_t i = (DataLen * 2) + 8; i < 40; i = i + 2)
    {
        memcpy(strByte, &line[i], 2);
        printf("  ", strtol(strByte, NULL, 16));
    }

    printf("%c\n", V_LINE);

    return OK;
}

void printTopOfTable()
{
    printf("%c", TL_CORNER);
        for(uint8_t i = 0; i < 101; i++ )
        {
            printf("%c", H_LINE);
        }
    printf("%c\n", TR_CORNER);
}

void printBottomOfTable()
{
    printf("%c", BL_CORNER);
        for(uint8_t i = 0; i < 101; i++ )
        {
            printf("%c", H_LINE);
        }
    printf("%c\n", BR_CORNER);
}

// reads lines of hex file one by one and display content to terminal
uint8_t displayContentFromData(struct data *currentData)
{
    FILE *fptr;
    fptr = fopen(currentData->filepath, "r");

    uint8_t line[45]; // one line should have 44 characters + endline character

    if (fptr != NULL) {

        printTopOfTable();

        while (fgets(line, sizeof(line), fptr)) 
        {
            displayLine(line);
            memset(line, 0x00, 45);
        }

        printBottomOfTable();

        fclose(fptr);
    }

    return OK;
}

void Helper_PrintASCI()
{
    printf("ASCI HELPER:\n\n");
    for (uint8_t i = 0; i < 256; i++)
    {
        printf("%x-%c  ", i, i);
    }
    printf("\n");
}

// main...
uint8_t main(uint8_t argc, uint8_t **argv) 
{
    struct data currentData = {"", false, 0x00, false, 0x00};

    if (parseArguments(argc, argv, &currentData) != OK)
    {
        return 0;
    }
    
    if (displayContentFromData(&currentData) != OK)
    {
        return 0;
    }
    
    return 0;
}


