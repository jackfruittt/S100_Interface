#include "AsyncJpegStreamResponse.h"
#include "WebHandlers.h"
#include "camera_pins.h"
#include "Arduino.h"
#include "esp_camera.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "WiFi.h"
#include "CytronMotorDriver.h"

#define LED_PIN 2


// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 3, 9);   // PWM 1A = Pin 3, PWM 1B = Pin 9.
CytronMD motorRight(PWM_PWM, 10, 11); // PWM 2A = Pin 10, PWM 2B = Pin 11.

// Wi-Fi credentials
const char* ssid = "ORBI88";
const char* password = "rockylotus108";

// Create Async Web Server on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket endpoint

//Setup enums for control commands
enum Command { CMD_ON, CMD_OFF, CMD_FORWARD, CMD_BACKWARD, CMD_LEFT, CMD_RIGHT,  CMD_STOP, CMD_UNKNOWN };

Command getCommand(String msg) {
    if (msg == "on") return CMD_ON;
    if (msg == "off") return CMD_OFF;
    if (msg == "drive_forwards") return CMD_FORWARD;
    if (msg == "drive_backwards") return CMD_BACKWARD;
    if (msg == "tank_left") return CMD_LEFT;
    if (msg == "tank_right") return CMD_RIGHT;
    if (msg == "motor_stop") return CMD_STOP;
    return CMD_UNKNOWN;
}

// Connect to WiFi
void connectToWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP address: " + WiFi.localIP().toString());
}

// Setup camera configuration
bool setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_VGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (psramFound()) {
        config.jpeg_quality = 20;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }
    return true;
}

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        String msg = String((char *)data);
        Command cmd = getCommand(msg);

        switch (cmd) {
            case CMD_ON:
                digitalWrite(LED_PIN, HIGH);
                client->text("LED turned ON");
                break;
            case CMD_OFF:
                digitalWrite(LED_PIN, LOW);
                client->text("LED turned OFF");
                break;
            case CMD_FORWARD:
                motorLeft.setSpeed(128);
                motorRight.setSpeed(128);
                client->text("Driving Forwards");
                break;
            case CMD_BACKWARD:
                motorLeft.setSpeed(-128);
                motorRight.setSpeed(-128);
                client->text("Driving Backwards");
                break;
            case CMD_LEFT:
                motorLeft.setSpeed(-128);
                motorRight.setSpeed(128);
                client->text("Turning Left");
                break;
            case CMD_RIGHT:
                motorLeft.setSpeed(128);
                motorRight.setSpeed(-128);
                client->text("Turning Right");
                break;
            case CMD_STOP:
                motorLeft.setSpeed(0);
                motorRight.setSpeed(0);
                client->text("Motors stopped");
                break;
            default:
                client->text("Unknown command");
        }
    }
}

//Setup Motor Endpoints
void setupMotorEndpoints(AsyncWebServer &server) {
    server.on("/motor/drive_forwards", HTTP_GET, [](AsyncWebServerRequest *request) {
        motorLeft.setSpeed(128);
        motorRight.setSpeed(128);
        request->send(200, "Driving Forwards");
    });

    server.on("/motor/drive_backwards", HTTP_GET, [](AsyncWebServerRequest *request) {
        motorLeft.setSpeed(-128);
        motorRight.setSpeed(-128);
        request->send(200, "Driving Backwards");
    });

    server.on("/motor/tank_left", HTTP_GET, [](AsyncWebServerRequest *request) {
        motorLeft.setSpeed(-128);
        motorRight.setSpeed(128);
        request->send(200, "Turning Left");
    });

    server.on("/motor/tank_right", HTTP_GET, [](AsyncWebServerRequest *request) {
        motorLeft.setSpeed(128);
        motorRight.setSpeed(-128);
        request->send(200, "Turning Right");
    });

    server.on("/motor/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
        motorLeft.setSpeed(0);
        motorRight.setSpeed(0);
        request->send(200, "Motors Stopped");
    });
}


// Setup the web server and WebSocket
void setupServer() {
    // WebSocket setup
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    // HTTP endpoints for LED control
    server.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(LED_PIN, HIGH);
        request->send(200, "LED is ON");
    });

    server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(LED_PIN, LOW);
        request->send(200, "LED is OFF");
    });
    
    setupMotorEndpoints(server);
    server.on("/stream", HTTP_GET, handleStreamRequest); // Video stream endpoint

    server.begin();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    connectToWiFi();

    if (!setupCamera()) {
        Serial.println("Camera initialization failed!");
        return;
    }

    setupServer();
}

void loop() {
    ws.cleanupClients(); // Clean up WebSocket clients
}













