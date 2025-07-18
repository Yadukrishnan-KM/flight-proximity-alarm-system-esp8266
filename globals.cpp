// globals.cpp
#include "globals.h"

// ============================================================================
// Global Variable Definitions
// ============================================================================

AppSettings currentSettings;
std::vector<FlightData> currentFlights;
std::vector<ScanHistoryEntry> scanHistory;
int currentOverallAlarmLevel = 0; // Initial state: No alarm

// Define AP_SSID and AP_PASSWORD here, and ONLY here
const char* AP_SSID = "FlightAlarmSetup";
const char* AP_PASSWORD = "password123";

// Initialize global objects
ESP8266WebServer server(HTTP_PORT);
DNSServer dnsServer;
WiFiUDP ntpUdp;
NTPClient timeClient(ntpUdp, "pool.ntp.org", UTC_OFFSET_SECONDS);
Ticker flightScanTicker;
Ticker audioTicker;

// Audio playback variables (definitions)
const uint8_t* currentAudioData = nullptr;
size_t currentAudioDataSize = 0;
size_t audioSampleIndex = 0;
unsigned long lastAudioSampleTime = 0;
