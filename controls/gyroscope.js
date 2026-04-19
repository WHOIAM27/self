const gyroBubble = document.getElementById('gyro-bubble');
const gyroOutput = document.getElementById('gyro-output');
let lastSentTime = 0;

window.addEventListener('deviceorientation', (e) => {
    if (currentMode !== "GYROSCOPE" || !isSystemOn) return;
    if (e.gamma !== null && e.beta !== null) {
        let conX = Math.max(-45, Math.min(45, e.gamma));
        let conY = Math.max(-45, Math.min(45, e.beta - 45));
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