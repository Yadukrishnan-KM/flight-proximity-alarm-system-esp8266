// wifi_manager.h
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "globals.h" // For AppSettings, server, dnsServer, timeClient

// Function declarations
void connectWiFi();
void startAP();
void handleWifiConnection();

#endif // WIFI_MANAGER_H
