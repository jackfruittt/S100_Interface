import React, { useState, useEffect } from 'react';
import './App.css';

const ESP32_IP = 'http://192.168.1.34';
const WS_ENDPOINT = `ws://${ESP32_IP.split('http://')[1]}/ws`;

function App() {
  const [statusMessage, setStatusMessage] = useState("");
  const [isHolding, setIsHolding] = useState(false);
  const [ws, setWs] = useState(null);

  // Set up WebSocket connection to ESP32
  useEffect(() => {
    const socket = new WebSocket(WS_ENDPOINT);
    setWs(socket);

    socket.onopen = () => {
      console.log("Connected to WebSocket");
      setStatusMessage("Connected to ESP32 WebSocket");
    };

    socket.onmessage = (event) => {
      console.log("Received from ESP32:", event.data);
      setStatusMessage(event.data);
    };

    socket.onerror = (error) => {
      console.error("WebSocket Error:", error);
      setStatusMessage("WebSocket connection error");
    };

    socket.onclose = () => {
      console.log("WebSocket closed");
      setStatusMessage("WebSocket connection closed");
    };

    // Cleanup on component unmount
    return () => {
      socket.close();
    };
  }, []);

  // Function to send WebSocket messages to ESP32
  const sendWebSocketMessage = (message) => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(message);
    }
  };

  // Handle the "Hold On" button
  const handleHoldOnMouseDown = () => {
    setIsHolding(true);
    sendWebSocketMessage("on");
  };

  // Handle mouse up event globally
  const handleMouseUp = () => {
    if (isHolding) {
      sendWebSocketMessage("off");
      setIsHolding(false);
    }
  };

  // Add a global mouseup event listener
  useEffect(() => {
    document.addEventListener('mouseup', handleMouseUp);
    return () => {
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isHolding]);

  return (
    <div className="App">
      <h1>ESP32 WebSocket Control</h1>

      {/* Button for controlling LED via WebSocket */}
      <div style={{ marginBottom: '20px' }}>
        <button
          onMouseDown={handleHoldOnMouseDown}
          onMouseUp={handleMouseUp}
        >
          Hold to Turn LED On (WebSocket)
        </button>
      </div>

      {statusMessage && <p>{statusMessage}</p>}

      {/* Live Video Stream */}
      <h2>Live Camera Stream</h2>
      <img
        src={`${ESP32_IP}/stream`}
        alt="ESP32 Stream"
        style={{ marginTop: '20px', border: '2px solid #333', width: '640px', height: '480px' }}
      />
    </div>
  );
}

export default App;







