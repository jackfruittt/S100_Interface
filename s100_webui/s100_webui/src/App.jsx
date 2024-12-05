import React, { useState, useEffect, useContext, createContext } from "react";
import ReactSwitch from "react-switch";
import IMU3DShape from "./components/IMU3DShape";
import ControlPanel from "./components/ControlPanel";
import IMUData from "./components/IMUData";
import useROS from "./components/rosConnector";
import "./App.css";

export const ThemeContext = createContext(null);

const ESP32_IP = "http://192.168.35.224";
const WS_ENDPOINT = `ws://${ESP32_IP.split("http://")[1]}/ws`;

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
  const [ws, setWs] = useState(null);
  const [statusMessage, setStatusMessage] = useState("");
  const [imuData, setImuData] = useState({ roll: 0, pitch: 0, yaw: 0 });

  const { rosStatusMessage, sendRosMessage, listenToRosTopic } = useROS();

  const toggleTheme = () => {
    setTheme((curr) => (curr === "light" ? "dark" : "light"));
  };

  useEffect(() => {
    const connectWebSocket = () => {
      const socket = new WebSocket(WS_ENDPOINT);
      setWs(socket);

      socket.onopen = () => setStatusMessage("Connected to ESP32 WebSocket");
      socket.onmessage = (event) => {
        try {
          //ESP32 sends IMU data as JSON
          const data = JSON.parse(event.data);
          if (
            data.roll !== undefined &&
            data.pitch !== undefined &&
            data.yaw !== undefined
          ) {
            setImuData(data); //Update IMU data
          } else {
            setStatusMessage(event.data); //Handle non-IMU messages
          }
        } catch (err) {
          setStatusMessage(event.data); //Handle plain text messages
        }
      };
      socket.onerror = () => {
        setStatusMessage("WebSocket connection error");
        alert("Failed to connect to ESP32. Please check your network.");
      };
      socket.onclose = () => {
        setStatusMessage("WebSocket connection closed. Reconnecting...");
        setTimeout(connectWebSocket, 5000); //Reconnect after 5 seconds
      };
    };

    connectWebSocket();

    return () => ws && ws.close();
  }, []);

  const sendWebSocketMessage = (message) => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(message);
    }
  };

  const debouncedSendSpeed = debounce((message) => {
    sendWebSocketMessage(message);
  }, 300);

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
        <ControlPanel ws={ws} />
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
        </div>
      </div>
    </ThemeContext.Provider>
  );
}

export default App;
