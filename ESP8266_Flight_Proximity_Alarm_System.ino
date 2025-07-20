// ESP8266_Flight_Proximity_Alarm_System.ino

#include <Arduino.h>
#include "globals.h"
#include "settings_manager.h"
#include "wifi_manager.h"
#include "alarm_manager.h"
#include "flight_scanner.h"
#include "web_server_handlers.h"
#include <FS.h>                  // For SPIFFS

// Existing listLittleFSContents function (already using SPIFFS)
void listLittleFSContents() {
  
    Serial.println(F("\n--- Listing SPIFFS Contents ---"));
    Dir dir = SPIFFS.openDir("/");
    int fileCount = 0;
    while (dir.next()) {
        fileCount++;
        Serial.print(F("File: "));
        Serial.print(dir.fileName());
        Serial.print(F(" (Size: "));
        Serial.print(dir.fileSize());
        Serial.println(F(" bytes)"));
    }
    if (fileCount == 0) {
        Serial.println(F("No files found on SPIFFS."));
    }
    Serial.println(F("--- SPIFFS Listing Complete ---\n"));
}


void setup() {
  
    Serial.begin(115200);
    Serial.println(F("\nFlight Proximity Alarm System Starting..."));

    // Initialize hardware pins
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW); // Turn off LED initially (active LOW)
    pinMode(SPEAKER_PIN, OUTPUT);
    analogWriteFreq(8000);              // Set PWM frequency for audio (e.g., 8kHz)
    analogWriteRange(1023);             // Set PWM range to 10-bit

    // --- File System Initialization (Using SPIFFS) ---
    Serial.println(F("Initializing File System (SPIFFS)..."));
    if (!SPIFFS.begin()) {
        Serial.println(F("❌ An Error has occurred while mounting SPIFFS. Check wiring or ESP8266 board settings (Flash Size > FS size)."));
        return;
    }
    Serial.println(F("✅ SPIFFS mounted successfully."));

    listLittleFSContents(); // This function now uses SPIFFS internally
    loadSettings(); // Load settings from file system (settings_manager.cpp will handle this)

    // Connect to WiFi or start AP
    connectWiFi();

    // --- Web Server Setup ---
    // IMPORTANT: Explicitly handle the root path ("/") to serve index.html
    server.on("/", HTTP_GET, []() {
        if (SPIFFS.exists("/index.html")) {
            server.send(200, "text/html", SPIFFS.open("/index.html", "r").readString());
        } else {
            Serial.println(F("Error: /index.html not found on SPIFFS for root request!"));
            server.send(404, "text/plain", "404: index.html not found for root path");
        }
    });

    // Handle requests for other static files (like /style.css, /script.js)
    server.serveStatic("/", SPIFFS, "/");

    // --- API ENDPOINT REGISTRATIONS
    server.on("/getSettings", HTTP_GET, handleGetSettings);
    server.on("/saveSettings", HTTP_POST, handleSaveSettings);
    server.on("/restoreDefaults", HTTP_GET, handleRestoreDefaults);
    server.on("/rebootESP", HTTP_GET, handleRebootESP);

    // You would add handlers for livedat_2.html here later:
    server.on("/getLiveData", HTTP_GET, handleGetLiveData); // For live data updates
    server.on("/getScanHistory", HTTP_GET, handleGetScanHistory); // For scan history

    // --- 404 Not Found Handler ---
    server.onNotFound([]() {
        Serial.print(F("404 Not Found: "));
        Serial.println(server.uri());
        server.send(404, "text/plain", "404: Not Found on Server. (General fallback)");
    });

    // Start HTTP server
    server.begin();
    Serial.println(F("✅ HTTP server started."));

    // Initial LED state based on loaded settings (if any alarm was active from previous run)
    updateLED(currentOverallAlarmLevel);

    // Start the first flight scan after a short delay
    startFlightScanTimer();
}

void loop() {
  
    performFlightScan();
    server.handleClient();
    handleWifiConnection();
    timeClient.update();

}
