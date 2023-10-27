#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>

#define MIN(a,b) (((a)<(b))?(a):(b))        ///< Definition of MIN() function

#define INPUT_FILE      ("data_in.txt")     ///< Input file to be used
#define OUTPUT_FILE     ("data_out.txt")    ///< Output file to be written to

static const char g_message_leading_keyword[] = "mess=";    ///< leading keyword for message
static const int g_message_leading_length = strlen(g_message_leading_keyword);  ///< leading keyword message length

static const char g_mask_leading_keyword[] = "mask=";       ///< leading keyword for mask
static const int g_mask_leading_length = strlen(g_mask_leading_keyword);    ///< leading keyword mask length

#define ALIGN_APPEND                UINT8_C(4)      ///< Definition of the requested alignment

#define ERROR_STRING_SIZE           UINT8_C(255)    ///< The size of error string
/**
 * @brief The below definitions are related to the raw data sizes (binary form)
 */
#define TYPE_SIZE                   UINT8_C(sizeof(uint8_t))
#define LENGTH_SIZE                 UINT8_C(sizeof(uint8_t))
#define DATA_SIZE                   UINT8_C(sizeof(uint8_t) * 251)
#define CRC_SIZE                    UINT8_C(sizeof(uint32_t))

#define PAYLOAD_SIZE                UINT8_C(DATA_SIZE + CRC_SIZE)
#define MASK_SIZE                   UINT8_C(sizeof(uint32_t))

#define MESSAGE_SIZE                UINT16_C(TYPE_SIZE + LENGTH_SIZE + PAYLOAD_SIZE)

#define ASCII_HEX_LENGTH            UINT8_C(sizeof(uint8_t) * 2)    ///< The length of 1-byte representation in ASCII

/**
 * @brief The below definitions are related to the ASCII data sizes representation
 */
#define TYPE_HEX_LENGTH             UINT8_C(TYPE_SIZE * 2)
#define LENGTH_HEX_LENGTH           UINT8_C(LENGTH_SIZE * 2)
#define DATA_HEX_LENGTH             UINT8_C(DATA_SIZE * 2)
#define CRC32_HEX_LENGTH            UINT8_C(CRC_SIZE * 2)

#define PAYLOAD_HEX_LENGTH          UINT8_C(PAYLOAD_SIZE * 2)
#define MASK_HEX_LENGTH             UINT8_C(MASK_SIZE * 2)

#define INCLUSIVE                   (true)  ///< Read until marker found, include marker
#define EXCLUSIVE                   (false) ///< Read until marker is found, do not include marker

#define APPEND                      (true)  ///< Append into the file
#define NOT_APPEND                  (false) ///< Not append into the file

#define ASCII_MESSAGE_MIN_SIZE      (TYPE_HEX_LENGTH + \
                                     LENGTH_HEX_LENGTH + \
                                     DATA_HEX_LENGTH + \
                                     CRC32_HEX_LENGTH)

/**
 * @brief Maximum possible size of a message
 */
#define ASCII_MESSAGE_MAX_SIZE      (TYPE_HEX_LENGTH + \
                                     LENGTH_HEX_LENGTH + \
                                     PAYLOAD_HEX_LENGTH)

#define ASCII_MASK_MAX_SIZE         (MASK_HEX_LENGTH)

#define CRC32_INIT_VALUE            UINT32_C(0xFFFFFFFF)    ///< Initial value for CRC32 calculation
#define CRC32_POLYNOME              UINT32_C(0x04C11DB7)    ///< CRC32 polynome to be used for calculating CRC32

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
} error_e;

static error_e g_errno = ERROR_NO_ERROR;            ///< Global application error code

/**
 * @brief Structure that represents the message and its parameters
 */
typedef struct message_s {
    char type;              ///< Stores the message type
    char length;            ///< Stores the message length
    char data[DATA_SIZE];   ///< Stores the message data
    char crc[CRC_SIZE];     ///< Stores the message CRC
    char mask_val[MASK_SIZE];   ///< stores the message mask

    /**
     * @brief Structure that stores the raw message bytes from the file
     */
    struct {
        char raw[ASCII_MESSAGE_MAX_SIZE + 1];   ///< The raw message bytes read from the file
        size_t size;                            ///< The size of the data read from the file
    } message;

    /**
     * @brief Structure that stores the raw mask bytes from the file
     */
    struct {
        char raw[ASCII_MASK_MAX_SIZE + 1];      ///< The raw mask bytes read from the file
        size_t size;                            ///< The size of the mask read from the file
    } mask;
} message_t;

/**
 * @brief Do the CRC32 for the given data
 *        Source: https://stackoverflow.com/a/21001712/2031180
 *
 * @param src The source data to execute the CRC32
 * @param size the Size of @p src buffer
 *
 * @return Returns the CRC value
 */
static uint32_t do_crc32(const char *src, size_t size)
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

/**
 * @brief Convert an hex ASCII string to its binary representation
 *
 * @param src The source buffer where the hex ASCII string is
 * @param src_size The size of the @p src buffer
 * @param dst The destination where we will store the binary representation
 * @param dst_size The size of the @p dst buffer
 *
 * @return Returns true if conversion succeed; false otherwise
 */
static bool hex_to_bin(char *src, size_t src_size, char *dst, size_t dst_size)
{
    size_t i = 0;
    char temp[ASCII_HEX_LENGTH] = {0};

    if ((src_size >> 1) > dst_size)
        return false;

    memset(dst, 0, dst_size);

    for (i = 0; i < (src_size >> 1); ++i)
    {
        memcpy(temp, src, 2);
        temp[2] = '\0';
        dst[i] = (unsigned int) strtoul(temp, NULL, 16);
        src += ASCII_HEX_LENGTH;
    }

    return true;
}

/**
 * @brief Apply the requested mask on the tetrads (4 bytes) of the given @p data
 *
 * @param data The data where the mask will be applied to
 * @param size The size of the @p data buffer
 * @param mask The mask that will be used
 *
 * @return no return; but if the data is not multiple of tetrad, then it wont apply the mask
 */
static void apply_mask_on_tetrads(char *data, size_t size, uint32_t mask)
{
    uint32_t *p;

    if (data == NULL)
    {
        printf("Error! null parameter, %s:%d\n", __func__, __LINE__);
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

/**
 * @brief Update the original message according to the project's specification
 *          which is regarding the data padding, CRC calculation and so on.
 *
 * @param original The original parsed message
 * @param modified The destination where the modified message will be stored
 *
 * @return True if it could update; false otherwise
 */
static bool update_message(const message_t *original, message_t *modified)
{
    uint32_t mask = 0;
    size_t append = 0;
    uint32_t crc = 0;

    if (original == NULL || modified == NULL)
    {
        printf("Error! Null parameters, %s:%d\n", __func__, __LINE__);
        return false;
    }

    modified->type = original->type;

    memcpy((char*)&mask, original->mask_val, sizeof(uint32_t));

    append = (original->length - CRC_SIZE) % ALIGN_APPEND;

    memcpy(modified->data, original->data, original->length - CRC_SIZE);
    modified->length = original->length;

    if (append)
    {
        printf("Info! appending %ld bytes on data bytes\n", append);
        modified->length += append;
        memset(&modified->data[(uint8_t)modified->length], 0, sizeof(char) * append);
    }

    apply_mask_on_tetrads(modified->data, modified->length - CRC_SIZE, *(uint32_t*)&original->mask_val);

    crc = do_crc32(modified->data, modified->length - CRC_SIZE);
    memcpy(modified->crc, (char*)&crc, sizeof(uint32_t));

    return true;
}

/**
 * @brief Convert a binary array into its hex ASCII representation
 *
 * @param src The source buffer where the binary data is
 * @param src_size The size of @p src buffer
 * @param dst The destination where the hex ASCII string will be stored
 * @param dst_size The size of @p dst buffer
 *
 * @return Returns the number of bytes converted into the @p dst buffer
 */
static int bin_to_hex(char *src, size_t src_size, char *dst, size_t dst_size)
{
    size_t i = 0;
    if (src_size * ASCII_HEX_LENGTH > dst_size)
        return 0;

    memset(dst, 0, dst_size);

    for (i = 0; i < src_size; i++)
    {
        snprintf(dst, 3, "%02x", 0xff & src[i]);
        dst+= 2;
    }

    return (i * ASCII_HEX_LENGTH);
}

/**
 * @brief Function to read from the given file pointer up to the delimiter specified
 *
 * @param fp The file pointer where it should read
 * @param dst The destination where it sould store the read data
 * @param size The size of @p dst buffer
 * @param delim The delimiter that is the anchor for reading
 * @param inclusive If true, then the @p delim will be included in the dst, if false then @p delim will not be included
 *
 * @return Returns the number of bytes read considering that it found the delim. If it does not found the delim, it returns 0 and it fseeks() back to where it started (atomic function)
 */
static size_t read_until(FILE *fp, char *dst, size_t size, char delim, bool inclusive)
{
    bool found = false;
    long start = 0;
    size_t read = 0;
    int c;

    if (fp == NULL || dst == NULL)
        return 0;

    start = ftell(fp);
    if (start == -1L)
    {
        printf("Error! Could not call ftell()\n");
        return 0;
    }

    memset(dst, 0, size);
    while ((c = fgetc(fp)) != EOF && read < size)
    {
        dst[read++] = (char) c;

        if (c == delim)
        {
            if (!inclusive)
                dst[--read] = '\0';

            found = true;
            break;
        }
    }

    if (found)
        return read;

    if (fseek(fp, start, SEEK_SET) != 0)
    {
        printf("Error! Could not fseek() back to starting point of the file\n");
    }

    return 0;
}

/**
 * @brief Loads the message from the specified file
 *
 * @param filename The filename where should read the message
 * @param message The pointer to the message structure that will store the message
 */
static bool load_message(const char *filename, message_t *message)
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

    if (access(filename, F_OK))
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

    size = read_until(fp, keyword_message, sizeof(keyword_message), '=', INCLUSIVE);
    if (size != sizeof(g_message_leading_keyword) - 1)
    {
        printf("Error! Read data different from expected\n");
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    if (strcmp(keyword_message, g_message_leading_keyword) != 0)
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
    message->type = strtoul(ascii_byte, NULL, 16);

    size = fread(ascii_byte, sizeof(uint8_t), MIN(ASCII_HEX_LENGTH, sizeof(ascii_byte)), fp);
    if (size != ASCII_HEX_LENGTH)
    {
        printf("Error! Could not read correctly, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_READING_FILE;
        return false;
    }
    message->length = strtoul(ascii_byte, NULL, 16);

    message->message.size = read_until(fp, message->message.raw,
                                       sizeof(message->message.raw),
                                       '\n', EXCLUSIVE);
    if (message->message.size != (message->length * ASCII_HEX_LENGTH))
    {
        printf("Error! Wrong message size, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_LENGTH;
        return false;
    }

    size = read_until(fp, keyword_mask, sizeof(keyword_mask), '=', INCLUSIVE);
    if (size != sizeof(keyword_mask) - 1)
    {
        printf("Error! Read data different from expected, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_DATA_NOT_EXPECTED;
        return false;
    }

    if (strcmp(keyword_mask, g_mask_leading_keyword) != 0)
    {
        printf("Error! Could not find anchor \"%s\" on the file, %s:%d\n", g_mask_leading_keyword, __func__, __LINE__);
        fclose(fp);
        return false;
    }

    message->mask.size = read_until(fp, message->mask.raw, sizeof(message->mask.raw), '\n', EXCLUSIVE);
    if (message->mask.size != MASK_HEX_LENGTH)
    {
        printf("Error! Wrong message size, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_LENGTH;
        return false;
    }

    if (hex_to_bin(message->message.raw, message->length * ASCII_HEX_LENGTH - CRC32_HEX_LENGTH,
                   message->data, sizeof(message->data)) == false)
    {
        printf("Error! Could not convert hex to bin, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    size_t pos = message->length * ASCII_HEX_LENGTH - CRC32_HEX_LENGTH;

    if (hex_to_bin(&message->message.raw[pos], CRC32_HEX_LENGTH,
                message->crc, sizeof(message->crc)) == false)
    {
        printf("Error! Could not convert hex to bin, %s:%d\n", __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_CONVERSION;
        return false;
    }


    uint32_t calculated = do_crc32(message->data, message->length - CRC_SIZE);

    if (htonl(*(uint32_t*)message->crc) != calculated)
    {
        printf("Error! Wrong CRC, should be=%"PRIu32", got=%"PRIu32", %s:%d\n", calculated, htonl(*(uint32_t*)message->crc), __func__, __LINE__);
        fclose(fp);
        g_errno = ERROR_CRC;
        return false;
    }

    if (hex_to_bin(message->mask.raw, message->mask.size, message->mask_val, sizeof(message->mask_val)) == false)
    {
        printf("Error! Could not convert hex to bin\n");
        fclose(fp);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    fclose(fp);

    return true;
}

/**
 * @brief Appends a pair of header & payload into the given buffer
 *
 * @param dst The destination where it should write the header and payload
 * @param dst_size The size of @p dst buffer
 * @param header The header to be used
 * @param header_size The size of @p header buffer
 * @param payload The payload to be used
 * @param payload_size The size of @p payload buffer
 *
 * @return Returns how many bytes it wrote on the @p dst buffer
 */
static size_t append_header_and_payload_into_buffer(char *dst,
                                                    size_t dst_size,
                                                    char *header,
                                                    size_t header_size,
                                                    char *payload,
                                                    size_t payload_size)
{
    size_t wrote = 0;

    if (dst == NULL || header == NULL || payload == NULL)
    {
        printf("Error! Null parameters received, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return 0;
    }

    memset(dst, 0, dst_size);

    if ((header_size + payload_size) >= dst_size)
    {
        printf("Error! Destination buffer is smaller than the data to be formatted, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_BUFFER_SIZE;
        return 0;
    }

    wrote = snprintf(dst, dst_size, "%s%s\n", header, payload);
    if (wrote == 0)
    {
        printf("Error! could not format string, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }

    return wrote;
}

/**
 * @brief Write the Output file for the original message
 *
 * @param filename The filename of the file to be written
 * @param message The message structure where should get the data
 * @param append If set to true, append if file exists; if false, write over
 *
 * @return True if success; false otherwise
 */
static bool write_output_original(const char *filename, message_t *message, bool append)
{
    char msg[ASCII_MESSAGE_MAX_SIZE] = {0};
    char temporary[ASCII_MESSAGE_MAX_SIZE * 2] = {0};
    size_t written = 0;
    size_t pos = 0;
    FILE *fp;

    if (filename == NULL || message == NULL)
    {
        printf("Error! NULL parameter, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (append)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("Error! Creating/opening \"%s\" file, %s:%d\n", filename, __func__, __LINE__);
        g_errno = ERROR_FILE_CREATION;
        return false;
    }

    written = bin_to_hex(&message->type, sizeof(message->type),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "message type:",
                                                    strlen("message type:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = bin_to_hex(&message->length, sizeof(message->length),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "initial message length:",
                                                    strlen("initial mesage length:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = bin_to_hex(message->data, MIN(message->length - CRC_SIZE, sizeof(message->data)),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "initial message data bytes:",
                                                    strlen("initial mesage data bytes:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = bin_to_hex(message->crc, sizeof(message->crc),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "initial CRC-32:",
                                                    strlen("initial CRC-32:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    fwrite(msg, sizeof(char), strlen(msg), fp);
    fclose(fp);

    return true;
}

/**
 * @brief Write the Output file for the modified message
 *
 * @param filename The filename of the file to be written
 * @param message The message structure where should get the data
 * @param append If set to true, append if file exists; if false, write over
 *
 * @return True if success; false otherwise
 */
static bool write_output_modified(const char *filename, message_t *message, bool append)
{
    char msg[ASCII_MESSAGE_MAX_SIZE] = {0};
    char temporary[ASCII_MESSAGE_MAX_SIZE * 2] = {0};
    size_t written = 0;
    size_t pos = 0;
    FILE *fp;

    if (filename == NULL || message == NULL)
    {
        printf("Error! NULL parameter, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_NULL_PARAMETER;
        return false;
    }

    if (append)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    if (fp == NULL)
    {
        printf("Error! Creating/opening \"%s\" file, %s:%d\n", filename, __func__, __LINE__);
        g_errno = ERROR_FILE_CREATION;
        return false;
    }

    written = bin_to_hex(&message->length, sizeof(message->length),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "modified message length:",
                                                    strlen("modified mesage length:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = bin_to_hex(message->data, MIN(message->length - CRC_SIZE, sizeof(message->data)),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "modified message data bytes with mask:",
                                                    strlen("modified mesage data bytes with mask:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    written = bin_to_hex(message->crc, sizeof(message->crc),
                         temporary, sizeof(temporary));
    if (written == 0)
    {
        printf("Error! could not convert bin to hex, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_CONVERSION;
        return false;
    }

    written = append_header_and_payload_into_buffer(&msg[pos],
                                                    sizeof(msg) - pos,
                                                    "modified CRC-32:",
                                                    strlen("modified CRC-32:"),
                                                    temporary,
                                                    strlen(temporary));
    if (written == 0)
    {
        printf("Error! Failed to append header and payload, %s:%d\n", __func__, __LINE__);
        g_errno = ERROR_STRING_FORMAT;
        return false;
    }
    pos += written;

    fwrite(msg, sizeof(char), strlen(msg), fp);
    fclose(fp);

    return true;
}

/**
 * @brief Write the error that happened into the output file
 *
 * @param filename The filename to write the error string
 * @param message The message buffer
 */
static void write_errors_on_output_file(const char *filename, const message_t *message)
{
    char error_string[ERROR_STRING_SIZE] = {0};
    size_t wrote = 0;
    FILE *fp = NULL;

    if (filename == NULL || message == NULL)
    {
        printf("Error! NULL parameter, %s:%d\n", __func__, __LINE__);
        return;
    }

    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error! Creating/opening \"%s\" file, %s:%d\n", filename, __func__, __LINE__);
        return;
    }

    switch (g_errno)
    {
        case ERROR_LENGTH:
            snprintf(error_string, ERROR_STRING_SIZE, "Error in length of the message\n");
            break;

        case ERROR_CRC:
            snprintf(error_string, ERROR_STRING_SIZE, "Error in CRC value of the message\n");
            break;

        case ERROR_NULL_PARAMETER:
            snprintf(error_string, ERROR_STRING_SIZE, "Error NULL parameter\n");
            break;

        case ERROR_FILE_NOT_EXIST:
            snprintf(error_string, ERROR_STRING_SIZE, "Error file not exist\n");
            break;

        case ERROR_NOT_OPEN_FILE:
            snprintf(error_string, ERROR_STRING_SIZE, "Error could not open file\n");
            break;

        case ERROR_DATA_NOT_EXPECTED:
            snprintf(error_string, ERROR_STRING_SIZE, "Data is not expected\n");
            break;

        case ERROR_READING_FILE:
            snprintf(error_string, ERROR_STRING_SIZE, "Error reading file\n");
            break;

        case ERROR_CONVERSION:
            snprintf(error_string, ERROR_STRING_SIZE, "Error converting string\n");
            break;

        case ERROR_BUFFER_SIZE:
            snprintf(error_string, ERROR_STRING_SIZE, "Error in buffer size\n");
            break;

        case ERROR_STRING_FORMAT:
            snprintf(error_string, ERROR_STRING_SIZE, "Error string format\n");
            break;

        case ERROR_FILE_CREATION:
            snprintf(error_string, ERROR_STRING_SIZE, "Error file creation\n");
            break;

        default:
            snprintf(error_string, ERROR_STRING_SIZE, "Unknown error value: %d\n", g_errno);
            break;
    }

    wrote = fwrite(error_string, sizeof(char), strlen(error_string), fp);
    if (wrote == 0)
        printf("Error! Could not write into file \"%s\"\n", filename);

    fclose(fp);
}

int main(int argc, char **argv)
{
    message_t original_message;
    message_t modified_message;

    if (load_message(INPUT_FILE, &original_message) == false)
    {
        printf("Error! Loading message, %s:%d\n", __func__, __LINE__);
        goto check_error;
    }

    update_message(&original_message, &modified_message);
    write_output_original(OUTPUT_FILE, &original_message, APPEND);

    if (g_errno == ERROR_NO_ERROR)
        write_output_modified(OUTPUT_FILE, &modified_message, APPEND);

    printf("Execution completed\n");
    return 0;

check_error:
    printf("Please check \"%s\" file for error message\n", OUTPUT_FILE);
    write_errors_on_output_file(OUTPUT_FILE, &original_message);
    return -g_errno;
}

