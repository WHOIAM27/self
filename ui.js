// ==========================================
// ui.js - Master Visual Controller
// ==========================================
const btnBalance = document.getElementById('btn-balance');
const btnAuto = document.getElementById('btn-auto');
const modeButtons = document.querySelectorAll('.mode-btn');
const displayText = document.getElementById('display-text');

// Grab all zones
const zones = {
    "JOYSTICK": document.getElementById('joystick-zone'),
    "ARROW BUTTON": document.getElementById('arrow-zone'),
    "STEERING SLIDER": document.getElementById('slider-zone'),
    "GYROSCOPE": document.getElementById('gyro-zone'),
    "AUTO": document.getElementById('auto-zone')
};

// Global App State
var isSystemOn = false;
var currentMode = null;

setTimeout(() => { document.getElementById('signal-fill').style.width = "85%"; }, 1000);

function updateDisplay() {
    // Hide all zones and reset text
    Object.values(zones).forEach(zone => zone.style.display = "none");
    displayText.style.display = "block";

    if (!isSystemOn) {
        displayText.textContent = "(SYSTEM STANDBY)";
        displayText.style.color = "rgba(255, 255, 255, 0.6)"; 
        return;
    }

    displayText.style.color = "#2ecc71"; 

    if (currentMode === "AUTO") {
        displayText.style.display = "none";
        zones["AUTO"].style.display = "flex";
    } else if (currentMode && zones[currentMode]) {
        displayText.style.display = "none";
        zones[currentMode].style.display = "flex"; // Show the specific control UI
    } else {
        displayText.textContent = "SYSTEM ACTIVE: AWAITING INPUT MODE";
    }
}

// Balance Button
btnBalance.addEventListener('click', () => {
    isSystemOn = !isSystemOn; 
    btnBalance.classList.toggle('active'); 
    if (!isSystemOn) {
        btnAuto.classList.remove('active');
        modeButtons.forEach(btn => btn.classList.remove('active'));
        currentMode = null;
    }
    updateDisplay();
});

// Auto Button (Overwrites Manual Modes)
btnAuto.addEventListener('click', () => {
    if (!isSystemOn) { alert("Turn Balance ON first."); return; }
    
    btnAuto.classList.toggle('active');
    if (btnAuto.classList.contains('active')) {
        currentMode = "AUTO";
        modeButtons.forEach(btn => btn.classList.remove('active'));
    } else {
        currentMode = null;
    }
    updateDisplay();
});

// Manual Mode Buttons
modeButtons.forEach(button => {
    button.addEventListener('click', () => {
        if (!isSystemOn) { alert("Turn Balance ON first."); return; }
        
        btnAuto.classList.remove('active'); // Turn off auto
        modeButtons.forEach(btn => btn.classList.remove('active'));
        
        button.classList.add('active');
        currentMode = button.getAttribute('data-mode'); 
        updateDisplay();
    });
});