// ESP8266_Flight_Proximity_Alarm_System.ino
#include <Arduino.h>
#include "globals.h"
#include "settings_manager.h" // Still needed for loadSettings(), saveSettings()
#include "wifi_manager.h"     // Still needed for connectWiFi(), handleWifiConnection()
#include "alarm_manager.h"
#include "flight_scanner.h"
// #include "web_server_handlers.h" // Removed: Its logic is now inline or integrated
#include <FS.h>               // NEW: Include FS.h for SPIFFS operations (replaces LittleFS.h)

// Add this function somewhere in your ESP8266_Flight_Proximity_Alarm_System.ino file,
// outside of setup() or loop(), e.g., at the end of the file.
void listLittleFSContents() { // Renamed from listLittleFSContents for clarity, but logic now uses SPIFFS
    Serial.println("\n--- Listing SPIFFS Contents ---");
    // Change LittleFS.openDir to SPIFFS.openDir
    Dir dir = SPIFFS.openDir("/"); // Open the root directory of SPIFFS
    int fileCount = 0;
    while (dir.next()) {
        fileCount++;
        Serial.print("File: ");
        Serial.print(dir.fileName()); // Get the file name
        Serial.print(" (Size: ");
        Serial.print(dir.fileSize()); // Get the file size
        Serial.println(" bytes)");
    }
    if (fileCount == 0) {
        Serial.println("No files found on SPIFFS.");
    }
    Serial.println("--- SPIFFS Listing Complete ---\n");
}


void setup() {
    Serial.begin(115200);
    Serial.println("\nFlight Proximity Alarm System Starting...");

    // Initialize hardware pins
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW); // Turn off LED initially (active LOW)
    pinMode(SPEAKER_PIN, OUTPUT);
    analogWriteFreq(8000);              // Set PWM frequency for audio (e.g., 8kHz)
    analogWriteRange(1023);             // Set PWM range to 10-bit

    // --- File System Initialization (Changed to SPIFFS) ---
    Serial.println("Initializing File System (SPIFFS)...");
    // Change LittleFS.begin() to SPIFFS.begin()
    if (!SPIFFS.begin()) {
        Serial.println("❌ An Error has occurred while mounting SPIFFS. Check wiring or ESP8266 board settings (Flash Size > FS size).");
        return; // Stop if SPIFFS can't be mounted
    }
    Serial.println("✅ SPIFFS mounted successfully.");

    // Call the function to list contents right after successful mount
    listLittleFSContents(); // This function now uses SPIFFS internally

    // Load settings from file system
    loadSettings();

    // Connect to WiFi or start AP
    connectWiFi();

    // --- Web Server Setup (Changed to SPIFFS) ---

    // IMPORTANT: Explicitly handle the root path ("/") to serve index.html
    server.on("/", HTTP_GET, []() {
        // Change LittleFS.exists to SPIFFS.exists
        if (SPIFFS.exists("/index.html")) {
            // Change LittleFS.open to SPIFFS.open
            server.send(200, "text/html", SPIFFS.open("/index.html", "r").readString());
        } else {
            Serial.println("Error: /index.html not found on SPIFFS for root request!");
            server.send(404, "text/plain", "404: index.html not found for root path");
        }
    });

    // Handle requests for other static files (like /style.css, /script.js, or other HTML pages like /livedata.html)
    // This will serve any file that exactly matches a path in the SPIFFS root
    // Change LittleFS to SPIFFS here
    server.serveStatic("/", SPIFFS, "/");

    // --- 404 Not Found Handler ---
    server.onNotFound([]() {
        Serial.print("404 Not Found: ");
        Serial.println(server.uri());
        server.send(404, "text/plain", "404: Not Found on Server. (General fallback)");
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
