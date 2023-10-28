#ifndef UTILS_H__
#define UTILS_H__

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>

#include "message.h"
#include "errors.h"

#define MIN(a,b) (size_t)(((a)<(b))?(a):(b))        ///< Definition of MIN() function
#define MAX(a,b) (size_t)(((a)>(b))?(a):(b))        ///< Definition of MAX() function

/**
 * @brief Convert an hex ASCII string to its binary representation
 *
 * @param[in] src The source buffer where the hex ASCII string is
 * @param[in] src_size The size of the @p src buffer
 * @param[out] dst The destination where we will store the binary representation
 * @param[in] dst_size The size of the @p dst buffer
 *
 * @return Returns the number of bytes converted into the @p dst buffer
 */
size_t utils_hex_to_bin(char *src, size_t src_size, char *dst, size_t dst_size);

/**
 * @brief Convert a binary array into its hex ASCII representation
 *
 * @param[in] src The source buffer where the binary data is
 * @param[in] src_size The size of @p src buffer
 * @param[out] dst The destination where the hex ASCII string will be stored
 * @param[in] dst_size The size of @p dst buffer
 *
 * @return Returns the number of bytes converted into the @p dst buffer
 */
size_t utils_bin_to_hex(char *src, size_t src_size, char *dst, size_t dst_size);

/**
 * @brief Apply the requested mask on the tetrads (4 bytes) of the given @p data
 *
 * @param[in,out] data The data where the mask will be applied to
 * @param[in] size The size of the @p data buffer
 * @param[in] mask The mask that will be used
 *
 * @retval no return; but if the data is not multiple of tetrad, then it wont apply the mask
 */
void utils_apply_mask_on_tetrads(char *data, size_t size, uint32_t mask);

/**
 * @brief Appends a pair of header & payload into the given buffer
 *
 * @param[out] dst The destination where it should write the header and payload
 * @param[in] dst_size The size of @p dst buffer
 * @param[in] header The header to be used
 * @param[in] header_size The size of @p header buffer
 * @param[in] payload The payload to be used
 * @param[in] payload_size The size of @p payload buffer
 *
 * @retval Returns how many bytes it wrote on the @p dst buffer
 */
size_t utils_append_header_and_payload_into_buffer(char *dst,
                                                   size_t dst_size,
                                                   char *header,
                                                   size_t header_size,
                                                   char *payload,
                                                   size_t payload_size);

#endif /* UTILS_H__ */
