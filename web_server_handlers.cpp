// web_server_handlers.cpp
#include "web_server_handlers.h"
#include "globals.h"        // For 'server' object and currentSettings
#include "settings_manager.h" // For saveSettings() and resetAppSettingsToDefaults()

// --- API Handler Implementations ---

// Handle GET request for current settings
void handleGetSettings() {
    Serial.println(F("Received /getSettings request."));
    DynamicJsonDocument doc(512); // Adjust size as needed

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

    String responseJson;
    serializeJson(doc, responseJson);
    server.send(200, "application/json", responseJson);
    Serial.println(F("Sent settings JSON."));
}

// Handle POST request to save settings
void handleSaveSettings() {
    Serial.println(F("Received /saveSettings POST request."));
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        Serial.print(F("Request body: "));
        Serial.println(body);

        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, body);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            server.send(400, "text/plain", "Invalid JSON data");
            return;
        }

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

        if (saveSettings()) {
            Serial.println(F("Settings saved to SPIFFS."));
            server.send(200, "text/plain", "Settings saved successfully!");
        } else {
            Serial.println(F("Failed to save settings to SPIFFS."));
            server.send(500, "text/plain", "Failed to save settings to SPIFFS.");
        }
    } else {
        Serial.println(F("No plain argument in POST request."));
        server.send(400, "text/plain", "No data in request body.");
    }
}

// Handle request to restore default settings
void handleRestoreDefaults() {
    Serial.println(F("Received /restoreDefaults request."));
    if (resetAppSettingsToDefaults()) {
        if (saveSettings()) {
            Serial.println(F("Default settings restored and saved to SPIFFS."));
            server.send(200, "text/plain", "Default settings restored. Reboot ESP to apply.");
        } else {
            Serial.println(F("Failed to save default settings to SPIFFS."));
            server.send(500, "text/plain", "Failed to save default settings.");
        }
    } else {
        Serial.println(F("Failed to restore default settings in memory."));
        server.send(500, "text/plain", "Failed to restore default settings in memory.");
    }
}

// Handle request to reboot ESP
void handleRebootESP() {
    Serial.println(F("Received /rebootESP request. Restarting ESP..."));
    server.send(200, "text/plain", "ESP is restarting...");
    delay(100); // Give time for response to be sent
    ESP.restart();
}

// Add implementations for handleGetLiveData() and handleGetScanHistory() here when ready

void handleGetLiveData() {
    Serial.println(F("Received /getLiveData request."));
    DynamicJsonDocument doc(2048); // Adjust size based on expected flight data

    doc["overallStatus"] = "ALL CLEAR"; // Placeholder
    doc["overallLevel"] = 0; // Placeholder
    doc["level1Count"] = 0; // Placeholder
    doc["level2Count"] = 0; // Placeholder
    doc["level3Count"] = 0; // Placeholder
    doc["totalFlights"] = 0; // Placeholder

    JsonArray flightsArray = doc.createNestedArray("currentFlights");
    // Populate flightsArray with currentFlights data from globals.h
    // Example:
    // for (const FlightData& flight : currentFlights) {
    //     JsonObject flightObj = flightsArray.add<JsonObject>();
    //     flightObj["icao24"] = flight.icao24;
    //     flightObj["callsign"] = flight.callsign;
    //     flightObj["altitude_baro"] = flight.altitude_baro;
    //     // ... add other flight data fields
    // }

    String responseJson;
    serializeJson(doc, responseJson);
    server.send(200, "application/json", responseJson);
    Serial.println("Sent live data JSON.");
}

void handleGetScanHistory() {
    Serial.println(F("Received /getScanHistory request."));
    DynamicJsonDocument doc(4096); // Adjust size based on expected history data (can be large)

    JsonArray historyArray = doc.to<JsonArray>();
    // Populate historyArray with scanHistory data from globals.h
    // Example:
    // for (const ScanHistoryEntry& entry : scanHistory) {
    //     JsonObject entryObj = historyArray.add<JsonObject>();
    //     entryObj["timestamp"] = entry.timestamp;
    //     entryObj["level1"] = entry.level1;
    //     entryObj["level2"] = entry.level2;
    //     entryObj["level3"] = entry.level3;
    //     entryObj["total"] = entry.total;
    //     entryObj["status"] = entry.status;

    //     JsonArray flightsAtScan = entryObj.createNestedArray("flights_at_scan");
    //     for (const FlightData& flight : entry.flights_at_scan) {
    //         JsonObject flightObj = flightsAtScan.add<JsonObject>();
    //         flightObj["icao24"] = flight.icao24;
    //         flightObj["callsign"] = flight.callsign;
    //         // ... add other flight data fields
    //     }
    // }

    String responseJson;
    serializeJson(doc, responseJson);
    server.send(200, "application/json", responseJson);
    Serial.println(F("Sent scan history JSON."));
}
