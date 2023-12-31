#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file_ops.h"
#include "errors.h"
#include "utils.h"
#include "debug.h"

bool file_ops_write_output_original(const char *filename, message_t *message, bool append)
{
    char msg[ASCII_MESSAGE_MAX_SIZE] = {0};
    char temporary[ASCII_MESSAGE_MAX_SIZE * 2] = {0};
    size_t written = 0;
    size_t pos = 0;
    FILE *fp;

    if (filename == NULL || message == NULL)
    {
        DEBUG_ERROR("NULL parameter");
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (append == true)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if (fp == NULL)
    {
        DEBUG_ERROR("Creating/opening \"%s\" file", filename);
        g_errno = ERROR_FILE_CREATION;
        return false;
    }

    written = utils_bin_to_hex(&message->type, sizeof(message->type),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "message type: 0x",
                                                          strlen("message type: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(&message->length, sizeof(message->length),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "initial message length: 0x",
                                                          strlen("initial mesage length: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->data, MIN((size_t) message->length - CRC_SIZE, sizeof(message->data)),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "initial message data bytes: 0x",
                                                          strlen("initial mesage data bytes: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->crc, sizeof(message->crc),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "initial CRC-32: 0x",
                                                          strlen("initial CRC-32: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    fwrite(msg, sizeof(char), strlen(msg), fp);
    fclose(fp);

    return true;
}

bool file_ops_write_output_modified(const char *filename, message_t *message, bool append)
{
    char msg[ASCII_MESSAGE_MAX_SIZE] = {0};
    char temporary[ASCII_MESSAGE_MAX_SIZE * 2] = {0};
    size_t written = 0;
    size_t pos = 0;
    FILE *fp;

    if (filename == NULL || message == NULL)
    {
        DEBUG_ERROR("NULL parameter");
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (append == true)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if (fp == NULL)
    {
        DEBUG_ERROR("Creating/opening \"%s\" file", filename);
        g_errno = ERROR_FILE_CREATION;
        return false;
    }

    written = utils_bin_to_hex(&message->length, sizeof(message->length),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "modified message length: 0x",
                                                          strlen("modified mesage length: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->data, MIN((size_t) message->length - CRC_SIZE, sizeof(message->data)),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "modified message data bytes with mask: 0x",
                                                          strlen("modified mesage data bytes with mask: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->crc, sizeof(message->crc),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Could not convert bin to hex");
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "modified CRC-32: 0x",
                                                          strlen("modified CRC-32: 0x"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        DEBUG_ERROR("Failed to append header and payload");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    fwrite(msg, sizeof(char), strlen(msg), fp);
    fclose(fp);

    return true;
}

size_t file_ops_read_until(FILE *fp, char *dst, size_t size, char delim, bool inclusive)
{
    bool found = false;
    long start = 0;
    size_t read = 0;
    int c;

    if (fp == NULL || dst == NULL)
    {
        g_errno = ERROR_NULL_PARAMETER;
        return 0;
    }

    start = ftell(fp);
    if (start == -1L)
    {
        DEBUG_ERROR("Could not call ftell()");
        g_errno = ERROR_FTELL;
        return 0;
    }

    memset(dst, 0, size);
    while ((c = fgetc(fp)) != EOF && read < size)
    {
        dst[read++] = (char) c;

        if (c == delim)
        {
            if (inclusive == false)
                dst[--read] = '\0';

            found = true;
            break;
        }
    }

    if (found == true)
        return read;

    if (fseek(fp, start, SEEK_SET) != 0)
    {
        DEBUG_ERROR("Could not fseek() back to starting point of the file");
        g_errno = ERROR_FSEEK;
    }

    return 0;
}
