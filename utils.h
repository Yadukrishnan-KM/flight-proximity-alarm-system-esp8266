// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "globals.h" // For AppSettings and timeClient

// Function declarations
float degToRad(float deg);
// --- FIX: Only declaration here ---
float calculateDistance(float lat1, float lon1, float lat2, float lon2);
int determineProximityLevel(float distance_km);
String getTimestamp(); // This seems redundant with getCurrentFormattedTime in flight_scanner.cpp

#endif // UTILS_H
