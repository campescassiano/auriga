#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "debug.h"

bool utils_hex_to_bin(char *src, size_t src_size, char *dst, size_t dst_size)
{
    size_t i = 0;
    char temp[ASCII_HEX_LENGTH] = {0};

    if ((src_size >> 1) > dst_size)
    {
        g_errno = ERROR_LENGTH;
        return false;
    }

    memset(dst, 0, dst_size);

    for (i = 0; i < (src_size >> 1); ++i)
    {
        memcpy(&temp[0], &src[0], 2);
        temp[2] = '\0';
        dst[i] = 0xff & (unsigned int) strtoul(temp, NULL, 16);
        src += ASCII_HEX_LENGTH;
    }

    return true;
}

int utils_bin_to_hex(char *src, size_t src_size, char *dst, size_t dst_size)
{
    size_t i = 0;
    if (src_size * ASCII_HEX_LENGTH > dst_size)
    {
        g_errno = ERROR_BUFFER_SIZE;
        return 0;
    }

    memset(dst, 0, dst_size);

    for (i = 0; i < src_size; i++)
    {
        snprintf(dst, 3, "%02x", 0xff & src[i]);
        dst+= 2;
    }

    return (i * ASCII_HEX_LENGTH);
}

void utils_apply_mask_on_tetrads(char *data, size_t size, uint32_t mask)
{
    uint32_t *p;

    if (data == NULL)
    {
        DEBUG_ERROR("NULL parameter");
        g_errno = ERROR_NULL_PARAMETER;
        return;
    }

    if ((size % 2) != 0)
        return;

    p = (uint32_t*) data;

    size = size / sizeof(uint32_t);

    for (size_t i = 0; i < size; i++)
    {
        if ((i % 2) == 0)
            p[i] = p[i] & mask;
    }
}

size_t utils_append_header_and_payload_into_buffer(char *dst,
                                                   size_t dst_size,
                                                   char *header,
                                                   size_t header_size,
                                                   char *payload,
                                                   size_t payload_size)
{
    size_t wrote = 0;

    if (dst == NULL || header == NULL || payload == NULL)
    {
        DEBUG_ERROR("NULL parameter");
        g_errno = ERROR_NULL_PARAMETER;
        return 0;
    }

    memset(dst, 0, dst_size);

    if ((header_size + payload_size) >= dst_size)
    {
        DEBUG_ERROR("Destination buffer is smaller than required");
        g_errno = ERROR_BUFFER_SIZE;
        return 0;
    }

    wrote = snprintf(dst, dst_size, "%s%s\n", header, payload);
    if (wrote == 0)
    {
        DEBUG_ERROR("Could not format string");
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }

    return wrote;
}

