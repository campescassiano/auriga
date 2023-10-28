#ifndef FILE_OPS_H__
#define FILE_OPS_H__

#include <stdint.h>
#include <stdbool.h>

#include "message.h"

#define FILE_OPS_APPEND             (true)  ///< Append into the file
#define FILE_OPS_NOT_APPEND         (false) ///< Not append into the file

/**
 * @brief Write the Output file for the modified message
 *
 * @param[in] filename The filename of the file to be written
 * @param[in] message The message structure where should get the data
 * @param[in] append If set to true, append if file exists; if false, write over
 *
 * @retval True if success; false otherwise
 */
bool file_ops_write_output_modified(const char *filename, message_t *message, bool append);

/**
 * @brief Write the Output file for the original message
 *
 * @param[in] filename The filename of the file to be written
 * @param[in] message The message structure where should get the data
 * @param[in] append If set to true, append if file exists; if false, write over
 *
 * @retval True if success; false otherwise
 */
bool file_ops_write_output_original(const char *filename, message_t *message, bool append);

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
size_t file_ops_read_until(FILE *fp, char *dst, size_t size, char delim, bool inclusive);

#endif /* FILE_OPS_H__ */
