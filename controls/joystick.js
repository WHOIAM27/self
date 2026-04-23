const joyBase = document.getElementById('joystick-base');
const joyKnob = document.getElementById('joystick-knob');
const joyOutput = document.getElementById('joystick-output');
let isJoyDragging = false;

function joyStart(e) { if (currentMode !== "JOYSTICK") return; isJoyDragging = true; joyKnob.style.transition = "none"; joyMove(e); }
function joyEnd() {
    if (!isJoyDragging) return;
    isJoyDragging = false;
    joyKnob.style.transition = "transform 0.2s ease-out";
    joyKnob.style.transform = `translate(0px, 0px)`;
    joyOutput.textContent = `X: 0 | Y: 0`;
    // Comment this out if it causes errors before ESP32 is connected
    sendToESP32(`JOY:0,0`);
}
function joyMove(e) {
    if (!isJoyDragging || currentMode !== "JOYSTICK") return;
    let clientX = e.touches ? e.touches[0].clientX : e.clientX;
    let clientY = e.touches ? e.touches[0].clientY : e.clientY;
    const rect = joyBase.getBoundingClientRect();
    let deltaX = clientX - (rect.left + rect.width / 2), deltaY = clientY - (rect.top + rect.height / 2);
    const max = (rect.width / 2) - (joyKnob.offsetWidth / 2);
    if (Math.sqrt(deltaX * deltaX + deltaY * deltaY) > max) {
        const angle = Math.atan2(deltaY, deltaX);
        deltaX = Math.cos(angle) * max; deltaY = Math.sin(angle) * max;
    }
    joyKnob.style.transform = `translate(${deltaX}px, ${deltaY}px)`;
    let finalX = Math.round((deltaX / max) * 100);
    let finalY = Math.round((deltaY / max) * -100);
    joyOutput.textContent = `X: ${finalX} | Y: ${finalY}`;

    // Comment this out if it causes errors before ESP32 is connected
    sendToESP32(`JOY:${finalX},${finalY}`);
}
joyBase.addEventListener('mousedown', joyStart); window.addEventListener('mousemove', joyMove); window.addEventListener('mouseup', joyEnd);
joyBase.addEventListener('touchstart', (e) => { e.preventDefault(); joyStart(e); }, { passive: false });
window.addEventListener('touchmove', (e) => { if (isJoyDragging) { e.preventDefault(); joyMove(e); } }, { passive: false });
window.addEventListener('touchend', joyEnd);