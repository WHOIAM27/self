document.addEventListener('DOMContentLoaded', () => {
    // These IDs must match the HTML elements of your joystick zone and the movable knob
    const zone = document.getElementById('joystick-zone'); 
    const knob = document.getElementById('joystick-knob');
    if (!zone || !knob) return;

    let active = false;
    let center = { x: 0, y: 0 };
    let radius = zone.offsetWidth / 2 || 75; // The max distance the knob can travel

    const start = (e) => {
        active = true;
        const rect = zone.getBoundingClientRect();
        // Find the exact center of the joystick pad
        center = { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
        move(e);
    };

    const move = (e) => {
        if (!active) return;
        if (e.cancelable) e.preventDefault(); // Stop screen scrolling
        
        let clientX = e.touches ? e.touches[0].clientX : e.clientX;
        let clientY = e.touches ? e.touches[0].clientY : e.clientY;

        let dx = clientX - center.x;
        let dy = clientY - center.y;
        let distance = Math.sqrt(dx * dx + dy * dy);

        // Keep the knob inside the circle
        if (distance > radius) {
            dx = (dx / distance) * radius;
            dy = (dy / distance) * radius;
        }

        // Move the physical knob graphic
        knob.style.transform = `translate(${dx}px, ${dy}px)`;

        // Map movement to -100 to +100 for the ESP32
        let mapX = Math.round((dx / radius) * 100);
        let mapY = Math.round(-(dy / radius) * 100); // Inverted so pushing UP is positive

        if (typeof sendToESP32 === 'function') sendToESP32(`ARR:${mapX},${mapY}`);
    };

    const stop = (e) => {
        active = false;
        // Snap back to center
        knob.style.transform = `translate(0px, 0px)`;
        // Send stop command
        if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,0`);
    };

    // Mobile touch events
    zone.addEventListener('touchstart', start, { passive: false });
    zone.addEventListener('touchmove', move, { passive: false });
    zone.addEventListener('touchend', stop);
    zone.addEventListener('touchcancel', stop);
    
    // PC mouse events (for testing)
    zone.addEventListener('mousedown', start);
    window.addEventListener('mousemove', move);
    window.addEventListener('mouseup', stop);
});
  