#include "crc32.h"

uint32_t crc32_calculate(const char *src, size_t size)
{
    unsigned int byte, crc, mask;

    crc = CRC32_INIT_VALUE;
    for (int i = 0; i < size; i++)
    {
        byte = src[i];
        crc = crc ^ byte;
        for (int j = 7; j >= 0; j--)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32_POLYNOME & mask);
        }
    }
    return ~crc;
}
