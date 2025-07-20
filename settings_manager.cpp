// settings_manager.cpp
#include "settings_manager.h"
#include <ArduinoJson.h> // For JSON serialization/deserialization

// Define default settings here, these should match your HTML's DEFAULT_SETTINGS
const AppSettings DEFAULT_APP_SETTINGS = {
    "", // ssid
    "", // password
    "http://api.opensky-network.org/api/states/all", // apiServer
    "", // apiKey
    34.0522, // latitude (Example: Los Angeles)
    -118.2437, // longitude
    5.0,  // radiusLevel1 (km)
    10.0, // radiusLevel2
    20.0, // radiusLevel3
    60,   // noFlightScanFreq (seconds)
    10,   // flightPresentScanFreq (seconds)
    true  // soundWarning
};


void loadSettings() {
    Serial.println(F("Loading settings..."));
    File settingsFile = SPIFFS.open("/settings.json", "r");
    if (!settingsFile) {
        Serial.println(F("settings.json not found. Creating default settings."));
        currentSettings = DEFAULT_APP_SETTINGS; // Set to hardcoded defaults
        saveSettings(); // Save these defaults to file
        return;
    }

    DynamicJsonDocument doc(512); // Adjust size as needed
    DeserializationError error = deserializeJson(doc, settingsFile);
    settingsFile.close();

    if (error) {
        Serial.print(F("Failed to parse settings.json: "));
        Serial.println(error.f_str());
        Serial.println(F("Using default settings."));
        currentSettings = DEFAULT_APP_SETTINGS; // Fallback to hardcoded defaults
        return;
    }

    // Populate currentSettings from JSON
    currentSettings.ssid = doc["ssid"].as<String>();
    currentSettings.password = doc["password"].as<String>();
    currentSettings.apiServer = doc["apiServer"].as<String>();
    currentSettings.apiKey = doc["apiKey"].as<String>();
    currentSettings.latitude = doc["latitude"].as<float>();
    currentSettings.longitude = doc["longitude"].as<float>();
    currentSettings.radiusLevel1 = doc["radiusLevel1"].as<float>();
    currentSettings.radiusLevel2 = doc["radiusLevel2"].as<float>();
    currentSettings.radiusLevel3 = doc["radiusLevel3"].as<float>();
    currentSettings.noFlightScanFreq = doc["noFlightScanFreq"].as<int>();
    currentSettings.flightPresentScanFreq = doc["flightPresentScanFreq"].as<int>();
    currentSettings.soundWarning = doc["soundWarning"].as<bool>();

    Serial.println(F("Settings loaded successfully."));
}

bool saveSettings() {
    Serial.println(F("Saving settings..."));
    DynamicJsonDocument doc(512); // Adjust size as needed

    // Populate JSON with currentSettings
    doc["ssid"] = currentSettings.ssid;
    doc["password"] = currentSettings.password;
    doc["apiServer"] = currentSettings.apiServer;
    doc["apiKey"] = currentSettings.apiKey;
    doc["latitude"] = currentSettings.latitude;
    doc["longitude"] = currentSettings.longitude;
    doc["radiusLevel1"] = currentSettings.radiusLevel1;
    doc["radiusLevel2"] = currentSettings.radiusLevel2;
    doc["radiusLevel3"] = currentSettings.radiusLevel3;
    doc["noFlightScanFreq"] = currentSettings.noFlightScanFreq;
    doc["flightPresentScanFreq"] = currentSettings.flightPresentScanFreq;
    doc["soundWarning"] = currentSettings.soundWarning;

    File settingsFile = SPIFFS.open("/settings.json", "w");
    if (!settingsFile) {
        Serial.println(F("Failed to open settings.json for writing."));
        return false;
    }

    if (serializeJson(doc, settingsFile) == 0) {
        Serial.println(F("Failed to write to settings.json."));
        settingsFile.close();
        return false;
    }

    settingsFile.close();
    Serial.println(F("Settings saved to settings.json."));
    return true;
}

// NEW: Function to reset settings to hardcoded default values
bool resetAppSettingsToDefaults() {
    Serial.println(F("Resetting app settings to default values in memory..."));
    currentSettings = DEFAULT_APP_SETTINGS; // Copy hardcoded defaults to current settings
    // Save these defaults to SPIFFS immediately
    return saveSettings();
}
