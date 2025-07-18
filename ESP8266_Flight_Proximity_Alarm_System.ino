// ESP8266_Flight_Proximity_Alarm_System.ino
#include <Arduino.h>
#include "globals.h"
#include "settings_manager.h" // Still needed for loadSettings(), saveSettings()
#include "wifi_manager.h"     // Still needed for connectWiFi(), handleWifiConnection()
#include "alarm_manager.h"
#include "flight_scanner.h"
// #include "web_server_handlers.h" // Removed: Its logic is now inline or integrated
#include <LittleFS.h>         // NEW: Include LittleFS for file system operations

void setup() {
    Serial.begin(115200);
    Serial.println("\nFlight Proximity Alarm System Starting...");

    // Initialize hardware pins
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW); // Turn off LED initially (active LOW)
    pinMode(SPEAKER_PIN, OUTPUT);
    analogWriteFreq(8000);              // Set PWM frequency for audio (e.g., 8kHz)
    analogWriteRange(1023);             // Set PWM range to 10-bit

    // --- File System Initialization (Moved from initFS()) ---
    Serial.println("Initializing File System...");
    if (!LittleFS.begin()) {
        Serial.println("❌ An Error has occurred while mounting LittleFS. Check wiring or ESP8266 board settings (Flash Size > FS size).");
        // You might want to halt or go into a recovery mode here
        return;
    }
    Serial.println("✅ LittleFS mounted successfully.");

    // Load settings from file system
    loadSettings();

    // Connect to WiFi or start AP
    connectWiFi();

    // --- Web Server Setup (Moved from setupWebServer()) ---
    // Handler for the root path ("/") and other static files (index.html, style.css, script.js)
    // This tells the server to look for requested files in the root directory (/) of LittleFS.
    // When a browser requests "http://192.168.4.1/", it will automatically try to serve "/index.html"
    // Other files like "/style.css" or "/script.js" will also be served if requested by their path.
    server.serveStatic("/", LittleFS, "/");

    // OPTIONAL: A handler for any requests that don't match other handlers (for better debugging)
    server.onNotFound([]() {
        Serial.print("404 Not Found: ");
        Serial.println(server.uri());
        server.send(404, "text/plain", "404: Not Found on Server. Did you upload all files to LittleFS?");
    });

    // Start HTTP server
    server.begin();
    Serial.println("✅ HTTP server started.");

    // Initial LED state based on loaded settings (if any alarm was active from previous run)
    updateLED(currentOverallAlarmLevel);

    // Start the first flight scan after a short delay
    // performFlightScan will re-schedule itself based on settings
    startFlightScanTimer();
}

void loop() {
    server.handleClient();     // Handle incoming web requests
    handleWifiConnection();    // Manage WiFi connection (reconnect if needed, handle AP DNS)
    timeClient.update();       // Keep NTP time updated

    // No blocking delays in loop! All periodic tasks are handled by Ticker.
    // If you need complex LED blinking patterns, implement them using millis()
    // or another Ticker in alarm_manager.cpp/h.
}
