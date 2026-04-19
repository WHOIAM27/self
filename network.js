// ==========================================
// network.js - Dual-Mode Connection Logic
// ==========================================

let websocket;
let bleCharacteristic;

const connectStatusBox = document.getElementById('btn-connect');
const signalFillBar = document.getElementById('signal-fill');
const connectionOverlay = document.getElementById('connection-overlay');
const ipInput = document.getElementById('esp-ip');

function toggleOverlay() {
    const isVisible = connectionOverlay.style.display === 'flex';
    connectionOverlay.style.display = isVisible ? 'none' : 'flex';
}

function setupTabs() {
    const tabWifi = document.getElementById('tab-wifi');
    const tabBle = document.getElementById('tab-ble');
    
    tabWifi.addEventListener('click', () => {
        tabWifi.classList.add('active');
        tabBle.classList.remove('active');
        document.getElementById('content-wifi').style.display = 'block';
        document.getElementById('content-ble').style.display = 'none';
    });
    
    tabBle.addEventListener('click', () => {
        tabBle.classList.add('active');
        tabWifi.classList.remove('active');
        document.getElementById('content-ble').style.display = 'block';
        document.getElementById('content-wifi').style.display = 'none';
    });
}

// Helper to quickly fill the IP box from a list (if used)
function setIP(ip) { 
    if(ipInput) ipInput.value = ip; 
}

// --- WI-FI (WEBSOCKET) LOGIC ---
function connectWifi() {
    let ip = ipInput ? ipInput.value : "192.168.4.1";
    if (!ip) ip = "192.168.4.1";

    websocket = new WebSocket(`ws://${ip}/ws`);
    
    websocket.onopen = () => {
        connectStatusBox.textContent = "CONNECTED (WIFI)";
        connectStatusBox.style.color = "#2ecc71";
        signalFillBar.style.width = "100%";
        toggleOverlay();
    };
    
    websocket.onclose = () => { 
        connectStatusBox.textContent = "DISCONNECTED"; 
        connectStatusBox.style.color = "white";
        signalFillBar.style.width = "0%"; 
    };

    websocket.onerror = (error) => {
        console.error("WebSocket Error:", error);
    };
}

// --- BLUETOOTH (BLE) LOGIC ---
async function connectBLE() {
    try {
        const device = await navigator.bluetooth.requestDevice({
            filters: [{ namePrefix: 'AeroBalance' }],
            optionalServices: ['4fafc201-1fb5-459e-8fcc-c5c9c331914b']
        });
        
        const server = await device.gatt.connect();
        const service = await server.getPrimaryService('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
        bleCharacteristic = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26a8');
        
        connectStatusBox.textContent = "CONNECTED (BLE)";
        connectStatusBox.style.color = "#2ecc71";
        signalFillBar.style.width = "100%";
        toggleOverlay();

        device.addEventListener('gattserverdisconnected', () => {
            connectStatusBox.textContent = "DISCONNECTED"; 
            connectStatusBox.style.color = "white";
            signalFillBar.style.width = "0%"; 
            bleCharacteristic = null;
        });

    } catch (e) { 
        console.log("BLE Error:", e); 
    }
}

// --- UNIVERSAL DATA SENDER ---
function sendToESP32(msg) {
    if (websocket && websocket.readyState === 1) {
        websocket.send(msg);
    } else if (bleCharacteristic) {
        const encoder = new TextEncoder();
        bleCharacteristic.writeValue(encoder.encode(msg));
    }
}

// --- INITIALIZATION & EVENT LISTENERS ---
window.addEventListener('DOMContentLoaded', () => {
    setupTabs();
    
    // Open menu when clicking the status box
    connectStatusBox.addEventListener('click', toggleOverlay);

    // Close when clicking the "side" (background)
    connectionOverlay.addEventListener('click', (event) => {
        if (event.target === connectionOverlay) {
            toggleOverlay();
        }
    });
});