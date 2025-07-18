// settings_manager.cpp
#include "settings_manager.h"

// Default settings (used if no saved settings are found)
const AppSettings DEFAULT_APP_SETTINGS = {
    "YOUR_WIFI_SSID",       // ssid - **CHANGE THIS**
    "YOUR_WIFI_PASSWORD",   // password - **CHANGE THIS**
    "opensky",              // apiServer
    "",                     // apiKey
    15.3582,                // latitude (IIT Dharwad)
    75.0210,                // longitude (IIT Dharwad)
    30.0,                   // radiusLevel1 (km)
    50.0,                   // radiusLevel2 (km)
    70.0,                   // radiusLevel3 (km)
    60,                     // noFlightScanFreq (seconds)
    10,                     // flightPresentScanFreq (seconds)
    true                    // soundWarning
};

/**
 * @brief Initializes the LittleFS file system.
 */
void initFS() {
    if (!LittleFS.begin()) { // Use SPIFFS.begin() for older core
        Serial.println("Failed to mount LittleFS. Formatting...");
        LittleFS.format(); // Use SPIFFS.format() for older core
        if (!LittleFS.begin()) { // Try again after format
            Serial.println("LittleFS mount failed after format. Critical error!");
            return;
        }
    }
    Serial.println("LittleFS mounted successfully.");
}

/**
 * @brief Loads settings from /settings.json.
 */
void loadSettings() {
    File settingsFile = LittleFS.open("/settings.json", "r");
    if (!settingsFile) {
        Serial.println("settings.json not found. Loading default settings.");
        currentSettings = DEFAULT_APP_SETTINGS;
        saveSettings(); // Save defaults for future use
        return;
    }

    StaticJsonDocument<512> doc; // Adjust size if settings grow
    DeserializationError error = deserializeJson(doc, settingsFile);
    settingsFile.close();

    if (error) {
        Serial.print(F("Failed to read settings.json, using default: "));
        Serial.println(error.f_str());
        currentSettings = DEFAULT_APP_SETTINGS;
        saveSettings(); // Save defaults for future use
        return;
    }

    currentSettings.ssid = doc["ssid"].as<String>();
    currentSettings.password = doc["password"].as<String>();
    currentSettings.apiServer = doc["apiServer"].as<String>();
    currentSettings.apiKey = doc["apiKey"].as<String>();
    currentSettings.latitude = doc["latitude"] | DEFAULT_APP_SETTINGS.latitude;
    currentSettings.longitude = doc["longitude"] | DEFAULT_APP_SETTINGS.longitude;
    currentSettings.radiusLevel1 = doc["radiusLevel1"] | DEFAULT_APP_SETTINGS.radiusLevel1;
    currentSettings.radiusLevel2 = doc["radiusLevel2"] | DEFAULT_APP_SETTINGS.radiusLevel2;
    currentSettings.radiusLevel3 = doc["radiusLevel3"] | DEFAULT_APP_SETTINGS.radiusLevel3;
    currentSettings.noFlightScanFreq = doc["noFlightScanFreq"] | DEFAULT_APP_SETTINGS.noFlightScanFreq;
    currentSettings.flightPresentScanFreq = doc["flightPresentScanFreq"] | DEFAULT_APP_SETTINGS.flightPresentScanFreq;
    currentSettings.soundWarning = doc["soundWarning"] | DEFAULT_APP_SETTINGS.soundWarning;

    Serial.println("Settings loaded successfully.");
}

/**
 * @brief Saves current settings to /settings.json.
 */
void saveSettings() {
    File settingsFile = LittleFS.open("/settings.json", "w");
    if (!settingsFile) {
        Serial.println("Failed to open settings.json for writing.");
        return;
    }

    StaticJsonDocument<512> doc;
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

    if (serializeJson(doc, settingsFile) == 0) {
        Serial.println(F("Failed to write to settings.json"));
    }
    settingsFile.close();
    Serial.println("Settings saved.");
}
