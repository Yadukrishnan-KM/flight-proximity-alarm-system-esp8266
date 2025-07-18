// web_server_handlers.h
#ifndef WEB_SERVER_HANDLERS_H
#define WEB_SERVER_HANDLERS_H

#include <Arduino.h>
#include <LittleFS.h> // For ESP8266 core 3.0.0+, use <FS.h> for older SPIFFS
#include <ArduinoJson.h>
#include "globals.h" // For server, currentSettings, currentFlights, scanHistory
#include "settings_manager.h" // For saveSettings
#include "wifi_manager.h" // For connectWiFi, startAP

// Function declarations
void serveStaticFile(const String& path, const String& contentType);
void handleRoot();
void handleSettingsPage(); // This will serve index.html with settings tab active
void handleSaveSettings();
void handleReboot();
void handleCurrentFlightsApi();
void handleScanHistoryApi();
void handleNotFound();
void setupWebServer();

#endif // WEB_SERVER_HANDLERS_H
