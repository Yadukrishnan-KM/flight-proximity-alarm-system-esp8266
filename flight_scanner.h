// flight_scanner.h
#ifndef FLIGHT_SCANNER_H
#define FLIGHT_SCANNER_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "globals.h" // For currentSettings, currentFlights, scanHistory, Ticker
#include "utils.h"   // For calculateDistance, determineProximityLevel, getTimestamp
#include "alarm_manager.h" // For playAlarmSound, updateLED

// Function declarations
void performFlightScan();
void startFlightScanTimer();

#endif // FLIGHT_SCANNER_H
