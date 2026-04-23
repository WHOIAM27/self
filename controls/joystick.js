document.addEventListener('DOMContentLoaded', () => {
    const joyBase = document.getElementById('joystick-base');
    const joyKnob = document.getElementById('joystick-knob');
    const joyOutput = document.getElementById('joystick-output');

    // Safety check: If the HTML elements are missing, don't crash
    if (!joyBase || !joyKnob) return;

    let isJoyDragging = false;

    function joyStart(e) {
        // REMOVED the "currentMode" check so the joystick always works!
        isJoyDragging = true;
        joyKnob.style.transition = "none";
        joyMove(e);
    }

    function joyEnd() {
        if (!isJoyDragging) return;
        isJoyDragging = false;
        joyKnob.style.transition = "transform 0.2s ease-out";
        joyKnob.style.transform = `translate(0px, 0px)`;

        if (joyOutput) joyOutput.textContent = `X: 0 | Y: 0`;

        // Safely send the STOP command to the ESP32
        if (typeof sendToESP32 === 'function') {
            sendToESP32(`ARR:0,0`);
        }
    }

    function joyMove(e) {
        if (!isJoyDragging) return;

        let clientX = e.touches ? e.touches[0].clientX : e.clientX;
        let clientY = e.touches ? e.touches[0].clientY : e.clientY;

        const rect = joyBase.getBoundingClientRect();
        let deltaX = clientX - (rect.left + rect.width / 2);
        let deltaY = clientY - (rect.top + rect.height / 2);

        const max = (rect.width / 2) - (joyKnob.offsetWidth / 2);

        if (Math.sqrt(deltaX * deltaX + deltaY * deltaY) > max) {
            const angle = Math.atan2(deltaY, deltaX);
            deltaX = Math.cos(angle) * max;
            deltaY = Math.sin(angle) * max;
        }

        joyKnob.style.transform = `translate(${deltaX}px, ${deltaY}px)`;

        let finalX = Math.round((deltaX / max) * 100);
        let finalY = Math.round((deltaY / max) * -100);

        if (joyOutput) joyOutput.textContent = `X: ${finalX} | Y: ${finalY}`;

        // Safely send the LIVE MOVEMENT command to the ESP32 using 'ARR'
        if (typeof sendToESP32 === 'function') {
            sendToESP32(`ARR:${finalX},${finalY}`);
        }
    }

    // --- Mouse Listeners (For PC testing) ---
    joyBase.addEventListener('mousedown', joyStart);
    window.addEventListener('mousemove', joyMove);
    window.addEventListener('mouseup', joyEnd);

    // --- Touch Listeners (For Mobile) ---
    joyBase.addEventListener('touchstart', (e) => {
        e.preventDefault(); // Stops the screen from scrolling
        joyStart(e);
    }, { passive: false });

    window.addEventListener('touchmove', (e) => {
        if (isJoyDragging) {
            e.preventDefault();
            joyMove(e);
        }
    }, { passive: false });

    window.addEventListener('touchend', joyEnd);
    window.addEventListener('touchcancel', joyEnd); // Safety: if finger slides off screen
});