/*
import React, { useState, useEffect } from "react";
import useROS from "./rosConnector";

function ControlPanel() {
  const [isMotorActive, setIsMotorActive] = useState(false);
  const [motorSpeed, setMotorSpeed] = useState(50);
  const [statusMessage, setStatusMessage] = useState("");

  const TEENSY_IP = "http://192.168.2.177";

  // Callback to handle messages received from ROS
  const handleRosMessage = (message) => {
    console.log(`Relaying ROS message via HTTP: ${message}`);
    sendHttpRequest(message);
  };

  const { rosConnected, rosStatusMessage, listenToRosTopic } = useROS(handleRosMessage);

  useEffect(() => {
    if (rosConnected) {
      listenToRosTopic(); // Start listening when ROS is connected
    }
  }, [rosConnected]);

  // Send HTTP request to Teensy
  const sendHttpRequest = async (path) => {
    try {
      const response = await fetch(`${TEENSY_IP}${path}`, { method: "GET" });
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.text();
      setStatusMessage(`Response: ${data}`);
    } catch (error) {
      setStatusMessage(`Failed to fetch: ${error.message}`);
      console.error("HTTP Request Error:", error);
    }
  };

  const handleSpeedChange = (event) => {
    const speed = parseInt(event.target.value, 10);
    setMotorSpeed(speed);
    sendHttpRequest(`/set_speed?value=${speed}`);
  };

  const handleDrive = (command) => {
    if (!isMotorActive) {
      setIsMotorActive(true); // Mark motor as active
      sendHttpRequest(`/${command}?speed=${motorSpeed}`);
    }
  };

  const handleStop = () => {
    if (isMotorActive) {
      setIsMotorActive(false); // Mark motor as inactive
      sendHttpRequest("/motor_stop");
    }
  };

  const handleLedControl = (state) => {
    sendHttpRequest(`/${state}`); // Send LED control directly via HTTP
  };

  return (
    <div className="control-panel">
      <div className="motor-status">
        <p className={isMotorActive ? "active" : "inactive"}>
          {isMotorActive ? "Motor is Active" : "Motor is Inactive"}
        </p>
      </div>
      <button
        className="led-button"
        onMouseDown={() => handleLedControl("on")}
        onMouseUp={() => handleLedControl("off")}
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
          Motor Speed: {motorSpeed}% ({Math.round((motorSpeed / 100) * 255)})
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
      <div className="ros-status">
        <p>{rosStatusMessage}</p> {/* Show ROS connection status */ /*} 
      </div>
      <p className="status-message">{statusMessage}</p> {/* Show HTTP response or errors */ /*}
    </div>
  );
}

export default ControlPanel;
*/

import React, { useState, useEffect } from "react";
import useROS from "./rosConnector";
import useMQTT from "./useMQTT"; // Import MQTT hook

function ControlPanel() {
  const [isMotorActive, setIsMotorActive] = useState(false);
  const [motorSpeed, setMotorSpeed] = useState(50);
  const [statusMessage, setStatusMessage] = useState("");

  // Use MQTT for sending commands
  const { sendCommand } = useMQTT();

  // Callback to handle messages received from ROS
  const handleRosMessage = (message) => {
    console.log(`Relaying ROS message via MQTT: ${message}`);
    sendCommand(message);
  };

  const { rosConnected, rosStatusMessage, listenToRosTopic } = useROS(handleRosMessage);

  useEffect(() => {
    if (rosConnected) {
      listenToRosTopic(); // Start listening when ROS is connected
    }
  }, [rosConnected]);

  // Handle motor speed change
  const handleSpeedChange = (event) => {
    const speed = parseInt(event.target.value, 10);
    setMotorSpeed(speed);
    sendCommand(`set_speed?value=${speed}`);
  };

  const handleDrive = (command) => {
    if (!isMotorActive) {
      setIsMotorActive(true);
      sendCommand(`${command}?speed=${motorSpeed}`);
    }
  };

  const handleStop = () => {
    if (isMotorActive) {
      setIsMotorActive(false);
      sendCommand("motor_stop");
    }
  };

  const handleLedControl = (state) => {
    sendCommand(state); // Send LED control via MQTT
  };

  return (
    <div className="control-panel">
      <div className="motor-status">
        <p className={isMotorActive ? "active" : "inactive"}>
          {isMotorActive ? "Motor is Active" : "Motor is Inactive"}
        </p>
      </div>
      <button
        className="led-button"
        onMouseDown={() => handleLedControl("on")}
        onMouseUp={() => handleLedControl("off")}
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
          Motor Speed: {motorSpeed}% ({Math.round((motorSpeed / 100) * 255)})
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
      <div className="ros-status">
        <p>{rosStatusMessage}</p> {/* Show ROS connection status */}
      </div>
      <p className="status-message">{statusMessage}</p> {/* Show MQTT response */}
    </div>
  );
}

export default ControlPanel;







