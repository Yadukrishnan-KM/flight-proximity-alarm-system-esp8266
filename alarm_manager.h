// alarm_manager.h
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <Arduino.h>
#include "globals.h" // For SPEAKER_PIN, LED_BUILTIN_PIN, audioTicker, currentSettings

// Function declarations
void playAudioSample();
void startAudioPlayback(const uint8_t* audioData, size_t dataSize);
void playAlarmSound(int level);
void updateLED(int level);

#endif
