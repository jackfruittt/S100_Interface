from flask import Flask, request, jsonify
from flask_cors import CORS
import requests

app = Flask(__name__)
CORS(app)

ESP32_IP = "http://192.168.1.34"

# Endpoint to turn LED on
@app.route('/on', methods=['GET'])
def led_on():
    try:
        response = requests.get(f"{ESP32_IP}/led/on")
        if response.status_code == 200:
            return jsonify({"status": "LED ON"})
        else:
            return jsonify({"error": "Failed to turn on LED"}), 500
    except requests.RequestException as e:
        return jsonify({"error": str(e)}), 500

# Endpoint to turn LED off
@app.route('/off', methods=['GET'])
def led_off():
    try:
        response = requests.get(f"{ESP32_IP}/led/off")
        if response.status_code == 200:
            return jsonify({"status": "LED OFF"})
        else:
            return jsonify({"error": "Failed to turn off LED"}), 500
    except requests.RequestException as e:
        return jsonify({"error": str(e)}), 500

# Endpoint for handling hold-on functionality
@app.route('/hold_on', methods=['GET'])
def hold_on():
    action = request.args.get('action')
    if action == 'start':
        # Turn LED on
        try:
            response = requests.get(f"{ESP32_IP}/led/on")
            if response.status_code == 200:
                return jsonify({"status": "LED ON"})
        except requests.RequestException as e:
            return jsonify({"error": str(e)}), 500

    elif action == 'stop':
        # Turn LED off
        try:
            response = requests.get(f"{ESP32_IP}/led/off")
            if response.status_code == 200:
                return jsonify({"status": "LED OFF"})
        except requests.RequestException as e:
            return jsonify({"error": str(e)}), 500

    return jsonify({"error": "Invalid action"}), 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)

