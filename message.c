#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "message.h"
#include "errors.h"
#include "file_ops.h"
#include "utils.h"
#include "crc32.h"

bool message_load(const char *filename, message_t *message)
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

    size = file_ops_read_until(fp, keyword_message, sizeof(keyword_message), '=', DELIMITER_INCLUSIVE);
    if (size != sizeof(g_message_leading_keyword) - 1)
    {
        printf("Error! Read data different from expected\n");
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    if (strncmp(keyword_message, g_message_leading_keyword,
                MAX(strlen(keyword_message), strlen(g_message_leading_keyword))) != 0)
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

    message->message.size = file_ops_read_until(fp, message->message.raw,
                                       sizeof(message->message.raw),
                                       '\n', DELIMITER_EXCLUSIVE);
    if (message->message.size != (message->length * ASCII_HEX_LENGTH))
    {
        printf("Error! Wrong message size, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_LENGTH;
        return false;
    }

    size = file_ops_read_until(fp, keyword_mask, sizeof(keyword_mask), '=', DELIMITER_INCLUSIVE);
    if (size != sizeof(keyword_mask) - 1)
    {
        printf("Error! Read data different from expected, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    if (strncmp(keyword_mask, g_mask_leading_keyword,
                MAX(strlen(keyword_mask), strlen(g_mask_leading_keyword))) != 0)
    {
        printf("Error! Could not find anchor \"%s\" on the file, %s:%d\n", g_mask_leading_keyword, __func__, __LINE__);
        fclose(fp);
        return false;
    }

    message->mask.size = file_ops_read_until(fp, message->mask.raw, sizeof(message->mask.raw), '\n', DELIMITER_EXCLUSIVE);
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

bool message_update(const message_t *original, message_t *modified)
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
