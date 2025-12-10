// Configuración
// Detectar IP del servidor automáticamente usando la misma IP del navegador
const API_URL = `http://${window.location.hostname}`;
const UPDATE_INTERVAL = 2000; // Actualizar cada 2 segundos

let updateTimer = null;

// ========== INICIALIZACIÓN ==========
document.addEventListener('DOMContentLoaded', function() {
  console.log("Página cargada. Inicializando...");
  actualizarEstado();
  obtenerParametros();
  // Auto-actualizar cada UPDATE_INTERVAL ms
  updateTimer = setInterval(actualizarEstado, UPDATE_INTERVAL);
});

// ========== FUNCIONES DE API ==========

/**
 * Actualizar estado de sensores y actuadores
 */
function actualizarEstado() {
  fetch(`${API_URL}/api/getStatus`)
    .then(response => {
      if (!response.ok) throw new Error("No se pudo conectar");
      return response.json();
    })
    .then(data => {
      console.log("Estado recibido:", data);
      
      // Actualizar servidor status
      document.getElementById('serverStatus').textContent = '✓ Conectado';
      document.getElementById('serverStatus').className = 'status-badge connected';
      
      // Actualizar valores
      document.getElementById('distancia').textContent = 
        (data.distancia !== undefined) ? data.distancia + ' cm' : '--';
      
      document.getElementById('rfidUID').textContent = 
        data.rfidUID || '--';
      
      // Pluma entrada
      const plumaEntradaEl = document.getElementById('plumaEntrada');
      if (data.plumaEntrada) {
        plumaEntradaEl.textContent = 'Abierta';
        plumaEntradaEl.className = 'valor estado-abierto';
        document.getElementById('barrierEntrance').classList.add('abierta');
      } else {
        plumaEntradaEl.textContent = 'Cerrada';
        plumaEntradaEl.className = 'valor estado-cerrado';
        document.getElementById('barrierEntrance').classList.remove('abierta');
      }
      
      // Pluma salida
      const plumaSalidaEl = document.getElementById('plumaSalida');
      if (data.plumaSalida) {
        plumaSalidaEl.textContent = 'Abierta';
        plumaSalidaEl.className = 'valor estado-abierto';
        document.getElementById('barrierExit').classList.add('abierta');
      } else {
        plumaSalidaEl.textContent = 'Cerrada';
        plumaSalidaEl.className = 'valor estado-cerrado';
        document.getElementById('barrierExit').classList.remove('abierta');
      }
      
      // Cajón 1
      const cajon1El = document.getElementById('cajon1');
      const slot1El = document.getElementById('slot1');
      if (data.cajon1) {
        cajon1El.textContent = 'Ocupado';
        cajon1El.className = 'valor estado-ocupado';
        slot1El.className = 'slot ocupado';
        slot1El.querySelector('.slot-status').textContent = 'Ocupado';
      } else {
        cajon1El.textContent = 'Libre';
        cajon1El.className = 'valor estado-libre';
        slot1El.className = 'slot libre';
        slot1El.querySelector('.slot-status').textContent = 'Libre';
      }
      
      // Cajón 2
      const cajon2El = document.getElementById('cajon2');
      const slot2El = document.getElementById('slot2');
      if (data.cajon2) {
        cajon2El.textContent = 'Ocupado';
        cajon2El.className = 'valor estado-ocupado';
        slot2El.className = 'slot ocupado';
        slot2El.querySelector('.slot-status').textContent = 'Ocupado';
      } else {
        cajon2El.textContent = 'Libre';
        cajon2El.className = 'valor estado-libre';
        slot2El.className = 'slot libre';
        slot2El.querySelector('.slot-status').textContent = 'Libre';
      }
      
      // Espacios disponibles
      const disponibles = (data.cajon1 ? 0 : 1) + (data.cajon2 ? 0 : 1);
      document.getElementById('disponibles').textContent = disponibles + ' / 2';
      
      // Timestamp
      const ahora = new Date();
      document.getElementById('timestamp').textContent = 
        ahora.toLocaleTimeString('es-ES');
    })
    .catch(error => {
      console.error("Error actualizando estado:", error);
      document.getElementById('serverStatus').textContent = '✗ Desconectado';
      document.getElementById('serverStatus').className = 'status-badge disconnected';
    });
}

/**
 * Obtener parámetros actuales del ESP32
 */
function obtenerParametros() {
  fetch(`${API_URL}/api/getParams`)
    .then(response => response.json())
    .then(data => {
      console.log("Parámetros recibidos:", data);
      
      if (data.ULTRASONIC_THRESHOLD !== undefined)
        document.getElementById('ultrasonicThreshold').value = data.ULTRASONIC_THRESHOLD;
      if (data.ULTRASONIC_TIMEOUT_MS !== undefined)
        document.getElementById('ultrasonicTimeout').value = data.ULTRASONIC_TIMEOUT_MS;
      if (data.LOWER_BARRIER_WAIT_MS !== undefined)
        document.getElementById('lowerBarrierWait').value = data.LOWER_BARRIER_WAIT_MS;
      if (data.DISPLAY_MESSAGE_MS !== undefined)
        document.getElementById('displayMessage').value = data.DISPLAY_MESSAGE_MS;
    })
    .catch(error => console.error("Error obteniendo parámetros:", error));
}

/**
 * Guardar parámetros en el ESP32
 */
function guardarParametros() {
  const params = {
    ULTRASONIC_THRESHOLD: parseInt(document.getElementById('ultrasonicThreshold').value),
    ULTRASONIC_TIMEOUT_MS: parseInt(document.getElementById('ultrasonicTimeout').value),
    LOWER_BARRIER_WAIT_MS: parseInt(document.getElementById('lowerBarrierWait').value),
    DISPLAY_MESSAGE_MS: parseInt(document.getElementById('displayMessage').value)
  };
  
  console.log("Guardando parámetros:", params);
  
  fetch(`${API_URL}/api/setParams`, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(params)
  })
    .then(response => response.json())
    .then(data => {
      console.log("Respuesta:", data);
      mostrarStatus('Parámetros guardados correctamente', 'success');
    })
    .catch(error => {
      console.error("Error guardando parámetros:", error);
      mostrarStatus('Error al guardar parámetros', 'error');
    });
}

/**
 * Mostrar mensaje de estado temporal
 */
function mostrarStatus(mensaje, tipo) {
  const el = document.getElementById('configStatus');
  el.textContent = mensaje;
  el.className = 'status-message ' + tipo;
  
  setTimeout(() => {
    el.className = 'status-message';
  }, 3000);
}

// ========== ACTUALIZACIÓN MANUAL ==========
function actualizarEstadoAhora() {
  clearInterval(updateTimer);
  actualizarEstado();
  updateTimer = setInterval(actualizarEstado, UPDATE_INTERVAL);
}
