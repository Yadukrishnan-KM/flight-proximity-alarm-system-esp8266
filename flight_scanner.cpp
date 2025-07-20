// flight_scanner.cpp
#include "flight_scanner.h"
#include "globals.h"
#include "settings_manager.h" // Assuming settings_manager has functions to get current settings
#include "alarm_manager.h"    // For updateLED and playAlarmSound
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> // MODIFIED: Use WiFiClientSecure for HTTPS
#include <math.h>             // For round() and other math functions
#include <ESP8266WiFi.h>      // Necessary for WiFi.status() and overall WiFi connectivity
//#include <WiFiClientSecureBearSSL.h>

// Forward declaration for calculateDistance (defined later in this file)
float calculateDistance(float lat1, float lon1, float lat2, float lon2);
int getProximityLevel(float distance, float r1, float r2, float r3);
void updateScanHistory(int level1Count, int level2Count, int level3Count, int totalCount, const std::vector<FlightData>& flights);
String getCurrentFormattedTime();

/**
 * @brief Performs a flight scan by querying the OpenSky Network API.
 */
void performFlightScan() {
    Serial.println(F("Performing flight scan..."));
    Serial.print("Free heap at start of scan: "); Serial.println(ESP.getFreeHeap()); // Debugging heap usage
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("WiFi not connected. Cannot perform flight scan."));
        return;
    }

    // Use WiFiClientSecure for HTTPS, or WiFiClient for HTTP
    // Based on your apiServer URL, if it's "http://", WiFiClient is technically sufficient.
    // However, if you intend HTTPS (as per your comment in the include), WiFiClientSecure is needed.
    // Assuming you've corrected the include to <WiFiClientSecure.h>
    //std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    //client->setInsecure();
    WiFiClientSecure client;
    HTTPClient http;

    //WiFiClientSecure client; // For HTTPS
    // For HTTP only: WiFiClient client;

    // IMPORTANT FOR HTTPS: Add this line to allow insecure connections (for testing).
    // For production, you should verify the server's certificate.
    //client.setInsecure(); // This is for testing with self-signed or unverified certs. Remove for production with proper certificates.

    
    /*String apiUrl = "http://opensky-network.org/api/states/all";
    // Add parameters to API URL
    apiUrl += "?lamin=" + String(currentSettings.latitude - 1.0, 2); // +/- 1 degree for example bounding box
    apiUrl += "&lamax=" + String(currentSettings.latitude + 1.0, 2);
    apiUrl += "&lomin=" + String(currentSettings.longitude - 1.0, 2);
    apiUrl += "&lomax=" + String(currentSettings.longitude + 1.0, 2);
    
    Serial.print("Requesting URL: ");
    Serial.println(apiUrl);*/

    Serial.println(F("Attempting http.begin()..."));
    // Ensure http.begin() uses the correct client (WiFiClient or WiFiClientSecure)
    if (http.begin(client,"https://opensky-network.org/api/states/all")) { // Pass the client object here
        Serial.println(F("http.begin() successful. Attempting http.GET()..."));
        int httpCode = http.GET();

       if (httpCode > 0) {
            Serial.printf("HTTP GET successful, code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                String payload = http.getString();
                Serial.println("Received payload length: " + String(payload.length()));
                // Serial.println("Payload: " + payload); // Uncomment for full payload debug

                // Process JSON data
                // ... (your existing JSON parsing logic) ...
            } else {
                Serial.printf("HTTP GET failed with code: %d\n", httpCode);
                // Handle non-200/301 responses
            }
        } else {
            Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
            // This is where connection errors or no response would be caught
        }
    } else {
       // Serial.println("❌ http.begin() failed to connect to " + apiUrl);
        // This indicates a fundamental issue like DNS resolution or initial connection setup.
    }

    // Always call end() to free resources, even if begin() failed
    http.end();
    Serial.print("Free heap after scan: "); Serial.println(ESP.getFreeHeap()); // Debugging heap usage
}



void startFlightScanTimer() {
    Serial.println(F("Setting next flight scan timer."));
    // Schedule the next scan based on current alarm level.
    // Use flightPresentScanFreq if any alarm level is active, otherwise noFlightScanFreq.
    int scanFrequency = (currentOverallAlarmLevel > 0) ? currentSettings.flightPresentScanFreq : currentSettings.noFlightScanFreq;

    // Detach any existing ticker before attaching a new one
    flightScanTicker.detach();

    // Attach the ticker to call performFlightScan() after 'scanFrequency' seconds
    flightScanTicker.once(scanFrequency, performFlightScan);
}

/**
 * @brief Determines the proximity level of a flight based on distance and configured radii.
 * @param distance Distance of the flight in kilometers.
 * @param r1 Radius for Level 1 (most critical).
 * @param r2 Radius for Level 2.
 * @param r3 Radius for Level 3.
 * @return Proximity level (1, 2, 3, or 0 for none).
 */
int getProximityLevel(float distance, float r1, float r2, float r3) {
    if (distance <= r1) return 1;
    if (distance <= r2) return 2;
    if (distance <= r3) return 3;
    return 0; // No proximity alarm
}

/**
 * @brief Updates the global scan history.
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
