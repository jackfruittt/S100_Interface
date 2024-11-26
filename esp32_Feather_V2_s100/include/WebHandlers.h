#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include "ESPAsyncWebServer.h"

#include "AsyncJpegStreamResponse.h"
#include "camera_pins.h"
void handleStreamRequest(AsyncWebServerRequest *request);

#endif  // WEB_HANDLERS_H