#include <zlib.h>

#include "crc32.h"

uint32_t crc32_calculate(const char *src, size_t size)
{
    return (uint32_t) crc32(CRC32_INIT_VALUE, (const Bytef *) src, (uInt) size);
}
