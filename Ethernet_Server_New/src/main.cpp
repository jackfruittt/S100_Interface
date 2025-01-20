#include <NativeEthernet.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include "CytronMotorDriver.h"

// PIN DEFINES
#define LED_PIN 13

// Motor PINOUT
CytronMD motorLeft(PWM_PWM, 2, 3);
CytronMD motorRight(PWM_PWM, 4, 5);

// IMU setup
Adafruit_BNO055 bno = Adafruit_BNO055(55);
unsigned long lastIMUSend = 0;
const unsigned long IMUSendInterval = 150; // Interval for IMU data sending

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
        client.println();
        client.println("Command executed: " + request);  // Send a response message
    } else if (request.startsWith("/imu")) {
        String imuData = createIMUJSON();  // Get IMU data as JSON
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Access-Control-Allow-Origin: *"); 
        client.println();
        client.println(imuData);  // Send the IMU data
    } else {
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/plain");
        client.println("Access-Control-Allow-Origin: *"); 
        client.println();
        client.println("Unknown command");
    }
}

// Main setup function
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // Initialize IMU
    if (!bno.begin()) {
        Serial.println("IMU not detected!");
    }

    // Start Ethernet
    Ethernet.begin(mac, ip);
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
        delay(1);
        client.stop();
        client.flush();
    }

    // Periodically send IMU data (non-blocking)
    if (millis() - lastIMUSend >= IMUSendInterval) {
        lastIMUSend = millis();
        //Serial.println("Sending IMU data...");
        
    }
}




