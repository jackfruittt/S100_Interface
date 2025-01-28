import { useState, useEffect } from "react";
import ROSLIB from "roslib";

const ROSBRIDGE_IP = "ws://localhost:9090"; // Use whatver configured with ROSBridge IP address (default 9090)

const useROS = (onMessageCallback) => {
  const [ros, setRos] = useState(null);
  const [rosStatusMessage, setRosStatusMessage] = useState("Disconnected from ROS");
  const [rosConnected, setRosConnected] = useState(false);

  useEffect(() => {
    const rosInstance = new ROSLIB.Ros({
      url: ROSBRIDGE_IP,
    });

    rosInstance.on("connection", () => {
      setRosConnected(true);
      setRosStatusMessage("Connected to ROS via ROSBridge");
    });

    rosInstance.on("error", (error) => {
      setRosConnected(false);
      setRosStatusMessage(`Error connecting to ROS: ${error.message}`);
    });

    rosInstance.on("close", () => {
      setRosConnected(false);
      setRosStatusMessage("Connection to ROS closed");
      setTimeout(() => rosInstance.connect(ROSBRIDGE_IP), 5000); // Retry connection
    });
    
    setRos(rosInstance);

    return () => {
      if (rosInstance) rosInstance.close(); // Cleanup ROS instance
    };
  }, []);

  const sendRosMessage = () => {
    if (ros && rosConnected) {
      const topic = new ROSLIB.Topic({
        ros: ros,
        name: "/react_to_ros",
        messageType: "std_msgs/String",
      });
  
      topic.advertise();
  
      const message = new ROSLIB.Message({
        data: "Hello from React",
      });
  
      topic.publish(message);
      setRosStatusMessage("Message sent to ROS: Hello from React");
    } else {
      setRosStatusMessage("Unable to send message. Not connected to ROS.");
    }
  };

  // Function to listen for ROS messages and call the callback
  const listenToRosTopic = () => {
    if (ros && rosConnected) {
      const listener = new ROSLIB.Topic({
        ros: ros,
        name: "/ros_to_react",
        messageType: "std_msgs/String",
      });

      listener.subscribe((message) => {
        console.log(`Received message from ROS: ${message.data}`);
        if (onMessageCallback) {
          onMessageCallback(message.data); // Relay message to WebSocket handler
        }
      });
    } else {
      setRosStatusMessage("Unable to listen. Not connected to ROS.");
    }
  };

  const publishIMUData = (imuData) => {
    if (ros && rosConnected) {
      const topic = new ROSLIB.Topic({
        ros: ros,
        name: "/react_imu",
        messageType: "geometry_msgs/Vector3",
      });
  
      topic.advertise();
  
      const message = new ROSLIB.Message({
        x: imuData.roll,
        y: imuData.pitch,
        z: imuData.yaw,
      });
  
      topic.publish(message);
      setRosStatusMessage(
        `IMU data published to ROS: Roll=${imuData.roll}, Pitch=${imuData.pitch}, Yaw=${imuData.yaw}`
      );
    } else {
      setRosStatusMessage("Unable to publish IMU data. Not connected to ROS.");
    }
  };  

  return {
    rosStatusMessage,
    rosConnected,
    listenToRosTopic,
    sendRosMessage,
    publishIMUData, 
  };
  
};

export default useROS;



