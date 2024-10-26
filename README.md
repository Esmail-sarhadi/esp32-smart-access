# 🚪 ESP32 Smart Access Control System

This project implements a smart access control system using an ESP32 microcontroller. The system features LED indicators, relay control for door locks, and a web interface for authentication and control.

## 📑 Table of Contents
- [⚙️ Installation](#installation)
- [🚀 Usage](#usage)
- [📂 Project Structure](#project-structure)
- [🔧 Code Explanation](#code-explanation)
- [🐞 Troubleshooting](#troubleshooting)
- [🤝 Contributing](#contributing)
- [📄 License](#license)

## ⚙️ Installation

To set up this project, you'll need:
1. Arduino IDE with ESP32 board support
2. Required Libraries:
   - WiFi
   - WebServer
   - EEPROM
   - Adafruit_NeoPixel
   - ArduinoJson
   - SPIFFS
   - ESPmDNS

## 🚀 Usage

1. Clone this repository:
    ```bash
    git clone https://github.com/esmail-sarhadi/esp32-smart-access.git
    ```
2. Navigate to the project directory:
    ```bash
    cd esp32-smart-access
    ```
3. Open `smart-access.ino` in Arduino IDE
4. Modify the WiFi settings:
    ```cpp
    const char* ssid = "your-ssid";
    const char* password = "your-password";
    ```
5. Upload the code to your ESP32
6. Access the web interface using the IP address shown in Serial Monitor

## 📂 Project Structure

- `smart-access.ino`: Main Arduino sketch containing the system logic
- `/data/index.html`: Web interface for the access control system
- `/data/`: Contains SPIFFS files for the web server

## 🔧 Code Explanation

### LED Control:
Controls NeoPixel LED strip for status indication.
```cpp
void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
}
```

### Authentication System:
Handles user authentication via the web interface.
```cpp
void handleAuth() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, body);
    String code = doc["code"];
    
    if (code == MASTER_CODE) {
        isAuthenticated = true;
        server.send(200, "text/plain", "OK");
    } else {
        isAuthenticated = false;
        server.send(401, "text/plain", "Unauthorized");
    }
}
```

### Door Control:
Manages the door lock relay and LED indicators.
```cpp
void handleToggle() {
    if (isLocked) {
        digitalWrite(RELAY_PIN, LOW);  // Unlock
        setStripColor(0, 255, 0);     // Green
    } else {
        digitalWrite(RELAY_PIN, HIGH); // Lock
        setStripColor(255, 0, 0);     // Red
    }
    isLocked = !isLocked;
    strip.show();
}
```

### Web Server:
Sets up the web interface and handles requests.
```cpp
void setupServer() {
    server.on("/", HTTP_GET, []() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    server.on("/auth", HTTP_POST, handleAuth);
    server.on("/control", HTTP_GET, handleControl);
    server.on("/toggle", HTTP_POST, handleToggle);

    server.begin();
}
```

## 🐞 Troubleshooting

- **LED Strip Issues:** Check the LED_PIN configuration and ensure proper power supply
- **Door Lock Problems:** Verify relay connections and RELAY_PIN setting
- **Web Interface Access:** Confirm WiFi connection and check Serial Monitor for IP address
- **Authentication Failures:** Verify MASTER_CODE setting and JSON request format

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📝 Description

This ESP32-based smart access control system provides secure door control through a web interface. It features LED status indicators, relay-controlled lock mechanism, and a user authentication system. The system is ideal for smart homes, offices, or any location requiring controlled access.
