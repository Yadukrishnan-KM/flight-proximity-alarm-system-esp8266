// utils.cpp
#include "utils.h"

/**
 * @brief Converts degrees to radians.
 * @param deg Angle in degrees.
 * @return Angle in radians.
 */
float degToRad(float deg) {
    return deg * PI / 180.0;
}

/**
 * @brief Calculates distance between two lat/lon points using Haversine formula.
 * @param lat1 Latitude of point 1 (degrees).
 * @param lon1 Longitude of point 1 (degrees).
 * @param lat2 Latitude of point 2 (degrees).
 * @param lon2 Longitude of point 2 (degrees).
 * @return Distance in kilometers.
 */
float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371.0; // Earth's radius in kilometers

    float dLat = degToRad(lat2 - lat1);
    float dLon = degToRad(lon2 - lon1);

    lat1 = degToRad(lat1);
    lat2 = degToRad(lat2);

    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lat1) * cos(lat2) *
              sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    float distance = R * c; // Distance in km

    return distance;
}

/**
 * @brief Determines the proximity level based on distance and configured radii.
 * @param distance_km Distance to flight in kilometers.
 * @return Proximity level (1, 2, 3) or 0 if outside all radii.
 */
int determineProximityLevel(float distance_km) {
    if (distance_km <= currentSettings.radiusLevel1) {
        return 1; // Level 1: Alarm
    } else if (distance_km <= currentSettings.radiusLevel2) {
        return 2; // Level 2: Warning
    } else if (distance_km <= currentSettings.radiusLevel3) {
        return 3; // Level 3: Detection
    }
    return 0; // No proximity
}

/**
 * @brief Gets current timestamp string.
 * @return Formatted timestamp string (e.g., "YYYY-MM-DD HH:MM:SS").
 */
String getTimestamp() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    char buf[20];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return String(buf);
}
