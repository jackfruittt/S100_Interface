/*
import React, { useState, useEffect, useContext, createContext } from "react";
import ReactSwitch from "react-switch";
import IMU3DShape from "./components/IMU3DShape";
import ControlPanel from "./components/ControlPanel";
import IMUData from "./components/IMUData";
import useROS from "./components/rosConnector";
import "./App.css";

export const ThemeContext = createContext(null);

const TEENSY_IP = "http://192.168.2.177"; // Replace with your Teensy's IP

//Debounce to limit frequent calls
const debounce = (func, delay) => {
  let timeout;
  return (...args) => {
    clearTimeout(timeout);
    timeout = setTimeout(() => func(...args), delay);
  };
};

function App() {
  const [theme, setTheme] = useState("light");
  const [statusMessage, setStatusMessage] = useState("");
  const [imuData, setImuData] = useState({ roll: 0, pitch: 0, yaw: 0 });

  const { rosStatusMessage, sendRosMessage, listenToRosTopic } = useROS();

  const toggleTheme = () => {
    setTheme((curr) => (curr === "light" ? "dark" : "light"));
  };// Create a new container component for ROS-connect components
  const RosContainer = () => {
    const { rosStatusMessage, sendRosMessage, listenToRosTopic } = useROS();
  
    return (
      <div className="ros-container">
        <p>Ros Status: {rosStatusMessage}</p>
        <button onClick={sendRosMessage}>Send ROS Message</button>
        <button onClick={listenToRosTopic}>Listen to ROS Topic</button>
      </div>
    );
  };

  // Function to send HTTP requests to Teensy
  const sendHttpRequest = async (path, method = "GET", signal = null) => {
    try {
      const options = { method };
      if (signal) options.signal = signal;
  
      const response = await fetch(`${TEENSY_IP}${path}`, options);
      if (!response.ok) throw new Error(`HTTP error! Status: ${response.status}`);
      
      return await response.json();
    } catch (error) {
      console.error("HTTP Request Error:", error);
      return null;
    }
  };
  
  const fetchIMUData = async () => {
    try {
      const data = await sendHttpRequest("/imu");
      if (data) setImuData(data);
    } catch (error) {
      console.error("Fetch IMU error:", error);
    }
  };

  useEffect(() => {
    const intervalId = setInterval(() => {
      fetchIMUData(); // Periodically fetch IMU data
    }, 100); // Every 2 seconds

    // Cleanup on unmount
    return () => clearInterval(intervalId);
  }, []);
   
  // Ping message to keep the connection alive
  const sendPing = async () => {
    await sendHttpRequest("/ping"); // Send a ping request to the Teensy
  };

  useEffect(() => {
    const pingInterval = setInterval(() => {
      sendPing(); 
    }, 5000); // Every 10 seconds

    return () => clearInterval(pingInterval);
  }, []);

  // Command Handlers
  const handleCommand = async (command) => {
    const response = await sendHttpRequest(command);
    if (response) {
      setStatusMessage(`Command executed: ${command}`);
    }
  };

  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      <div className="App" id={theme}>
        <div className="switch-container">
          <label className="theme-label">
            {theme === "dark" ? "Dark Mode" : "Light Mode"}
          </label>
          <ReactSwitch
            className="theme-switch"
            onChange={toggleTheme}
            checked={theme === "dark"}
          />
        </div>
        <ControlPanel
          onCommand={handleCommand}
          onSetSpeed={(speed) => handleCommand(`/set_speed?value=${speed}`)}
        />
        <div className="imu-container">
          <IMUData imuData={imuData} />
          <IMU3DShape
            roll={imuData.roll}
            pitch={imuData.pitch}
            yaw={imuData.yaw}
          />
        </div>
        <p className="status-message">{statusMessage}</p>
        <div className="ros-connection">
        <RosContainer />
        </div>
      </div>
    </ThemeContext.Provider>
  );
}

export default App;
*/
import React, { useState, useEffect, useContext, createContext } from "react";
import ReactSwitch from "react-switch";
import IMU3DShape from "./components/IMU3DShape";
import ControlPanel from "./components/ControlPanel";
import IMUData from "./components/IMUData";
import useMQTT from "./components/useMQTT"; // Use MQTT instead of HTTP
import "./App.css";

export const ThemeContext = createContext(null);

function App() {
  const [theme, setTheme] = useState("light");
  const [statusMessage, setStatusMessage] = useState("");

  // **Use MQTT for IMU data & command sending**
  const { imuData, sendCommand } = useMQTT();

  const toggleTheme = () => {
    setTheme((curr) => (curr === "light" ? "dark" : "light"));
  };

  // **Command Handler now uses MQTT**
  const handleCommand = (command) => {
    sendCommand(command);
    setStatusMessage(`Sent command: ${command}`);
  };

  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      <div className="App" id={theme}>
        <div className="switch-container">
          <label className="theme-label">
            {theme === "dark" ? "Dark Mode" : "Light Mode"}
          </label>
          <ReactSwitch
            className="theme-switch"
            onChange={toggleTheme}
            checked={theme === "dark"}
          />
        </div>

        <ControlPanel
          onCommand={handleCommand}
          onSetSpeed={(speed) => handleCommand(`drive_forwards?speed=${speed}`)}
        />

        <div className="imu-container">
          <IMUData imuData={imuData} />
          <IMU3DShape
            roll={imuData.roll}
            pitch={imuData.pitch}
            yaw={imuData.yaw}
          />
        </div>

        <p className="status-message">{statusMessage}</p>
      </div>
    </ThemeContext.Provider>
  );
}

export default App;

