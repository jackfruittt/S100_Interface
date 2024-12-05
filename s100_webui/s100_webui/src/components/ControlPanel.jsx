import React, { useState, useEffect } from "react";
import useROS from "./rosConnector"; 

function ControlPanel({ ws }) {
  const [isMotorActive, setIsMotorActive] = useState(false);
  const [motorSpeed, setMotorSpeed] = useState(50);

  // Callback to handle messages received from ROS
  const handleRosMessage = (message) => {
    console.log(`Relaying ROS message to WebSocket: ${message}`);
    sendWebSocketMessage(message);
  };

  const { rosConnected, rosStatusMessage, listenToRosTopic } = useROS(handleRosMessage);

  useEffect(() => {
    if (rosConnected) {
      listenToRosTopic(); // Start listening when ROS is connected
    }
  }, [rosConnected]);

  const sendWebSocketMessage = (message) => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      console.log(`Sending WebSocket message: ${message}`);
      ws.send(message);
    } else {
      console.error("WebSocket is not connected.");
    }
  };

  const debouncedSendSpeed = (message) => {
    let timeout;
    clearTimeout(timeout);
    timeout = setTimeout(() => sendWebSocketMessage(message), 300);
  };

  const handleSpeedChange = (event) => {
    const speed = parseInt(event.target.value, 10);
    setMotorSpeed(speed);
    debouncedSendSpeed(`speed:${speed}`);
  };

  const handleDrive = (command) => {
    if (!isMotorActive) {
      setIsMotorActive(true); // Mark motor as active
      sendWebSocketMessage(`${command}:${motorSpeed}`); // Include speed in command
    }
  };

  const handleStop = () => {
    if (isMotorActive) {
      setIsMotorActive(false); // Mark motor as inactive
      sendWebSocketMessage("motor_stop");
    }
  };

  const handleLedControl = (state) => {
    sendWebSocketMessage(state); // Send LED control directly via WebSocket
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
    </div>
  );
}

export default ControlPanel;





