#include <opencv2/opencv.hpp>
#include <microhttpd.h>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

std::atomic<bool> running(true);
std::vector<uchar> frameBuffer;
std::mutex bufferMutex;

void processVideo() {
    cv::VideoCapture cap(0); // Open default camera
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open webcam." << std::endl;
        running = false;
        return;
    }

    cv::Mat frame;
    while (running) {
        cap >> frame; // Capture frame
        if (frame.empty()) continue;

        // Adjust brightness
        frame += cv::Scalar(50, 50, 50); // Brightness adjustment by adding intensity

        // Encode frame as JPEG
        std::vector<uchar> encodedFrame;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
        cv::imencode(".jpg", frame, encodedFrame, params);

        // Update shared buffer
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            frameBuffer = std::move(encodedFrame);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // Approx 30 FPS
    }
    cap.release();
}

MHD_Result answerToConnection(void *cls, struct MHD_Connection *connection,
                       const char *url, const char *method, const char *version,
                       const char *upload_data, size_t *upload_data_size, void **con_cls) {
    static const char *boundary = "frame";
    struct MHD_Response *response;
    int ret;

    // HTTP Header for MJPEG
    response = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN,
                                                 1024,
                                                 [](void *cls, uint64_t pos, char *buf, size_t max) -> ssize_t {
                                                     static std::vector<uchar> localBuffer;
                                                     {
                                                         std::lock_guard<std::mutex> lock(bufferMutex);
                                                         localBuffer = frameBuffer;
                                                     }
                                                     if (localBuffer.empty()) return MHD_CONTENT_READER_END_OF_STREAM;

                                                     // Write MJPEG frame
                                                     std::ostringstream oss;
                                                     oss << "--" << boundary << "\r\n"
                                                         << "Content-Type: image/jpeg\r\n"
                                                         << "Content-Length: " << localBuffer.size() << "\r\n\r\n";
                                                     std::string header = oss.str();
                                                     memcpy(buf, header.c_str(), header.size());
                                                     memcpy(buf + header.size(), localBuffer.data(), localBuffer.size());
                                                     return header.size() + localBuffer.size();
                                                 },
                                                 nullptr,
                                                 [](void *cls) {});

    if (!response)
        return MHD_NO;

    MHD_add_response_header(response, "Content-Type", "multipart/x-mixed-replace; boundary=frame");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return static_cast<MHD_Result>(ret);
}

int main() {
    const int PORT = 8080;

    // Start video processing thread
    std::thread videoThread(processVideo);

    // Start HTTP server
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, nullptr, nullptr,
                                                 &answerToConnection, nullptr, MHD_OPTION_END);
    if (!daemon) {
        std::cerr << "Error: Could not start HTTP server." << std::endl;
        running = false;
        videoThread.join();
        return 1;
    }

    std::cout << "Server running on http://localhost:" << PORT << std::endl;
    std::cout << "Press Enter to stop the server." << std::endl;
    std::cin.get(); // Wait for user input to exit

    // Clean up
    running = false;
    MHD_stop_daemon(daemon);
    videoThread.join();

    return 0;
}
