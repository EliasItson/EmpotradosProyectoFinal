# Web Server & API REST - Sistema de Estacionamiento Inteligente

## Descripción General

El subsistema de servidor web proporciona una interfaz remota para monitorear y configurar el sistema de estacionamiento en tiempo real. Se ejecuta en el ESP32 y utiliza:

- **AsyncWebServer**: Servidor web asincrónico de alto rendimiento
- **LittleFS**: Sistema de archivos para almacenar HTML/CSS/JS
- **ArduinoJson**: Serialización/deserialización de JSON para la API

## Estructura de Archivos

```
data/www/                  # Archivos a ser embebidos en LittleFS del ESP32
├── index.html            # Dashboard principal (interfaz web)
├── style.css             # Estilos CSS (responsive, temas)
└── script.js             # JavaScript para comunicación API + actualización DOM

include/
└── webserver.h           # Cabecera del servidor web (inicialización, handlers API)

src/
├── main.cpp              # Firmware principal (NO MODIFICADO - lógica de sensores)
└── [webserver.cpp]       # (Opcional) Implementación separada si crece

web/                       # Desarrollo local (espejo de data/www)
├── index.html
├── style.css
└── script.js
```

## Endpoints de la API REST

### GET /api/getStatus

**Descripción:** Obtiene el estado actual de todos los sensores y actuadores.

**Response (JSON):**
```json
{
  "distancia": 25,                    // cm (lectúra ultrasónica)
  "rfidUID": "1C:21:09:49",          // última tarjeta RFID leída
  "plumaEntrada": true,              // true=abierta, false=cerrada
  "plumaSalida": false,              // true=abierta, false=cerrada
  "cajon1": false,                   // true=ocupado, false=libre
  "cajon2": true,                    // true=ocupado, false=libre
  "disponibles": 1,                  // espacios libres (0-2)
  "uptime": 3600,                    // segundos desde boot
  "temp": 32,                        // temperatura del ESP32 (°C)
  "firmware": "v1.0.0",              // versión de firmware
  "ip": "192.168.1.100"              // dirección IP del ESP32
}
```

**Frecuencia de actualización:** Cada 2 segundos (configurable en script.js)

---

### GET /api/getParams

**Descripción:** Obtiene parámetros configurables del sistema.

**Response (JSON):**
```json
{
  "ULTRASONIC_THRESHOLD": 30,        // cm (distancia para detectar auto)
  "ULTRASONIC_TIMEOUT_MS": 5000,     // ms (timeout para esperar auto)
  "LOWER_BARRIER_WAIT_MS": 3000,     // ms (espera antes bajar pluma)
  "DISPLAY_MESSAGE_MS": 3000         // ms (duración mensajes en pantalla OLED)
}
```

---

### POST /api/setParams

**Descripción:** Actualiza parámetros configurables. Solo los campos presentes serán actualizados.

**Request (JSON):**
```json
{
  "ULTRASONIC_THRESHOLD": 35,
  "ULTRASONIC_TIMEOUT_MS": 6000
}
```

**Response (JSON):**
```json
{
  "status": "success",
  "message": "Parámetros actualizados"
}
```

**Nota:** Los parámetros se almacenan en RAM. Para persistencia entre reinicios, requiere integración EEPROM (fase futura).

---

## Dashboard Web (index.html)

### Secciones

1. **Header**
   - Título del sistema
   - Badge de conexión (verde=conectado, rojo=desconectado)

2. **Estado Actual (📊)**
   - Distancia ultrasónica (cm)
   - Última tarjeta RFID
   - Estado de plumas (entrada/salida)
   - Ocupancia de cajones
   - Espacios disponibles
   - Timestamp de última actualización
   - Botón "Actualizar Ahora"

3. **Configuración (⚙️)**
   - 4 inputs numéricos para parámetros ajustables
   - Botones "Cargar" (GET /api/getParams) y "Guardar" (POST /api/setParams)
   - Mensaje de estado de guardado

4. **Gráfica del Estacionamiento (🏗️)**
   - Representación visual de plumas (entrada/salida)
   - Grid de 2 cajones (códigos de color: verde=libre, rojo=ocupado)
   - Animaciones CSS para mostrar estado

5. **Información del Sistema (ℹ️)**
   - Estado del sistema
   - Versión de firmware
   - Uptime (horas)
   - Temperatura del ESP32

---

## JavaScript (script.js)

### Funciones Principales

#### `actualizarEstado()`
- Realiza GET a `/api/getStatus`
- Parsea respuesta JSON
- Actualiza elementos DOM con valores
- Aplica clases CSS para visualización de estados
- Se ejecuta cada 2 segundos (configurable)

#### `obtenerParametros()`
- Realiza GET a `/api/getParams`
- Rellena inputs del formulario de configuración

#### `guardarParametros()`
- Lee valores de inputs
- Realiza POST a `/api/setParams`
- Muestra mensaje de éxito/error

#### `mostrarStatus(mensaje, tipo)`
- Muestra mensaje temporal con clase CSS (success/error)
- Se oculta después de 3 segundos

---

## Integración con main.cpp

El servidor web accede a variables globales del firmware principal:

```cpp
// Variables que expone main.cpp:
extern float currentDistance;
extern String lastRFIDCard;
extern bool entranceBarrierRaised;
extern bool exitBarrierRaised;
extern bool slotOccupied[];
extern int availableSlots;
extern unsigned long systemUptime;
extern float systemTemperature;

// Variables de configuración (modificables):
extern int ULTRASONIC_THRESHOLD_CONFIG;
extern int ULTRASONIC_TIMEOUT_CONFIG;
extern int LOWER_BARRIER_WAIT_CONFIG;
extern int DISPLAY_MESSAGE_CONFIG;
```

**Nota:** El servidor WEB NO modifica la lógica principal de sensado. Solo lee variables globales y permite actualizar constantes de configuración.

---

## Tecnologías Utilizadas

- **ESP Async WebServer v1.2.7**: Servidor HTTP/WebSocket asincrónico
- **ArduinoJson v7.0.0**: Procesamiento de JSON
- **LittleFS**: Sistema de archivos embebido (datos estáticos)

---

## Compilación y Carga de Firmware

### Con PlatformIO

```bash
# Compilar
platformio run -e esp32doit-devkit-v1

# Subir firmware + LittleFS
platformio run -e esp32doit-devkit-v1 -t upload
platformio run -e esp32doit-devkit-v1 -t uploadfs
```

### Con Arduino IDE (alternativo)

1. Instalar librerías: ESPAsyncWebServer, ArduinoJson
2. Cargar `src/main.cpp`
3. Usar herramienta LittleFS Uploader para `data/www/`

---

## Configuración de Red

### Conexión WiFi (futura fase)

El ESP32 debe conectarse a una red WiFi. Una vez implementada:

```cpp
const char* ssid = "MI_RED";
const char* password = "MI_PASSWORD";

WiFi.begin(ssid, password);
```

**IP del ESP32:** Se obtiene dinámicamente (mDNS recomendado: `http://estacionamiento.local`)

### Acceso Remoto

El dashboard web está disponible en:
```
http://<IP_DEL_ESP32>/
```

---

## Troubleshooting

### "Error: No se pudo montar LittleFS"
- Verificar que `board_build.filesystem = littlefs` está en platformio.ini
- Subir LittleFS con `platformio run -t uploadfs`

### "Desconectado" en el dashboard
- Verificar dirección IP en script.js (`const ESP32_IP`)
- Asegurar que el ESP32 está en la misma red
- Revisar serial monitor para logs del servidor

### Los parámetros no se guardan entre reinicios
- Implementar almacenamiento EEPROM en fase 2
- Por ahora, los cambios son solo en RAM

---

## Próximas Fases

1. **Fase 2:** Integración WiFi + mDNS
2. **Fase 3:** Almacenamiento persistente (EEPROM)
3. **Fase 4:** WebSocket para updates en tiempo real
4. **Fase 5:** Autenticación y certificados SSL

---

## Notas Importantes

- ✅ El código principal de sensado (`main.cpp`) **NO HA SIDO MODIFICADO**
- ✅ El servidor web es completamente opcional (puede deshabilitarse en compilación)
- ✅ La API es agnóstica a la fuente de datos (aplica a futuras versiones con múltiples sensores)
- ✅ Los datos mostrados en el dashboard son **en tiempo real** (lectura directa de variables globals)
