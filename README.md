![Web UI](readme_resources/webui.png)

# Web UI for SX00 Robots 
The web UI is built in react and is currently being used to drive the motors via a microcontroller, set the motor speed and show IMU data with an IMU visualizer to show the robot moving in a pipe (currently a cube but a robot model can be added)

# Web UI for ESP32
esp_web_server_pio is for an esp32 S3 Dev and uses wifi to connect to the server. It is quite slow on some wifi networks. 

# Web UI for Teensy
webserver_teensy_eth is to run the same code as the esp32 but instead for a teensy via ethernet. This should allow for much greater throughput. Code is to be tested (TBT).

# React requirements
Make sure npm is installed globally. Once the repo is cloned cd into `S100_Interface/s100_webui/s100_webui/` and do `npm install`. This should install vite and all packages that the web UI uses. also install React version `18.3.1`
When you clone the repository the npm packages may not clone. Verify the following packages (`npm list`)are installed on your device (applies to all OS you are running this code in). If they are not there just do `npm install <PACKAGE_NAME>`, the version number doesnt directly matter
- npm `v10.9.0`
- vite `v5.4.11`
- roslib `v1.4.1`
- aedes `v0.51.3`
- concurrent `v9.1.2`
- eslint `v9.15.0`
- globals `v15.12.0`
- mqtt `v5.10.3`
- three-csg-ts `v3.2.0`
- three `v0.171.0`
- vite `v5.4.14`
- ws `v8.18.0`

# Running GUI
There are two ways to run the GUI. Running in dev mode and running application tests if you've built a desktop app using concurrently. The instructions below go over both. 

## Configuration

Ensure your `package.json` includes the necessary scripts for concurrently running multiple processes. It should currently look like this based off my last push:

```json
{
  "name": "s100_webui",
  "private": true,
  "proxy": "http://192.168.2.177",
  "version": "0.0.0",
  "type": "module",
  "scripts": {
    "dev": "vite",
    "build": "vite build",
    "lint": "eslint .",
    "preview": "vite preview",
    "start": "concurrently \"node src/mqttBroker.cjs\" \"vite\""
  },
  "dependencies": {
    "aedes": "^0.51.3",
    "mqtt": "^5.10.3",
    "react": "^18.3.1",
    "react-dom": "^18.3.1",
    "react-switch": "^7.0.0",
    "roslib": "^1.4.1",
    "three": "^0.171.0",
    "three-csg-ts": "^3.2.0",
    "ws": "^8.18.0"
  },
  "devDependencies": {
    "@eslint/js": "^9.13.0",
    "@types/react": "^18.3.12",
    "@types/react-dom": "^18.3.1",
    "@vitejs/plugin-react": "^4.3.3",
    "concurrently": "^9.1.2",
    "eslint": "^9.13.0",
    "eslint-plugin-react": "^7.37.2",
    "eslint-plugin-react-hooks": "^5.0.0",
    "eslint-plugin-react-refresh": "^0.4.14",
    "globals": "^15.11.0",
    "vite": "^5.4.11"
  }
}
```

*Note: Adjust the server path as needed based on your project structure.*

## Running the Application

### Development Mode

To run both the React frontend and backend server concurrently:

`npm start`
This uses Concurrently to run `node src/mqttBroker.cjs` and `vite` simultaneously.
Alternatively, if you only want to run the Vite development server without the MQTT broker:
`npm run dev`
Using `npm run dev` is only useful if you want to see the layout of the GUI, it will not allow you to connect to the robot properly

Once run, the application will open in your default browser at [http://localhost:3000](http://localhost:3000)

### Production Build

To create a production build:

1. Build the React application:
   ```
   npm run build
   ```

2. The optimized production files will be created in the `build` folder.

3. To serve the production build:
   ```
   npx serve -s build
   ```

## Testing

Run tests with:

```
npm test
```

This will launch the test runner in interactive watch mode. See the [testing documentation](https://create-react-app.dev/docs/running-tests/) for more information.

For specific test files:

```
npm test -- [filename]
```

## Debugging

### Frontend Debugging

1. **Using Chrome DevTools:**
   - Open Chrome DevTools by pressing F12 or right-clicking and selecting "Inspect"
   - Navigate to the "Sources" tab
   - Find your source files under the "webpack://" section
   - Set breakpoints by clicking on line numbers
   - Use the Console panel for logging

2. **React Developer Tools:**
   - Install the [React Developer Tools](https://chrome.google.com/webstore/detail/react-developer-tools/fmkadmapgofadopljbjfkapdkoienihi) extension
   - Inspect component hierarchy and state

### Backend Debugging

1. **Using VS Code:**
   - Create a `.vscode/launch.json` file with the following configuration:
   ```json
   {
     "version": "0.2.0",
     "configurations": [
       {
         "type": "node",
         "request": "launch",
         "name": "Launch Server",
         "program": "${workspaceFolder}/../server/server.js",
         "skipFiles": ["<node_internals>/**"]
       }
     ]
   }
   ```
   - Set breakpoints in your server code
   - Start debugging with F5

2. **Using nodemon for automatic restarts:**
   - Install nodemon:
   ```
   npm install nodemon --save-dev
   ```
   - Add to your package.json:
   ```json
   "scripts": {
     "server-dev": "nodemon ../server/server.js"
   }
   ```

### Console Logging

Add structured console logs to help with debugging:

```javascript
// For grouping related logs
console.group('Data Fetching');
console.log('Request URL:', url);
console.log('Request Options:', options);
console.log('Response:', data);
console.groupEnd();

// For timing operations
console.time('Operation');
// ... your code here
console.timeEnd('Operation');
```

## Common Issues and Solutions

1. **Port already in use:**
   - Change the port in your React application:
     Create a `.env` file in the root directory with:
     ```
     PORT=3001
     ```
   - Change the port in your server:
     ```javascript
     const PORT = process.env.PORT || 5001;
     ```

2. **CORS issues:**
   - Ensure your server has CORS properly configured:
     ```javascript
     const cors = require('cors');
     app.use(cors());
     ```

3. **Node module resolution:**
   - If you encounter module resolution issues, check your import paths and ensure all dependencies are installed correctly.
  
## MQTT Setup

The application includes an MQTT broker using `aedes` that runs alongside the Vite development server when using the `npm start` command. The MQTT broker is defined in `src/mqttBroker.cjs`.

### MQTT Debugging

To debug MQTT communication:

1. Use MQTT Explorer or similar tools to monitor messages
2. Add console logging in your MQTT subscription handlers
3. Check browser console for connection errors

### Vite Configuration

If you need to modify Vite's configuration:

1. Edit the `vite.config.js` file at the root of your project
2. Restart your development server to apply changes

## Common Issues and Solutions

1. **MQTT Connectivity Issues:**
   - Ensure the MQTT broker is running
   - Check that client IDs are unique
   - Verify connection parameters in your code

2. **CORS issues:**
   - Remember that the application is proxied to `http://192.168.2.177` as specified in your package.json
   - For additional CORS settings, modify your Vite configuration

3. **ROS Connection Issues:**
   - Ensure your ROS environment is properly set up and running
   - Check network connectivity between the browser and ROS nodes
   - Verify websocket connections are not being blocked


# ROS Integration
Currently runs on ROS Noetic via ROSBridge. Theretically can run on ROS2 because the topics don't generate unless the bridge is running and all data comes from the react UI.

To run it in ROS do the following 

1. Create a new workspace which contains this repo
2. In that workspace also create a ROSBRIDGE configuration (you may already have a general configuration for this) more on ROSBRIDE here: https://wiki.ros.org/rosbridge_suite
3. Start ROS and the ROSBRIDGE
4. As long as the GUI is successfully running andf connected to the robot the bridge should also be running, you can do a `rostopic list` to confirm the topics generated from the UI have appeared
5. Once successful, you now have to option to integrate ROS with the robot such as integrating simulations or creating a rosboard 
