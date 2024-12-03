#include <QNEthernet.h>
using namespace qindesign::network;
#include <Teensy41_AsyncTCP.h>
#include <AsyncFSWebServer_Teensy41.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <SPI.h>
#include "CytronMotorDriver.h"

// LED PIN
#define LED_PIN 2

// Ethernet Configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
IPAddress ip(192, 10, 0, 50);                        
IPAddress gateway(192, 10, 0, 1);                    
IPAddress subnet(255, 255, 255, 0);                  

// Async Web Server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); 

// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 14, 17);   
CytronMD motorRight(PWM_PWM, 10, 11);  

// IMU setup
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
unsigned long lastIMUSend = 0;         // Timer for IMU data streaming
const unsigned long IMUSendInterval = 100; 

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

void EthernetSetup() {
    Serial.println("Initializing Ethernet...");

    // Monitor link state changes
    Ethernet.onLinkState([](bool state) {
        if (state) {
            Serial.println("Ethernet link is UP");
        } else {
            Serial.println("Ethernet link is DOWN");
        }
    });

    // Monitor IP address changes
    Ethernet.onAddressChanged([]() {
        IPAddress address = Ethernet.localIP();
        Serial.print("Ethernet IP Address: ");
        Serial.println(address);
    });

    // Attempt to start Ethernet with DHCP
    if (!Ethernet.begin()) {
        Serial.println("Failed to configure Ethernet using DHCP. Falling back to static IP.");
        Ethernet.begin(ip, subnet, gateway); // Static IP configuration
    }

    Serial.println("Ethernet initialization complete. Waiting for link...");
}

void handleWebSocketMessage(AsyncWebSocketClient *client, String message) {
    int delimiterIndex = message.indexOf(':');
    int sliderValue = delimiterIndex != -1 ? message.substring(delimiterIndex + 1).toInt() : 50;
    int speed = map(sliderValue, 0, 100, 0, 255);

    Command cmd = getCommand(message);

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
            client->text("Unknown command: " + message);
            break;
    }
}

void sendIMUData() {
    sensors_event_t event;
    bno.getEvent(&event);

    double roll = event.orientation.x; 
    double pitch = event.orientation.y; 
    double yaw = event.orientation.z;   

    // Create a JSON string for IMU data
    String imuData = "{\"roll\":" + String(roll, 2) + ",\"pitch\":" + String(pitch, 2) + ",\"yaw\":" + String(yaw, 2) + "}";

    ws.textAll(imuData);
}

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_DATA:
            handleWebSocketMessage(client, String((char *)data));
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("Client #%u disconnected\n", client->id());
            break;
        case WS_EVT_CONNECT:
            Serial.printf("Client #%u connected\n", client->id());
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    EthernetSetup();

    if (!bno.begin()) {
        Serial.println("BNO055 not detected. Check wiring or I2C address!");
        while (1);
        ;
    }
    bno.setExtCrystalUse(true);

    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
    server.begin();

    Serial.println("WebSocket server started.");
}

void loop() {
    // Periodically send IMU data
    if (millis() - lastIMUSend > IMUSendInterval) {
        sendIMUData();
        lastIMUSend = millis();
    }
}



