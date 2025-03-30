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
  To run the GUI there are 2 ways
  Option 1: With no application built (still developing)
  in terminal do:
  ```
  npm run dev
  ```
This will genrate a localHost link, simply click on that to open if it does not automatically open in your default browser

Option 2: Application built (using concurrent)

The concurrent library is used to build a react desktop application as to using a webUI, this way it's more a bit more professional and also the preferred option when working with industry partners (which is why it can also run without ROS, ROS is good for universitites but not for clients where operators aren't engineers).

## Configuration

Ensure your `package.json` includes the necessary scripts for concurrently running multiple processes. Add or modify the scripts section:

```json
"scripts": {
  "start": "react-scripts start",
  "build": "react-scripts build",
  "test": "react-scripts test",
  "eject": "react-scripts eject",
  "server": "node ../server/server.js",
  "dev": "concurrently \"npm run server\" \"npm run start\""
}
```

*Note: Adjust the server path as needed based on your project structure.*

## Running the Application

### Development Mode

To run both the React frontend and backend server concurrently:

```
npm run dev
```

This command will:
- Start your backend server
- Start the React development server
- Open the application in your default browser at [http://localhost:3000](http://localhost:3000)

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


# ROS Integration
Currently runs on ROS Noetic via ROSBridge. Theretically can run on ROS2 because the topics don't generate unless the bridge is running and all data comes from the react UI.

To run it in ROS do the following 

1. Create a new workspace which contains this repo
2. In that workspace also create a ROSBRIDGE configuration (you may already have a general configuration for this) more on ROSBRIDE here: https://wiki.ros.org/rosbridge_suite
3. Start ROS and the ROSBRIDGE
4. As long as the GUI is successfully running andf connected to the robot the bridge should also be running, you can do a `rostopic list` to confirm the topics generated from the UI have appeared
5. Once successful, you now have to option to integrate ROS with the robot such as integrating simulations or creating a rosboard 
