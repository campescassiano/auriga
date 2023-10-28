#include <zlib.h>

#include "crc32.h"

uint32_t crc32_calculate(const char *src, size_t size)
{
    return crc32(CRC32_INIT_VALUE, (const Bytef *) src, size);
}
