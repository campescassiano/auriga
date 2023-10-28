#ifndef CRC32_H__
#define CRC32_H__

#include <stdint.h>
#include <stddef.h>

#define CRC32_INIT_VALUE            UINT32_C(0xFFFFFFFF)    ///< Initial value for CRC32 calculation
#define CRC32_POLYNOME              UINT32_C(0x04C11DB7)    ///< CRC32 polynome to be used for calculating CRC32

/**
 * @brief Do the CRC32 for the given data
 *        Source: https://stackoverflow.com/a/21001712/2031180
 *
 * @param[in] src The source data to execute the CRC32
 * @param[in] size the Size of @p src buffer
 *
 * @retval Returns the CRC value
 */
uint32_t crc32_calculate(const char *src, size_t size);

#endif /* CRC32_H__ */
