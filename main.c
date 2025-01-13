#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <curl/curl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WB2SVG_IMPLEMENTATION
#include "wb2svg.h"

#define _RUN_IMPLEMENTATION
#include "run.h"
#undef _RUN_IMPLEMENTATION

#define _DEVICES_IMPLEMENTATION
#include "devices.h"


#define MAX_BODY_SIZE (1024*1024)

#define CTRLS_COUNT 3

static const char* CTRLS[CTRLS_COUNT] = {
    "brightness",
    "contrast",
    "saturation"
};

// const int ctrls_init_values[], const char* svg, const char* stream_url

        // INDEX_PAGE,
        // stream_url,
        // ctrls_init_values[0],
        // ctrls_init_values[0],
        // ctrls_init_values[0],
        // ctrls_init_values[1],
        // ctrls_init_values[1],
        // ctrls_init_values[1],
        // ctrls_init_values[2],
        // ctrls_init_values[2],
        // ctrls_init_values[2],
        // svg

void build_response(char* response, const char* status, char* body) {
    sprintf(
        response,
        "HTTP/1.1 %s\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status,
        body
    );
}


void build_index_page(char* response, const int ctrls_init_values[], const char* stream_url) {
    FILE* index_page_tpl_file = fopen("pages/index.html", "r");    
    fseek(index_page_tpl_file, 0, SEEK_END);
    size_t fsize = ftell(index_page_tpl_file);
    fseek(index_page_tpl_file, 0, SEEK_SET);
    char* index_page_tpl = malloc(fsize + 1);
    fread(index_page_tpl, fsize, 1, index_page_tpl_file);
    fclose(index_page_tpl_file);
    index_page_tpl[fsize] = '\0';

    char* body = malloc(fsize + 1000);
    sprintf(
        body, 
        index_page_tpl,
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

    build_response(response, "200 OK", body);
    free(body);
    free(index_page_tpl);

    return;
}



int download_file(const char *url, const char *output_file);

void handle_request(int client_socket, const char* device, const int ctrls_init_values[], const char* stream_url, const char* snapshot_url) {
    char request[MAX_BODY_SIZE];
    int bytes_read = read(client_socket, request, sizeof(request) - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "[ERROR] Failed to read request\n");
        close(client_socket); 
        return;
    }
    request[bytes_read] = '\0';

    
    char* response = malloc(sizeof(char) * 1024 * 1024 * 10);

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
        build_response(response, "200 OK", "");
    } else if (strstr(request, "GET /svg")) {
        char* svg = (char*)malloc(1024*1024*10);
        sprintf(svg, "");
    
        printf("Start converting to SVG\n");
        char output[4096];
        download_file(snapshot_url, "image.jpg");

        int width, height;
        wb2svg_rgba* pixels = (wb2svg_rgba*)stbi_load("image.jpg", &width, &height, NULL, 4);
        if (pixels == NULL) {
            fprintf(stderr, "ERROR: could not read %s\n", "image.jpg");    free(svg);
            return;
        }
        wb2svg_img img = { .pixels = pixels, .width = width, .height = height };

        if (wb2svg_wb2svg(img, svg, 1024*1024*10) < 0) {
            fprintf(stderr, "ERROR: buffer size exceeded.");
        } else {
            printf("Converted successful\n");
        }

        printf("[INFO] SVG generated\n");
        build_response(response, "200 OK", svg);
        free(svg);
    } else {
        build_index_page(response, ctrls_init_values, stream_url);
    }
    write(client_socket, response, strlen(response));
    close(client_socket);
    free(response);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int download_file(const char *url, const char *output_file) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    printf("Start downloading\n");
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return 1;
    }

    fp = fopen(output_file, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s for writing\n", output_file);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    printf("Perform curl\n");

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        fclose(fp);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Clean up
    fclose(fp);
    curl_easy_cleanup(curl);
    return 0;
}

int main(int argc, char* argv[]) {
    // main <port> <dev/video> <stream-url>
    if (argc < 5) {
        fprintf(stderr, "[ERROR] Usage: main <port> <dev/video> <stream-url> <snapshot-url>\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    char* device = argv[2];
    char* stream_url = argv[3];
    char* snapshot_url = argv[4];

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
        handle_request(client_socket, device, ctrlhttp://192.168.31.105:8080/?action=streams_init_values, stream_url, snapshot_url);
    }

    close(server_fd);
    return 0;
}
