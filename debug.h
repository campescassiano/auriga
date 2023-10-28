#ifndef DEBUG_H__
#define DEBUG_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RESET_STYLE "\033[0m"   ///< Reset the style
#define RED "\033[31m"          ///< Set red color font
#define YELLOW "\033[33m"       ///< Set yellow color font
#define GREEN "\033[32m"        ///< Set green color font
#define BOLD "\033[1m"          ///< Set bold font

/**
 * @brief Specify each debugging level in which the file stream will be used.
 */
#ifndef __DEBUG_INFO_FILE__
#define __DEBUG_INFO_FILE__ (stdout)    ///< The file stream to print debug information
#endif /* __DEBUG_INFO_FILE__ */

#ifndef __DEBUG_WARN_FILE__
#define __DEBUG_WARN_FILE__ (stdout)    ///< The file stream to print warning information
#endif /* __DEBUG_WARN_FILE__ */

#ifndef __DEBUG_ERROR_FILE__
#define __DEBUG_ERROR_FILE__ (stderr)   ///< The file stream to print error information
#endif /* __DEBUG_ERROR_FILE__ */

//! Filename macro
#define __DEBUGFILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/**
 * The base debug format to use
 * @param[in] file The file stream used
 * @param[in] type String to indicate the debug level to be printed
 * @param[in] format Format used
 * @param[in] ... Variables cited in the format string
 * @return Number of characters printed.
 */
#define __DEBUG__(file, type, format, ...) \
    fprintf(file, BOLD type " %s:%d %s()]: " RESET_STYLE format "\n", \
            __DEBUGFILENAME__, __LINE__, __func__, ##__VA_ARGS__)

/**
 * Info print level
 * @param[in] format Format used
 * @param[in] ... Variables cited in the format string
 * @return Number of characters printed
 */
#define DEBUG_INFO(format, ...) \
    __DEBUG__(__DEBUG_INFO_FILE__, GREEN "[INFO", format, ##__VA_ARGS__)

/**
 * Warning print level
 * @param[in] format Format used
 * @param[in] ... Variables cited in the format string
 * @return Number of characters printed
 */
#define DEBUG_WARN(format, ...) \
    __DEBUG__(__DEBUG_WARN_FILE__, YELLOW "[WARN", format, ##__VA_ARGS__)

/**
 * Error print level
 * @param[in] format Format used
 * @param[in] ... Variables cited in the format string
 * @return Number of characters printed
 */
#define DEBUG_ERROR(format, ...) \
    __DEBUG__(__DEBUG_ERROR_FILE__, RED "[ERROR", format, ##__VA_ARGS__)

#endif /* DEBUG_H__ */
