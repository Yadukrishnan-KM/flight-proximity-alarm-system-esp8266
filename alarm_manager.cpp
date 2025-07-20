// alarm_manager.cpp
#include "alarm_manager.h"
#include "level1breach.h" // Include your audio data headers
#include "level2breach.h"
#include "level3breach.h"

/*
 * @brief Plays an audio sample via PWM.
 * This function is called repeatedly by the audioTicker.
 */
void playAudioSample() {
  
    if (currentAudioData && audioSampleIndex < currentAudioDataSize) {
        uint8_t sample = currentAudioData[audioSampleIndex];
        int pwmValue = map(sample, 0, 255, 0, 1023); // Map 8-bit to 10-bit PWM
        analogWrite(SPEAKER_PIN, pwmValue);
        audioSampleIndex++;
    } else {
        // Audio finished, stop PWM
        analogWrite(SPEAKER_PIN, 0); // Set PWM to 0 (off)
        audioTicker.detach(); // Stop the ticker
        currentAudioData = nullptr; // Clear current audio data
        Serial.println(F("Audio playback finished."));
    }
}

/**
 * @brief Starts playing a specific audio sequence.
 * @param audioData Pointer to the audio data array.
 * @param dataSize Size of the audio data array.
 */
void startAudioPlayback(const uint8_t* audioData, size_t dataSize) {
  
    if (!currentSettings.soundWarning) {
        Serial.println(F("Sound warning disabled in settings."));
        return;
    }
    if (audioTicker.active()) {
        audioTicker.detach(); // Stop any currently playing audio
    }
    currentAudioData = audioData;
    currentAudioDataSize = dataSize;
    audioSampleIndex = 0;
    Serial.print(F("Starting audio playback of size: "));
    Serial.println(dataSize);
    audioTicker.attach_ms(1, playAudioSample); // Will result in approx 1kHz playback
    Serial.print(F("Starting audio playback of size: "));
    Serial.println(dataSize);
}

/**
 * @brief Plays a specific alarm sound based on the level.
 * @param level The alarm level (1, 2, 3).
 */
void playAlarmSound(int level) {
  
    if (!currentSettings.soundWarning) {
        // Ensure sound is off if warning is disabled
        analogWrite(SPEAKER_PIN, 0);
        audioTicker.detach();
        return;
    }

    switch (level) {
        case 1: // Level 1: Alarm (most urgent)
            Serial.println(F("Playing Level 1 Alarm Sound!"));
            startAudioPlayback(audioData_level1breach, sizeof(audioData_level1breach));
            break;
        case 2: // Level 2: Warning
            Serial.println(F("Playing Level 2 Warning Sound!"));
            startAudioPlayback(audioData_level2breach, sizeof(audioData_level2breach));
            break;
        case 3: // Level 3: Detection
            Serial.println(F("Playing Level 3 Detection Sound!"));
            startAudioPlayback(audioData_level3breach, sizeof(audioData_level3breach));
            break;
        default: // No alarm or all clear
            Serial.println(F("Stopping Alarm Sound."));
            analogWrite(SPEAKER_PIN, 0); // Set PWM to 0 (off)
            audioTicker.detach(); // Stop the ticker if active
            break;
    }
}

/**
 * @brief Updates the built-in LED based on the alarm level.
 * @param level The alarm level.
 */
void updateLED(int level) {
  
    digitalWrite(LED_BUILTIN_PIN, LOW); // Turn off by default (LED is active LOW)
    switch (level) {
        case 1: // Level 1: Solid on (or rapid blink if implemented in loop)
            digitalWrite(LED_BUILTIN_PIN, HIGH); // Turn on (active LOW)
            // For rapid blinking, you'd need a separate Ticker or millis() logic in loop()
            break;
        case 2: // Level 2: Solid on
            digitalWrite(LED_BUILTIN_PIN, HIGH); // Turn on (active LOW)
            break;
        case 3: // Level 3: Solid on
            digitalWrite(LED_BUILTIN_PIN, HIGH); // Turn on (active LOW)
            break;
        default: // All Clear
            digitalWrite(LED_BUILTIN_PIN, LOW); // Turn off (active LOW)
            break;
    }
}
