#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "errors.h"
#include "debug.h"

#define ERROR_STRING_SIZE           UINT8_C(255)    ///< The size of error string

error_e g_errno = ERROR_NO_ERROR;            ///< Global application error code

void error_write_error_on_file(const char *filename)
{
    char error_string[ERROR_STRING_SIZE] = {0};
    size_t wrote = 0;
    FILE *fp = NULL;

    if (filename == NULL)
    {

        DEBUG_ERROR("NULL parameter");
        g_errno = ERROR_NULL_PARAMETER;
        return;
    }

    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        DEBUG_ERROR("Creating/opening \"%s\" file", filename);
        g_errno = ERROR_NOT_OPEN_FILE;
        return;
    }

    switch (g_errno)
    {
        case ERROR_LENGTH:
            snprintf(error_string, ERROR_STRING_SIZE, "Error in length of the message\n");
            break;

        case ERROR_CRC:
            snprintf(error_string, ERROR_STRING_SIZE, "Error in CRC value of the message\n");
            break;

        case ERROR_NULL_PARAMETER:
            snprintf(error_string, ERROR_STRING_SIZE, "Error NULL parameter\n");
            break;

        case ERROR_FILE_NOT_EXIST:
            snprintf(error_string, ERROR_STRING_SIZE, "Error file not exist\n");
            break;

        case ERROR_NOT_OPEN_FILE:
            snprintf(error_string, ERROR_STRING_SIZE, "Error could not open file\n");
            break;

        case ERROR_DATA_NOT_EXPECTED:
            snprintf(error_string, ERROR_STRING_SIZE, "Data is not expected\n");
            break;

        case ERROR_READING_FILE:
            snprintf(error_string, ERROR_STRING_SIZE, "Error reading file\n");
            break;

        case ERROR_CONVERSION:
            snprintf(error_string, ERROR_STRING_SIZE, "Error converting string\n");
            break;

        case ERROR_BUFFER_SIZE:
            snprintf(error_string, ERROR_STRING_SIZE, "Error in buffer size\n");
            break;

        case ERROR_STRING_FORMAT:
            snprintf(error_string, ERROR_STRING_SIZE, "Error string format\n");
            break;

        case ERROR_FILE_CREATION:
            snprintf(error_string, ERROR_STRING_SIZE, "Error file creation\n");
            break;

        case ERROR_FTELL:
            snprintf(error_string, ERROR_STRING_SIZE, "Error calling ftell()\n");
            break;

        case ERROR_FSEEK:
            snprintf(error_string, ERROR_STRING_SIZE, "Error calling fseek()\n");
            break;

        default:
            snprintf(error_string, ERROR_STRING_SIZE, "Unknown error value: %d\n", g_errno);
            break;
    }

    wrote = fwrite(error_string, sizeof(char), strlen(error_string), fp);
    if (wrote == 0)
        DEBUG_ERROR("Could not write into file \"%s\"", filename);

    fclose(fp);
}

