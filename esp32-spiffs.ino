#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

// Pin Definitions
#define LED_PIN       15    
#define LED_COUNT     8     
#define RELAY_PIN     16    
#define EEPROM_SIZE   512   

// WiFi Settings
const char* ssid = "charon";
const char* password = "12121212";
const char* hostname = "smartdoor";  

// Server Instance
WebServer server(80);

// NeoPixel Strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Authentication
const String MASTER_CODE = "123456";  
bool isLocked = true;
bool isAuthenticated = false;

void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting Smart Access Control System");

    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);
    
    // Initialize NeoPixel strip
    strip.begin();
    strip.setBrightness(100);
    setStripColor(255, 165, 0);  
    strip.show();
    
    // Initialize relay pin
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);  
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Setup WiFi and Web Server
    setupWiFi();
    setupServer();
}

void setupWiFi() {
    WiFi.mode(WIFI_STA);  
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    if (!MDNS.begin(hostname)) {
        Serial.println("Error setting up MDNS responder");
    } else {
        MDNS.addService("http", "tcp", 80);  
    }
}

void setupServer() {
    // Serve the main HTML page
    server.on("/", HTTP_GET, []() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    // Handle authentication
    server.on("/auth", HTTP_POST, handleAuth);
    
    // Serve control page (this can be a separate HTML or just an API endpoint)
    server.on("/control", HTTP_GET, handleControl);
    
    // Handle door toggle requests
    server.on("/toggle", HTTP_POST, handleToggle);

    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();
}

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

void handleControl() {
    if (isAuthenticated) {
        // Optionally return some control options as JSON
        server.send(200, "application/json", "{\"status\": \"ready\"}");
    } else {
        server.send(403, "text/plain", "Forbidden");
    }
}

void handleToggle() {
    if (isLocked) {
        digitalWrite(RELAY_PIN, LOW);  // Unlock the door
        setStripColor(0, 255, 0);       // Set strip color to green
        server.send(200, "text/plain", "unlocked");
    } else {
        digitalWrite(RELAY_PIN, HIGH);  // Lock the door
        setStripColor(255, 0, 0);       // Set strip color to red
        server.send(200, "text/plain", "locked");
    }
    isLocked = !isLocked;  // Toggle the lock state
    strip.show();          // Update the strip
}

void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
}
