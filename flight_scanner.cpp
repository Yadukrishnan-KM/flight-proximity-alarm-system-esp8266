// flight_scanner.cpp
#include "flight_scanner.h"

/**
 * @brief Performs a single flight data scan.
 * This function is called periodically by the flightScanTicker.
 */
void performFlightScan() {
    Serial.println("Performing flight scan...");
    HTTPClient http;
    String url = "http://api.opensky-network.org/api/states/all?"; // Use HTTP for OpenSky
    url += "lamin=" + String(currentSettings.latitude - 1.0); // Approx 1 degree lat = 111km
    url += "&lomin=" + String(currentSettings.longitude - 1.0);
    url += "&lamax=" + String(currentSettings.latitude + 1.0);
    url += "&lomax=" + String(currentSettings.longitude + 1.0);

    // Note: OpenSky API does not directly support radius.
    // We fetch a bounding box and then filter by calculated distance.
    // The 1.0 degree delta is a rough approximation for a 100-110km box.

    Serial.print("Fetching from: ");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();

    currentFlights.clear(); // Clear previous flights
    int newOverallAlarmLevel = 0;
    String scanStatus = "Success";

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            // Serial.println(payload); // For debugging API response

            StaticJsonDocument<8192> doc; // Increased size for larger payloads
            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                scanStatus = "JSON Error";
            } else {
                JsonArray states = doc["states"].as<JsonArray>();
                if (!states) {
                    Serial.println("No 'states' array found in JSON response.");
                    scanStatus = "No States Data";
                } else {
                    int level1Count = 0;
                    int level2Count = 0;
                    int level3Count = 0;

                    for (JsonObject state : states) {
                        // Example data structure from OpenSky:
                        // [icao24, callsign, origin_country, time_position, last_contact, longitude, latitude, baro_altitude,
                        //  on_ground, velocity, true_track, vertical_rate, sensors, geo_altitude, squawk, spi, position_source]
                        float flightLat = state[6] | 0.0; // latitude
                        float flightLon = state[5] | 0.0; // longitude
                        float baroAltitude = state[7] | 0.0; // baro_altitude in meters
                        float velocity_ms = state[9] | 0.0; // velocity in m/s
                        float trueTrack = state[10] | 0.0; // true_track in degrees

                        float distance = calculateDistance(currentSettings.latitude, currentSettings.longitude, flightLat, flightLon);
                        int proximity = determineProximityLevel(distance);

                        // Only add flights within the outermost radius
                        if (proximity > 0) {
                            FlightData flight;
                            flight.icao24 = state[0].as<String>();
                            flight.callsign = state[1].as<String>();
                            flight.origin_country = state[2].as<String>();
                            flight.altitude_baro = baroAltitude;
                            flight.velocity = velocity_ms;
                            flight.true_track = trueTrack;
                            flight.distance_km = distance;
                            flight.proximity_level = proximity;
                            // Dummy values for operator, source, destination (OpenSky does not provide these directly in /states/all)
                            flight.operatorName = "N/A"; // OpenSky /states/all doesn't have operator
                            flight.source = "N/A";       // OpenSky /states/all doesn't have source
                            flight.destination = "N/A";  // OpenSky /states/all doesn't have destination
                            flight.international_domestic = "N/A"; // OpenSky /states/all doesn't have this

                            currentFlights.push_back(flight);

                            // Update counts and overall alarm level
                            if (proximity == 1) level1Count++;
                            if (proximity == 2) level2Count++;
                            if (proximity == 3) level3Count++;
                            newOverallAlarmLevel = max(newOverallAlarmLevel, proximity);
                        }
                    }
                    Serial.printf("Flights detected: L1: %d, L2: %d, L3: %d, Total: %d\n",
                                  level1Count, level2Count, level3Count, currentFlights.size());
                }
            }
        } else {
            Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
            scanStatus = "HTTP Error: " + String(httpCode);
        }
    } else {
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        scanStatus = "Network Error";
    }

    http.end(); // Free resources

    // Update global alarm level and trigger sound/LED
    if (newOverallAlarmLevel != currentOverallAlarmLevel) {
        currentOverallAlarmLevel = newOverallAlarmLevel;
        playAlarmSound(currentOverallAlarmLevel);
        updateLED(currentOverallAlarmLevel);
    }

    // Add to scan history
    ScanHistoryEntry newEntry;
    newEntry.timestamp = getTimestamp();
    newEntry.level1 = 0; // Will be calculated from currentFlights
    newEntry.level2 = 0;
    newEntry.level3 = 0;
    newEntry.total = currentFlights.size();
    newEntry.status = scanStatus;
    newEntry.flights_at_scan = currentFlights; // Store a copy of current flights

    for(const auto& flight : currentFlights) {
        if(flight.proximity_level == 1) newEntry.level1++;
        if(flight.proximity_level == 2) newEntry.level2++;
        if(flight.proximity_level == 3) newEntry.level3++;
    }

    scanHistory.insert(scanHistory.begin(), newEntry); // Add to front
    if (scanHistory.size() > MAX_SCAN_HISTORY) {
        scanHistory.pop_back(); // Remove oldest if history exceeds limit
    }

    // Adjust scan frequency based on current alarm level
    float nextScanInterval = currentSettings.noFlightScanFreq;
    if (currentOverallAlarmLevel > 0) { // If any flights detected
        nextScanInterval = currentSettings.flightPresentScanFreq;
    }
    flightScanTicker.once(nextScanInterval, performFlightScan); // Schedule next scan
    Serial.printf("Next scan scheduled in %d seconds.\n", (int)nextScanInterval);
}

/**
 * @brief Starts the flight scan timer.
 * Initial call and re-scheduling after each scan.
 */
void startFlightScanTimer() {
    flightScanTicker.once(5, performFlightScan); // First scan 5 seconds after boot
    Serial.printf("Initial flight scan scheduled in 5 seconds.\n");
}
