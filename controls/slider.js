document.addEventListener('DOMContentLoaded', () => {
    // Assumes you have an HTML slider: <input type="range" id="speed-slider" min="-100" max="100" value="0">
    const slider = document.getElementById('speed-slider'); 
    
    if (slider) {
        // Send live data as you drag it
        slider.addEventListener('input', (e) => {
            let val = e.target.value; 
            // Send X as 0, and Y as the slider value
            if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,${val}`);
        });

        // SAFETY: Snap back to zero when you let go
        const snapToZero = () => {
            slider.value = 0;
            if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,0`);
        };

        slider.addEventListener('touchend', snapToZero);
        slider.addEventListener('mouseup', snapToZero);
        slider.addEventListener('touchcancel', snapToZero);
    }
});
