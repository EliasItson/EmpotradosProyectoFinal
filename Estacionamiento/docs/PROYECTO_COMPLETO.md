# Sistema de Estacionamiento Inteligente - Documentación Completa

## 📋 Contenido

1. [Descripción General](#descripción-general)
2. [Arquitectura del Sistema](#arquitectura-del-sistema)
3. [Componentes de Hardware](#componentes-de-hardware)
4. [Estructura del Proyecto](#estructura-del-proyecto)
5. [Guía de Compilación](#guía-de-compilación)
6. [Guía de Configuración](#guía-de-configuración)
7. [Uso del Dashboard Web](#uso-del-dashboard-web)
8. [API REST Endpoints](#api-rest-endpoints)
9. [Troubleshooting](#troubleshooting)
10. [Notas de Desarrollo](#notas-de-desarrollo)

---

## Descripción General

Sistema integral de control de estacionamiento con 2 espacios de parqueo. Utiliza RFID para autenticación, ultrasónica para detección de vehículos, servos para control de barreras y una interfaz web para monitoreo remoto.

**Características principales:**
- ✅ Autenticación por tarjeta RFID
- ✅ Detección de vehículos con sensor ultrasónico
- ✅ Control automático de barreras de entrada/salida
- ✅ Indicadores visuales (LEDs, pantalla OLED)
- ✅ Dashboard web responsive
- ✅ API REST para configuración remota
- ✅ Monitoreo en tiempo real

---

## Arquitectura del Sistema

### Visión General

```
┌─────────────────────────────────────────────────────────┐
│                   ESTACIONAMIENTO                       │
│                                                         │
│  ┌────────────────────────────────────────────────┐   │
│  │         ESP32 (Microcontrolador)               │   │
│  │  ┌──────────────────────────────────────────┐ │   │
│  │  │  FIRMWARE PRINCIPAL (main.cpp)           │ │   │
│  │  │  - Sensado (RFID, ultrasónica)          │ │   │
│  │  │  - Control de actuadores (servos, LEDs) │ │   │
│  │  │  - Lógica de estacionamiento            │ │   │
│  │  └──────────────────────────────────────────┘ │   │
│  │                    ↕                            │   │
│  │  ┌──────────────────────────────────────────┐ │   │
│  │  │  SERVIDOR WEB (webserver.h)              │ │   │
│  │  │  - AsyncWebServer en puerto 80           │ │   │
│  │  │  - Sirve dashboard (HTML/CSS/JS)         │ │   │
│  │  │  - API REST endpoints                    │ │   │
│  │  └──────────────────────────────────────────┘ │   │
│  └────────────────────────────────────────────────┘   │
│         ↕                                              │
│  ┌─────────────────────────────────────────────────┐  │
│  │  SENSORES & ACTUADORES                          │  │
│  │  - RFID (MFRC522)                               │  │
│  │  - Ultrasónica (HC-SR04)                        │  │
│  │  - Servos (entrada, salida)                     │  │
│  │  - LEDs (ocupancia)                             │  │
│  │  - OLED SSD1306                                 │  │
│  │  - DIP Switches (ocupancia manual)              │  │
│  └─────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
         ↕                                    ↕
    [Red WiFi]                          [Usuarios]
         ↓                                    ↓
    [Cliente TCP]                     [Dashboard Web]
         ↓                                    ↓
    [Servidor PC]                     [Navegador]
     (Futuro)                         (Cualquier dispositivo)
```

### Subsistemas

#### 1. **Firmware Principal (main.cpp)**
- Controlador de lógica del estacionamiento
- Lectura de sensores (RFID, ultrasónica, switches)
- Control de actuadores (servos, LEDs, pantalla)
- **NO MODIFICADO** - funcionalidad preservada
- Exporta variables globales para acceso de la API

#### 2. **Servidor Web (webserver.h)**
- AsyncWebServer en puerto 80
- Sirve archivos estáticos desde LittleFS (HTML/CSS/JS)
- API REST con 3 endpoints:
  - `GET /api/getStatus` - Estado actual
  - `GET /api/getParams` - Parámetros configurables
  - `POST /api/setParams` - Actualizar parámetros

#### 3. **Dashboard Web (HTML/CSS/JS)**
- Interfaz responsive multi-plataforma
- Actualización automática en tiempo real
- Visualización gráfica del estacionamiento
- Panel de configuración de parámetros

---

## Componentes de Hardware

| Componente | Modelo | Puerto/Pin | Descripción |
|-----------|--------|-----------|------------|
| **MCU** | ESP32 DOIT v1 | - | Microcontrolador principal |
| **Pantalla** | SSD1306 OLED 128x64 | I2C (SDA=21, SCL=22) | Feedback visual OLED |
| **RFID** | MFRC522 | SPI (SS=5, RST=4) | Lector de tarjetas RFID |
| **Ultrasónica** | HC-SR04 | GPIO14 (TRIG), GPIO33 (ECHO) | Detector de vehículos |
| **Servo Entrada** | Standard | GPIO13 | Barrera de entrada |
| **Servo Salida** | Standard | GPIO15 | Barrera de salida |
| **LED Rojo 1** | LED | GPIO25 | Indicador cajón 1 |
| **LED Rojo 2** | LED | GPIO26 | Indicador cajón 2 |
| **Switch 1** | DIP/Push | GPIO32 (PULLUP) | Ocupancia cajón 1 |
| **Switch 2** | DIP/Push | GPIO27 (PULLUP) | Ocupancia cajón 2 |

---

## Estructura del Proyecto

```
Estacionamiento/
├── platformio.ini              # Configuración de PlatformIO
├── include/
│   ├── config.h               # Configuración centralizada (pins, mensajes, timing)
│   └── webserver.h            # Servidor web (AsyncWebServer + API)
├── src/
│   ├── main.cpp               # Firmware principal (sensado + actuadores)
│   └── rfid_register.cpp      # Utilidad para registrar nuevas tarjetas RFID
├── data/
│   └── www/                   # Sistema de archivos LittleFS (para ESP32)
│       ├── index.html         # Dashboard principal
│       ├── style.css          # Estilos CSS
│       └── script.js          # Lógica del dashboard (actualización, API)
├── web/                        # Desarrollo local (espejo de data/www)
│   ├── index.html
│   ├── style.css
│   └── script.js
├── python/                     # Subsistema Python (futuro)
│   ├── collector.py           # Servidor TCP de recolección de datos
│   ├── main_gui.py            # GUI con estadísticas
│   └── database.py            # Módulo de base de datos (MySQL)
├── docs/
│   ├── WEB_API_DOCUMENTATION.md  # Documentación detallada de API
│   ├── DIAGRAMA_FLUJO.md         # Diagrama de flujo del sistema
│   └── README.md                 # Este archivo
├── test/
└── CAMBIOS_NODELAY.md, INDICE.md, etc. (documentación del proyecto)
```

---

## Guía de Compilación

### Requisitos

- **Software:**
  - PlatformIO (extensión VS Code recomendada)
  - Python 3.8+
  - Node.js (opcional, para herramientas auxiliares)

- **Dependencias (librerías ESP32):**
  - NoDelay 2.2.0
  - Servo 1.3.0
  - Adafruit SSD1306 2.5.7
  - Adafruit GFX Library 1.11.7
  - MFRC522 1.4.12
  - ESP Async WebServer 1.2.7
  - ArduinoJson 7.0.0

### Pasos

1. **Clonar/Descargar el proyecto:**
   ```bash
   cd Estacionamiento
   ```

2. **Compilar el firmware:**
   ```bash
   platformio run -e esp32doit-devkit-v1
   ```

3. **Compilar + cargar firmware al ESP32:**
   ```bash
   platformio run -e esp32doit-devkit-v1 -t upload
   ```

4. **Cargar sistema de archivos (LittleFS) con archivos web:**
   ```bash
   platformio run -e esp32doit-devkit-v1 -t uploadfs
   ```

5. **Verificar en serial monitor:**
   ```bash
   platformio device monitor -e esp32doit-devkit-v1 -b 115200
   ```

   Debería ver mensajes como:
   ```
   [WEB] LittleFS montado correctamente
   [WEB] Servidor web iniciado en puerto 80
   ```

---

## Guía de Configuración

### 1. Configuración de Pins (config.h)

Editar `include/config.h` para cambiar pines si es necesario:

```cpp
// Ejemplo: cambiar pin del LED rojo 1
#define LED_RED_SLOT1 25  // Cambiar a GPIO que prefieras
```

### 2. Parámetros de Operación

En `config.h` se encuentran:

```cpp
// Timing (todos en ms)
#define DISPLAY_MESSAGE_MS 3000         // Duración de mensajes
#define ULTRASONIC_TIMEOUT_MS 5000      // Timeout para detectar auto
#define LOWER_BARRIER_WAIT_MS 3000      // Espera antes bajar pluma

// Umbral de distancia para detección
#define ULTRASONIC_THRESHOLD 30         // Distancia en cm para detectar auto

// Mensajes OLED
#define MSG_WELCOME_1 "Bienvenido"
#define MSG_WELCOME_2 "Estacionamiento"
```

### 3. Tarjetas RFID Autorizadas

Las tarjetas autorizadas se definen en `config.h`:

```cpp
static const char* AUTHORIZED_CARDS[] = {
    "1C:21:09:49",  // Tarjeta 1
    "43:23:7A:1A"   // Tarjeta 2
};
static const int AUTHORIZED_CARDS_COUNT = 2;
```

**Para registrar una nueva tarjeta:**

1. Descomentar en `platformio.ini`: `build_flags = -D RFID_REGISTER`
2. Compilar y cargar en ESP32
3. Acercar la tarjeta al lector RFID
4. Ver el UID en el Serial Monitor (formato: `XX:XX:XX:XX`)
5. Copiar el UID a `config.h` en el array `AUTHORIZED_CARDS`
6. Comentar nuevamente `RFID_REGISTER` y recompilar

### 4. Configuración de Red WiFi (Futuro)

Cuando se implemente WiFi, los datos de conexión irán en:

```cpp
// En setup() o en un archivo de configuración
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD";
WiFi.begin(ssid, password);
```

---

## Uso del Dashboard Web

### Acceso

Una vez que el ESP32 esté ejecutando el firmware y conectado a la red:

```
http://<IP_DEL_ESP32>/
```

Donde `<IP_DEL_ESP32>` es la dirección IP asignada al ESP32 (se mostrará en el Serial Monitor).

### Secciones del Dashboard

#### 📊 Estado Actual
- **Distancia Ultrasónica:** Lectura en tiempo real del sensor HC-SR04
- **Última Tarjeta RFID:** UID de la última tarjeta leída
- **Pluma Entrada/Salida:** Estado de las barreras (Abierta/Cerrada)
- **Cajón 1/2:** Ocupancia de los espacios de parqueo
- **Espacios Disponibles:** Contador de lugares libres (0-2)
- **Última Actualización:** Timestamp de la última lectura
- **Botón "Actualizar Ahora":** Fuerza una actualización inmediata

#### ⚙️ Configuración
4 inputs numéricos para ajustar parámetros del sistema:

- **Umbral Ultrasónico (cm):** Distancia para detectar auto (default: 30)
- **Timeout Ultrasónico (ms):** Tiempo máximo de espera (default: 5000)
- **Espera Bajada Pluma (ms):** Tiempo antes de cerrar barrera (default: 3000)
- **Duración Mensajes (ms):** Duración de avisos en pantalla OLED (default: 3000)

Botones:
- **Cargar:** Recupera los valores actuales desde el ESP32
- **Guardar:** Envía los cambios al ESP32 y muestra confirmación

#### 🏗️ Gráfica del Estacionamiento
Representación visual:
- **Barreras:** Animación de apertura/cierre de plumas
- **Cajones:** Grid con estados (🟢 libre, 🔴 ocupado)
- **Colores en tiempo real:** Cambios inmediatos según estado del sistema

#### ℹ️ Información del Sistema
- **Estado:** Operativo/Error
- **Versión Firmware:** v1.0.0
- **Uptime:** Tiempo desde último reinicio
- **Temperatura:** Temperatura interna del ESP32

---

## API REST Endpoints

### GET /api/getStatus

**Retorna el estado actual de todos los sensores y actuadores.**

```bash
curl http://<ESP32_IP>/api/getStatus
```

**Response (200 OK):**
```json
{
  "distancia": 25.5,
  "rfidUID": "1C:21:09:49",
  "plumaEntrada": true,
  "plumaSalida": false,
  "cajon1": false,
  "cajon2": true,
  "disponibles": 1,
  "uptime": 3600,
  "temp": 32,
  "firmware": "v1.0.0",
  "ip": "192.168.1.100"
}
```

**Campos:**
- `distancia` (float): Distancia en cm del sensor ultrasónico
- `rfidUID` (string): UID de la última tarjeta RFID leída
- `plumaEntrada` (bool): true=abierta, false=cerrada
- `plumaSalida` (bool): true=abierta, false=cerrada
- `cajon1` (bool): true=ocupado, false=libre
- `cajon2` (bool): true=ocupado, false=libre
- `disponibles` (int): Número de espacios libres (0-2)
- `uptime` (int): Tiempo de ejecución en segundos
- `temp` (float): Temperatura interna en °C
- `firmware` (string): Versión del firmware
- `ip` (string): Dirección IP del ESP32

---

### GET /api/getParams

**Retorna los parámetros configurables del sistema.**

```bash
curl http://<ESP32_IP>/api/getParams
```

**Response (200 OK):**
```json
{
  "ULTRASONIC_THRESHOLD": 30,
  "ULTRASONIC_TIMEOUT_MS": 5000,
  "LOWER_BARRIER_WAIT_MS": 3000,
  "DISPLAY_MESSAGE_MS": 3000
}
```

---

### POST /api/setParams

**Actualiza parámetros configurables (solo los campos presentes serán modificados).**

```bash
curl -X POST http://<ESP32_IP>/api/setParams \
  -H "Content-Type: application/json" \
  -d '{
    "ULTRASONIC_THRESHOLD": 35,
    "ULTRASONIC_TIMEOUT_MS": 6000
  }'
```

**Request Body (JSON):**
- `ULTRASONIC_THRESHOLD` (int): 5-400 cm
- `ULTRASONIC_TIMEOUT_MS` (int): 1000-20000 ms
- `LOWER_BARRIER_WAIT_MS` (int): 500-5000 ms
- `DISPLAY_MESSAGE_MS` (int): 1000-10000 ms

**Response (200 OK):**
```json
{
  "status": "success",
  "message": "Parámetros actualizados"
}
```

**Response (400 Bad Request):**
```json
{
  "error": "JSON inválido"
}
```

---

## Troubleshooting

### ❌ "No se pudo montar LittleFS"

**Causas:**
- LittleFS no se cargó correctamente
- Partición de flash dañada

**Soluciones:**
1. Verificar que `board_build.filesystem = littlefs` esté en `platformio.ini`
2. Ejecutar: `platformio run -e esp32doit-devkit-v1 -t uploadfs`
3. Borrar flash completamente: `espressif32 flash-erase`
4. Recompilar y cargar

---

### ❌ Dashboard muestra "Desconectado"

**Causas:**
- ESP32 sin dirección IP (WiFi no configurada)
- Problema de red
- Servidor web no iniciado

**Soluciones:**
1. Verificar en Serial Monitor la dirección IP del ESP32
2. Asegurarse que el ESP32 y el navegador están en la misma red
3. Verificar que `initWebServer()` se llama en `setup()` (línea 102 de main.cpp)
4. Intentar acceder a `http://192.168.1.X` (donde X es la IP real del ESP32)

---

### ❌ Los parámetros no se guardan después de reiniciar

**Razón:** Los cambios se almacenan en RAM, no persisten tras apagado.

**Solución (próxima fase):** Implementar almacenamiento EEPROM.

---

### ❌ Servo no se mueve

**Causas:**
- Conexión física defectuosa
- Pin incorrecto en config.h
- Pin PWM no disponible en GPIO especificado

**Soluciones:**
1. Verificar cables servo ↔ ESP32
2. Cambiar pin a otro GPIO con soporte PWM (GPIO 13, 15, 18, 19, etc.)
3. Verificar que el servo tiene alimentación (5V correctamente)

---

### ❌ RFID no detecta tarjetas

**Causas:**
- Antena MFRC522 desalineada
- SPI mal configurada
- Pin CS (SS) incorrecto

**Soluciones:**
1. Mantener la tarjeta RFID cerca de la antena
2. Verificar pines SPI en config.h (SS=5, RST=4)
3. Probar con la utilidad `EJEMPLOS_EXTENSION.cpp` incluida

---

### ❌ Sensor ultrasónico siempre activo/inactivo

**Causas:**
- Umbral de distancia mal configurado
- Sensor defectuoso
- Lógica invvertida

**Soluciones:**
1. Verificar `ULTRASONIC_THRESHOLD` en config.h (default: 30cm)
2. Usar `Debug > Attach` en VS Code para ver valores en tiempo real
3. Ver Serial Monitor: `[US] Distancia: XX cm | Detectado: 1/0`

---

## Notas de Desarrollo

### Cambios Realizados en Esta Sesión

1. ✅ **Creación de servidor web completo:**
   - Implementado `webserver.h` con AsyncWebServer
   - 3 endpoints API REST funcionales
   - LittleFS para servir archivos estáticos

2. ✅ **Dashboard web responsive:**
   - HTML5 semántico con 5 secciones
   - CSS moderno con grid layout y animaciones
   - JavaScript con actualización automática en tiempo real

3. ✅ **Integración firmware-API:**
   - Exportación de variables globales sin modificar lógica
   - Actualización de datos en tiempo real
   - Parámetros configurables remotamente

4. ✅ **Documentación completa:**
   - API REST documentada
   - Guías de compilación y configuración
   - Troubleshooting detallado

### Próximos Pasos (Futuras Fases)

#### Fase 2: Conectividad WiFi
- [ ] Implementar WiFi.begin() con credenciales
- [ ] Integrar mDNS (http://estacionamiento.local)
- [ ] WebSocket para actualizaciones bidireccionales en tiempo real

#### Fase 3: Persistencia de Datos
- [ ] Almacenamiento EEPROM de parámetros configurables
- [ ] Historial de accesos en SPIFFS

#### Fase 4: Sistema Python Distribuido
- [ ] Servidor TCP en PC para recolección de datos
- [ ] Base de datos MySQL
- [ ] GUI con estadísticas, gráficas, reportes

#### Fase 5: Seguridad y Robustez
- [ ] Certificados SSL/TLS
- [ ] Autenticación en dashboard web
- [ ] Rate limiting en API
- [ ] Logs persistentes

### Limitaciones Conocidas

1. **Sin persistencia:** Los parámetros configurados se pierden al apagar
2. **Sin WiFi:** El servidor web solo accesible por IP local, requiere configuración manual
3. **Capacidad limitada:** ESP32 con 4MB flash, cuidado con agregar mucho contenido web

### Debugging

**Ver logs en tiempo real:**
```bash
platformio device monitor -e esp32doit-devkit-v1 -b 115200
```

**Símbolos útiles en Serial Monitor:**
- `[RFID]` - Eventos de tarjeta RFID
- `[US]` - Sensor ultrasónico
- `[WEB]` - Servidor web
- `[SERVO]` - Movimiento de barreras

---

## Licencia

Proyecto de educación embebida - Semestre 2025

---

## Soporte

Para reportar bugs o sugerencias, ver el archivo `INDICE.md` con el historial completo de cambios.

**Última actualización:** Diciembre 2025
