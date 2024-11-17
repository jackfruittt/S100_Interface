import React, { useState, useEffect } from 'react';
import './App.css';

const ESP32_IP = 'http://192.168.1.34';
const WS_ENDPOINT = `ws://${ESP32_IP.split('http://')[1]}/ws`;

function App() {
  const [statusMessage, setStatusMessage] = useState("");
  const [isHolding, setIsHolding] = useState(false);
  const [ws, setWs] = useState(null);

  useEffect(() => {
    const socket = new WebSocket(WS_ENDPOINT);
    setWs(socket);

    socket.onopen = () => {
      setStatusMessage("Connected to ESP32 WebSocket");
    };

    socket.onmessage = (event) => {
      setStatusMessage(event.data);
    };

    socket.onerror = (error) => {
      setStatusMessage("WebSocket connection error");
    };

    socket.onclose = () => {
      setStatusMessage("WebSocket connection closed");
    };

    return () => socket.close();
  }, []);

  const sendWebSocketMessage = (message) => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(message);
    }
  };

  const handleHoldOnMouseDown = () => {
    setIsHolding(true);
    sendWebSocketMessage("on");
  };

  const handleMouseUp = () => {
    if (isHolding) {
      sendWebSocketMessage("off");
      setIsHolding(false);
    }
  };

  useEffect(() => {
    document.addEventListener('mouseup', handleMouseUp);
    return () => {
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isHolding]);

  return (
    <div className="App">
      <h1>ESP32 WebSocket Control</h1>
      <button onMouseDown={handleHoldOnMouseDown} onMouseUp={handleMouseUp}>
        Hold to Turn LED On
      </button>
      {statusMessage && <p>{statusMessage}</p>}
      <img
        src={`${ESP32_IP}/stream`}
        alt="ESP32 Stream"
        style={{ marginTop: '20px', border: '2px solid #333', width: '640px', height: '480px' }}
      />
    </div>
  );
}

export default App;

