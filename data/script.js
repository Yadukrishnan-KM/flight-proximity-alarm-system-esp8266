document.addEventListener('DOMContentLoaded', () => {
    // --- DOM Element References ---
    const liveDataTabBtn = document.getElementById('liveDataTabBtn');
    const settingsTabBtn = document.getElementById('settingsTabBtn');
    const liveDataTabContent = document.getElementById('liveDataTabContent');
    const settingsTabContent = document.getElementById('settingsTabContent');

    const currentOverallStatus = document.getElementById('currentOverallStatus').querySelector('p');
    const level1CountCard = document.getElementById('level1CountCard').querySelector('p');
    const level2CountCard = document.getElementById('level2CountCard').querySelector('p');
    const level3CountCard = document.getElementById('level3CountCard').querySelector('p');
    const totalFlightsCard = document.getElementById('totalFlightsCard').querySelector('p');
    const currentFlightsTableBody = document.getElementById('currentFlightsTable').querySelector('tbody');
    const scanHistoryTableBody = document.getElementById('scanHistoryTable').querySelector('tbody');

    const saveSettingsBtn = document.getElementById('saveSettingsBtn');
    const restoreDefaultsBtn = document.getElementById('restoreDefaultsBtn');
    const rebootEspBtn = document.getElementById('rebootEspBtn');

    const statusModal = document.getElementById('statusModal');
    const statusMessageText = document.getElementById('statusMessageText');
    let statusTimeout; // To clear previous timeouts

    const confirmActionModal = document.getElementById('confirmActionModal');
    const confirmActionTitle = document.getElementById('confirmActionTitle');
    const confirmActionMessage = document.getElementById('confirmActionMessage');
    const confirmActionBtn = document.getElementById('confirmActionBtn');
    const cancelActionBtn = document.getElementById('cancelActionBtn');

    const scanDetailsModal = document.getElementById('scanDetailsModal');
    const scanDetailsTableBody = document.getElementById('scanDetailsTableBody');
    const scanDetailsTitle = document.getElementById('scanDetailsTitle');

    // --- Tab Switching Logic ---
    liveDataTabBtn.addEventListener('click', () => {
        liveDataTabBtn.classList.add('active');
        settingsTabBtn.classList.remove('active');
        liveDataTabContent.classList.add('active');
        settingsTabContent.classList.remove('active');
        fetchLiveData(); // Fetch live data when tab is active
        fetchScanHistory(); // Fetch scan history when tab is active
        startLiveDataUpdateInterval(); // Start polling
    });

    settingsTabBtn.addEventListener('click', () => {
        settingsTabBtn.classList.add('active');
        liveDataTabBtn.classList.remove('active');
        settingsTabContent.classList.add('active');
        liveDataTabContent.classList.remove('active');
        loadSettings(); // Load settings when settings tab is active
        stopLiveDataUpdateInterval(); // Stop polling when not on live data tab
    });

    // --- Status Message Display ---
    function displayStatus(message, type = 'info', duration = 3000) {
        clearTimeout(statusTimeout); // Clear any existing timeout
        statusMessageText.textContent = message;
        statusModal.className = `modal status-modal ${type}`; // Add type for styling
        statusModal.style.display = 'block';
        statusTimeout = setTimeout(() => {
            statusModal.style.display = 'none';
        }, duration);
    }

    // --- Confirmation Modal Logic ---
    let pendingAction = null;

    function showConfirmModal(title, message, actionCallback) {
        confirmActionTitle.textContent = title;
        confirmActionMessage.textContent = message;
        confirmActionModal.style.display = 'block';
        pendingAction = actionCallback; // Store the function to be called on confirmation
    }

    confirmActionBtn.addEventListener('click', () => {
        confirmActionModal.style.display = 'none';
        if (pendingAction) {
            pendingAction();
            pendingAction = null; // Clear pending action
        }
    });

    cancelActionBtn.addEventListener('click', () => {
        confirmActionModal.style.display = 'none';
        pendingAction = null; // Clear pending action
    });

    window.addEventListener('click', (event) => {
        if (event.target === confirmActionModal) {
            confirmActionModal.style.display = 'none';
            pendingAction = null;
        }
        if (event.target === scanDetailsModal) {
            scanDetailsModal.style.display = 'none';
        }
        if (event.target === statusModal) {
            statusModal.style.display = 'none';
            clearTimeout(statusTimeout);
        }
    });

    // --- SETTINGS TAB FUNCTIONS (AJAX/Fetch API Integration) ---

    // Initial settings for UI before fetching from ESP (should match currentSettings defaults or be empty)
    // NOTE: This DEFAULT_SETTINGS is for UI fallback. Actual defaults are on ESP.
    const DEFAULT_FORM_SETTINGS = {
        ssid: "",
        password: "",
        apiServer: "http://api.opensky-network.org/api/states/all",
        apiKey: "",
        latitude: 15.3582, // Example: Dharwad
        longitude: 75.0210,
        radiusLevel1: 30.0, // km
        radiusLevel2: 50.0,
        radiusLevel3: 70.0,
        noFlightScanFreq: 60, // seconds
        flightPresentScanFreq: 10, // seconds
        soundWarning: true
    };

    // Populate form with default JS values if loading from ESP fails
    function populateFormWithDefaults() {
        document.getElementById('ssid').value = DEFAULT_FORM_SETTINGS.ssid;
        document.getElementById('password').value = DEFAULT_FORM_SETTINGS.password;
        document.getElementById('apiServer').value = DEFAULT_FORM_SETTINGS.apiServer;
        document.getElementById('apiKey').value = DEFAULT_FORM_SETTINGS.apiKey;
        document.getElementById('latitude').value = DEFAULT_FORM_SETTINGS.latitude;
        document.getElementById('longitude').value = DEFAULT_FORM_SETTINGS.longitude;
        document.getElementById('radiusLevel1').value = DEFAULT_FORM_SETTINGS.radiusLevel1;
        document.getElementById('radiusLevel2').value = DEFAULT_FORM_SETTINGS.radiusLevel2;
        document.getElementById('radiusLevel3').value = DEFAULT_FORM_SETTINGS.radiusLevel3;
        document.getElementById('noFlightScanFreq').value = DEFAULT_FORM_SETTINGS.noFlightScanFreq;
        document.getElementById('flightPresentScanFreq').value = DEFAULT_FORM_SETTINGS.flightPresentScanFreq;
        document.getElementById('soundWarning').checked = DEFAULT_FORM_SETTINGS.soundWarning;
        displayStatus("Loaded default UI values.", 'info', 3000);
    }

    // --- FETCH SETTINGS FROM ESP8266 ---
    function loadSettings() {
        displayStatus("Loading settings from ESP...", 'info');
        fetch('/getSettings')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json();
            })
            .then(data => {
                // Populate form fields with data from ESP8266
                document.getElementById('ssid').value = data.ssid || '';
                document.getElementById('password').value = data.password || '';
                document.getElementById('apiServer').value = data.apiServer || '';
                document.getElementById('apiKey').value = data.apiKey || '';
                document.getElementById('latitude').value = data.latitude || 0;
                document.getElementById('longitude').value = data.longitude || 0;
                document.getElementById('radiusLevel1').value = data.radiusLevel1 || 0;
                document.getElementById('radiusLevel2').value = data.radiusLevel2 || 0;
                document.getElementById('radiusLevel3').value = data.radiusLevel3 || 0;
                document.getElementById('noFlightScanFreq').value = data.noFlightScanFreq || 0;
                document.getElementById('flightPresentScanFreq').value = data.flightPresentScanFreq || 0;
                document.getElementById('soundWarning').checked = data.soundWarning || false;

                displayStatus("Settings loaded successfully!", 'success');
            })
            .catch(error => {
                console.error("Error loading settings:", error);
                displayStatus("Failed to load settings: " + error.message, 'error', 5000);
                // Fallback to default UI if fetch fails
                populateFormWithDefaults();
            });
    }

    // --- SAVE SETTINGS TO ESP8266 ---
    saveSettingsBtn.addEventListener('click', (event) => {
        event.preventDefault(); // Prevent default form submission
        // Basic validation (can be expanded)
        const latitude = parseFloat(document.getElementById('latitude').value);
        const longitude = parseFloat(document.getElementById('longitude').value);
        const radius1 = parseFloat(document.getElementById('radiusLevel1').value);
        const radius2 = parseFloat(document.getElementById('radiusLevel2').value);
        const radius3 = parseFloat(document.getElementById('radiusLevel3').value);

        if (isNaN(latitude) || latitude < -90 || latitude > 90) { displayStatus("Invalid Latitude!", 'error'); return; }
        if (isNaN(longitude) || longitude < -180 || longitude > 180) { displayStatus("Invalid Longitude!", 'error'); return; }
        if (isNaN(radius1) || radius1 < 1) { displayStatus("Invalid Level 1 Radius!", 'error'); return; }
        if (isNaN(radius2) || radius2 < 1 || radius2 <= radius1) { displayStatus("Invalid Level 2 Radius! Must be > Level 1.", 'error'); return; }
        if (isNaN(radius3) || radius3 < 1 || radius3 <= radius2) { displayStatus("Invalid Level 3 Radius! Must be > Level 2.", 'error'); return; }
        
        const settings = {
            ssid: document.getElementById('ssid').value,
            password: document.getElementById('password').value,
            apiServer: document.getElementById('apiServer').value, // This will always send "opensky" if using select
            apiKey: document.getElementById('apiKey').value,
            latitude: latitude,
            longitude: longitude,
            radiusLevel1: radius1,
            radiusLevel2: radius2,
            radiusLevel3: radius3,
            noFlightScanFreq: parseInt(document.getElementById('noFlightScanFreq').value),
            flightPresentScanFreq: parseInt(document.getElementById('flightPresentScanFreq').value),
            soundWarning: document.getElementById('soundWarning').checked
        };

        displayStatus("Saving settings...", 'info');
        fetch('/saveSettings', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(settings)
        })
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.text();
        })
        .then(data => {
            displayStatus("Settings saved: " + data, 'success', 5000);
        })
        .catch(error => {
            console.error("Error saving settings:", error);
            displayStatus("Failed to save settings: " + error.message, 'error', 5000);
        });
    });

    // --- RESTORE DEFAULTS ON ESP8266 ---
    restoreDefaultsBtn.addEventListener('click', () => {
        showConfirmModal(
            "Confirm Restore Defaults",
            "Are you sure you want to restore default settings? This will require a reboot to take full effect.",
            performRestoreDefaults
        );
    });

    function performRestoreDefaults() {
        displayStatus("Sending restore defaults request...", 'info');
        fetch('/restoreDefaults')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.text();
            })
            .then(data => {
                displayStatus("Defaults restored on ESP: " + data + " Please reboot.", 'success', 6000);
                loadSettings(); // Fetch the newly restored defaults to update UI
            })
            .catch(error => {
                console.error("Error restoring defaults:", error);
                displayStatus("Failed to restore defaults: " + error.message, 'error', 5000);
            });
    }

    // --- REBOOT ESP8266 ---
    rebootEspBtn.addEventListener('click', () => {
        showConfirmModal(
            "Confirm Reboot ESP",
            "Are you sure you want to reboot the ESP8266? The device will become temporarily unavailable.",
            performRebootEsp
        );
    });

    function performRebootEsp() {
        displayStatus("Sending reboot command...", 'info');
        fetch('/rebootESP')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.text();
            })
            .then(data => {
                displayStatus("ESP is rebooting... Page will disconnect. " + data, 'success', 10000);
                setTimeout(() => {
                    window.location.reload(); // Attempt to reload after a delay
                }, 7000); // Give ESP time to reboot (adjust if needed)
            })
            .catch(error => {
                console.error("Error rebooting ESP:", error);
                displayStatus("Failed to reboot ESP: " + error.message, 'error', 5000);
            });
    }

    // --- LIVE DATA TAB FUNCTIONS (AJAX/Fetch API Integration) ---

    let liveDataInterval; // Variable to hold the interval timer

    function startLiveDataUpdateInterval() {
        if (!liveDataInterval) { // Prevent multiple intervals
            liveDataInterval = setInterval(fetchLiveData, 5000); // Poll every 5 seconds
            // Also fetch scan history less frequently, e.g., every 15 seconds
            // if you want separate polling for history
            // setInterval(fetchScanHistory, 15000);
        }
    }

    function stopLiveDataUpdateInterval() {
        if (liveDataInterval) {
            clearInterval(liveDataInterval);
            liveDataInterval = null;
        }
    }

    // Fetch live flight data from ESP8266
    function fetchLiveData() {
        // console.log("Fetching live data...");
        fetch('/getLiveData') // This endpoint needs to be implemented on the ESP
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json();
            })
            .then(data => {
                updateLiveDataUI(data);
            })
            .catch(error => {
                console.error("Error fetching live data:", error);
                // displayStatus("Failed to get live data: " + error.message, 'error', 3000); // Don't spam status
            });
    }

    // Update the UI with fetched live data
    function updateLiveDataUI(data) {
        currentOverallStatus.textContent = data.overallStatus || "ALL CLEAR";
        currentOverallStatus.className = `status-card-text ${getOverallStatusClass(data.overallLevel)}`;

        level1CountCard.textContent = `${data.level1Count} Flights`;
        level2CountCard.textContent = `${data.level2Count} Flights`;
        level3CountCard.textContent = `${data.level3Count} Flights`;
        totalFlightsCard.textContent = `${data.totalFlights} Flights`;

        currentFlightsTableBody.innerHTML = ''; // Clear previous entries
        if (data.currentFlights && data.currentFlights.length > 0) {
            data.currentFlights.forEach(flight => {
                const row = currentFlightsTableBody.insertRow();
                row.className = `proximity-level-${flight.proximity_level}`; // Apply class for styling
                row.innerHTML = `
                    <td>${flight.icao24}</td>
                    <td>${flight.callsign || 'N/A'}</td>
                    <td>${Math.round(flight.altitude_baro)}</td>
                    <td>${Math.round(flight.velocity * 3.6)}</td> <td>${getDirectionArrow(flight.true_track)} ${Math.round(flight.true_track)}°</td>
                    <td>${flight.origin_country || 'N/A'} &rarr; N/A</td>
                    <td>Level ${flight.proximity_level}</td>
                    <td>${flight.distance_km.toFixed(2)}</td>
                `;
            });
        } else {
            currentFlightsTableBody.innerHTML = '<tr class="no-data"><td colspan="8">No flights currently detected within monitoring radius.</td></tr>';
        }
    }

    // Helper for overall status class
    function getOverallStatusClass(level) {
        switch (level) {
            case 1: return 'level-1-alarm';
            case 2: return 'level-2-warning';
            case 3: return 'level-3-detection';
            default: return 'all-clear';
        }
    }


    // Fetch scan history from ESP8266
    function fetchScanHistory() {
        // console.log("Fetching scan history...");
        fetch('/getScanHistory') // This endpoint needs to be implemented on the ESP
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json();
            })
            .then(data => {
                updateScanHistoryUI(data);
            })
            .catch(error => {
                console.error("Error fetching scan history:", error);
                // displayStatus("Failed to get scan history: " + error.message, 'error', 3000); // Don't spam status
            });
    }

    // Update the UI with fetched scan history
    function updateScanHistoryUI(history) {
        scanHistoryTableBody.innerHTML = ''; // Clear previous entries
        if (history && history.length > 0) {
            history.forEach((entry, index) => {
                const row = scanHistoryTableBody.insertRow();
                row.innerHTML = `
                    <td>${entry.timestamp}</td>
                    <td>${entry.level1}</td>
                    <td>${entry.level2}</td>
                    <td>${entry.level3}</td>
                    <td>${entry.total}</td>
                    <td>${entry.status}</td>
                `;
                // Add click event for scan details modal
                row.addEventListener('click', () => showScanDetails(entry));
                row.style.cursor = 'pointer'; // Indicate it's clickable
            });
        } else {
            scanHistoryTableBody.innerHTML = '<tr class="no-data"><td colspan="6">No scan history available yet.</td></tr>';
        }
    }

    // Show details for a specific scan entry in a modal
    function showScanDetails(entry) {
        scanDetailsTitle.textContent = `Scan Details: ${entry.timestamp} (${entry.status})`;
        scanDetailsTableBody.innerHTML = ''; // Clear previous modal entries

        if (entry.flights_at_scan && entry.flights_at_scan.length > 0) {
            entry.flights_at_scan.forEach(flight => {
                const row = scanDetailsTableBody.insertRow();
                row.className = `proximity-level-${flight.proximity_level}`;
                row.innerHTML = `
                    <td>${flight.icao24}</td>
                    <td>${flight.callsign || 'N/A'}</td>
                    <td>${Math.round(flight.altitude_baro)}</td>
                    <td>${Math.round(flight.velocity * 3.6)}</td>
                    <td>${getDirectionArrow(flight.true_track)} ${Math.round(flight.true_track)}°</td>
                    <td>${flight.origin_country || 'N/A'} &rarr; N/A</td>
                    <td>Level ${flight.proximity_level}</td>
                    <td>${flight.distance_km.toFixed(2)}</td>
                `;
            });
        } else {
            scanDetailsTableBody.innerHTML = '<tr class="no-data"><td colspan="8">No flights detected in this scan.</td></tr>';
        }
        scanDetailsModal.style.display = 'block';
    }


    // Helper function to get direction arrow
    function getDirectionArrow(degrees) {
        if (degrees >= 337.5 || degrees < 22.5) return '↑'; // N
        if (degrees >= 22.5 && degrees < 67.5) return '↗'; // NE
        if (degrees >= 67.5 && degrees < 112.5) return '→'; // E
        if (degrees >= 112.5 && degrees < 157.5) return '↘'; // SE
        if (degrees >= 157.5 && degrees < 202.5) return '↓'; // S
        if (degrees >= 202.5 && degrees < 247.5) return '↙'; // SW
        if (degrees >= 247.5 && degrees < 292.5) return '←'; // W
        if (degrees >= 292.5 && degrees < 337.5) return '↖'; // NW
        return '';
    }

    // --- Sorting Table Columns (for Live Data & Scan Details Modals) ---
    function setupTableSorting(tableId, isModal = false) {
        const table = document.getElementById(tableId);
        if (!table) return;

        const headers = table.querySelectorAll('.sortable');
        headers.forEach(header => {
            header.addEventListener('click', () => {
                const key = header.dataset.sortKey;
                let order = header.dataset.sortOrder === 'asc' ? 'desc' : 'asc';

                // Reset other headers
                headers.forEach(h => {
                    if (h !== header) {
                        h.dataset.sortOrder = '';
                        h.querySelector('.sort-indicator').innerHTML = '<span class="arrow-up"></span><span class="arrow-down"></span>';
                    }
                });

                header.dataset.sortOrder = order;
                const indicator = header.querySelector('.sort-indicator');
                indicator.innerHTML = order === 'asc' ? '▲' : '▼'; // Use simple arrows for now

                const tbody = table.querySelector('tbody');
                const rows = Array.from(tbody.rows);

                const sortedRows = rows.sort((a, b) => {
                    const aVal = isModal ? getCellValueModal(a, key) : getCellValue(a, key);
                    const bVal = isModal ? getCellValueModal(b, key) : getCellValue(b, key);

                    if (typeof aVal === 'number' && typeof bVal === 'number') {
                        return order === 'asc' ? aVal - bVal : bVal - aVal;
                    }
                    return order === 'asc' ? aVal.localeCompare(bVal) : bVal.localeCompare(aVal);
                });

                tbody.innerHTML = '';
                sortedRows.forEach(row => tbody.appendChild(row));
            });
        });
    }

    // Helper to get cell value for live data table (based on column index)
    function getCellValue(row, key) {
        switch (key) {
            case 'altitude_baro': return parseFloat(row.cells[2].textContent);
            case 'velocity': return parseFloat(row.cells[3].textContent);
            case 'distance_km': return parseFloat(row.cells[7].textContent);
            default: return row.cells[0].textContent; // Default to first column for string sort
        }
    }

    // Helper to get cell value for modal table (based on column index, assumes same order)
    function getCellValueModal(row, key) {
        switch (key) {
            case 'altitude_baro': return parseFloat(row.cells[2].textContent);
            case 'velocity': return parseFloat(row.cells[3].textContent);
            case 'distance_km': return parseFloat(row.cells[7].textContent);
            default: return row.cells[0].textContent;
        }
    }

    // Initialize sorting for main live data table
    setupTableSorting('currentFlightsTable');
    // Initialize sorting for modal live data table
    setupTableSorting('scanDetailsFlightsTable', true);


    // Initial setup: activate Live Data tab and start fetching
    liveDataTabBtn.click(); // Simulate click on Live Data tab to load content
});
