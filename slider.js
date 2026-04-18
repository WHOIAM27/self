// ==========================================
// slider.js - Vertical Speed Control
// ==========================================

const speedSlider = document.getElementById('speed-slider');
const sliderOutput = document.getElementById('slider-output');

// 1. Update the numbers while the user is actively dragging
function updateSlider() {
    // Only run if we are actually in the slider mode
    if (currentMode !== "STEERING SLIDER") return;
    
    let speedValue = speedSlider.value;
    sliderOutput.textContent = `SPEED: ${speedValue}%`;
    
    // NOTE: Send `speedValue` to the ESP32 here!
}

// 2. Snap back to 0 when the user lets go (Auto-Brake)
function resetSlider() {
    if (currentMode !== "STEERING SLIDER") return;
    
    // Reset the physical slider back to center
    speedSlider.value = 0;
    
    // Update the text to show 0
    updateSlider();
    
    // NOTE: Send 0 to the ESP32 here to stop the motors!
}

// Listen for dragging movement
speedSlider.addEventListener('input', updateSlider);

// Listen for the user releasing the mouse or lifting their finger
speedSlider.addEventListener('change', resetSlider); // Mouse let go
speedSlider.addEventListener('mouseup', resetSlider); // Mouse let go
speedSlider.addEventListener('touchend', resetSlider); // Finger lifted off screen