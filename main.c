#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "errors.h"
#include "crc32.h"
#include "message.h"
#include "utils.h"
#include "file_ops.h"

#define INPUT_FILE      ("data_in.txt")     ///< Input file to be used
#define OUTPUT_FILE     ("data_out.txt")    ///< Output file to be written to

int main(int argc, char **argv)
{
    message_t original_message;
    message_t modified_message;

    if (message_load(INPUT_FILE, &original_message) == false)
    {
        printf("Error! Loading message, %s:%d\n", __func__, __LINE__);
        goto check_error;
    }

    message_update(&original_message, &modified_message);
    file_ops_write_output_original(OUTPUT_FILE, &original_message, FILE_OPS_APPEND);

    if (g_errno == ERROR_NO_ERROR)
        file_ops_write_output_modified(OUTPUT_FILE, &modified_message, FILE_OPS_APPEND);

    printf("Execution completed\n");
    return 0;

check_error:
    printf("Please check \"%s\" file for error message\n", OUTPUT_FILE);
    error_write_error_on_file(OUTPUT_FILE);
    return -g_errno;
}

