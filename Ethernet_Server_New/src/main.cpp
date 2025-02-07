/*
#include <NativeEthernet.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include "CytronMotorDriver.h"
#include <IntervalTimer.h>

// PIN DEFINES
#define LED_PIN 13

// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 2, 3);
CytronMD motorRight(PWM_PWM, 4, 5);

// IMU setup
Adafruit_BNO055 bno = Adafruit_BNO055(55);
//unsigned long lastIMUSend = 0;
//const unsigned long IMUSendInterval = 10; // Interval for IMU data sending

// Ethernet Configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 177);
EthernetServer server(80); // HTTP server on port 80

// Command Enumeration
enum Command {
    CMD_ON, CMD_OFF, CMD_FORWARD, CMD_BACKWARD,
    CMD_LEFT, CMD_RIGHT, CMD_STOP, CMD_SET_SPEED, CMD_UNKNOWN
};

// Function to parse commands from HTTP requests
Command getCommand(String msg) {
    if (msg == "/on") return CMD_ON;
    if (msg == "/off") return CMD_OFF;
    if (msg.startsWith("/drive_forwards")) return CMD_FORWARD;  
    if (msg.startsWith("/drive_backwards")) return CMD_BACKWARD;  
    if (msg.startsWith("/tank_left")) return CMD_LEFT;  
    if (msg.startsWith("/tank_right")) return CMD_RIGHT;  
    if (msg == "/motor_stop") return CMD_STOP;
    return CMD_UNKNOWN;
}

// Function to extract speed from the request string using delimiters
int extractSpeed(String msg) {
    int sliderValue = 150;  // Default speed value if not found

    int idx = msg.indexOf("speed=");
    if (idx != -1) {
        sliderValue = msg.substring(idx + 6).toInt();
        //Serial.print("Extracted slider value: ");
        //Serial.println(sliderValue);  // See the value extracted from the request

        // Interpolate the slider value from 0-100 to 0-255
        int speed = map(sliderValue, 0, 100, 0, 255); 
        //Serial.print("Interpolated speed: ");
        //Serial.println(speed);  // See the interpolated value
        return constrain(speed, 0, 255);  // Ensure the speed is within bounds
    }
    return 150;  // Default speed if "speed=" is not in the request
}

void processCommand(Command cmd, String request) {
    int speed = extractSpeed(request); 
    //Serial.print("Extracted speed: ");
    //Serial.println(speed); 

    switch (cmd) {
        case CMD_ON:
            digitalWrite(LED_PIN, HIGH);
            break;
        case CMD_OFF:
            digitalWrite(LED_PIN, LOW);
            break;
        case CMD_FORWARD:
            motorLeft.setSpeed(speed);
            motorRight.setSpeed(speed);
            //Serial.print("Setting motor speed: ");
            //Serial.println(speed);
            break;
        case CMD_BACKWARD:
            motorLeft.setSpeed(-speed);
            motorRight.setSpeed(-speed);
            //Serial.print("Setting motor speed: ");
            //Serial.println(speed);
            break;
        case CMD_LEFT:
            motorLeft.setSpeed(speed);
            motorRight.setSpeed(-speed);
            //Serial.print("Setting motor speed: ");
            //Serial.println(speed);
            break;
        case CMD_RIGHT:
            motorLeft.setSpeed(-speed);
            motorRight.setSpeed(speed);
            //Serial.print("Setting motor speed: ");
            //Serial.println(speed);
            break;
        case CMD_STOP:
            motorLeft.setSpeed(0);
            motorRight.setSpeed(0);
            //Serial.print("Setting motor speed: ");
            //Serial.println(speed);
            break;
        default:
            break;
    }
}

// Function to calculate roll and pitch
void calculateRP(double &roll, double &pitch) {
    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    double ax = accel.x(), ay = accel.y(), az = accel.z();
    roll = atan2(ay, sqrt(ax * ax + az * az)) * (180.0 / M_PI);
    pitch = atan2(-ax, sqrt(ay * ay + az * az)) * (180.0 / M_PI);
}

// Function to create IMU JSON string
String createIMUJSON() {
    double roll, pitch;
    calculateRP(roll, pitch);
    return "{\"roll\":" + String(roll, 2) +
           ",\"pitch\":" + String(pitch, 2) +
           ",\"yaw\":0}";
}

// Function to handle HTTP requests
void handleRequest(EthernetClient &client, String request) {
   // Serial.println("Received request: " + request);  // Log the request to the Serial Monitor

    Command cmd = getCommand(request);  // Get the command from the request path

    if (cmd != CMD_UNKNOWN) {
        processCommand(cmd, request);  // Process the command and handle the speed logic
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println("Access-Control-Allow-Origin: *");  // Allow all origins (CORS header)
        client.println("Connection: keep-alive");
        client.println();
        client.println("Command executed: " + request);  // Send a response message
    } else if (request.startsWith("/imu")) {
        String imuData = createIMUJSON();  // Get IMU data as JSON
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Access-Control-Allow-Origin: *"); 
        client.println("Connection: keep-alive");
        client.println();
        client.println(imuData);  // Send the IMU data
    } else if (request.startsWith("/ping")) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Access-Control-Allow-Origin: *");
    client.println();
    client.println("Pong");
    } else {
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/plain");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Connection: keep-alive"); 
        client.println();
        client.println("Unknown command");
    }
}

bool linkStatus()
{
    auto link_status = Ethernet.linkStatus();
    if (link_status == LinkON)
        return 1;
    else
        return 0;
}


// Main setup function
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // Initialize IMU
    if (!bno.begin()) {
        Serial.println("IMU not detected!");
    }
    delay(1000);
    bno.setExtCrystalUse(true);

    // Start Ethernet
    Ethernet.begin(mac, ip);
    delay(1000);
    linkStatus();
    delay(2000);
    //Serial.print("Server IP: ");
    //Serial.println(Ethernet.localIP());

    // Start HTTP server
    server.begin();
}

void loop() {
    EthernetClient client = server.available();
    if (client) {
        String request = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                request += c;
                if (c == '\n' && request.endsWith("\r\n\r\n")) {
                    //Serial.println("Received request: " + request);
                    handleRequest(client, request.substring(4, request.indexOf(' ', 4))); // Extract path
                    break;
                }
            }
        }
        if (request.startsWith("/imu")) {
            Serial.println("Client requested IMU data stream");
            while (client.connected()) {
                String imuData = createIMUJSON(); 
                client.println(imuData);
                delay(50); // Adjust update rate
            }
        }
        
        client.stop();
        String pseudoData = "{\"status\":\"waiting\"}";  // A simple placeholder message
        client.println(pseudoData);
        client.flush();
        //delay(50);
    }
}
*/

#include <NativeEthernet.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include "CytronMotorDriver.h"

// **MQTT Configuration**
#define MQTT_SERVER "192.168.2.1"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "TeensyClient"
#define MQTT_TOPIC_CMD "teensy/cmd"
#define MQTT_TOPIC_IMU "teensy/imu"

// **Ethernet Configuration**
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 2, 177);

// **MQTT and Ethernet Client**
EthernetClient ethClient;
PubSubClient client(ethClient);

// **Motor Setup**
CytronMD motorLeft(PWM_PWM, 2, 3);
CytronMD motorRight(PWM_PWM, 4, 5);
int motorSpeed = 150;

// **IMU Setup**
Adafruit_BNO055 bno = Adafruit_BNO055(55);

// **Command Enumeration**
enum Command {
    CMD_ON, CMD_OFF, CMD_FORWARD, CMD_BACKWARD, CMD_LEFT, CMD_RIGHT,
    CMD_STOP, CMD_UNKNOWN
};

// **Extract speed from the message (e.g., "drive_forwards?speed=80")**
int extractSpeed(const String &msg) {
    int defaultSpeed = 150;
    int idx = msg.indexOf("speed=");
    if (idx != -1) {
        int extractedSpeed = msg.substring(idx + 6).toInt();
        return constrain(map(extractedSpeed, 0, 100, 0, 255), 0, 255);
    }
    return defaultSpeed;
}

// **Function to parse commands from MQTT messages**
Command parseCommand(const String &msg) {
    if (msg == "on") return CMD_ON;
    if (msg == "off") return CMD_OFF;
    if (msg.startsWith("drive_forwards")) return CMD_FORWARD;
    if (msg.startsWith("drive_backwards")) return CMD_BACKWARD;
    if (msg.startsWith("tank_left")) return CMD_LEFT;
    if (msg.startsWith("tank_right")) return CMD_RIGHT;
    if (msg == "motor_stop") return CMD_STOP;
    return CMD_UNKNOWN;
}

// **Function to process commands using switch**
void processCommand(const String &message) {
    Command cmd = parseCommand(message);
    int speed = extractSpeed(message);

    switch (cmd) {
        case CMD_ON:
            digitalWrite(LED_BUILTIN, HIGH);
            break;

        case CMD_OFF:
            digitalWrite(LED_BUILTIN, LOW);
            break;

        case CMD_FORWARD:
            motorLeft.setSpeed(speed);
            motorRight.setSpeed(speed);
            break;

        case CMD_BACKWARD:
            motorLeft.setSpeed(-speed);
            motorRight.setSpeed(-speed);
            break;

        case CMD_LEFT:
            motorLeft.setSpeed(-speed);
            motorRight.setSpeed(speed);
            break;

        case CMD_RIGHT:
            motorLeft.setSpeed(speed);
            motorRight.setSpeed(-speed);
            break;

        case CMD_STOP:
            motorLeft.setSpeed(0);
            motorRight.setSpeed(0);
            break;

        case CMD_UNKNOWN:
        default:
            Serial.println("Unknown command received: " + message);
            break;
    }
}

// **MQTT Callback function**
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    processCommand(message);
}

// **Reconnect to MQTT Broker**
void reconnectMQTT() {
    while (!client.connected()) {
        if (client.connect(MQTT_CLIENT_ID)) {
            client.subscribe(MQTT_TOPIC_CMD);
        } else {
            delay(5000);
        }
    }
}

// **Function to create IMU JSON string**
String createIMUJSON() {
    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    double roll = atan2(accel.y(), sqrt(accel.x() * accel.x() + accel.z() * accel.z())) * (180.0 / M_PI);
    double pitch = atan2(-accel.x(), sqrt(accel.y() * accel.y() + accel.z() * accel.z())) * (180.0 / M_PI);
    
    return "{\"roll\":" + String(roll, 2) +
           ",\"pitch\":" + String(pitch, 2) +
           ",\"yaw\":0}";
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    if (!bno.begin()) {
        Serial.println("IMU not detected!");
    }
    delay(1000);
    bno.setExtCrystalUse(true);

    Ethernet.begin(mac, ip);
    delay(1000);

    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
}

void loop() {
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();

    // **Publish IMU data every 100ms**
    static unsigned long lastIMUSend = 0;
    if (millis() - lastIMUSend > 100) {
        lastIMUSend = millis();
        String imuData = createIMUJSON();
        client.publish(MQTT_TOPIC_IMU, imuData.c_str());
    }
}





