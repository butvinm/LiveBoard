#ifndef _CURL_DOWNLOAD_H
#define _CURL_DOWNLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int download_file(const char *url, const char *output_file);

#endif // _CURL_DOWNLOAD_H

#ifdef _CURL_DOWNLOAD_IMPLEMENTATION


int download_file(const char *url, const char *output_file) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[ERROR] Failed to initialize libcurl\n");
        return 1;
    }

    fp = fopen(output_file, "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open file %s for writing\n", output_file);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        fclose(fp);
        curl_easy_cleanup(curl);
        return 1;
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    return 0;
}
#endif // _CURL_DOWNLOAD_IMPLEMENTATION
