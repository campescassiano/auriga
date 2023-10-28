#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "errors.h"
#include "crc32.h"
#include "message.h"
#include "utils.h"

#define INPUT_FILE      ("data_in.txt")     ///< Input file to be used
#define OUTPUT_FILE     ("data_out.txt")    ///< Output file to be written to

static const char g_message_leading_keyword[] = "mess=";    ///< leading keyword for message
static const int g_message_leading_length = strlen(g_message_leading_keyword);  ///< leading keyword message length

static const char g_mask_leading_keyword[] = "mask=";       ///< leading keyword for mask
static const int g_mask_leading_length = strlen(g_mask_leading_keyword);    ///< leading keyword mask length

#define ALIGN_APPEND                UINT8_C(4)      ///< Definition of the requested alignment

#define INCLUSIVE                   (true)  ///< Read until marker found, include marker
#define EXCLUSIVE                   (false) ///< Read until marker is found, do not include marker

#define APPEND                      (true)  ///< Append into the file
#define NOT_APPEND                  (false) ///< Not append into the file

/**
 * @brief Update the original message according to the project's specification
 *          which is regarding the data padding, CRC calculation and so on.
 *
 * @param[in] original The original parsed message
 * @param[out] modified The destination where the modified message will be stored
 *
 * @retval True if it could update; false otherwise
 */
static bool update_message(const message_t *original, message_t *modified)
{
    uint32_t mask = 0;
    size_t append = 0;
    uint32_t crc = 0;

    if (original == NULL || modified == NULL)
    {
        printf("Error! Null parameters, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    modified->type = original->type;

    memcpy((char*)&mask, &original->mask_val[0], sizeof(uint32_t));

    append = (original->length - CRC_SIZE) % ALIGN_APPEND;

    memcpy(&modified->data[0], &original->data[0], original->length - CRC_SIZE);
    modified->length = original->length;

    if (append != 0)
    {
        printf("Info! appending %ld bytes on data bytes\n", append);
        modified->length += append;
        memset(&modified->data[(uint8_t)modified->length], 0, sizeof(char) * append);
    }

    utils_apply_mask_on_tetrads(modified->data, modified->length - CRC_SIZE, *(uint32_t*)&original->mask_val);

    crc = crc32_calculate(modified->data, modified->length - CRC_SIZE);
    memcpy(&modified->crc[0], (char*)&crc, sizeof(uint32_t));

    return true;
}

/**
 * @brief Function to read from the given file pointer up to the delimiter specified
 *
 * @param[in] fp The file pointer where it should read
 * @param[out] dst The destination where it sould store the read data
 * @param[in] size The size of @p dst buffer
 * @param[in] delim The delimiter that is the anchor for reading
 * @param[in] inclusive If true, then the @p delim will be included in the dst, if false then @p delim will not be included
 *
 * @retval Returns the number of bytes read considering that it found the delim. If it does not found the delim, it returns 0 and it fseeks() back to where it started (atomic function)
 */
static size_t read_until(FILE *fp, char *dst, size_t size, char delim, bool inclusive)
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
        printf("Error! Could not call ftell()\n");
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
        printf("Error! Could not fseek() back to starting point of the file\n");
        g_errno = ERROR_FSEEK;
    }

    return 0;
}

/**
 * @brief Loads the message from the specified file
 *
 * @param[in] filename The filename where should read the message
 * @param[out] message The pointer to the message structure that will store the message
 */
static bool load_message(const char *filename, message_t *message)
{
    char ascii_byte[ASCII_HEX_LENGTH + 1] = {0};
    char keyword_message[g_message_leading_length + 1];
    char keyword_mask[g_mask_leading_length + 1];
    FILE *fp = NULL;
    size_t size = 0;

    if (filename == NULL || message == NULL)
    {
        printf("Error! NULL parameter, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (access(filename, F_OK) != 0)
    {
        printf("Error! File \"%s\" does not exist\n", filename);
        g_errno = ERROR_FILE_NOT_EXIST;
        return false;
    }

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error! Could not open file \"%s\"\n", filename);
        g_errno = ERROR_NOT_OPEN_FILE;
        return false;
    }

    size = read_until(fp, keyword_message, sizeof(keyword_message), '=', INCLUSIVE);
    if (size != sizeof(g_message_leading_keyword) - 1)
    {
        printf("Error! Read data different from expected\n");
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    if (strcmp(keyword_message, g_message_leading_keyword) != 0)
    {
        printf("Error! Could not find anchor \"%s\" on the file\n", g_message_leading_keyword);
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    size = fread(ascii_byte, sizeof(uint8_t), MIN(ASCII_HEX_LENGTH, sizeof(ascii_byte)), fp);
    if (size != ASCII_HEX_LENGTH)
    {
        printf("Error! Could not read correctly, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }
    message->type = 0xff & strtoul(ascii_byte, NULL, 16);

    size = fread(ascii_byte, sizeof(uint8_t), MIN(ASCII_HEX_LENGTH, sizeof(ascii_byte)), fp);
    if (size != ASCII_HEX_LENGTH)
    {
        printf("Error! Could not read correctly, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_READING_FILE;
        return false;
    }
    message->length = 0xff & strtoul(ascii_byte, NULL, 16);

    message->message.size = read_until(fp, message->message.raw,
                                       sizeof(message->message.raw),
                                       '\n', EXCLUSIVE);
    if (message->message.size != (message->length * ASCII_HEX_LENGTH))
    {
        printf("Error! Wrong message size, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_LENGTH;
        return false;
    }

    size = read_until(fp, keyword_mask, sizeof(keyword_mask), '=', INCLUSIVE);
    if (size != sizeof(keyword_mask) - 1)
    {
        printf("Error! Read data different from expected, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    if (strcmp(keyword_mask, g_mask_leading_keyword) != 0)
    {
        printf("Error! Could not find anchor \"%s\" on the file, %s:%d\n", g_mask_leading_keyword, __func__, __LINE__);
        fclose(fp);
        return false;
    }

    message->mask.size = read_until(fp, message->mask.raw, sizeof(message->mask.raw), '\n', EXCLUSIVE);
    if (message->mask.size != MASK_HEX_LENGTH)
    {
        printf("Error! Wrong message size, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_LENGTH;
        return false;
    }

    if (utils_hex_to_bin(message->message.raw, message->length * ASCII_HEX_LENGTH - CRC32_HEX_LENGTH,
                         message->data, sizeof(message->data)) == false)
    {
        printf("Error! Could not convert hex to bin, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    size_t pos = message->length * ASCII_HEX_LENGTH - CRC32_HEX_LENGTH;

    if (utils_hex_to_bin(&message->message.raw[pos], CRC32_HEX_LENGTH,
                         message->crc, sizeof(message->crc)) == false)
    {
        printf("Error! Could not convert hex to bin, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    uint32_t calculated = crc32_calculate(message->data, message->length - CRC_SIZE);

    if (htonl(*(uint32_t*)message->crc) != calculated)
    {
        printf("Error! Wrong CRC, should be=%"PRIu32", got=%"PRIu32", %s:%d\n", calculated, htonl(*(uint32_t*)message->crc), __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_CRC;
        return false;
    }

    if (utils_hex_to_bin(message->mask.raw, message->mask.size, message->mask_val, sizeof(message->mask_val)) == false)
    {
        printf("Error! Could not convert hex to bin\n");
        fclose(fp);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    fclose(fp);

    return true;
}

/**
 * @brief Write the Output file for the original message
 *
 * @param[in] filename The filename of the file to be written
 * @param[in] message The message structure where should get the data
 * @param[in] append If set to true, append if file exists; if false, write over
 *
 * @retval True if success; false otherwise
 */
static bool write_output_original(const char *filename, message_t *message, bool append)
{
    char msg[ASCII_MESSAGE_MAX_SIZE] = {0};
    char temporary[ASCII_MESSAGE_MAX_SIZE * 2] = {0};
    size_t written = 0;
    size_t pos = 0;
    FILE *fp;

    if (filename == NULL || message == NULL)
    {
        printf("Error! NULL parameter, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (append == true)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("Error! Creating/opening \"%s\" file, %s:%d\n", filename, __func__, __LINE__);
        g_errno = ERROR_FILE_CREATION;
        return false;
    }

    written = utils_bin_to_hex(&message->type, sizeof(message->type),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "message type:",
                                                          strlen("message type:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(&message->length, sizeof(message->length),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "initial message length:",
                                                          strlen("initial mesage length:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->data, MIN(message->length - CRC_SIZE, sizeof(message->data)),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "initial message data bytes:",
                                                          strlen("initial mesage data bytes:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->crc, sizeof(message->crc),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "initial CRC-32:",
                                                          strlen("initial CRC-32:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    fwrite(msg, sizeof(char), strlen(msg), fp);
    fclose(fp);

    return true;
}

/**
 * @brief Write the Output file for the modified message
 *
 * @param[in] filename The filename of the file to be written
 * @param[in] message The message structure where should get the data
 * @param[in] append If set to true, append if file exists; if false, write over
 *
 * @retval True if success; false otherwise
 */
static bool write_output_modified(const char *filename, message_t *message, bool append)
{
    char msg[ASCII_MESSAGE_MAX_SIZE] = {0};
    char temporary[ASCII_MESSAGE_MAX_SIZE * 2] = {0};
    size_t written = 0;
    size_t pos = 0;
    FILE *fp;

    if (filename == NULL || message == NULL)
    {
        printf("Error! NULL parameter, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (append == true)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("Error! Creating/opening \"%s\" file, %s:%d\n", filename, __func__, __LINE__);
        g_errno = ERROR_FILE_CREATION;
        return false;
    }

    written = utils_bin_to_hex(&message->length, sizeof(message->length),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "modified message length:",
                                                          strlen("modified mesage length:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->data, MIN(message->length - CRC_SIZE, sizeof(message->data)),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "modified message data bytes with mask:",
                                                          strlen("modified mesage data bytes with mask:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = utils_bin_to_hex(message->crc, sizeof(message->crc),
                               temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = utils_append_header_and_payload_into_buffer(&msg[pos],
                                                          sizeof(msg) - pos,
                                                          "modified CRC-32:",
                                                          strlen("modified CRC-32:"),
                                                          temporary,
                                                          strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    fwrite(msg, sizeof(char), strlen(msg), fp);
    fclose(fp);

    return true;
}

int main(int argc, char **argv)
{
    message_t original_message;
    message_t modified_message;

    if (load_message(INPUT_FILE, &original_message) == false)
    {
        printf("Error! Loading message, %s:%d\n", __func__, __LINE__);
        goto check_error;
    }

    update_message(&original_message, &modified_message);
    write_output_original(OUTPUT_FILE, &original_message, APPEND);

    if (g_errno == ERROR_NO_ERROR)
        write_output_modified(OUTPUT_FILE, &modified_message, APPEND);

    printf("Execution completed\n");
    return 0;

check_error:
    printf("Please check \"%s\" file for error message\n", OUTPUT_FILE);
    error_write_error_on_file(OUTPUT_FILE);
    return -g_errno;
}

