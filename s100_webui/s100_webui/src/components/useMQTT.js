import { useEffect, useState } from "react";
import mqtt from "mqtt";

const MQTT_BROKER = "ws://192.168.2.100:9001"; // Change to your MQTT broker's WebSocket URL
const CMD_TOPIC = "teensy/cmd";
const IMU_TOPIC = "teensy/imu";

const useMQTT = () => {
  const [client, setClient] = useState(null);
  const [imuData, setImuData] = useState({ roll: 0, pitch: 0, yaw: 0 });

  useEffect(() => {
    const mqttClient = mqtt.connect(MQTT_BROKER);

    mqttClient.on("connect", () => {
      console.log("Connected to MQTT Broker");
      mqttClient.subscribe(IMU_TOPIC);
    });

    mqttClient.on("message", (topic, message) => {
      if (topic === IMU_TOPIC) {
        try {
          const imu = JSON.parse(message.toString());
          setImuData(imu);
        } catch (error) {
          console.error("Error parsing IMU data:", error);
        }
      }
    });

    mqttClient.on("error", (error) => {
      console.error("MQTT Connection Error:", error);
    });

    setClient(mqttClient);

    return () => {
      mqttClient.end();
    };
  }, []);

  const sendCommand = (command) => {
    if (client) {
      client.publish(CMD_TOPIC, command);
      console.log("Sent MQTT Command:", command);
    }
  };

  return { imuData, sendCommand };
};

export default useMQTT;
