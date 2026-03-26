# WiFi OLED Message Terminal (MQTT Edition)

**[🌐 Access the Live Web Dashboard Here](https://wifi-message-board.vercel.app)**

This is an ESP32-based IoT project that allows you to send dynamic text messages to an SSD1306 OLED display remotely over the internet. The project uses a public MQTT broker (EMQX) as a backend, and features a clean, hacker-style HTML/JS web interface for composing messages.

## Features

- **Global Connectivity**: Bypasses local network restrictions by using a public MQTT broker, allowing you to send messages to the ESP32 from anywhere in the world.
- **Dynamic Display Engine**: The ESP32 formats text and dynamically wraps words to fit cleanly on the `128x64` OLED screen. 
- **Modern Web Dashboard**: A sleek, dark-themed responsive web UI using WebSockets. It features:
  - "Lock screen" security utilizing a personalized Topic Key token.
  - Live OLED message previews.
  - A scrollable message history log.
- **Instant Clear**: An option to wipe the screen entirely right from the web UI.

## Components Needed

- **ESP32** (e.g., NodeMCU-32S)
- **SSD1306 OLED Display** (128x64 resolution, I2C interface)
- Jumper wires (connect OLED `SDA` to ESP32 `P21` and OLED `SCL` to ESP32 `P22`, depending on your specific board)

## Project Structure

- `src/main.cpp` - The core ESP32 C++ firmware.
- `platformio.ini` - PlatformIO configuration file including dependencies.
- `index.html` - The static frontend web application. 
- `favicon.png` - Tab icon for the web app.

## Deployment

### Vercel (Recommended)

The frontend web dashboard is deployed on [Vercel](https://vercel.com/).

1. Push this repository to GitHub.
2. Import the repository in [Vercel](https://vercel.com/new).
3. Vercel will auto-detect the static site — no build configuration needed.
4. The dashboard will be live at your Vercel project URL.

> **Note:** Only `index.html` and `favicon.png` are served by Vercel. The ESP32 firmware files (`src/`, `platformio.ini`, etc.) are part of the repository but are not deployed.

## Setup Instructions

### 1. Configure the ESP32 Firmware

Open the project in [PlatformIO](https://platformio.org/):

Before flashing, edit the configuration variables located at the top of `src/main.cpp`:

```cpp
// ── CONFIG ── change these ───────────────────────────────────
const char* SSID       = "Your_WiFi_Name";
const char* PASSWORD   = "Your_WiFi_Password";

// This acts as a password for your web UI so no one else sends messages to your board!
const char* APP_TOKEN  = "YOUR_SECRET_TOKEN"; 
```

Then, compile and upload the firmware to your ESP32 board. Upon booting, the ESP32 will connect to Wi-Fi and the EMQX broker, displaying connection statuses directly on the OLED.

### 2. Launch the Web Frontend

1. Visit the live dashboard at your Vercel deployment URL, or open `index.html` locally in any modern web browser.
2. The initial "Lock Screen" will appear.
3. Enter the exact `APP_TOKEN` you compiled into the firmware as your "Access Topic Key".
4. Click **Connect**. You can now type messages and hit send!

## Libraries & Dependencies

This project relies on the following open-source libraries (managed automatically by PlatformIO):

- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) (`^2.5.16`)
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) (`^1.12.5`)
- [PubSubClient](https://github.com/knolleary/pubsubclient) (`^2.8`)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) (`^6.21.3`)
- [MQTT.js](https://github.com/mqttjs/MQTT.js) (Loaded dynamically via unpkg on the frontend)

## Custom Server & Hardware Setup

If you prefer to host your own private MQTT broker (such as [Eclipse Mosquitto](https://mosquitto.org/) or [EMQX](https://www.emqx.com/)) rather than using the public server, follow these steps:

### 1. Update the Firmware Broker Settings
Open `src/main.cpp` and locate the MQTT configuration section. Change the broker URL and TCP port to point to your private server:

```cpp
const char* MQTT_BROKER = "YOUR_BROKER_IP_OR_DOMAIN";
const int   MQTT_PORT   = 1883; // Standard unencrypted MQTT port
```

### 2. Update the Frontend WebSockets URL
In order for the web UI to communicate with your broker from a browser, your broker must have **WebSockets** enabled. Open `index.html` and look for the connection line:

```javascript
// Connect to free public broker over WSS
mqttClient = mqtt.connect('wss://YOUR_BROKER_IP_OR_DOMAIN:8084/mqtt');
```
*Note: If you are hosting the HTML file on a secure site (HTTPS), browsers will enforce strict security, meaning your broker's WebSocket port must be secured with SSL certificates (`wss://`). If hosting locally or via `localhost`, you can use standard `ws://`.*

### 3. Hardware Adjustments
If you are using a different ESP board variant, verify the I2C pins for your OLED. By default, the `Wire` library uses your board's default I2C pins (typically GPIO21 and GPIO22 on standard ESP32 boards). If you need to remap them, you can initialize `Wire.begin(SDA_PIN, SCL_PIN);` inside the `setup()` function before `display.begin()`.

---

<div align="center">
  <i>Made by SilverCipher for IoT explorations.</i>
</div>
