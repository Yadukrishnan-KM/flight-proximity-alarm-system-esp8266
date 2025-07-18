// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <vector> // For std::vector

// Forward declarations for classes
class ESP8266WebServer;
class DNSServer;
class NTPClient;
class Ticker;
class WiFiUDP;

// ============================================================================
// Data Structures
// ============================================================================

// Structure to hold all configurable settings
struct AppSettings {
    String ssid;
    String password;
    String apiServer;
    String apiKey;
    float latitude;
    float longitude;
    float radiusLevel1;
    float radiusLevel2;
    float radiusLevel3;
    int noFlightScanFreq;
    int flightPresentScanFreq;
    bool soundWarning;
};

// Structure to hold flight data
struct FlightData {
    String icao24;
    String callsign;
    String operatorName;
    float altitude_baro; // meters
    float velocity;      // m/s
    float true_track;    // degrees
    String origin_country;
    float distance_km;
    int proximity_level; // 0=none, 1=Level1, 2=Level2, 3=Level3
    String destination;
    String source;
    String international_domestic;
};

// Structure for scan history entry
struct ScanHistoryEntry {
    String timestamp;
    int level1;
    int level2;
    int level3;
    int total;
    String status;
    std::vector<FlightData> flights_at_scan; // Store flights for modal
};

// ============================================================================
// Global Variables (declared as extern to be defined once in globals.cpp)
// ============================================================================

extern AppSettings currentSettings;
extern std::vector<FlightData> currentFlights;
extern std::vector<ScanHistoryEntry> scanHistory;
extern int currentOverallAlarmLevel;

extern ESP8266WebServer server;
extern DNSServer dnsServer;
extern WiFiUDP ntpUdp;
extern NTPClient timeClient;
extern Ticker flightScanTicker;
extern Ticker audioTicker;

// Audio playback variables (extern)
extern const uint8_t* currentAudioData;
extern size_t currentAudioDataSize;
extern size_t audioSampleIndex;
extern unsigned long lastAudioSampleTime;

// ============================================================================
// Constants (defined directly in header as they are const)
// ============================================================================

const int HTTP_PORT = 80;
const int SPEAKER_PIN = D3; // GPIO0 on Wemos D1 Mini (D3 pin) for PWM audio
const int LED_BUILTIN_PIN = D4; // GPIO2 on Wemos D1 Mini (D4 pin) for built-in LED
const char* AP_SSID = "FlightAlarmSetup";
const char* AP_PASSWORD = "password123"; // Min 8 chars for WPA2
const long UTC_OFFSET_SECONDS = 5.5 * 3600; // IST is UTC+5:30
const int MAX_SCAN_HISTORY = 10;
const unsigned long AUDIO_SAMPLE_INTERVAL_US = 1000000 / 8000; // 8kHz playback rate (125 us per sample)

#endif // GLOBALS_H
