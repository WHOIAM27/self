document.addEventListener('DOMContentLoaded', () => {
    const btnGyro = document.getElementById('btn-gyro'); 
    let gyroActive = false;

    const constrain = (val, min, max) => Math.min(Math.max(val, min), max);

    const handleOrientation = (event) => {
        if (!gyroActive) return;
        
        let pitch = event.beta;  
        let roll = event.gamma;  

        let y = constrain(Math.round((-pitch / 30) * 100), -100, 100);
        let x = constrain(Math.round((roll / 30) * 100), -100, 100);

        if (typeof sendToESP32 === 'function') sendToESP32(`ARR:${x},${y}`);
    };

    if (btnGyro) {
        btnGyro.addEventListener('click', async () => {
            gyroActive = !gyroActive;
            
            if (gyroActive) {
                btnGyro.style.backgroundColor = "#2ecc71"; 
                
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
                    window.addEventListener('deviceorientation', handleOrientation);
                }
            } else {
                btnGyro.style.backgroundColor = "";
                window.removeEventListener('deviceorientation', handleOrientation);
                if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,0`); 
            }
        });
    }
});
