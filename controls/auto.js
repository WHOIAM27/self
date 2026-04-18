const autoOutput = document.getElementById('auto-output');

setInterval(() => {
    if (currentMode === "AUTO") {
        autoOutput.textContent = `AI CORRECTION -> X: ${Math.floor(Math.random()*20)-10} | Y: ${Math.floor(Math.random()*50)+20}`;
    }
}, 1000);