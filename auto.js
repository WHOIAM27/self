// ==========================================
// auto.js
// ==========================================
const autoOutput = document.getElementById('auto-output');

// Just a fun mock effect showing the AI is "thinking"
setInterval(() => {
    if (currentMode === "AUTO") {
        let fakeDecisionX = Math.floor(Math.random() * 20) - 10; // -10 to 10
        let fakeDecisionY = Math.floor(Math.random() * 50) + 20; // 20 to 70 moving forward
        autoOutput.textContent = `AI CORRECTION -> X: ${fakeDecisionX} | Y: ${fakeDecisionY}`;
    }
}, 1000);