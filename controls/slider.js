const speedSlider = document.getElementById('speed-slider');
const sliderOutput = document.getElementById('slider-output');
const dirForward = document.getElementById('dir-forward');
const dirBackward = document.getElementById('dir-backward');

function updateSpeedSlider() {
    if (currentMode !== "STEERING SLIDER") return;
    let val = parseInt(speedSlider.value);
    sliderOutput.textContent = Math.abs(val) + '%';
    
    if (val > 0) {
        dirForward.classList.add('active-forward'); dirBackward.classList.remove('active-backward');
        sliderOutput.style.color = '#2ecc71'; speedSlider.classList.add('thumb-forward'); speedSlider.classList.remove('thumb-backward');
    } else if (val < 0) {
        dirForward.classList.remove('active-forward'); dirBackward.classList.add('active-backward');
        sliderOutput.style.color = '#e74c3c'; speedSlider.classList.remove('thumb-forward'); speedSlider.classList.add('thumb-backward');
    } else {
        dirForward.classList.remove('active-forward'); dirBackward.classList.remove('active-backward');
        sliderOutput.style.color = 'var(--text-main)'; speedSlider.classList.remove('thumb-forward'); speedSlider.classList.remove('thumb-backward');
    }
}
function resetSpeedSlider() { if (currentMode === "STEERING SLIDER") { speedSlider.value = 0; updateSpeedSlider(); } }

speedSlider.addEventListener('input', updateSpeedSlider);
speedSlider.addEventListener('change', resetSpeedSlider);
speedSlider.addEventListener('mouseup', resetSpeedSlider);
speedSlider.addEventListener('touchend', resetSpeedSlider);