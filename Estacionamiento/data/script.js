// Obtiene el estado de sensores y actuadores
function actualizarEstado() {
    fetch("/api/getStatus")
        .then(res => res.json())
        .then(data => {
            document.getElementById("rfid").innerText = "RFID: " + data.rfidUID;
            document.getElementById("distancia").innerText = "Distancia: " + data.distancia + " cm";
            document.getElementById("plumaEntrada").innerText = "Pluma Entrada: " + (data.plumaEntrada ? "Abierta" : "Cerrada");
            document.getElementById("plumaSalida").innerText = "Pluma Salida: " + (data.plumaSalida ? "Abierta" : "Cerrada");
            document.getElementById("cajon1").innerText = "Cajón 1: " + (data.cajon1 ? "Ocupado" : "Libre");
            document.getElementById("cajon2").innerText = "Cajón 2: " + (data.cajon2 ? "Ocupado" : "Libre");
        });
}

// Obtiene los parámetros configurables
function actualizarParametros() {
    fetch("/api/getParams")
        .then(res => res.json())
        .then(data => {
            document.getElementById("tiempoPluma").value = data.TIEMPO_APERTURA_MS;
            document.getElementById("delaySalida").value = data.SALIDA_DELAY_MS;
        });
}

// Envía los parámetros modificados al ESP32
function guardarParametros() {
    let params = {
        TIEMPO_APERTURA_MS: parseInt(document.getElementById("tiempoPluma").value),
        SALIDA_DELAY_MS: parseInt(document.getElementById("delaySalida").value)
    };

    fetch("/api/setParams", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(params)
    }).then(() => {
        alert("Parámetros guardados");
    });
}

// Actualiza cada segundo
setInterval(() => {
    actualizarEstado();
    actualizarParametros();
}, 1000);
