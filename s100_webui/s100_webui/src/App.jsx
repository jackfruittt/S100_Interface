import React, { useState, useEffect } from "react";
import "./App.css";

const ESP32_IP = "http://192.168.1.34";
const WS_ENDPOINT = `ws://${ESP32_IP.split("http://")[1]}/ws`;

function App() {
  const [statusMessage, setStatusMessage] = useState("");
  const [ws, setWs] = useState(null);

  useEffect(() => {
    const socket = new WebSocket(WS_ENDPOINT);
    setWs(socket);

    socket.onopen = () => setStatusMessage("Connected to ESP32 WebSocket");
    socket.onmessage = (event) => setStatusMessage(event.data);
    socket.onerror = () => setStatusMessage("WebSocket connection error");
    socket.onclose = () => setStatusMessage("WebSocket connection closed");

    return () => socket.close();
  }, []);

  const sendWebSocketMessage = (message) => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(message);
    }
  };

  const handleHoldOnMouseDown = () => sendWebSocketMessage("on");
  const handleMouseUp = () => sendWebSocketMessage("off");

  const handleDrive = (command) => sendWebSocketMessage(command);

  return (
    <div className="App">
      <div className="stream-container">
        <h1>ESP32 WebSocket Control</h1>
        <img src={`${ESP32_IP}/stream`} alt="ESP32 Stream" />
        {statusMessage && <p className="status-message">{statusMessage}</p>}
      </div>
      <div className="control-buttons">
        <button
          className="led-button"
          onMouseDown={handleHoldOnMouseDown}
          onMouseUp={handleMouseUp}
        >
          LED
        </button>
        <button onClick={() => handleDrive("/drive_forwards")}>Up</button>

        <div className="middle-row">
          <button onClick={() => handleDrive("/tank_left")}>Left</button>
          <button onClick={() => handleDrive("/tank_right")}>Right</button>
        </div>
        <button onClick={() => handleDrive("/drive_backwards")}>Down</button>
      </div>
    </div>
  );
}

export default App;
