const aedes = require('aedes')({
    heartbeatInterval: 0, // Disbale heartbeat
    timeout: 0           // Disable timeout
});
const { createServer } = require('net');
const http = require('http');
const ws = require('ws');

// Create HTTP and WebSocket servers
const httpServer = http.createServer();
const wsServer = new ws.Server({ 
    server: httpServer,
    path: '/mqtt',
    perMessageDeflate: false
});

// Handle WebSocket connections
wsServer.on('connection', function connection(socket, request) {
    console.log('New WebSocket connection attempt from:', request.socket.remoteAddress);
    
    const stream = ws.createWebSocketStream(socket);
    stream.on('error', (err) => {
        console.error('WebSocket stream error:', err);
    });

    aedes.handle(stream);
});

// Start HTTP server for WebSocket - bind to all interfaces
httpServer.listen(9001, '0.0.0.0', () => {
    console.log('WebSocket MQTT server listening on ws://0.0.0.0:9001/mqtt');
});

// Create and start TCP server - bind to all interfaces
const tcpServer = createServer(aedes.handle);
tcpServer.listen(1883, '0.0.0.0', () => {
    console.log('TCP MQTT server listening on mqtt://0.0.0.0:1883');
});

// Aedes authentication (optional - remove if not needed)
aedes.authenticate = (client, username, password, callback) => {
    // Allow all connections for now
    callback(null, true);
};

// Aedes broker events with more detailed logging
aedes.on('client', (client) => {
    console.log(`Client Connected: ${client.id}`);
});

aedes.on('clientDisconnect', (client) => {
    console.log(`Client Disconnected: ${client.id}`);
});

aedes.on('clientError', (client, err) => {
    console.error('Client error:', client.id, err.message);
});

aedes.on('connectionError', (client, err) => {
    console.error('Connection error:', err.message);
});

aedes.on('publish', (packet, client) => {
    if (client) {
        console.log(`Client ${client.id} published to ${packet.topic}:`, packet.payload.toString());
    }
});

aedes.on('subscribe', (subscriptions, client) => {
    if (client) {
        console.log(`Client ${client.id} subscribed to:`, 
            subscriptions.map(sub => `${sub.topic} (QoS: ${sub.qos})`).join(', '));
    }
});

// Error handling
aedes.on('error', (error) => {
    console.error('Aedes error:', error);
});

tcpServer.on('error', (error) => {
    console.error('TCP server error:', error);
});

httpServer.on('error', (error) => {
    console.error('HTTP server error:', error);
});

// Graceful shutdown
process.on('SIGINT', function() {
    tcpServer.close(() => {
        httpServer.close(() => {
            aedes.close(() => {
                console.log('Servers closed');
                process.exit(0);
            });
        });
    });
});








