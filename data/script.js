// Default values for restoration
const DEFAULT_SETTINGS = {
    ssid: "MyHomeNetwork",
    password: "yourwifipassword",
    apiServer: "opensky",
    apiKey: "",
    latitude: 15.3582, // IIT Dharwad
    longitude: 75.0210, // IIT Dharwad
    radiusLevel1: 30, // km
    radiusLevel2: 50, // km
    radiusLevel3: 70, // km
    noFlightScanFreq: 60, // seconds
    flightPresentScanFreq: 10, // seconds
    soundWarning: true
};

// --- Dummy Data for Live Data Page ---
// A full list of dummy flights available
const ALL_DUMMY_FLIGHTS = [
    {
        icao24: "a81fe4",
        callsign: "VISTARA",
        operator: "Vistara",
        altitude_baro: 10000, // meters
        velocity: 222.22, // m/s (approx 800 km/h)
        true_track: 270, // degrees
        origin_country: "India",
        last_seen: Date.now() / 1000,
        distance_km: 25,
        proximity_level: 1, // Within 30km
        destination: "DEL",
        source: "BOM",
        international_domestic: "Domestic"
    },
    {
        icao24: "ac123a",
        callsign: "AIRINDIA",
        operator: "Air India",
        altitude_baro: 12000,
        velocity: 236.11, // m/s (approx 850 km/h)
        true_track: 90,
        origin_country: "India",
        last_seen: Date.now() / 1000 - 30,
        distance_km: 45,
        proximity_level: 2, // Within 50km
        destination: "BLR",
        source: "CCU",
        international_domestic: "Domestic"
    },
    {
        icao24: "b100e4",
        callsign: "QTR908",
        operator: "Qatar Airways",
        altitude_baro: 11000,
        velocity: 255.56, // m/s (approx 920 km/h)
        true_track: 180,
        origin_country: "Qatar",
        last_seen: Date.now() / 1000 - 60,
        distance_km: 65,
        proximity_level: 3, // Within 70km
        destination: "DOH",
        source: "MAA",
        international_domestic: "International"
    },
    {
        icao24: "c245f1",
        callsign: "UAE201",
        operator: "Emirates",
        altitude_baro: 11500,
        velocity: 250, // m/s (approx 900 km/h)
        true_track: 0,
        origin_country: "UAE",
        last_seen: Date.now() / 1000 - 90,
        distance_km: 68,
        proximity_level: 3, // Within 70km
        destination: "DXB",
        source: "HYD",
        international_domestic: "International"
    },
    {
        icao24: "d333a5",
        callsign: "INDIGO",
        operator: "IndiGo",
        altitude_baro: 9000,
        velocity: 216.67, // m/s (approx 780 km/h)
        true_track: 45,
        origin_country: "India",
        last_seen: Date.now() / 1000 - 120,
        distance_km: 32,
        proximity_level: 2, // Within 50km, but closest is 30. Example of just outside L1
        destination: "GOI",
        source: "BOM",
        international_domestic: "Domestic"
    }
];

// Dummy Scan History (updated to include flights_at_scan)
const DUMMY_SCAN_HISTORY = [
    // Current time for the first entry to reflect live scenario
    { timestamp: "2025-07-18 12:06:25 PM IST", level1: 1, level2: 2, level3: 2, total: 5, status: "Success", flights_at_scan: JSON.parse(JSON.stringify(ALL_DUMMY_FLIGHTS)).filter(f => [1,2,3].includes(f.proximity_level)) },
    { timestamp: "2025-07-18 12:05:00 PM IST", level1: 0, level2: 1, level3: 2, total: 3, status: "Success", flights_at_scan: JSON.parse(JSON.stringify(ALL_DUMMY_FLIGHTS)).filter(f => f.proximity_level === 2 || f.proximity_level === 3).slice(0,3) },
    { timestamp: "2025-07-18 12:04:00 PM IST", level1: 0, level2: 0, level3: 0, total: 0, status: "API Error", flights_at_scan: [] }, // Scan Failure Example
    { timestamp: "2025-07-18 12:03:00 PM IST", level1: 0, level2: 0, level3: 2, total: 2, status: "Success", flights_at_scan: JSON.parse(JSON.stringify(ALL_DUMMY_FLIGHTS)).filter(f => f.proximity_level === 3) },
    { timestamp: "2025-07-18 12:02:00 PM IST", level1: 0, level2: 0, level3: 0, total: 0, status: "All Clear", flights_at_scan: [] },
    { timestamp: "2025-07-18 12:01:00 PM IST", level1: 0, level2: 0, level3: 1, total: 1, status: "Success", flights_at_scan: JSON.parse(JSON.stringify(ALL_DUMMY_FLIGHTS)).filter(f => f.callsign === "UAE201") },
    { timestamp: "2025-07-18 12:00:00 PM IST", level1: 0, level2: 0, level3: 0, total: 0, status: "All Clear", flights_at_scan: [] },
    { timestamp: "2025-07-18 11:59:00 AM IST", level1: 0, level2: 0, level3: 0, total: 0, status: "Success", flights_at_scan: [] },
    { timestamp: "2025-07-18 11:58:00 AM IST", level1: 0, level2: 0, level3: 0, total: 0, status: "API Timeout", flights_at_scan: [] }, // Another Scan Failure Example
    { timestamp: "2025-07-18 11:57:00 AM IST", level1: 0, level2: 0, level3: 0, total: 0, status: "All Clear", flights_at_scan: [] }
];

// Global variable to store current sort state for main flights table
let currentSort = {
    key: 'altitude_baro', // Default sort key
    order: 'asc'         // Default sort order
};

// Global variable to store current sort state for modal flights table
let modalSort = {
    key: 'altitude_baro',
    order: 'asc'
};

document.addEventListener('DOMContentLoaded', () => {
    const settingsTabBtn = document.getElementById('settingsTabBtn');
    const liveDataTabBtn = document.getElementById('liveDataTabBtn');
    const settingsTabContent = document.getElementById('settingsTabContent');
    const liveDataTabContent = document.getElementById('liveDataTabContent');
    const settingsForm = document.getElementById('settingsForm');
    const restoreDefaultsBtn = document.getElementById('restoreDefaultsBtn');
    const rebootEspBtn = document.getElementById('rebootEspBtn');
    const saveSettingsBtn = document.getElementById('saveSettingsBtn');

    // Input elements for validation (from settings page)
    const latitudeInput = document.getElementById('latitude');
    const longitudeInput = document.getElementById('longitude');
    const radiusLevel1Input = document.getElementById('radiusLevel1');
    const radiusLevel2Input = document.getElementById('radiusLevel2');
    const radiusLevel3Input = document.getElementById('radiusLevel3');
    const noFlightScanFreqInput = document.getElementById('noFlightScanFreq');
    const flightPresentScanFreqInput = document.getElementById('flightPresentScanFreq');

    // Error message elements (from settings page)
    const errorLatitude = document.getElementById('errorLatitude');
    const errorLongitude = document.getElementById('errorLongitude');
    const errorRadius1 = document.getElementById('errorRadius1');
    const errorRadius2 = document.getElementById('errorRadius2');
    const errorRadius3 = document.getElementById('errorRadius3');
    const errorNoFlightScanFreq = document.getElementById('errorNoFlightScanFreq');
    const errorFlightPresentScanFreq = document.getElementById('errorFlightPresentScanFreq');

    // Modals
    const statusModal = document.getElementById('statusModal');
    const statusMessageText = document.getElementById('statusMessageText');
    const confirmActionModal = document.getElementById('confirmActionModal');
    const confirmActionTitle = document.getElementById('confirmActionTitle');
    const confirmActionMessage = document.getElementById('confirmActionMessage');
    const confirmActionBtn = document.getElementById('confirmActionBtn');
    const cancelActionBtn = document.getElementById('cancelActionBtn');

    // Live Data Page Elements
    const currentOverallStatus = document.getElementById('currentOverallStatus');
    const level1CountCard = document.getElementById('level1CountCard');
    const level2CountCard = document.getElementById('level2CountCard');
    const level3CountCard = document.getElementById('level3CountCard');
    const totalFlightsCard = document.getElementById('totalFlightsCard');
    const currentFlightsTableBody = document.querySelector('#currentFlightsTable tbody');
    const scanHistoryTableBody = document.querySelector('#scanHistoryTable tbody');
    const mainSortableHeaders = document.querySelectorAll('#currentFlightsTable th.sortable');

    // Scan Details Modal Elements
    const scanDetailsModal = document.getElementById('scanDetailsModal');
    const scanDetailsModalContent = document.getElementById('scanDetailsModalContent'); // Added for dragging
    const scanDetailsTitle = document.getElementById('scanDetailsTitle'); // This will be the drag handle
    const scanDetailsTableBody = document.getElementById('scanDetailsTableBody');
    const modalSortableHeaders = document.querySelectorAll('#scanDetailsFlightsTable th.sortable-modal');

    let currentActionCallback = null; // To store the function to call on confirmation

    // Data for modal (will be populated from DUMMY_SCAN_HISTORY)
    let currentModalFlightsData = [];

    // Dragging variables for scanDetailsModal
    let isDragging = false;
    let initialX;
    let initialY;
    let xOffset = 0;
    let yOffset = 0;


    function showTab(tabName) {
        // Remove active class from all tab buttons and content
        settingsTabBtn.classList.remove('active');
        liveDataTabBtn.classList.remove('active');
        settingsTabContent.classList.remove('active');
        liveDataTabContent.classList.remove('active');

        // Add active class to the selected tab
        if (tabName === 'settings') {
            settingsTabBtn.classList.add('active');
            settingsTabContent.classList.add('active');
        } else if (tabName === 'liveData') {
            liveDataTabBtn.classList.add('active');
            liveDataTabContent.classList.add('active');
            // When Live Data tab is activated, update its content with dummy data
            // Use the latest scan to determine current flights and overall status
            const latestScan = DUMMY_SCAN_HISTORY[0]; // Assuming latest is always first
            let flightsToShow = [];
            if (latestScan && (latestScan.status === "Success" || latestScan.status === "All Clear")) {
                flightsToShow = latestScan.flights_at_scan;
            }
            updateLiveData(flightsToShow, DUMMY_SCAN_HISTORY);
        }
    }

    settingsTabBtn.addEventListener('click', () => showTab('settings'));
    liveDataTabBtn.addEventListener('click', () => showTab('liveData'));

    // Function to display status messages in the modal
    function displayStatus(message, type = 'info', duration = 3000) {
        statusMessageText.textContent = message;
        statusModal.className = 'modal status-modal ' + type; // Reset classes and add type
        statusModal.style.display = 'flex'; // Use flex to center

        setTimeout(() => {
            statusModal.style.display = 'none';
        }, duration);
    }

    // Function to load settings from DEFAULT_SETTINGS (for restore defaults and initial load)
    function loadSettings(settings) {
        for (const key in settings) {
            const element = document.getElementById(key);
            if (element) {
                if (element.type === 'checkbox') {
                    element.checked = settings[key];
                } else {
                    element.value = settings[key];
                }
            }
        }
        // After loading, immediately validate to update UI
        validateAllInputs();
    }

    // --- Validation Logic ---
    function validateLatitudeLongitude() {
        const lat = parseFloat(latitudeInput.value);
        const lon = parseFloat(longitudeInput.value);
        let isValid = true;

        latitudeInput.classList.remove('invalid');
        longitudeInput.classList.remove('invalid');
        errorLatitude.style.display = 'none';
        errorLongitude.style.display = 'none';

        if (isNaN(lat) || lat < -90 || lat > 90) {
            latitudeInput.classList.add('invalid');
            errorLatitude.textContent = "Latitude must be between -90 and 90.";
            errorLatitude.style.display = 'block';
            isValid = false;
        }
        if (isNaN(lon) || lon < -180 || lon > 180) {
            longitudeInput.classList.add('invalid');
            errorLongitude.textContent = "Longitude must be between -180 and 180.";
            errorLongitude.style.display = 'block';
            isValid = false;
        }
        return isValid;
    }

    function validateRadii() {
        const r1 = parseFloat(radiusLevel1Input.value);
        const r2 = parseFloat(radiusLevel2Input.value);
        const r3 = parseFloat(radiusLevel3Input.value);
        let isValid = true;

        // Clear previous errors
        radiusLevel1Input.classList.remove('invalid');
        radiusLevel2Input.classList.remove('invalid');
        radiusLevel3Input.classList.remove('invalid');
        errorRadius1.style.display = 'none';
        errorRadius2.style.display = 'none';
        errorRadius3.style.display = 'none';

        if (isNaN(r1) || r1 < 1 || r1 > 1000) {
            radiusLevel1Input.classList.add('invalid');
            errorRadius1.textContent = "Level 1 must be between 1-1000 km.";
            errorRadius1.style.display = 'block';
            isValid = false;
        }
        if (isNaN(r2) || r2 < 1 || r2 > 1000) {
            radiusLevel2Input.classList.add('invalid');
            errorRadius2.textContent = "Level 2 must be between 1-1000 km.";
            errorRadius2.style.display = 'block';
            isValid = false;
        }
        if (isNaN(r3) || r3 < 1 || r3 > 1000) {
            radiusLevel3Input.classList.add('invalid');
            errorRadius3.textContent = "Level 3 must be between 1-1000 km.";
            errorRadius3.style.display = 'block';
            isValid = false;
        }

        // Check hierarchy only if individual values are valid numbers (and not NaN)
        if (!isNaN(r1) && !isNaN(r2) && !isNaN(r3)) {
            if (r1 >= r2) {
                radiusLevel1Input.classList.add('invalid');
                radiusLevel2Input.classList.add('invalid');
                errorRadius2.textContent = "Level 1 must be less than Level 2.";
                errorRadius2.style.display = 'block';
                isValid = false;
            }
            if (r2 >= r3) {
                radiusLevel2Input.classList.add('invalid');
                radiusLevel3Input.classList.add('invalid');
                errorRadius3.textContent = "Level 2 must be less than Level 3.";
                errorRadius3.style.display = 'block';
                isValid = false;
            }
        }
        return isValid;
    }

    function validateFrequencies() {
        const noFreq = parseFloat(noFlightScanFreqInput.value);
        const flightFreq = parseFloat(flightPresentScanFreqInput.value);
        let isValid = true;

        // Clear previous errors
        noFlightScanFreqInput.classList.remove('invalid');
        flightPresentScanFreqInput.classList.remove('invalid');
        errorNoFlightScanFreq.style.display = 'none';
        errorFlightPresentScanFreq.style.display = 'none';

        if (isNaN(noFreq) || noFreq < 10) {
            noFlightScanFreqInput.classList.add('invalid');
            errorNoFlightScanFreq.textContent = "Min frequency is 10 seconds.";
            errorNoFlightScanFreq.style.display = 'block';
            isValid = false;
        }
        if (isNaN(flightFreq) || flightFreq < 10) {
            flightPresentScanFreqInput.classList.add('invalid');
            errorFlightPresentScanFreq.textContent = "Min frequency is 10 seconds.";
            errorFlightPresentScanFreq.style.display = 'block';
            isValid = false;
        }
        return isValid;
    }

    function validateAllInputs() {
        const isValidLatLon = validateLatitudeLongitude();
        const isValidRadii = validateRadii();
        const isValidFrequencies = validateFrequencies();
        // Check form's built-in validity for other required fields (SSID, Password)
        const formBuiltInValid = settingsForm.checkValidity();

        const formIsValid = formBuiltInValid && isValidLatLon && isValidRadii && isValidFrequencies;
        saveSettingsBtn.disabled = !formIsValid;
        return formIsValid;
    }

    // Add event listeners for real-time validation on input change
    latitudeInput.addEventListener('input', validateLatitudeLongitude);
    longitudeInput.addEventListener('input', validateLatitudeLongitude);
    radiusLevel1Input.addEventListener('input', validateRadii);
    radiusLevel2Input.addEventListener('input', validateRadii);
    radiusLevel3Input.addEventListener('input', validateRadii);
    noFlightScanFreqInput.addEventListener('input', validateFrequencies);
    flightPresentScanFreqInput.addEventListener('input', validateFrequencies);

    // Also check on any form input change to enable/disable save button based on HTML5 validity + custom checks
    settingsForm.addEventListener('input', validateAllInputs);

    // Initial load and validation check
    loadSettings(DEFAULT_SETTINGS);
    // Initial message when page loads first time (or settings tab is active by default)
    displayStatus("System ready. Configure your settings.", 'info', 2500);


    // --- Modal Control Functions ---
    function showConfirmModal(title, message, callback) {
        confirmActionTitle.textContent = title;
        confirmActionMessage.textContent = message;
        currentActionCallback = callback;
        confirmActionModal.style.display = 'flex';
    }

    confirmActionBtn.addEventListener('click', () => {
        confirmActionModal.style.display = 'none';
        if (currentActionCallback) {
            currentActionCallback();
        }
    });

    cancelActionBtn.addEventListener('click', () => {
        confirmActionModal.style.display = 'none';
        displayStatus("Action cancelled.", 'info', 2000);
    });

    // --- Action Handlers ---

    // Save Settings
    settingsForm.addEventListener('submit', (event) => {
        event.preventDefault(); // Prevent default form submission
        if (validateAllInputs()) { // Check validity one last time
            showConfirmModal(
                "Confirm Save Settings",
                "Are you sure you want to save these settings? The ESP will reboot if network settings change.",
                performSaveSettings
            );
        } else {
            displayStatus("Please correct input errors before saving.", 'error', 3500);
        }
    });

    async function performSaveSettings() {
        displayStatus("Saving settings...", 'info', 5000);

        const formData = new FormData(settingsForm);
        const data = {};
        formData.forEach((value, key) => {
            data[key] = key === 'soundWarning' ? document.getElementById('soundWarning').checked : value;
        });
        console.log("Simulating save data:", data);

        // In a real scenario, this would be an AJAX request to the ESP8266
        /*
        try {
            const response = await fetch('/saveSettings', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            });
            if (response.ok) {
                displayStatus("Settings saved successfully!", 'success', 3000);
                // If network settings changed, you might need to reboot
                // window.location.reload(); // Or redirect if IP changes
            } else {
                displayStatus("Error saving settings.", 'error', 5000);
            }
        } catch (error) {
            console.error('Error:', error);
            displayStatus("Network error while saving settings.", 'error', 5000);
        }
        */
        setTimeout(() => {
            displayStatus("Settings saved successfully! (Simulated)", 'success', 3000);
        }, 1500);
    }

    // Restore Defaults
    restoreDefaultsBtn.addEventListener('click', () => {
        showConfirmModal(
            "Confirm Restore Defaults",
            "Are you sure you want to restore default settings? This will require a reboot to take full effect.",
            performRestoreDefaults
        );
    });

    function performRestoreDefaults() {
        displayStatus("Restoring defaults...", 'info', 5000);
        loadSettings(DEFAULT_SETTINGS); // Update UI
        // In a real scenario, this would also send an AJAX request to ESP8266
        setTimeout(() => {
            displayStatus("Defaults restored! Please save and reboot to apply. (Simulated)", 'success', 4000);
        }, 1500);
    }

    // Reboot ESP
    rebootEspBtn.addEventListener('click', () => {
        showConfirmModal(
            "Confirm Reboot ESP",
            "Are you sure you want to reboot the ESP8266?",
            performRebootEsp
        );
    });

    function performRebootEsp() {
        displayStatus("Rebooting ESP...", 'info', 5000);
        // In a real scenario, this would be an AJAX request to the ESP8266
        /*
        try {
            await fetch('/reboot', { method: 'POST' });
            // The page will likely become unreachable for a moment
        } catch (error) {
            console.error('Error:', error);
            displayStatus("Network error while rebooting ESP.", 'error', 5000);
        }
        */
        setTimeout(() => {
            displayStatus("ESP Rebooting... (Simulated)", 'success', 3000);
        }, 1500);
    }


    // --- Live Data Page Functions ---

    // Helper to convert degrees to cardinal direction
    function degreesToCardinal(degrees) {
        const val = Math.floor((degrees / 22.5) + 0.5);
        const arr = ["N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"];
        return arr[(val % 16)];
    }

    /**
     * Function to populate a flight table.
     * @param {HTMLElement} tableBodyElement - The <tbody> element to populate.
     * @param {Array<Object>} flights - An array of flight objects.
     * @param {string} [velocityUnit='m/s'] - The unit for velocity in input data ('m/s' or 'km/h').
     */
    function populateFlightTable(tableBodyElement, flights, velocityUnit = 'm/s') {
        tableBodyElement.innerHTML = ''; // Clear existing rows
        if (flights.length === 0) {
            tableBodyElement.innerHTML = '<tr class="no-data"><td colspan="8">No flights detected.</td></tr>';
        } else {
            flights.forEach(flight => {
                const row = tableBodyElement.insertRow();
                row.insertCell().textContent = flight.callsign || flight.icao24 || 'N/A';
                row.insertCell().textContent = flight.operator || 'N/A';

                // Altitude
                row.insertCell().textContent = flight.altitude_baro ? `${(flight.altitude_baro).toFixed(0)} m` : 'N/A';

                // Speed (convert to km/h if input is m/s)
                let speedKmH = 'N/A';
                if (flight.velocity !== undefined && flight.velocity !== null) {
                    speedKmH = (velocityUnit === 'm/s' ? flight.velocity * 3.6 : flight.velocity).toFixed(0) + ' km/h';
                }
                row.insertCell().textContent = speedKmH;

                row.insertCell().textContent = flight.true_track !== undefined ? `${flight.true_track}° ${degreesToCardinal(flight.true_track)}` : 'N/A';
                row.insertCell().textContent = `${flight.source || 'N/A'} → ${flight.destination || 'N/A'}`;
                row.insertCell().textContent = flight.proximity_level ? `Level ${flight.proximity_level}` : 'N/A';
                row.insertCell().textContent = flight.distance_km ? `${(flight.distance_km).toFixed(1)} km` : 'N/A';
            });
        }
    }


    function updateLiveData(flights, scanHistory) {
        // 1. Determine current flights to display based on latest scan status
        const latestScan = scanHistory[0];
        let displayFlights = [];
        let isScanFailed = false;

        if (latestScan && (latestScan.status.includes("Error") || latestScan.status.includes("Timeout"))) {
            isScanFailed = true;
        } else {
            // Make a deep copy to ensure sorting doesn't modify the original dummy data for history
            displayFlights = JSON.parse(JSON.stringify(flights));
        }

        // 2. Update Current Proximity Status Cards
        let level1Count = 0;
        let level2Count = 0;
        let level3Count = 0;
        let totalFlights = displayFlights.length;
        let overallStatus = "ALL CLEAR";
        let overallStatusClass = "all-clear";

        displayFlights.forEach(flight => {
            if (flight.proximity_level === 1) {
                level1Count++;
            } else if (flight.proximity_level === 2) {
                level2Count++;
            } else if (flight.proximity_level === 3) {
                level3Count++;
            }
        });

        if (isScanFailed) {
            overallStatus = "SCAN FAILED!";
            overallStatusClass = "error"; // Use error class for general failed scan status card
            // Reset counts for failed scan
            level1Count = 0;
            level2Count = 0;
            level3Count = 0;
            totalFlights = 0;
        } else if (level1Count > 0) {
            overallStatus = "LEVEL 1 ALARM!";
            overallStatusClass = "level-1";
        } else if (level2Count > 0) {
            overallStatus = "LEVEL 2 WARNING";
            overallStatusClass = "level-2";
        } else if (level3Count > 0) {
            overallStatus = "LEVEL 3 DETECTION";
            overallStatusClass = "level-3";
        }

        currentOverallStatus.querySelector('p').textContent = overallStatus;
        currentOverallStatus.className = 'status-card ' + overallStatusClass; // Update class for styling
        // Add/remove blinking effect for Level 1 Alarm
        if (overallStatus === "LEVEL 1 ALARM!") {
            currentOverallStatus.classList.add('blink');
        } else {
            currentOverallStatus.classList.remove('blink');
        }

        level1CountCard.querySelector('p').textContent = `${level1Count} Flights`;
        level1CountCard.className = 'status-card level-1';
        if (level1Count === 0) level1CountCard.classList.remove('level-1');

        level2CountCard.querySelector('p').textContent = `${level2Count} Flights`;
        level2CountCard.className = 'status-card level-2';
        if (level2Count === 0) level2CountCard.classList.remove('level-2');

        level3CountCard.querySelector('p').textContent = `${level3Count} Flights`;
        level3CountCard.className = 'status-card level-3';
        if (level3Count === 0) level3CountCard.classList.remove('level-3');

        totalFlightsCard.querySelector('p').textContent = `${totalFlights} Flights`;

        // 3. Populate Current Flights Table (with sorting)
        // Apply sorting before populating the table
        const sortedFlights = [...displayFlights].sort((a, b) => {
            let valA = a[currentSort.key];
            let valB = b[currentSort.key];

            // Special handling for velocity if it's stored in m/s but displayed in km/h
            if (currentSort.key === 'velocity') {
                valA = valA !== undefined && valA !== null ? valA * 3.6 : -Infinity;
                valB = valB !== undefined && valB !== null ? valB * 3.6 : -Infinity;
            } else {
                // Handle undefined/null values for other sorting keys
                if (valA === undefined || valA === null) valA = -Infinity;
                if (valB === undefined || valB === null) valB = -Infinity;
            }

            if (currentSort.order === 'asc') {
                return valA - valB;
            } else {
                return valB - valA;
            }
        });

        currentFlightsTableBody.innerHTML = ''; // Clear existing rows
        if (isScanFailed) {
            currentFlightsTableBody.innerHTML = '<tr class="no-data"><td colspan="8">Last scan failed. No flight data available.</td></tr>';
        } else if (sortedFlights.length === 0) {
            currentFlightsTableBody.innerHTML = '<tr class="no-data"><td colspan="8">No flights currently detected within monitoring radius.</td></tr>';
        } else {
            populateFlightTable(currentFlightsTableBody, sortedFlights);
        }

        // 4. Populate Last 10 Scan History Table
        scanHistoryTableBody.innerHTML = ''; // Clear existing rows
        if (scanHistory.length === 0) {
            scanHistoryTableBody.innerHTML = '<tr class="no-data"><td colspan="6">No scan history available yet.</td></tr>';
        } else {
            scanHistory.forEach((scan, index) => {
                const row = scanHistoryTableBody.insertRow();
                row.insertCell().textContent = scan.timestamp;
                row.insertCell().textContent = scan.level1;
                row.insertCell().textContent = scan.level2;
                row.insertCell().textContent = scan.level3;
                row.insertCell().textContent = scan.total;
                row.insertCell().textContent = scan.status;
                row.dataset.scanIndex = index; // Store index to retrieve full data later
            });
        }
    }

    // --- Sorting Logic for Main Flights Table ---
    mainSortableHeaders.forEach(header => {
        header.addEventListener('click', () => {
            const key = header.dataset.sortKey;
            let order = header.dataset.sortOrder;

            // If clicking the currently active sort key, toggle order
            if (key === currentSort.key) {
                order = (order === 'asc') ? 'desc' : 'asc';
            } else {
                // If clicking a new sort key, set default to asc
                order = 'asc';
            }

            // Update global sort state
            currentSort.key = key;
            currentSort.order = order;

            // Reset all header classes and set current one
            mainSortableHeaders.forEach(h => {
                h.classList.remove('asc', 'desc');
                h.dataset.sortOrder = ''; // Clear stored order
            });
            header.classList.add(order);
            header.dataset.sortOrder = order;

            // Re-render the flights with new sort order
            const latestScan = DUMMY_SCAN_HISTORY[0];
            let flightsToShow = [];
            if (latestScan && (latestScan.status === "Success" || latestScan.status === "All Clear")) {
                flightsToShow = latestScan.flights_at_scan;
            }
            updateLiveData(flightsToShow, DUMMY_SCAN_HISTORY);
        });
    });

    // Initialize sorting arrows display on load for main table
    function initializeMainSortArrows() {
        mainSortableHeaders.forEach(header => {
            if (header.dataset.sortKey === currentSort.key) {
                header.classList.add(currentSort.order);
                header.dataset.sortOrder = currentSort.order;
            } else {
                // Ensure other headers are reset
                header.classList.remove('asc', 'desc');
                header.dataset.sortOrder = '';
            }
        });
    }

    // --- Sorting Logic for Modal Flights Table ---
    modalSortableHeaders.forEach(header => {
        header.addEventListener('click', () => {
            const key = header.dataset.sortKey;
            let order = header.dataset.sortOrder;

            // If clicking the currently active sort key, toggle order
            if (key === modalSort.key) {
                order = (order === 'asc') ? 'desc' : 'asc';
            } else {
                // If clicking a new sort key, set default to asc
                order = 'asc';
            }

            // Update modal sort state
            modalSort.key = key;
            modalSort.order = order;

            // Reset all modal header classes and set current one
            modalSortableHeaders.forEach(h => {
                h.classList.remove('asc', 'desc');
                h.dataset.sortOrder = ''; // Clear stored order
            });
            header.classList.add(order);
            header.dataset.sortOrder = order;

            // Re-render the flights in the modal with new sort order
            sortAndPopulateModalFlights();
        });
    });

    // Function to sort and populate the modal's flight table
    function sortAndPopulateModalFlights() {
        const sortedModalFlights = [...currentModalFlightsData].sort((a, b) => {
            let valA = a[modalSort.key];
            let valB = b[modalSort.key];

            // Special handling for velocity if it's stored in m/s but displayed in km/h
            if (modalSort.key === 'velocity') {
                valA = valA !== undefined && valA !== null ? valA * 3.6 : -Infinity;
                valB = valB !== undefined && valB !== null ? valB * 3.6 : -Infinity;
            } else {
                 // Handle undefined/null values for other sorting keys
                if (valA === undefined || valA === null) valA = -Infinity;
                if (valB === undefined || valB === null) valB = -Infinity;
            }

            if (modalSort.order === 'asc') {
                return valA - valB;
            } else {
                return valB - valA;
            }
        });
        populateFlightTable(scanDetailsTableBody, sortedModalFlights);
    }

    // Initialize sorting arrows display for modal on load
    function initializeModalSortArrows() {
        modalSortableHeaders.forEach(header => {
            if (header.dataset.sortKey === modalSort.key) {
                header.classList.add(modalSort.order);
                header.dataset.sortOrder = modalSort.order;
            } else {
                 // Ensure other headers are reset
                header.classList.remove('asc', 'desc');
                header.dataset.sortOrder = '';
            }
        });
    }

    // --- Scan History Row Click Handler ---
    scanHistoryTableBody.addEventListener('click', (event) => {
        const clickedRow = event.target.closest('tr');
        if (clickedRow && clickedRow.dataset.scanIndex !== undefined) {
            const index = parseInt(clickedRow.dataset.scanIndex);
            const scanData = DUMMY_SCAN_HISTORY[index];

            scanDetailsTitle.textContent = `Flights detected during scan at ${scanData.timestamp}`;
            currentModalFlightsData = JSON.parse(JSON.stringify(scanData.flights_at_scan)); // Store a copy for sorting

            // Reset modal sort state to default and apply
            modalSort.key = 'altitude_baro';
            modalSort.order = 'asc';
            initializeModalSortArrows(); // Set initial arrow state for modal
            sortAndPopulateModalFlights(); // Populate the table with default sort

            scanDetailsModal.style.display = 'flex';

            // Reset modal position when it's opened
            scanDetailsModalContent.style.left = '50%';
            scanDetailsModalContent.style.top = '50%';
            scanDetailsModalContent.style.transform = 'translate(-50%, -50%)';
            xOffset = 0;
            yOffset = 0;
        }
    });

    // --- Draggable Modal Logic (New Feature) ---
    // Variables are already declared globally at the top of the script.

    function dragStart(e) {
        if (e.button === 0) { // Only left click
            isDragging = true;
            initialX = e.clientX - xOffset;
            initialY = e.clientY - yOffset;
            scanDetailsModalContent.style.cursor = 'grabbing';
            document.addEventListener('mousemove', drag);
            document.addEventListener('mouseup', dragEnd);
        }
    }

    function drag(e) {
        if (!isDragging) return;
        e.preventDefault();

        xOffset = e.clientX - initialX;
        yOffset = e.clientY - initialY;

        scanDetailsModalContent.style.left = `${xOffset + scanDetailsModalContent.offsetWidth / 2}px`;
        scanDetailsModalContent.style.top = `${yOffset + scanDetailsModalContent.offsetHeight / 2}px`;
        scanDetailsModalContent.style.transform = 'translate(-50%, -50%)'; // Keep centered relative to offset
    }

    function dragEnd() {
        isDragging = false;
        scanDetailsModalContent.style.cursor = 'grab';
        document.removeEventListener('mousemove', drag);
        document.removeEventListener('mouseup', dragEnd);
    }

    // Attach drag listeners to the modal content's title area
    scanDetailsTitle.addEventListener('mousedown', dragStart);


    // Initially show the Live Data tab on page load with dummy data
    showTab('liveData');
    initializeMainSortArrows(); // Set initial arrow state for main table
});
