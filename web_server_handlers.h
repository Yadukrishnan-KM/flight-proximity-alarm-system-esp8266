// web_server_handlers.h
#ifndef WEB_SERVER_HANDLERS_H
#define WEB_SERVER_HANDLERS_H

#include <Arduino.h>
#include <ESP8266WebServer.h> // For server object
#include <FS.h>               // For SPIFFS (if handlers use it directly)
#include <ArduinoJson.h>      // For JSON parsing/serialization in handlers
#include <Esp.h>              // For ESP.restart() in handleRebootESP

// Function declarations for API handlers
void handleGetSettings();
void handleSaveSettings();
void handleRestoreDefaults();
void handleRebootESP();
// Declare Live Data/Scan History handlers here when you implement them
void handleGetLiveData();
void handleGetScanHistory();

// If you had a setupWebServer() function, its declaration would go here too:
// void setupWebServer();

#endif // WEB_SERVER_HANDLERS_H
