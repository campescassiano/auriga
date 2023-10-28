#ifndef MESSAGE_H__
#define MESSAGE_H__

#include <stdint.h>
#include <inttypes.h>

/**
 * @brief The below definitions are related to the raw data sizes (binary form)
 */
#define TYPE_SIZE                   UINT8_C(sizeof(uint8_t))
#define LENGTH_SIZE                 UINT8_C(sizeof(uint8_t))
#define DATA_SIZE                   UINT8_C(sizeof(uint8_t) * 251)
#define CRC_SIZE                    UINT8_C(sizeof(uint32_t))

#define PAYLOAD_SIZE                UINT8_C(DATA_SIZE + CRC_SIZE)
#define MASK_SIZE                   UINT8_C(sizeof(uint32_t))

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

/**
 * @brief Maximum possible size of a message
 */
#define ASCII_MESSAGE_MAX_SIZE      (TYPE_HEX_LENGTH + \
                                     LENGTH_HEX_LENGTH + \
                                     PAYLOAD_HEX_LENGTH)

#define ASCII_MASK_MAX_SIZE         (MASK_HEX_LENGTH)       ///< The maximum size of the mask in hex ASCII

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

#endif /* MESSAGE_H__ */
