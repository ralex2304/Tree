#include "graph_log.h"

bool create_img(const char* input_filename, const char* output_filename) {
    assert(input_filename);
    assert(output_filename);

    static const size_t MAX_COMMAND_LEN = 256;
    char command[MAX_COMMAND_LEN] = {};

    strncat_len(command, "dot -Tsvg ", MAX_COMMAND_LEN);
    strncat_len(command, input_filename, MAX_COMMAND_LEN);
    strncat_len(command, " > ", MAX_COMMAND_LEN);
    strncat_len(command, output_filename, MAX_COMMAND_LEN);

    if (system(command) != 0) {
        fprintf(stderr, "Error executing \"%s\"\n", command);
        return false;
    }

    return true;
}
