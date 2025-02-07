import { useEffect, useState } from "react";
import mqtt from "mqtt";

const MQTT_BROKER = "ws://localhost:9001/mqtt";
const CMD_TOPIC = "teensy/cmd";
const IMU_TOPIC = "teensy/imu";

const useMQTT = () => {
  const [client, setClient] = useState(null);
  const [isConnected, setIsConnected] = useState(false);
  const [imuData, setImuData] = useState({ roll: 0, pitch: 0, yaw: 0 });

  useEffect(() => {
    console.log("🔄 Attempting to connect to MQTT broker...");
    
    const mqttClient = mqtt.connect(MQTT_BROKER, {
      protocol: "ws",
      keepalive: 0,             // Increase keepalive interval to 120 seconds
      reconnectPeriod: 2000,
      connectTimeout: 5000,
      debug: true,
      clientId: `webui_${Math.random().toString(16).slice(2, 8)}`,
      clean: true,               // Start fresh session each time
      resubscribe: true,         // Resubscribe to topics after reconnection
      will: {                    // Last Will and Testament
        topic: 'client/status',
        payload: 'offline',
        qos: 1,
        retain: false
      }
    });

    // Keep connection alive by publishing a heartbeat
    let heartbeatInterval;

    mqttClient.on("connect", () => {
      console.log("✅ Connected to MQTT Broker at", MQTT_BROKER);
      setIsConnected(true);
      
      // Set up heartbeat every 20 seconds
      heartbeatInterval = setInterval(() => {
        if (mqttClient.connected) {
          mqttClient.publish('client/heartbeat', 'ping', { qos: 0, retain: false });
        }
      }, 20000);

      mqttClient.subscribe([IMU_TOPIC, 'client/heartbeat'], (err) => {
        if (err) {
          console.error("❌ Subscription error:", err);
        } else {
          console.log("✅ Subscribed to topics");
        }
      });
    });

    mqttClient.on("packetsend", (packet) => {
      if (packet.cmd === "pingreq") {
        console.log("📡 Sent PINGREQ to keep connection alive");
      }
    });

    mqttClient.on("reconnect", () => {
      console.log("🔄 Attempting to reconnect to MQTT broker...");
    });

    mqttClient.on("offline", () => {
      console.log("📴 MQTT client is offline");
      setIsConnected(false);
    });

    mqttClient.on("message", (topic, message) => {
      if (topic === IMU_TOPIC) {
        try {
          const imu = JSON.parse(message.toString());
          setImuData(imu);
        } catch (error) {
          console.error("❌ Error parsing IMU data:", error);
        }
      }
    });

    mqttClient.on("error", (error) => {
      console.error("❌ MQTT Connection Error:", error);
      setIsConnected(false);
    });

    mqttClient.on("close", () => {
      console.warn("⚠️ Disconnected from MQTT Broker");
      setIsConnected(false);
    });

    setClient(mqttClient);

    return () => {
      if (heartbeatInterval) {
        clearInterval(heartbeatInterval);
      }
      mqttClient.end();
    };
  }, []);

  const sendCommand = (command) => {
    if (client && isConnected) {
      client.publish(CMD_TOPIC, command, { qos: 1 }, (err) => {
        if (err) {
          console.error("❌ Error sending command:", err);
        } else {
          console.log("📡 Sent command:", command);
        }
      });
    } else {
      console.warn("⚠️ Cannot send command, MQTT is not connected");
    }
  };

  return { imuData, sendCommand, isConnected };
};

export default useMQTT;

