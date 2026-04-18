// GLOBAL STATE (var allows child scripts to read these)
var isSystemOn = false;
var currentMode = null;

const btnBalance = document.getElementById('btn-balance');
const btnAuto = document.getElementById('btn-auto');
const modeButtons = document.querySelectorAll('.mode-btn');
const displayText = document.getElementById('display-text');
const zones = {
    "JOYSTICK": document.getElementById('joystick-zone'),
    "ARROW BUTTON": document.getElementById('arrow-zone'),
    "STEERING SLIDER": document.getElementById('slider-zone'),
    "GYROSCOPE": document.getElementById('gyro-zone'),
    "AUTO": document.getElementById('auto-zone')
};

// Simulate Connection
setTimeout(() => { document.getElementById('signal-fill').style.width = "85%"; }, 1000);

function updateDisplay() {
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
        zones[currentMode].style.display = "flex"; 
    } else {
        displayText.textContent = "SYSTEM ACTIVE: AWAITING INPUT MODE";
    }
}

// Master Buttons
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

btnAuto.addEventListener('click', () => {
    if (!isSystemOn) return alert("Turn Balance ON first.");
    btnAuto.classList.toggle('active');
    if (btnAuto.classList.contains('active')) {
        currentMode = "AUTO";
        modeButtons.forEach(btn => btn.classList.remove('active'));
    } else {
        currentMode = null;
    }
    updateDisplay();
});

modeButtons.forEach(button => {
    button.addEventListener('click', () => {
        if (!isSystemOn) return alert("Turn Balance ON first.");
        btnAuto.classList.remove('active'); 
        modeButtons.forEach(btn => btn.classList.remove('active'));
        button.classList.add('active');
        currentMode = button.getAttribute('data-mode'); 
        updateDisplay();
    });
});

// BACKGROUND LIGHTS & GYROSCOPE TRACKING
const orbs = [
    { el: document.querySelector('.orb-1'), x: 0, y: 0, speed: 0.02, offsetX: 0, offsetY: 0 },
    { el: document.querySelector('.orb-2'), x: 0, y: 0, speed: 0.015, offsetX: 300, offsetY: -200 },
    { el: document.querySelector('.orb-3'), x: 0, y: 0, speed: 0.035, offsetX: -250, offsetY: 150 }
];
let targetX = window.innerWidth / 2, targetY = window.innerHeight / 2;

window.addEventListener('mousemove', (e) => { targetX = e.clientX; targetY = e.clientY; });
window.addEventListener('deviceorientation', (e) => {
    let tiltX = e.gamma, tiltY = e.beta;  
    if (tiltX !== null && tiltY !== null) {
        let constrainedX = Math.max(-45, Math.min(45, tiltX));
        let constrainedY = Math.max(0, Math.min(90, tiltY)) - 45; 
        targetX = (window.innerWidth / 2) + (constrainedX / 45) * (window.innerWidth / 2);
        targetY = (window.innerHeight / 2) + (constrainedY / 45) * (window.innerHeight / 2);
    }
});
function animateLights() {
    orbs.forEach(orb => {
        orb.x += ((targetX + orb.offsetX) - orb.x) * orb.speed;
        orb.y += ((targetY + orb.offsetY) - orb.y) * orb.speed;
        orb.el.style.transform = `translate(calc(${orb.x}px - 50%), calc(${orb.y}px - 50%))`;
    });
    requestAnimationFrame(animateLights);
}
animateLights();