// PondSense Real-Time Dashboard Client

const API_BASE = window.location.origin.includes(":5000") ? "" : "http://localhost:5000";
let telemetryChart = null;

// Initialize Chart.js Trend Chart
function initChart() {
    const ctx = document.getElementById("telemetryChart").getContext("2d");
    
    telemetryChart = new Chart(ctx, {
        type: "line",
        data: {
            labels: [],
            datasets: [
                {
                    label: "Dissolved Oxygen (mg/L)",
                    data: [],
                    borderColor: "#38bdf8",
                    backgroundColor: "rgba(56, 189, 248, 0.1)",
                    borderWidth: 2,
                    tension: 0.4,
                    fill: true,
                    yAxisID: "y"
                },
                {
                    label: "Water Temperature (°C)",
                    data: [],
                    borderColor: "#f59e0b",
                    borderWidth: 1.5,
                    borderDash: [4, 4],
                    tension: 0.4,
                    fill: false,
                    yAxisID: "y1"
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                mode: "index",
                intersect: false
            },
            plugins: {
                legend: {
                    labels: { color: "#94a3b8", font: { size: 12 } }
                }
            },
            scales: {
                x: {
                    grid: { color: "#1e293b" },
                    ticks: { color: "#64748b", font: { size: 11 } }
                },
                y: {
                    type: "linear",
                    display: true,
                    position: "left",
                    min: 0,
                    max: 12,
                    grid: { color: "#1e293b" },
                    ticks: { color: "#38bdf8" },
                    title: { display: true, text: "DO (mg/L)", color: "#38bdf8" }
                },
                y1: {
                    type: "linear",
                    display: true,
                    position: "right",
                    min: 15,
                    max: 40,
                    grid: { drawOnChartArea: false },
                    ticks: { color: "#f59e0b" },
                    title: { display: true, text: "Temp (°C)", color: "#f59e0b" }
                }
            }
        }
    });
}

// Fetch Latest Telemetry Packet
async function fetchLatest() {
    try {
        const res = await fetch(`${API_BASE}/api/telemetry/latest?node_id=1`);
        if (!res.ok) throw new Error("API Offline");
        const data = await res.json();
        updateUI(data);
    } catch (err) {
        document.getElementById("connectionText").textContent = "DISCONNECTED";
        document.getElementById("connectionBadge").style.color = "#ef4444";
        document.getElementById("connectionBadge").style.borderColor = "rgba(239,68,68,0.4)";
    }
}

// Fetch 24-Hour Historical Readings
async function fetchHistory() {
    try {
        const res = await fetch(`${API_BASE}/api/telemetry/history?node_id=1&limit=25`);
        if (!res.ok) return;
        const history = await res.json();
        
        if (history && history.length > 0 && telemetryChart) {
            telemetryChart.data.labels = history.map(r => r.timestamp ? r.timestamp.split(" ")[1] : "TX");
            telemetryChart.data.datasets[0].data = history.map(r => r.dissolved_oxygen);
            telemetryChart.data.datasets[1].data = history.map(r => r.water_temp);
            telemetryChart.update();
        }
    } catch (err) {
        console.warn("Could not load history:", err);
    }
}

// Update DOM Elements with Live Sensor Values
function updateUI(data) {
    // 1. Dissolved Oxygen
    const doVal = data.dissolved_oxygen.toFixed(2);
    document.getElementById("doValue").textContent = doVal;
    const doBar = document.getElementById("doMeterBar");
    const doTag = document.getElementById("doTag");
    const alertBanner = document.getElementById("hypoxiaAlertBanner");
    
    // Percentage for progress bar (0 to 10 mg/L)
    const doPercent = Math.min(100, Math.max(0, (data.dissolved_oxygen / 8.0) * 100));
    doBar.style.width = `${doPercent}%`;

    if (data.dissolved_oxygen < 3.0) {
        doTag.textContent = "CRITICAL HYPOXIA";
        doTag.style.background = "rgba(239, 68, 68, 0.25)";
        doTag.style.color = "#ef4444";
        alertBanner.style.display = "flex";
        document.getElementById("alertMessage").textContent = 
            `CRITICAL HYPOXIA ALERT: DO level is ${doVal} mg/L! Turn on paddlewheel aerators immediately!`;
    } else if (data.dissolved_oxygen < 4.5) {
        doTag.textContent = "CAUTION";
        doTag.style.background = "rgba(245, 158, 11, 0.2)";
        doTag.style.color = "#f59e0b";
        alertBanner.style.display = "none";
    } else {
        doTag.textContent = "OPTIMAL";
        doTag.style.background = "rgba(16, 185, 129, 0.15)";
        doTag.style.color = "#10b981";
        alertBanner.style.display = "none";
    }

    // 2. pH Level
    document.getElementById("phValue").textContent = data.ph.toFixed(2);
    const phTag = document.getElementById("phTag");
    if (data.ph < 6.5) {
        phTag.textContent = "ACIDIC STRESS";
        phTag.style.color = "#f59e0b";
    } else if (data.ph > 8.5) {
        phTag.textContent = "ALKALINE / TOXIC NH3";
        phTag.style.color = "#ef4444";
    } else {
        phTag.textContent = "OPTIMAL";
        phTag.style.color = "#10b981";
    }

    // 3. Water Temp
    document.getElementById("tempValue").textContent = data.water_temp.toFixed(1);
    const satDo = (14.652 - (0.41022 * data.water_temp) + (0.007991 * Math.pow(data.water_temp, 2))).toFixed(1);
    document.getElementById("maxSatText").textContent = `Max Saturation Capacity: ~${satDo} mg/L`;

    // 4. Turbidity
    document.getElementById("turbValue").textContent = data.turbidity.toFixed(1);

    // 5. Diagnostics
    document.getElementById("batValue").textContent = `${data.battery_voltage.toFixed(2)} V`;
    document.getElementById("rssiValue").textContent = `${data.rssi} dBm`;
    document.getElementById("snrValue").textContent = `${data.snr.toFixed(1)} dB`;
    document.getElementById("lastTxValue").textContent = data.timestamp ? data.timestamp.split(" ")[1] : "Just now";
}

function dismissAlert() {
    document.getElementById("hypoxiaAlertBanner").style.display = "none";
}

// Auto-refresh loops
window.addEventListener("DOMContentLoaded", () => {
    initChart();
    fetchLatest();
    fetchHistory();
    setInterval(fetchLatest, 3000);   // Live card polling every 3 seconds
    setInterval(fetchHistory, 15000);  // Chart update every 15 seconds
});
