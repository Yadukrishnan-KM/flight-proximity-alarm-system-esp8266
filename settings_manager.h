// settings_manager.h
#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Arduino.h>
#include <FS.h> // For SPIFFS
#include "globals.h" // For AppSettings struct

// Function declarations
void loadSettings();
bool saveSettings();
bool resetAppSettingsToDefaults(); // NEW: Function to reset settings to default values

#endif // SETTINGS_MANAGER_H
