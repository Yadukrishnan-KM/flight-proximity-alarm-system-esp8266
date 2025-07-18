// flight_scanner.cpp
#include "flight_scanner.h"
#include "globals.h"
#include "settings_manager.h" // Assuming settings_manager has functions to get current settings
#include "alarm_manager.h"    // For updateLED and playAlarmSound
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>       // <-- ADDED THIS LINE: Required for the updated http.begin() method
#include <math.h>             // For round() and other math functions
#include <ESP8266WiFi.h>      // <-- ADDED THIS LINE: Necessary for WiFi.status() and overall WiFi connectivity

// Forward declaration for calculateDistance (defined later in this file)
// float calculateDistance(float lat1, float lon1, float lat2, float lon2); // Moved to utils.h
// int getProximityLevel(float distance, float r1, float r2, float r3); // Renamed to determineProximityLevel in utils.h
void updateScanHistory(int level1Count, int level2Count, int level3Count, int totalCount, const std::vector<FlightData>& flights);
String getCurrentFormattedTime();

/**
 * @brief Performs a flight scan by querying the OpenSky Network API.
 */
void performFlightScan() {
    Serial.println("Performing flight scan...");
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Cannot perform flight scan.");
        return;
    }

    WiFiClient client;
    HTTPClient http;

    // Construct the API URL using current settings
    String url = currentSettings.apiServer + "/api/states/all?" +
                 "lamin=" + String(currentSettings.latitude - 1.0) + // +/- 1 degree for bbox
                 "&lamax=" + String(currentSettings.latitude + 1.0) +
                 "&lomin=" + String(currentSettings.longitude - 1.0) +
                 "&lomax=" + String(currentSettings.longitude + 1.0);

    Serial.print("Requesting URL: ");
    Serial.println(url);

    http.begin(client, url);

    // Add API Key header if available
    if (currentSettings.apiKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + currentSettings.apiKey);
    }

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpResponseCode);
        if (httpResponseCode == HTTP_CODE_OK) {
            String payload = http.getString();
            // Serial.println(payload); // For debugging: print the full JSON payload

            // Parse JSON response
            DynamicJsonDocument doc(4096); // Increased buffer size
            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                // Attempt to print part of the payload if deserialization fails for debugging
                if (payload.length() > 200) {
                    Serial.print("Partial payload: ");
                    Serial.println(payload.substring(0, 200));
                } else {
                    Serial.print("Payload: ");
                    Serial.println(payload);
                }
                http.end();
                return;
            }

            JsonArray states_array = doc["states"].as<JsonArray>(); // Get the states array
            if (states_array.isNull()) {
                Serial.println("Error: 'states' array not found or invalid in JSON response.");
                http.end();
                return;
            }

            currentFlights.clear(); // Clear previous flight data
            int level1Count = 0;
            int level2Count = 0;
            int level3Count = 0;

            for (JsonArray state : states_array) { // Iterate through each flight state (which is an array)
                if (state.isNull()) {
                    Serial.println("Warning: Found null state entry in JSON response. Skipping.");
                    continue;
                }

                // OpenSky Network API states array indices:
                // 0: icao24 (string)
                // 1: callsign (string, can be null)
                // 2: origin_country (string)
                // 5: longitude (float, can be null)
                // 6: latitude (float, can be null)
                // 7: baro_altitude (float, can be null)
                // 9: velocity (float, can be null)
                // 10: true_track (float, can be null)

                // Check if latitude and longitude exist and are not null
                if (!state[6].isNull() && !state[5].isNull()) {
                    float flightLat = state[6].as<float>();
                    float flightLon = state[5].as<float>();

                    float distance = calculateDistance(currentSettings.latitude, currentSettings.longitude, flightLat, flightLon);
                    int proximityLevel = determineProximityLevel(distance);

                    FlightData flight;
                    flight.icao24 = state[0].as<String>();
                    flight.callsign = state[1].isNull() ? "N/A" : state[1].as<String>(); // Handle null callsign
                    flight.origin_country = state[2].as<String>();
                    flight.latitude = flightLat;    // ADDED: Assign latitude
                    flight.longitude = flightLon;   // ADDED: Assign longitude
                    flight.altitude_baro = state[7] | 0.0; // Use | for default if null
                    flight.velocity = state[9] | 0.0;       // Use | for default if null
                    flight.true_track = state[10] | 0.0;    // Use | for default if null
                    flight.distance_km = distance;
                    flight.proximity_level = proximityLevel;
                    flight.operatorName = "N/A"; // OpenSky /states/all does not directly provide this
                    flight.destination = "N/A";  // OpenSky /states/all does not directly provide this
                    flight.source = "N/A";       // OpenSky /states/all does not directly provide this
                    flight.international_domestic = "Unknown"; // Derive if possible from other data, else default

                    currentFlights.push_back(flight);

                    if (proximityLevel == 1) {
                        level1Count++;
                    } else if (proximityLevel == 2) {
                        level2Count++;
                    } else if (proximityLevel == 3) {
                        level3Count++;
                    }
                }
            }

            int totalFlights = currentFlights.size();
            Serial.printf("Scan complete. Total flights: %d, Level 1: %d, Level 2: %d, Level 3: %d\n",
                          totalFlights, level1Count, level2Count, level3Count);

            updateScanHistory(level1Count, level2Count, level3Count, totalFlights, currentFlights);

            // Determine overall alarm level and update LED/sound
            int newOverallAlarmLevel = 0;
            if (level1Count > 0) {
                newOverallAlarmLevel = 1;
            } else if (level2Count > 0) {
                newOverallAlarmLevel = 2;
            } else if (level3Count > 0) {
                newOverallAlarmLevel = 3;
            }

            if (newOverallAlarmLevel != currentOverallAlarmLevel) {
                currentOverallAlarmLevel = newOverallAlarmLevel;
                updateLED(currentOverallAlarmLevel);
                if (currentSettings.soundWarning) {
                    playAlarmSound(currentOverallAlarmLevel);
                }
            }
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end(); // Free the resources
    startFlightScanTimer(); // Restart the timer for the next scan
}

/**
 * @brief Starts or restarts the flight scan timer based on detected flights.
 */
void startFlightScanTimer() {
    unsigned long scanInterval = (currentFlights.empty() || currentOverallAlarmLevel == 0) ?
                                 (unsigned long)currentSettings.noFlightScanFreq * 1000 :
                                 (unsigned long)currentSettings.flightPresentScanFreq * 1000;

    Serial.printf("Setting next scan in %lu seconds.\n", scanInterval / 1000);
    flightScanTicker.once_ms(scanInterval, performFlightScan);
}

/**
 * @brief Updates the scan history with results from the latest scan.
 * @param level1Count Number of flights in Level 1.
 * @param level2Count Number of flights in Level 2.
 * @param level3Count Number of flights in Level 3.
 * @param totalCount Total flights detected.
 * @param flights Current list of flights (for modal details).
 */
void updateScanHistory(int level1Count, int level2Count, int level3Count, int totalCount, const std::vector<FlightData>& flights) {
    ScanHistoryEntry newEntry;
    newEntry.timestamp = getCurrentFormattedTime();
    newEntry.level1 = level1Count;
    newEntry.level2 = level2Count;
    newEntry.level3 = level3Count;
    newEntry.total = totalCount;
    newEntry.status = (totalCount > 0) ? "Flights Detected" : "All Clear";
    newEntry.flights_at_scan = flights; // Copy current flights for this scan

    // Add new entry to the front
    scanHistory.insert(scanHistory.begin(), newEntry);

    // Keep history size limited
    if (scanHistory.size() > MAX_SCAN_HISTORY) {
        scanHistory.pop_back();
    }
}

/**
 * @brief Gets the current time formatted as a string.
 * @return Formatted time string (e.g., "YYYY-MM-DD HH:MM:SS").
 */
String getCurrentFormattedTime() {
    time_t rawTime = timeClient.getEpochTime();
    struct tm * ti;
    ti = localtime(&rawTime);

    char buffer[20];
    // Format: YYYY-MM-DD HH:MM:SS
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday,
            ti->tm_hour, ti->tm_min, ti->tm_sec);
    return String(buffer);
}
