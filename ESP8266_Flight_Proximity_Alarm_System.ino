// ESP8266_Flight_Proximity_Alarm_System.ino
#include <Arduino.h>
#include "globals.h"
#include "settings_manager.h"
#include "wifi_manager.h"
#include "alarm_manager.h"
#include "flight_scanner.h"
#include "web_server_handlers.h"

void setup() {
    Serial.begin(115200);
    Serial.println("\nFlight Proximity Alarm System Starting...");

    // Initialize hardware pins
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW); // Turn off LED initially (active LOW)
    pinMode(SPEAKER_PIN, OUTPUT);
    analogWriteFreq(8000); // Set PWM frequency for audio (e.g., 8kHz)
    analogWriteRange(1023); // Set PWM range to 10-bit

    // Initialize file system and load settings
    initFS();
    loadSettings();

    // Connect to WiFi or start AP
    connectWiFi();

    // Setup web server routes
    setupWebServer();

    // Initial LED state based on loaded settings (if any alarm was active from previous run)
    updateLED(currentOverallAlarmLevel);

    // Start the first flight scan after a short delay
    // performFlightScan will re-schedule itself based on settings
    startFlightScanTimer();
}

void loop() {
    server.handleClient();      // Handle incoming web requests
    handleWifiConnection();     // Manage WiFi connection (reconnect if needed, handle AP DNS)
    timeClient.update();        // Keep NTP time updated

    // No blocking delays in loop! All periodic tasks are handled by Ticker.
    // If you need complex LED blinking patterns, implement them using millis()
    // or another Ticker in alarm_manager.cpp/h.
}
