#ifndef ERRORS_H__
#define ERRORS_H__

/**
 * @brief Enumerator with the possible error codes
 */
typedef enum error_e {
    ERROR_NO_ERROR,         ///< No error
    ERROR_LENGTH,           ///< Error in the message length
    ERROR_CRC,              ///< Error with the message CRC
    ERROR_NULL_PARAMETER,   ///< Error regarding null parameter
    ERROR_FILE_NOT_EXIST,   ///< The given file does not exist to read from
    ERROR_NOT_OPEN_FILE,    ///< Could not open the file
    ERROR_DATA_NOT_EXPECTED,///< The data is not what is expected
    ERROR_READING_FILE,     ///< Error while reading the file
    ERROR_CONVERSION,       ///< Error in converting
    ERROR_BUFFER_SIZE,      ///< Error in buffer size
    ERROR_STRING_FORMAT,    ///< Error in string format
    ERROR_FILE_CREATION,    ///< Error creating the file
    ERROR_FTELL,            ///< Error in ftell()
    ERROR_FSEEK,            ///< Error in fseek()
} error_e;

extern error_e g_errno;     ///< Forward declaration of the global error variable

/**
 * @brief Write the error that happened into the output file
 *
 * @param[in] filename The filename to write the error string
 */
void error_write_error_on_file(const char *filename);

#endif /* ERRORS_H__ */
