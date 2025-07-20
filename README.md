# flight-proximity-alarm-system-esp8266

Note: This Project is ABANDONING due to out of memory(OOM) issue in the esp8266. Low memory of esp8266 is not allowing to continue the project. Same project is continuing on esp32 as a new project named esp32-flight-proximity-alarm-system-esp8266. Check it out.

 
It is an Internet based flight proximity alarm system to find the Aeroplane activity near you. It is based on the ESp8266 chip. It have a web interface to interact with user and show the flight details and change the settings. Currently the flight data captured using API call from Openskynetworks using the inbuilt WiFi.

Note: This Project is ABANDONING due to out of memory(OOM) issue in the esp8266. Low memory of esp8266 is not allowing to continue the project. Same project is continuing on esp32 as a new project named esp32-flight-proximity-alarm-system-esp8266. Check it out.

## System Overview

The system retrieves real-time flight data primarily from the **OpenSky Network API**, with the flexibility to integrate additional APIs in the future. It offers a comprehensive web-based user interface for configuring various settings and viewing live flight information. Furthermore, it provides **visual (LED)** and **auditory (sound)** alarms triggered by flight proximity.

---

## Key Features

### Web-based User Interface

The user interface is accessible via a web browser by entering the ESP8266's IP address. It comprises two main sections:

#### 1. Settings Tab

This tab allows users to configure essential parameters:

* **Network Configuration:**
    * Input fields for **WiFi SSID** and **Password**.
* **API Configuration:**
    * Selection for the **API server** (initially OpenSky, with future extensibility).
    * **API Key input** for the selected server.
* **Location Settings:**
    * User's geographical location (**Latitude, Longitude**).
* **Breach Radius Levels:** Define concentric circular proximity zones around the user's location.
    * **"Level 1 Radius"**: The innermost, most critical proximity zone.
    * **"Level 2 Radius"**: The middle proximity zone.
    * **"Level 3 Radius"**: The outermost proximity zone, which also sets the overall maximum monitoring radius.
* **Scan Frequency:** Separate settings for when flights are detected within the radius and when no flights are detected.
* **Sound Warning Toggle:** An enable/disable option for auditory alerts.
* **Configuration Management:**
    * **"Save" button**: Persists settings to Flash memory for power cycle retention.
    * **"Default Settings" button**: Resets all configurations to their default values.
    * **"Reboot" button**: Restarts the ESP8266 to apply new settings.

#### 2. Live Data Tab

This tab displays real-time flight information and alerts:

* **Scan Data Snapshots:** Shows details of the last 10 scanning data snapshots.
    * **Timestamp** of each scan.
    * **Number of flights detected** in Level 1, Level 2, and Level 3 zones during that scan.
    * **Details of detected flights** (altitude, speed, direction, destination, source, operator, flight code) for that specific scan.
* **Real-time Updates**: The UI provides real-time updates using **AJAX**, eliminating the need for full webpage refreshes.
* **Breach Level Indication**: The UI will clearly show which proximity level a flight is in and the **total number of flights within each level**.
* **Error Display**: Any system errors (e.g., WiFi issues, API failures) are prominently displayed on the webpage's user interface, supplementing the LED indicators.

---

### Data Persistence

All settings configured via the web interface are automatically saved to the ESP8266's **Flash memory**. This ensures that all configurations are **retained even after power cycles**.

---

### Real-time Operation & Multitasking

The entire system leverages **FreeRTOS tasks** to enable the concurrent operation of different functionalities. This ensures that the web server, API calls, LED control, and sound playback can operate simultaneously **without blocking each other**.

#### API Data Fetching Optimization

To optimize API calls to the OpenSky API:

* The system requests flight data only for the area encompassing the **largest user-defined radius (Level 3)**.
* Since the OpenSky API typically uses **bounding boxes** (latitude/longitude ranges) rather than direct circular radii, the system calculates a **rectangular window** that fully encloses the Level 3 circle.
* These rectangular window coordinates are then sent to the API to fetch all relevant aircraft data within that broader area.
* Finally, the ESP8266 **filters these results** to include only flights within the actual defined circular radius and categorizes them into the three proximity levels. 

---

### LED Indicator

A dedicated LED provides immediate visual feedback on the system's status and flight proximity:

* **Off**: No flights detected within the entire user-defined monitoring radius, and no errors.
* **Slow Blink**: Flights detected in the **Level 3** (outermost) breach zone.
* **Medium Blink**: Flights detected in the **Level 2** (middle) breach zone.
* **Fast Blink**: Flights detected in the **Level 1** (innermost) breach zone, signifying an immediate alarm condition.
* **Solid On**: Indicates an **error condition** (e.g., WiFi disconnected, API call failed). This state overrides any proximity-based LED pattern when an error is present.

---

### Sound Warning

The system includes an auditory warning feature:

* Plays **pre-defined raw audio data**.
* Sound is played using the ESP8266's **PWM capabilities** on a digital pin.
* Sound warnings are played only **after each scan is completed**.

---

### Modular Code Structure

The project's code is organized into **separate functions** for enhanced readability, usability, and scalability. These functions are then called from within the various **FreeRTOS tasks**, ensuring a clean and efficient architecture.
