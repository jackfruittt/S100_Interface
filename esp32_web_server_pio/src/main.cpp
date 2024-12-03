#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "WiFi.h"
#include "CytronMotorDriver.h"
#include "Wire.h"
#include <SPI.h>
#include "utility/imumaths.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BNO055.h"

// PIN DEFINES
#define LED_PIN 2
#define SCL 20
#define SDA 21

// Wi-Fi credentials
const char* ssid = "Jackfruittt";
const char* password = "pewpewpew";

// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 14, 17);  // PWM 1A = Pin 14, PWM 1B = Pin 17.
CytronMD motorRight(PWM_PWM, 10, 11); // PWM 2A = Pin 10, PWM 2B = Pin 11.

// WebSocket and server setup
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket endpoint


// IMU setup
Adafruit_BNO055 bno = Adafruit_BNO055(55);
unsigned long lastIMUSend = 0; // Timer for IMU data streaming
const unsigned long IMUSendInterval = 250; 

void calculateRP(double &roll, double &pitch) {
    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER); // Raw accel data

    double ax = accel.x();
    double ay = accel.y();
    double az = accel.z();

    // Calculate roll and pitch
    roll = atan2(ay, sqrt(ax * ax + az * az)) * (180.0 / M_PI); // Convert to degrees
    pitch = atan2(-ax, sqrt(ay * ay + az * az)) * (180.0 / M_PI); // Convert to degrees
}

// Enums for commands
enum Command { CMD_ON, CMD_OFF, CMD_FORWARD, CMD_BACKWARD, CMD_LEFT, CMD_RIGHT, CMD_STOP, CMD_UNKNOWN };

Command getCommand(String msg) {
    int delimiterIndex = msg.indexOf(':');
    String command = delimiterIndex != -1 ? msg.substring(0, delimiterIndex) : msg;

    if (command == "on") return CMD_ON;
    if (command == "off") return CMD_OFF;
    if (command == "drive_forwards") return CMD_FORWARD;
    if (command == "drive_backwards") return CMD_BACKWARD;
    if (command == "tank_left") return CMD_LEFT;
    if (command == "tank_right") return CMD_RIGHT;
    if (command == "motor_stop") return CMD_STOP;
    return CMD_UNKNOWN;
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP address: " + WiFi.localIP().toString());
}

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        String msg = String((char *)data);
        int delimiterIndex = msg.indexOf(':');
        int sliderValue = delimiterIndex != -1 ? msg.substring(delimiterIndex + 1).toInt() : 50;
        int speed = map(sliderValue, 0, 100, 0, 255);

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
                motorLeft.setSpeed(speed);
                motorRight.setSpeed(speed);
                client->text("Driving forwards at speed: " + String(speed));
                break;
            case CMD_BACKWARD:
                motorLeft.setSpeed(-speed);
                motorRight.setSpeed(-speed);
                client->text("Driving backwards at speed: " + String(speed));
                break;
            case CMD_LEFT:
                motorLeft.setSpeed(-speed);
                motorRight.setSpeed(speed);
                client->text("Turning left at speed: " + String(speed));
                break;
            case CMD_RIGHT:
                motorLeft.setSpeed(speed);
                motorRight.setSpeed(-speed);
                client->text("Turning right at speed: " + String(speed));
                break;
            case CMD_STOP:
                motorLeft.setSpeed(0);
                motorRight.setSpeed(0);
                client->text("Motors stopped");
                break;
            default:
                client->text("Unknown command: " + msg);
        }
    }
}

void sendIMUData() {
    double roll, pitch;

    calculateRP(roll, pitch);

    // Create JSON string for IMU data
    String imuData = "{\"roll\":" + String(roll, 2) +
                     ",\"pitch\":" + String(pitch, 2) +
                     ",\"yaw\":null}"; // No yaw if using just accelerometer

    ws.textAll(imuData);
}

void setupServer() {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
    server.begin();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    Wire.begin(SDA, SCL);
    
    if (!bno.begin(adafruit_bno055_opmode_t::OPERATION_MODE_ACCONLY)) {
        Serial.println("BNO055 not detected. Check wiring or I2C address!");
        while (1);
    }
    bno.setExtCrystalUse(true);

    connectToWiFi();
    setupServer();
}

void loop() {
    ws.cleanupClients();

    // Send IMU data periodically
    if (millis() - lastIMUSend > IMUSendInterval) {
        sendIMUData();
        lastIMUSend = millis();
    }
}







