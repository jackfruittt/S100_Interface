import React, { useState, useEffect } from "react";
import IMU3DShape from "./IMU3DShape";
import "./App.css";

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
  
  const [statusMessage, setStatusMessage] = useState("");
  const [ws, setWs] = useState(null);
  const [isMotorActive, setIsMotorActive] = useState(false); 
  const [motorSpeed, setMotorSpeed] = useState(50); 
  const [imuData, setImuData] = useState({ roll: 0, pitch: 0, yaw: 0 }); 

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

  const handleSpeedChange = (event) => {
    const speed = parseInt(event.target.value, 10);
    setMotorSpeed(speed);
    debouncedSendSpeed(`speed:${speed}`);
  };

  const handleDrive = (command) => {
    if (!isMotorActive) {
      setIsMotorActive(true); //Mark motor as active
      sendWebSocketMessage(`${command}:${motorSpeed}`); //Include speed in command
    }
  };

  const handleStop = () => {
    if (isMotorActive) {
      setIsMotorActive(false); //Mark motor as inactive
      sendWebSocketMessage("motor_stop");
    }
  };

  return (
    <div className="App">
      <h1>S100 Control Panel</h1>
      <div className="status">
        <p>Status: {statusMessage}</p>
        <div
          className={`motor-status ${isMotorActive ? "active" : "inactive"}`}
        >
          {isMotorActive ? "Motor is Active" : "Motor is Inactive"}
        </div>
      </div>

      <div className="control-container">
        {/* Control Buttons */}
        <div className="control-buttons">
          <button
            className="led-button"
            onMouseDown={() => sendWebSocketMessage("on")}
            onMouseUp={() => sendWebSocketMessage("off")}
          >
            LED
          </button>
          <button
            onMouseDown={() => handleDrive("drive_forwards")}
            onMouseUp={handleStop}
          >
            Up
          </button>

          <div className="middle-row">
            <button
              onMouseDown={() => handleDrive("tank_left")}
              onMouseUp={handleStop}
            >
              Left
            </button>
            <button
              onMouseDown={() => handleDrive("tank_right")}
              onMouseUp={handleStop}
            >
              Right
            </button>
          </div>
          <button
            onMouseDown={() => handleDrive("drive_backwards")}
            onMouseUp={handleStop}
          >
            Down
          </button>
          <div className="speed-slider">
            <label htmlFor="speed">
              Motor Speed: {motorSpeed}% ({Math.round((motorSpeed / 100) * 255)}
              )
            </label>
            <input
              type="range"
              id="speed"
              min="0"
              max="100"
              value={motorSpeed}
              onChange={handleSpeedChange}
            />
          </div>
        </div>

        {/* IMU Data */}
        <div className="imu-data">
        <h2>IMU Data</h2>
          <p>Roll: {imuData.roll}°</p>
          <p>Pitch: {imuData.pitch}°</p>
          <p>Yaw: {imuData.yaw}°</p>
          <IMU3DShape
            roll={imuData.roll}
            pitch={imuData.pitch}
            yaw={imuData.yaw}
          />
        </div>
      </div>
    </div>
  );
}

export default App;
