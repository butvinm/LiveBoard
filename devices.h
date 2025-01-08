#ifndef _DEVICES_H
#define _DEVICES_H

#include "run.h"

int dev_get_ctrl(const char* device, const char* ctrl, int* value);

int dev_set_ctrl(const char* device, const char* ctrl, const int value);


#endif // _DEVICES_H

#ifdef _DEVICES_IMPLEMENTATION

int dev_get_ctrl(const char* device, const char* ctrl, int* value) {
    char cmd[1024];
    char output[4096];
    size_t output_size;

    sprintf(cmd, "v4l2-ctl -d %s --get-ctrl=%s", device, ctrl);
    const char exit_code = run(cmd, output, &output_size);

    char* ptr = output;
    while (*ptr != ':') ptr++;
    *value = atoi(ptr + 1);

    return exit_code;
}

int dev_set_ctrl(const char* device, const char* ctrl, int value) {
    char cmd[1024];
    char output[4096];
    size_t output_size;

    sprintf(cmd, "v4l2-ctl -d %s --set-ctrl=%s=%d", device, ctrl, value);
    const char exit_code = run(cmd, output, &output_size);

    return exit_code;
}

#endif // _DEVICES_IMPLEMENTATION
