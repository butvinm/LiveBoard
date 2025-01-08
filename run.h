#ifndef _RUN_H
#define _RUN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char run(const char* cmd, char* output, size_t* output_size);

#endif // _RUN_H

#ifdef _RUN_IMPLEMENTATION

#define BUFFER_SIZE 1024

char run(const char* cmd, char* output, size_t* output_size) {
    FILE *fp;

    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run the command: %s", cmd);
        return 69;
    }

    *output_size = 0;
    while (fgets(output + *output_size, BUFFER_SIZE - *output_size, fp) != NULL) {
        *output_size += strlen(output + *output_size);
        if (*output_size >= BUFFER_SIZE - 1) {
            break;
        }
    }
    return WEXITSTATUS(pclose(fp));
}

#endif // _RUN_IMPLEMENTATION
