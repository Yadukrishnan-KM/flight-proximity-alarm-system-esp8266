// web_server_handlers.cpp
#include "web_server_handlers.h"
#include "globals.h"        // Assuming this includes currentSettings, scanHistory, etc.
#include "settings_manager.h" // Assuming saveSettings() is defined here
#include <ArduinoJson.h>    // For JSON parsing and serialization
#include <ESP8266WebServer.h> // For WebServer functions
#include <FS.h>             // For LittleFS filesystem access
#include <LittleFS.h>       // For LittleFS specific functions
// #include <ESP.h>            // Removed: ESP.h is typically not needed for ESP.restart() on ESP8266

// Assuming 'server' is an extern declaration in web_server_handlers.h or globals.h
// extern ESP8266WebServer server;

/**
 * @brief Serves a file from LittleFS.
 * @param path The path to the file.
 * @param contentType The MIME type of the file.
 */
void serveStaticFile(const String& path, const String& contentType) {
    String filePath = path;
    if (filePath == "/") {
        filePath = "/index.html"; // Default to index.html for root
    }

    if (LittleFS.exists(filePath)) {
        File file = LittleFS.open(filePath, "r");
        server.streamFile(file, contentType);
        file.close();
        Serial.print("Served: ");
        Serial.println(filePath);
    } else {
        Serial.print("File not found: ");
        Serial.println(filePath);
        server.send(404, "text/plain", "404 Not Found");
    }
}

/**
 * @brief Handles the root URL (serves index.html with Live Data tab active).
 */
void handleRoot() {
    serveStaticFile("/index.html", "text/html");
}

/**
 * @brief Handles the settings page URL (serves index.html with Settings tab active).
 * Note: The frontend JavaScript will handle showing the correct tab.
 */
void handleSettingsPage() {
    serveStaticFile("/index.html", "text/html");
}

/**
 * @brief Handles saving settings via POST request.
 */
void handleSaveSettings() {
    if (server.method() == HTTP_POST) {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
            return;
        }

        // Store old WiFi settings to check for changes
        String oldSsid = currentSettings.ssid;
        String oldPassword = currentSettings.password;

        currentSettings.ssid = doc["ssid"].as<String>();
        currentSettings.password = doc["password"].as<String>();
        currentSettings.apiServer = doc["apiServer"].as<String>();
        currentSettings.apiKey = doc["apiKey"].as<String>();
        currentSettings.latitude = doc["latitude"] | 0.0;
        currentSettings.longitude = doc["longitude"] | 0.0;
        currentSettings.radiusLevel1 = doc["radiusLevel1"] | 0.0;
        currentSettings.radiusLevel2 = doc["radiusLevel2"] | 0.0;
        currentSettings.radiusLevel3 = doc["radiusLevel3"] | 0.0;
        currentSettings.noFlightScanFreq = doc["noFlightScanFreq"] | 0;
        currentSettings.flightPresentScanFreq = doc["flightPresentScanFreq"] | 0;
        currentSettings.soundWarning = doc["soundWarning"] | false;

        saveSettings(); // Save updated settings to LittleFS

        bool wifiSettingsChanged = (oldSsid != currentSettings.ssid || oldPassword != currentSettings.password);

        server.send(200, "application/json", "{\"status\":\"success\", \"message\":\"Settings saved!\"}");

        if (wifiSettingsChanged) {
            Serial.println("WiFi settings changed. Rebooting in 3 seconds...");
            delay(3000);
            ESP.restart(); // Reboot to apply new WiFi settings
        }
    } else {
        // For GET request to /saveSettings (unlikely, but handle)
        server.send(405, "text/plain", "Method Not Allowed");
    }
}

/**
 * @brief Handles reboot request.
 */
void handleReboot() {
    Serial.println("Rebooting ESP...");
    server.send(200, "text/plain", "Rebooting...");
    delay(1000);
    ESP.restart();
}

/**
 * @brief Handles API request for current flight data.
 */
void handleCurrentFlightsApi() {
    StaticJsonDocument<1024> doc; // Adjust size based on expected flight data size
    JsonArray flightsArray = doc.to<JsonArray>();

    for (const auto& flight : currentFlights) {
        JsonObject flightObj = flightsArray.createNestedObject();
        flightObj["icao24"] = flight.icao24;
        flightObj["callsign"] = flight.callsign;
        flightObj["operator"] = flight.operatorName;
        flightObj["altitude_baro"] = flight.altitude_baro;
        flightObj["velocity"] = flight.velocity;
        flightObj["true_track"] = flight.true_track;
        flightObj["origin_country"] = flight.origin_country;
        flightObj["distance_km"] = flight.distance_km;
        flightObj["proximity_level"] = flight.proximity_level;
        flightObj["destination"] = flight.destination;
        flightObj["source"] = flight.source;
        flightObj["international_domestic"] = flight.international_domestic;
    }

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

/**
 * @brief Handles API request for scan history.
 */
void handleScanHistoryApi() {
    StaticJsonDocument<20480> doc; // Adjust size based on expected history data size
    JsonArray historyArray = doc.to<JsonArray>();

    for (const auto& entry : scanHistory) {
        JsonObject entryObj = historyArray.createNestedObject();
        entryObj["timestamp"] = entry.timestamp;
        entryObj["level1"] = entry.level1;
        entryObj["level2"] = entry.level2;
        entryObj["level3"] = entry.level3;
        entryObj["total"] = entry.total;
        entryObj["status"] = entry.status;

        JsonArray flightsAtScanArray = entryObj.createNestedArray("flights_at_scan");
        for (const auto& flight : entry.flights_at_scan) {
            JsonObject flightObj = flightsAtScanArray.createNestedObject();
            flightObj["icao24"] = flight.icao24;
            flightObj["callsign"] = flight.callsign;
            flightObj["operator"] = flight.operatorName;
            flightObj["altitude_baro"] = flight.altitude_baro;
            flightObj["velocity"] = flight.velocity;
            flightObj["true_track"] = flight.true_track;
            flightObj["origin_country"] = flight.origin_country;
            flightObj["distance_km"] = flight.distance_km;
            flightObj["proximity_level"] = flight.proximity_level;
            flightObj["destination"] = flight.destination;
            flightObj["source"] = flight.source;
            flightObj["international_domestic"] = flight.international_domestic;
        }
    }

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

/**
 * @brief Handles requests for files not found in specific handlers.
 * Attempts to serve them from LittleFS.
 */
void handleNotFound() {
    String path = server.uri();
    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".ico")) contentType = "image/x-icon";

    serveStaticFile(path, contentType);
}

/**
 * @brief Configures and starts the web server.
 */
void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/settings", handleSettingsPage); // Use handleRoot, frontend JS will switch tabs
    server.on("/saveSettings", handleSaveSettings);
    server.on("/reboot", handleReboot);
    server.on("/api/currentFlights", handleCurrentFlightsApi);
    server.on("/api/scanHistory", handleScanHistoryApi);

    // Serve static files (CSS, JS) from LittleFS
    server.on("/style.css", [](){ serveStaticFile("/style.css", "text/css"); });
    server.on("/script.js", [](){ serveStaticFile("/script.js", "application/javascript"); });

    // Handle any other requests (e.g., favicon.ico)
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started.");
}
