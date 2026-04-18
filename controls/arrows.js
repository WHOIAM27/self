const arrowOutput = document.getElementById('arrow-output');
const arrows = { 'btn-up': {x:0, y:100}, 'btn-down': {x:0, y:-100}, 'btn-left': {x:-100, y:0}, 'btn-right': {x:100, y:0} };

Object.keys(arrows).forEach(id => {
    const btn = document.getElementById(id);
    const press = () => { if (currentMode === "ARROW BUTTON") arrowOutput.textContent = `X: ${arrows[id].x} | Y: ${arrows[id].y}`; };
    const release = () => { if (currentMode === "ARROW BUTTON") arrowOutput.textContent = `X: 0 | Y: 0`; };
    btn.addEventListener('mousedown', press); btn.addEventListener('mouseup', release); btn.addEventListener('mouseleave', release);
    btn.addEventListener('touchstart', (e) => { e.preventDefault(); press(); }, {passive: false}); btn.addEventListener('touchend', release);
});