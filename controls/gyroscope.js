document.addEventListener('DOMContentLoaded', () => {
    const btnGyro = document.getElementById('btn-gyro'); // The button to turn tilt on/off
    let gyroActive = false;

    const constrain = (val, min, max) => Math.min(Math.max(val, min), max);

    const handleOrientation = (event) => {
        if (!gyroActive) return;
        
        let pitch = event.beta;  // Front/Back tilt
        let roll = event.gamma;  // Left/Right tilt

        // Map a 30-degree phone tilt to 100% motor speed
        let y = constrain(Math.round((-pitch / 30) * 100), -100, 100);
        let x = constrain(Math.round((roll / 30) * 100), -100, 100);

        if (typeof sendToESP32 === 'function') sendToESP32(`ARR:${x},${y}`);
    };

    if (btnGyro) {
        btnGyro.addEventListener('click', async () => {
            gyroActive = !gyroActive;
            
            if (gyroActive) {
                btnGyro.style.backgroundColor = "#2ecc71"; // Turn button green
                
                // Security check for iPhones (iOS 13+)
                if (typeof DeviceOrientationEvent !== 'undefined' && typeof DeviceOrientationEvent.requestPermission === 'function') {
                    try {
                        const permission = await DeviceOrientationEvent.requestPermission();
                        if (permission === 'granted') {
                            window.addEventListener('deviceorientation', handleOrientation);
                        } else {
                            alert("Gyroscope permission denied by phone.");
                            gyroActive = false;
                            btnGyro.style.backgroundColor = "";
                        }
                    } catch (e) { console.error(e); }
                } else {
                    // Android and older devices
                    window.addEventListener('deviceorientation', handleOrientation);
                }
            } else {
                // Turn Gyro off
                btnGyro.style.backgroundColor = "";
                window.removeEventListener('deviceorientation', handleOrientation);
                if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,0`); // Safety stop
            }
        });
    }
});
