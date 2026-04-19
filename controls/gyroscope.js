const gyroBubble = document.getElementById('gyro-bubble');
const gyroOutput = document.getElementById('gyro-output');
let lastSentTime = 0;

// Make horizontal (flat) device the neutral (0,0) position
let gyroNeutral = { gamma: 0, beta: 0 };
let gyroCalibrated = false;

function calibrateGyro(e) {
    if (!gyroCalibrated && e.gamma !== null && e.beta !== null) {
        gyroNeutral.gamma = e.gamma;
        gyroNeutral.beta = e.beta;
        gyroCalibrated = true;
    }
}

window.addEventListener('deviceorientation', (e) => {
    if (currentMode !== "GYROSCOPE" || !isSystemOn) return;
    if (!gyroCalibrated) calibrateGyro(e);
    if (e.gamma !== null && e.beta !== null) {
        // Subtract neutral values so horizontal = 0,0
        let adjGamma = e.gamma - gyroNeutral.gamma;
        let adjBeta = e.beta - gyroNeutral.beta;
        let conX = Math.max(-45, Math.min(45, adjGamma));
        let conY = Math.max(-45, Math.min(45, adjBeta));
        let mX = Math.round((conX / 45) * 100), mY = Math.round((conY / 45) * -100);
        gyroBubble.style.transform = `translate(${(mX / 100) * 60}px, ${(mY / -100) * 60}px)`;
        gyroOutput.textContent = `X: ${mX} | Y: ${mY}`;

        // Only send to ESP32 every 100ms to avoid flooding the network
        let now = Date.now();
        if (now - lastSentTime > 100) {
            sendToESP32(`GYRO:${mX},${mY}`);
            lastSentTime = now;
        }
    }
});