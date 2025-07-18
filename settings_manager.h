// settings_manager.h
#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h> // For ESP8266 core 3.0.0+, use <FS.h> for older SPIFFS
#include <ArduinoJson.h>
#include "globals.h" // For AppSettings struct

// Function declarations
void initFS();
void loadSettings();
void saveSettings();

#endif // SETTINGS_MANAGER_H
