#if !( defined(CORE_TEENSY) && defined(__IMXRT1062__) && defined(ARDUINO_TEENSY41) )
  #error Only Teensy 4.1 supported
#endif

// Debug Level from 0 to 4
#define _TEENSY41_ASYNC_TCP_LOGLEVEL_       1
#define _AWS_TEENSY41_LOGLEVEL_             1

#define SHIELD_TYPE     "Teensy4.1 QNEthernet"

#if (_AWS_TEENSY41_LOGLEVEL_ > 3)
  #warning Using QNEthernet lib for Teensy 4.1. Must also use Teensy Packages Patch or error
#endif

#define USING_DHCP            true
//#define USING_DHCP            false

#if !USING_DHCP
  // Set the static IP address to use if the DHCP fails to assign
  IPAddress myIP(192, 168, 2, 222);
  IPAddress myNetmask(255, 255, 255, 0);
  IPAddress myGW(192, 168, 2, 1);
  //IPAddress mydnsServer(192, 168, 2, 1);
  IPAddress mydnsServer(8, 8, 8, 8);
#endif

#include "QNEthernet.h"       // https://github.com/ssilverman/QNEthernet
using namespace qindesign::network;

#include <AsyncFSWebServer_Teensy41.h>


#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <SPI.h>
#include "CytronMotorDriver.h"
#define LED_PIN 13
// Async Web Server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 14, 17);
CytronMD motorRight(PWM_PWM, 10, 11);

// IMU setup
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
unsigned long lastIMUSend = 0; // Timer for IMU data streaming
const unsigned long IMUSendInterval = 100;

void handleRoot(AsyncWebServerRequest *request)
{
    digitalWrite(LED_PIN, 1);
    request->send(200, "text/plain", String("Hello from AsyncWebServer_Teensy41 on ") + BOARD_NAME);
    digitalWrite(LED_PIN, 0);
}

void handleNotFound(AsyncWebServerRequest *request)
{
    digitalWrite(LED_PIN, 1);
    String message = "File Not Found\n\n";

    message += "URI: ";
    // message += server.uri();
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for (uint8_t i = 0; i < request->args(); i++)
    {
        message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }

    request->send(404, "text/plain", message);
    digitalWrite(LED_PIN, 0);
}

void EthernetSetup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, 0);

    Serial.begin(115200);

    while (!Serial)
        ;

    delay(200);

    Serial.print("\nStart Async_HelloServer on ");
    Serial.print(BOARD_NAME);
    Serial.print(" with ");
    Serial.println(SHIELD_TYPE);
    Serial.println(ASYNC_FSWEBSERVER_TEENSY41_VERSION);

    delay(500);

#if USING_DHCP
    // Start the Ethernet connection, using DHCP
    Serial.print("Initialize Ethernet using DHCP => ");
    Ethernet.begin();
#else
    // Start the Ethernet connection, using static IP
    Serial.print("Initialize Ethernet using static IP => ");
    Ethernet.begin(myIP, myNetmask, myGW);
    Ethernet.setDNSServerIP(mydnsServer);
#endif

    if (!Ethernet.waitForLocalIP(5000))
    {
        Serial.println(F("Failed to configure Ethernet"));

        if (!Ethernet.linkStatus())
        {
            Serial.println(F("Ethernet cable is not connected."));
        }

        // Stay here forever
        while (true)
        {
            delay(1);
        }
    }
    else
    {
        Serial.print(F("Connected! IP address:"));
        Serial.println(Ethernet.localIP());
    }

#if USING_DHCP
    delay(1000);
#else
    delay(2000);
#endif
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { handleRoot(request); });

    server.on("/inline", [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "This works as well"); });

    server.onNotFound(handleNotFound);

    server.begin();

    Serial.print(F("HTTP EthernetWebServer is @ IP : "));
    Serial.println(Ethernet.localIP());
}

// Enums for commands
enum Command
{
    CMD_ON,
    CMD_OFF,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_STOP,
    CMD_UNKNOWN
};

Command getCommand(String msg)
{
    int delimiterIndex = msg.indexOf(':');
    String command = delimiterIndex != -1 ? msg.substring(0, delimiterIndex) : msg;

    if (command == "on")
        return CMD_ON;
    if (command == "off")
        return CMD_OFF;
    if (command == "drive_forwards")
        return CMD_FORWARD;
    if (command == "drive_backwards")
        return CMD_BACKWARD;
    if (command == "tank_left")
        return CMD_LEFT;
    if (command == "tank_right")
        return CMD_RIGHT;
    if (command == "motor_stop")
        return CMD_STOP;
    return CMD_UNKNOWN;
}

void handleWebSocketMessage(AsyncWebSocketClient *client, String message)
{
    int delimiterIndex = message.indexOf(':');
    int sliderValue = delimiterIndex != -1 ? message.substring(delimiterIndex + 1).toInt() : 50;
    int speed = map(sliderValue, 0, 100, 0, 255);

    Command cmd = getCommand(message);

    switch (cmd)
    {
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

void sendIMUData()
{
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
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
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

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    EthernetSetup();
    /*
        if (!bno.begin()) {
            Serial.println("BNO055 not detected. Check wiring or I2C address!");
            while (1);
            ;
        }
        bno.setExtCrystalUse(true);
        */
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
    server.begin();

    Serial.println("WebSocket server started.");
}

void loop()
{
    // Periodically send IMU data
    if (millis() - lastIMUSend > IMUSendInterval)
    {
        sendIMUData();
        lastIMUSend = millis();
    }
}
