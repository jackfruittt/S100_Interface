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
  const [imuData, setImuData,] = useState({ roll: 0, pitch: 0, yaw: 0 });

  const { rosStatusMessage, sendRosMessage, listenToRosTopic, publishIMUData } = useROS();

  const toggleTheme = () => {
    setTheme((curr) => (curr === "light" ? "dark" : "light"));
  };

  // Function to send HTTP requests to Teensy
  const sendHttpRequest = async (path, method = "GET") => {
    try {
      const response = await fetch(`${TEENSY_IP}${path}`, { method });
      const data = await response.json();
      return data;
    } catch (error) {
      setStatusMessage(`Error: ${error.message}`);
      console.error("HTTP Request Error:", error);
    }
  };

  
  // Fetch IMU Data periodically
  useEffect(() => {
    const fetchIMUData = async () => {
      const data = await sendHttpRequest("/imu");
      if (data) {
        setImuData(data);
        publishIMUData(data);
        
      }
    };

    const interval = setInterval(fetchIMUData, 150); // Fetch IMU data every 500ms
    return () => clearInterval(interval); // Cleanup interval on unmount
  }, []);

  useEffect(() => {
    // Publish data every 150ms
    const publishInterval = setInterval(() => {
      publishIMUData(imuData);
    }, 100);

    return () => clearInterval(publishInterval); 
  }, [imuData, publishIMUData]);

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
          <p>{rosStatusMessage}</p>
          <button onClick={sendRosMessage}>Send Message to ROS</button>
          <button onClick={listenToRosTopic}>Listen to ROS Topic</button>
          <button onClick={() => publishIMUData(imuData)}>Publish IMU Data</button>
        </div>
      </div>
    </ThemeContext.Provider>
  );
}

export default App;

