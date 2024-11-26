#include "WebHandlers.h"

void handleStreamRequest(AsyncWebServerRequest *request) {
    AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse();
    if (!response) {
        request->send(500); // Use 500 for internal server error
        return;
    }
    response->addHeader("Access-Control-Allow-Origin", "*"); // Allow all origins
    response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
}