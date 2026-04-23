document.addEventListener('DOMContentLoaded', () => {
    const slider = document.getElementById('speed-slider'); 
    
    if (slider) {
        slider.addEventListener('input', (e) => {
            let val = e.target.value; 
            if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,${val}`);
        });

        const snapToZero = () => {
            slider.value = 0;
            if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,0`);
        };

        slider.addEventListener('touchend', snapToZero);
        slider.addEventListener('mouseup', snapToZero);
        slider.addEventListener('touchcancel', snapToZero);
    }
});
