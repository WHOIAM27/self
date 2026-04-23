document.addEventListener('DOMContentLoaded', () => {
    const btnAuto = document.getElementById('btn-auto');
    let isAuto = false;
    
    if (btnAuto) {
        btnAuto.addEventListener('click', () => {
            isAuto = !isAuto;
            
            if (isAuto) {
                btnAuto.style.backgroundColor = "#3498db"; 
                if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,55`);
            } else {
                btnAuto.style.backgroundColor = "";
                if (typeof sendToESP32 === 'function') sendToESP32(`ARR:0,0`);
            }
        });
    }
});
