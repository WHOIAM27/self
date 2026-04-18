// ==========================================
// gyroscope.js
// ==========================================
const gyroBubble = document.getElementById('gyro-bubble');
const gyroOutput = document.getElementById('gyro-output');

window.addEventListener('deviceorientation', (e) => {
    if (currentMode !== "GYROSCOPE" || !isSystemOn) return;

    let tiltX = e.gamma; // Left/Right
    let tiltY = e.beta;  // Front/Back

    if (tiltX !== null && tiltY !== null) {
        // Constrain tilts to realistic phone holding angles
        let constrainedX = Math.max(-45, Math.min(45, tiltX));
        let constrainedY = Math.max(-45, Math.min(45, tiltY - 45)); 

        // Convert to -100 to 100 speed
        let motorX = Math.round((constrainedX / 45) * 100);
        let motorY = Math.round((constrainedY / 45) * -100);

        // Move the visual bubble level
        let maxPixelMove = 60; // radius of base - radius of bubble
        let pixX = (motorX / 100) * maxPixelMove;
        let pixY = (motorY / -100) * maxPixelMove;
        gyroBubble.style.transform = `translate(${pixX}px, ${pixY}px)`;

        gyroOutput.textContent = `X: ${motorX} | Y: ${motorY}`;
    }
});