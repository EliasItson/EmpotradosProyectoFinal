# Estacionamiento Inteligente

Sistema de gestión de estacionamiento basado en ESP32 con interfaz web y aplicación Python para monitoreo y control remoto.

## Estructura del Proyecto

```
Estacionamiento/
├── src/                       # Código fuente del firmware ESP32
│   └── main.cpp               # Lógica principal del microcontrolador
│
├── include/                   # Headers del proyecto
│   └── config.h               # Configuración de pines y parámetros
│
├── data/                      # Archivos para LittleFS (memoria flash ESP32)
│   ├── index.html             # Página web principal
│   ├── style.css              # Estilos CSS
│   ├── script.js              # Lógica JavaScript del cliente
│   └── config.json            # Configuración persistente en flash
│
├── pc/                        # Aplicaciones Python para PC
│   ├── main_gui.py            # GUI de monitoreo y control
│   ├── collector.py           # Recolector de datos telemetría
│   └── setup_db.py            # Script de inicialización de base de datos
│
├── lib/                       # Librerías externas (gestionadas por PlatformIO)
├── .pio/                      # Compilados PlatformIO (no incluir en git)
├── .venv/                     # Entorno virtual Python (no incluir en git)
├── .vscode/                   # Configuración VS Code
│
├── platformio.ini             # Configuración de PlatformIO (ESP32)
├── requirements.txt           # Dependencias Python (pip)
├── .gitignore                 # Archivos ignorados por git
└── README.md                  # Este archivo
```

## Requisitos

### Hardware
- ESP32 DOIT DEVKIT V1
- Servos (entrada/salida)
- Sensor ultrasónico
- Lector RFID (MFRC522)
- Pantalla OLED (SSD1306)
- Conexión WiFi

### Software - Firmware (ESP32)
- PlatformIO + Arduino Framework
- Librerías: AsyncWebServer, ArduinoJson, LittleFS, NoDelay, MFRC522, Adafruit SSD1306

### Software - PC
- Python 3.8+
- MySQL Server (localhost)
- Ver `requirements.txt` para dependencias Python

## Instalación

### 1. Configurar ESP32
```bash
# Instalar PlatformIO
pip install platformio

# Compilar y subir firmware
pio run --target upload --environment esp32doit-devkit-v1

# Subir archivos a LittleFS
pio run --target uploadfs --environment esp32doit-devkit-v1
```

### 2. Configurar Base de Datos (MySQL)
```bash
python pc/setup_db.py
```

### 3. Instalar dependencias Python
```bash
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

## Uso

### Interfaz Web
1. Conectar ESP32 a WiFi
2. Acceder a localhost
3. Ver estado en tiempo real y configurar parámetros

### GUI de Monitoreo (Python)
```bash
python pc/main_gui.py
```

### Recolector de Telemetría
```bash
python pc/collector.py
```

## Parámetros Configurables

- **SALIDA_DELAY_MS**: Tiempo de espera antes de cerrar pluma de salida (ms)
- **ULTRASONIC_TIMEOUT_MS**: Timeout para sensor ultrasónico (ms)

Estos se pueden configurar desde:
- Interfaz web: `http://192.168.100.91`
- API REST: `POST /api/setParams`
- GUI Python: Pestaña "Parámetros"

## Endpoints API

- `GET /api/getStatus` - Obtener estado actual
- `GET /api/getParams` - Obtener parámetros configurables
- `POST /api/setParams` - Establecer parámetros

## Telemetría y Base de Datos

Los eventos se registran automáticamente:
- Lecturas de sensores
- Eventos de barrera (entrada/salida)
- Acceso RFID
- Cambios de configuración
- Ocupación de espacios

Base de datos: `estacionamiento` (localhost)
- Tabla: `sensor_readings`
- Tabla: `barrier_events`
- Tabla: `rfid_access`
- Tabla: `config_changes`
- Tabla: `slot_occupancy`
- Tabla: `system_events`

## Licencia

Proyecto educativo - ITSON Empotratos 2025
