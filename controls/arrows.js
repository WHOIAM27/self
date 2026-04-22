document.addEventListener('DOMContentLoaded', () => {

    // We assign exact coordinates to each arrow
    const arrows = {
        'btn-up': { x: 0, y: 100 },
        'btn-down': { x: 0, y: -100 },
        'btn-left': { x: -100, y: 0 },
        'btn-right': { x: 100, y: 0 }
    };

    Object.keys(arrows).forEach(id => {
        const btn = document.getElementById(id);
        if (!btn) return;

        // --- PRESS LOGIC ---
        const press = (e) => {
            // Stop the screen from scrolling or zooming
            if (e && e.cancelable) e.preventDefault();

            // Visual Feedback: Make the button shrink and flash green so you KNOW it was tapped!
            btn.style.transform = "scale(0.85)";
            btn.style.backgroundColor = "rgba(46, 204, 113, 0.4)";

            // Send the exact movement string to the ESP32
            if (typeof sendToESP32 === 'function') {
                sendToESP32(`ARR:${arrows[id].x},${arrows[id].y}`);
            }
        };

        // --- RELEASE LOGIC ---
        const release = (e) => {
            if (e && e.cancelable) e.preventDefault();

            // Visual Feedback: Pop the button back to normal
            btn.style.transform = "scale(1)";
            btn.style.backgroundColor = "";

            // Send the Stop command
            if (typeof sendToESP32 === 'function') {
                sendToESP32(`ARR:0,0`);
            }
        };

        // 1. Desktop Mouse Events
        btn.addEventListener('mousedown', press);
        btn.addEventListener('mouseup', release);
        btn.addEventListener('mouseleave', release); // Stops if mouse drags off

        // 2. Mobile Touch Events (The Fix!)
        btn.addEventListener('touchstart', press, { passive: false });
        btn.addEventListener('touchend', release, { passive: false });
        // CRITICAL: Stops the robot if your thumb slides off the edge of the button!
        btn.addEventListener('touchcancel', release, { passive: false });
    });
});