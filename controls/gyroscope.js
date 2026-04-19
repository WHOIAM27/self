const gyroBubble = document.getElementById('gyro-bubble');
const gyroOutput = document.getElementById('gyro-output');
const gyroResetBtn = document.getElementById('btn-gyro-reset');
let lastSentTime = 0;

// Make horizontal (flat) device the neutral (0,0) position
let gyroNeutral = { gamma: 0, beta: 0 };

function resetGyroCalibration() {
    pendingCalibration = true;
    if (gyroResetBtn) {
        gyroResetBtn.style.borderColor = 'var(--accent-green)';
        gyroResetBtn.style.color = 'var(--accent-green)';
        gyroResetBtn.textContent = 'CENTERING...';
    }
}

let pendingCalibration = false;

let forceBubbleCenterFrames = 0;

window.addEventListener('deviceorientation', (e) => {
    if (currentMode !== "GYROSCOPE" || !isSystemOn) return;

    if (pendingCalibration && e.gamma !== null && e.beta !== null) {
        gyroNeutral.gamma = e.gamma;
        gyroNeutral.beta = e.beta;
        pendingCalibration = false;

        // Reset button UI after short delay
        if (gyroResetBtn) {
            gyroResetBtn.textContent = 'CENTER SET';
            setTimeout(() => {
                gyroResetBtn.textContent = 'SET CENTER';
                gyroResetBtn.style.borderColor = '';
                gyroResetBtn.style.color = '';
            }, 1000);
        }

        // Force bubble to center for next 2 frames for visual feedback
        forceBubbleCenterFrames = 2;
        gyroBubble.style.transform = `translate(0px, 0px)`;
        gyroOutput.textContent = `X: 0 | Y: 0`;
        console.log("Gyro Recalibrated:", gyroNeutral);
        return;
    }

    if (forceBubbleCenterFrames > 0) {
        gyroBubble.style.transform = `translate(0px, 0px)`;
        gyroOutput.textContent = `X: 0 | Y: 0`;
        forceBubbleCenterFrames--;
        return;
    }

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