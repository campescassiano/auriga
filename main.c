#include "errors.h"
#include "message.h"
#include "file_ops.h"
#include "debug.h"

#define INPUT_FILE      ("data_in.txt")     ///< Input file to be used
#define OUTPUT_FILE     ("data_out.txt")    ///< Output file to be written to

int main(int argc, char **argv)
{
    message_t original_message;
    message_t modified_message;

    (void) argc;
    (void) argv;

    if (message_load(INPUT_FILE, &original_message) == false)
    {
        DEBUG_WARN("Please check \"%s\" file for error message\n", OUTPUT_FILE);
        error_write_error_on_file(OUTPUT_FILE);

        return g_errno;
    }

    message_update(&original_message, &modified_message);
    file_ops_write_output_original(OUTPUT_FILE, &original_message, FILE_OPS_APPEND);

    if (g_errno == ERROR_NO_ERROR)
        file_ops_write_output_modified(OUTPUT_FILE, &modified_message, FILE_OPS_APPEND);

    DEBUG_INFO("Execution completed");

    return 0;
}

