// wifi_manager.cpp
#include "wifi_manager.h"

/**
 * @brief Connects to WiFi or starts AP if connection fails.
 */
void connectWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(currentSettings.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(currentSettings.ssid.c_str(), currentSettings.password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) { // Try for 20 seconds
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected.");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        timeClient.begin(); // Start NTP client once connected
    } else {
        Serial.println("\nFailed to connect to WiFi. Starting AP mode.");
        startAP();
    }
}

/**
 * @brief Starts Access Point (AP) mode for configuration.
 */
void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.print("AP Started. IP Address: ");
    Serial.println(WiFi.softAPIP());

    // DNS server for captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());
}

/**
 * @brief Handles WiFi connection status in the loop.
 * If in STA mode and disconnected, attempts to reconnect.
 * If in AP mode, processes DNS requests.
 */
void handleWifiConnection() {
    if (WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
        Serial.println("WiFi disconnected. Attempting to reconnect...");
        connectWiFi(); // Try reconnecting
    }
    // If in AP mode, handle DNS requests
    if (WiFi.getMode() == WIFI_AP) {
        dnsServer.processNextRequest();
    }
}
