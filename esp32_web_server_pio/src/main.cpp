#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "WiFi.h"
#include "CytronMotorDriver.h"

#define LED_PIN 2


// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 14, 17);   // PWM 1A = Pin 3, PWM 1B = Pin 9.
CytronMD motorRight(PWM_PWM, 10, 11); // PWM 2A = Pin 10, PWM 2B = Pin 11.

// Wi-Fi credentials
const char* ssid = "Jackfruittt";
const char* password = "pewpewpew";

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

// WebSocket event handler
unsigned long lastCommandTime = 0; // Track the last command timestamp
const unsigned long commandCooldown = 100; // Cooldown period in milliseconds

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        String msg = String((char *)data);
        unsigned long currentTime = millis();

        // Ignore commands received within the cooldown period
        if (currentTime - lastCommandTime < commandCooldown) {
            return;
        }
        lastCommandTime = currentTime;

        Serial.println("Received WebSocket command: " + msg); // Debug received command
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
                motorLeft.setSpeed(0);
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

    server.begin();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    connectToWiFi();

    setupServer();
}

void loop() {
    ws.cleanupClients(); // Clean up WebSocket clients
}



/*
#if !( defined(ESP32) )
#error This code is designed for (ESP32_S2/3, ESP32_C3 + W5500) to run on ESP32 platform! Please check your Tools->Board setting.
#endif

#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_       3



// Optional values to override default settings
// Don't change unless you know what you're doing
//#define ETH_SPI_HOST        SPI3_HOST
//#define SPI_CLOCK_MHZ       25

// Must connect INT to GPIOxx or not working



#define INT_GPIO            16

#define MISO_GPIO           21
#define MOSI_GPIO           14
#define SCK_GPIO            47
#define CS_GPIO             38


//////////////////////////////////////////////////////////

#include <WebServer_ESP32_SC_W5500.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 };

void setup() {

Serial.begin(115200);

  // To be called before ETH.begin()
  ESP32_W5500_onEvent();

  ETH.begin(MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);

  ESP32_W5500_waitForConnect();

  String ethernet_dhcp_ip = ETH.localIP().toString();
  Serial.print(F("HTTP EthernetWebServer is @ IP : "));
  Serial.println(ethernet_dhcp_ip);

}

void loop() {

}
*/


/*******************************************************************************
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTY AND SUPPORT
 * IS APPLICABLE TO THIS SOFTWARE IN ANY FORM. CYTRON TECHNOLOGIES SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR CONSEQUENTIAL
 * DAMAGES, FOR ANY REASON WHATSOEVER.
 ********************************************************************************
 * DESCRIPTION:
 *
 * This example shows how to drive 2 motors using 4 PWM pins (2 for each motor)
 * with 2-channel motor driver.
 * 
 * 
 * CONNECTIONS:
 * 
 * Arduino D3  - Motor Driver PWM 1A Input
 * Arduino D9  - Motor Driver PWM 1B Input
 * Arduino D10 - Motor Driver PWM 2A Input
 * Arduino D11 - Motor Driver PWM 2B Input
 * Arduino GND - Motor Driver GND
 *
 *
 * AUTHOR   : Kong Wai Weng
 * COMPANY  : Cytron Technologies Sdn Bhd
 * WEBSITE  : www.cytron.io
 * EMAIL    : support@cytron.io
 *
 *******************************************************************************/
/*
 #include "CytronMotorDriver.h"


// Configure the motor driver.
CytronMD motor1(PWM_PWM, 14, 17);   // PWM 1A = Pin 3, PWM 1B = Pin 9.
CytronMD motor2(PWM_PWM, 10, 11); // PWM 2A = Pin 10, PWM 2B = Pin 11.


// The setup routine runs once when you press reset.
void setup() {
  
}


// The loop routine runs over and over again forever.
void loop() {
  motor1.setSpeed(128);   // Motor 1 runs forward at 50% speed.
  motor2.setSpeed(-128);  // Motor 2 runs backward at 50% speed.
  delay(1000);
  
  motor1.setSpeed(255);   // Motor 1 runs forward at full speed.
  motor2.setSpeed(-255);  // Motor 2 runs backward at full speed.
  delay(1000);

  motor1.setSpeed(0);     // Motor 1 stops.
  motor2.setSpeed(0);     // Motor 2 stops.
  delay(1000);

  motor1.setSpeed(-128);  // Motor 1 runs backward at 50% speed.
  motor2.setSpeed(128);   // Motor 2 runs forward at 50% speed.
  delay(1000);
  
  motor1.setSpeed(-255);  // Motor 1 runs backward at full speed.
  motor2.setSpeed(255);   // Motor 2 runs forward at full speed.
  delay(1000);

  motor1.setSpeed(0);     // Motor 1 stops.
  motor2.setSpeed(0);     // Motor 2 stops.
  delay(1000);
}



*/







