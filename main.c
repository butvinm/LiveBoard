#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "index.h"

#define _RUN_IMPLEMENTATION
#include "run.h"
#undef _RUN_IMPLEMENTATION

#define _DEVICES_IMPLEMENTATION
#include "devices.h"
#undef _DEVICES_IMPLEMENTATION

#define BODY_SIZE (1024*16)

#define CTRLS_COUNT 3

static const char* CTRLS[CTRLS_COUNT] = {
    "brightness",
    "contrast",
    "saturation"
};


void build_page(char* content, const int ctrls_init_values[], const char* stream_url) {
    char body[BODY_SIZE];

    sprintf(
        content,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        INDEX_PAGE,
        stream_url,
        ctrls_init_values[0],
        ctrls_init_values[0],
        ctrls_init_values[0],
        ctrls_init_values[1],
        ctrls_init_values[1],
        ctrls_init_values[1],
        ctrls_init_values[2],
        ctrls_init_values[2],
        ctrls_init_values[2]
    );
}


void handle_request(int client_socket, const char* device, const int ctrls_init_values[], const char* stream_url) {
    char request[BODY_SIZE];
    int bytes_read = read(client_socket, request, sizeof(request) - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "[ERROR] Failed to read request\n");
        close(client_socket);
        return;
    }
    request[bytes_read] = '\0';

    // POST /ctrls/?brightness=128
    if (strstr(request, "POST /ctrls/?")) {
        char ctrl_name[32];
        char ctrl_value[32];

        const char* query = strchr(request, '?') + 1;
        const char* equals = strchr(query, '=');
        if (!equals) {
            fprintf(stderr, "[ERROR] Invalid request: no '=' found in query.\n");
            return;
        }

        size_t ctrl_name_len = equals - query;
        strncpy(ctrl_name, query, ctrl_name_len);
        ctrl_name[ctrl_name_len] = '\0';

        const char* value = equals + 1;
        const char* space = strchr(query, ' ');
        if (!space) {
            fprintf(stderr, "[ERROR] Invalid request: no ' ' found after query value.\n");
            return;
        }

        size_t ctrl_value_len = space - value;
        strncpy(ctrl_value, value, ctrl_value_len);
        ctrl_value[ctrl_value_len] = '\0';

        bool ctrl_exist = false;
        for (size_t i = 0; i < CTRLS_COUNT; ++i) {
            if (strcmp(ctrl_name, CTRLS[i]) == 0) {
                ctrl_exist = true;
                break;
            }
        }
        printf("[INFO] Set control value: %s=%d\n", ctrl_name, atoi(ctrl_value));

        if (!ctrl_exist) {
            fprintf(stderr, "[ERROR] Unknown control: %s\n", ctrl_name);
            return;
        }

        dev_set_ctrl(device, ctrl_name, atoi(ctrl_value));
    }

    char response[BODY_SIZE];
    build_page(response, ctrls_init_values, stream_url);
    write(client_socket, response, strlen(response));
    close(client_socket);
}



int main(int argc, char* argv[]) {
    // main <port> <dev/video> <stream-url>
    if (argc < 4) {
        fprintf(stderr, "[ERROR] Usage: main <port> <dev/video> <stream-url>\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    char* device = argv[2];
    char* stream_url = argv[3];

    int ctrls_init_values[CTRLS_COUNT];

    char cmd[1024];
    char output[4096];
    size_t output_size;
    for (size_t i = 0; i < CTRLS_COUNT; ++i) {
        if (dev_get_ctrl(device, CTRLS[i], &ctrls_init_values[i]) != 0) {
            fprintf(stderr, "[ERROR] Failed to get device control value: %s", CTRLS[i]);
            exit(EXIT_FAILURE);
        }
    }

    printf("[INFO] Initial device controls preference:\n");
    for (size_t i = 0; i < CTRLS_COUNT; ++i) {
        printf("\t%s=%d\n", CTRLS[i], ctrls_init_values[i]);
    }

    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[ERROR] Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[ERROR] setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[ERROR] Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("[ERROR] Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] HTTP server listening on http://127.0.0.1:%d\n", port);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed\n");
            continue;
        }
        handle_request(client_socket, device, ctrls_init_values, stream_url);
    }

    close(server_fd);
    return 0;
}
