# Arquitectura del Sistema - Diagramas de Flujo

## 1. Arquitectura General del Sistema

```
┌─────────────────────────────────────────────────────────────────────┐
│                     SISTEMA DE ESTACIONAMIENTO                      │
│                                                                     │
│  ┌──────────────────────────────────────────────────────────────┐  │
│  │                    ESP32 DOIT DevKit v1                      │  │
│  │  ┌────────────────────────────────────────────────────────┐ │  │
│  │  │           FIRMWARE PRINCIPAL (main.cpp)               │ │  │
│  │  │                                                        │ │  │
│  │  │  • RFID Scanner → Lee tarjetas autorizadas           │ │  │
│  │  │  • Ultrasónica → Detecta presencia de vehículos      │ │  │
│  │  │  • Servos → Controla barreras de entrada/salida      │ │  │
│  │  │  • LEDs → Indica ocupancia de cajones                │ │  │
│  │  │  • OLED SSD1306 → Mensajes y estado                  │ │  │
│  │  │  • DIP Switches → Confirma ocupancia manual          │ │  │
│  │  │                                                        │ │  │
│  │  │  Variables Globales Expuestas:                        │ │  │
│  │  │  - currentDistance (float)                            │ │  │
│  │  │  - lastRFIDCard (String)                              │ │  │
│  │  │  - entranceBarrierRaised (bool)                       │ │  │
│  │  │  - slotOccupied[] (bool[2])                           │ │  │
│  │  │  - systemUptime (unsigned long)                       │ │  │
│  │  │  - systemTemperature (float)                          │ │  │
│  │  └────────────────────────────────────────────────────────┘ │  │
│  │                         ↕                                      │  │
│  │  ┌────────────────────────────────────────────────────────┐ │  │
│  │  │          SERVIDOR WEB (webserver.h)                  │ │  │
│  │  │                                                        │ │  │
│  │  │  • AsyncWebServer (puerto 80)                         │ │  │
│  │  │  • Sirve contenido estático desde LittleFS            │ │  │
│  │  │  • API REST con 3 endpoints                           │ │  │
│  │  │  • Lee variables globales del firmware                │ │  │
│  │  │  • Permite configuración remota                       │ │  │
│  │  │                                                        │ │  │
│  │  │  Endpoints:                                            │ │  │
│  │  │  - GET  /api/getStatus     → Estado actual            │ │  │
│  │  │  - GET  /api/getParams     → Parámetros              │ │  │
│  │  │  - POST /api/setParams     → Actualizar              │ │  │
│  │  │  - GET  /                  → Dashboard HTML            │ │  │
│  │  └────────────────────────────────────────────────────────┘ │  │
│  │                         ↕                                      │  │
│  │  ┌────────────────────────────────────────────────────────┐ │  │
│  │  │      ARCHIVOS WEB (HTML/CSS/JS en data/www/)         │ │  │
│  │  │                                                        │ │  │
│  │  │  • index.html → Dashboard responsive (128KB)          │ │  │
│  │  │  • style.css  → Estilos modernos (350+ líneas)        │ │  │
│  │  │  • script.js  → Actualización automática (300+ líneas)│ │  │
│  │  │                                                        │ │  │
│  │  │  Funcionalidades:                                      │ │  │
│  │  │  - 5 secciones (Estado, Config, Gráfica, Info)        │ │  │
│  │  │  - Actualización en tiempo real (cada 2s)             │ │  │
│  │  │  - Parámetros configurables remotamente               │ │  │
│  │  │  - Visualización gráfica del estacionamiento          │ │  │
│  │  └────────────────────────────────────────────────────────┘ │  │
│  └──────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  SENSORES & ACTUADORES:                                             │
│  ├── MFRC522 (RFID)           [GPIO 5, 4]                          │
│  ├── HC-SR04 (Ultrasónica)    [GPIO 14, 33]                        │
│  ├── Servo Entrada            [GPIO 13]                            │
│  ├── Servo Salida             [GPIO 15]                            │
│  ├── LED Rojo 1               [GPIO 25]                            │
│  ├── LED Rojo 2               [GPIO 26]                            │
│  ├── Switch 1                 [GPIO 32]                            │
│  ├── Switch 2                 [GPIO 27]                            │
│  └── OLED SSD1306             [I2C: SDA=21, SCL=22]                │
└─────────────────────────────────────────────────────────────────────┘
         ↕ WiFi (Futuro)
    Red Local
         ↓
    [Navegador Web]
    [Otros Dispositivos]
```

---

## 2. Flujo de Operación Principal

```
┌─────────────────────────────────────────────────────────────────┐
│                        LOOP PRINCIPAL                           │
│                      (repetido continuamente)                   │
└─────────────────────────────────────────────────────────────────┘
               ↓
    ┌──────────────────────┐
    │ systemUptime = millis()│  ← Actualizar tiempo para API
    └──────────────────────┘
               ↓
    ┌────────────────────────────────┐
    │ updateBarrierLogic()           │
    │ - Maneja fases de entrada      │
    │ - Timeout de ultrasónica       │
    │ - Secuencia de salida          │
    └────────────────────────────────┘
               ↓
    ┌────────────────────────────────┐
    │ updateDisplayLogic()           │
    │ - Restaura pantalla OLED       │
    │ - Maneja timers de mensajes    │
    └────────────────────────────────┘
               ↓
    ┌────────────────────────────────┐
    │ checkRFID()                    │
    │ - Lee tarjeta RFID             │
    │ - Valida autorización          │
    │ - Levanta pluma si OK          │
    │ - Actualiza lastRFIDCard       │
    └────────────────────────────────┘
               ↓
    ┌────────────────────────────────┐
    │ checkUltrasonicSensor()        │
    │ - Lee distancia (HC-SR04)      │
    │ - Detecta auto (< 30cm)        │
    │ - Maneja timeout y bajada      │
    │ - Actualiza currentDistance    │
    └────────────────────────────────┘
               ↓
    ┌────────────────────────────────┐
    │ checkParkingSlots()            │
    │ - Lee switches de ocupancia    │
    │ - Actualiza LEDs               │
    │ - Activa secuencia de salida   │
    └────────────────────────────────┘
               ↓
    (Vuelta a inicio del loop)
```

---

## 3. Flujo de Acceso Autorizado (RFID válida)

```
                    ┌─────────────────┐
                    │ RFID válida leída│
                    └────────┬────────┘
                             ↓
                  ¿Espacios disponibles?
                    │              │
                NO  │              │ SÍ
                    ↓              ↓
            ┌────────────┐  ┌──────────────────┐
            │ Mostrar    │  │ Levantar pluma   │
            │ "LLENO"    │  │ (servo entrada)  │
            │ (3s)       │  └────────┬─────────┘
            └────────────┘           ↓
                                  Fase 1:
                        ┌────────────────────────┐
                        │ Esperar auto           │
                        │ (sensor ultrasónica)   │
                        │ Timeout: 5 segundos    │
                        └────────────┬───────────┘
                                     ↓
                        ¿Auto detectado?
                        │              │
                    SÍ  │              │ NO
                        ↓              ↓ (timeout)
                    Fase 2:         ┌──────────┐
                ┌──────────────┐    │ Bajar    │
                │ Esperar que  │    │ pluma    │
                │ auto pase    │    │ "Timeout"│
                │ (desbloqueada)   │ 3s       │
                └────────┬─────┘    └──────────┘
                         ↓
                    Fase 3:
                ┌──────────────────┐
                │ Bajar pluma      │
                │ después de 3s    │
                └────────┬─────────┘
                         ↓
                 ┌──────────────┐
                 │ Auto dentro  │
                 │ Pluma cerrada│
                 └──────────────┘
```

---

## 4. Interacción API - Dashboard

```
┌──────────────────────────────────────┐
│      NAVEGADOR (Dashboard)            │
│                                       │
│  ┌─────────────────────────────────┐ │
│  │  Estado Actual                  │ │
│  │  [Distancia] [RFID] [Pluma]     │ │
│  │  [Cajón 1] [Cajón 2] [Disp.]    │ │
│  │                                 │ │
│  │  Actualizar cada 2 segundos →   │ │
│  │  script.js hace fetch()         │ │
│  └────────┬────────────────────────┘ │
│           ↓                           │
│  ┌─────────────────────────────────┐ │
│  │  Configuración                  │ │
│  │  [Input Umbral]  [Guardar] ←    │ │
│  │  [Input Timeout] [POST /api]    │ │
│  │  ...                             │ │
│  └────────┬────────────────────────┘ │
└───────────┼────────────────────────────┘
            ↓ Fetch API
 ┌──────────────────────────────┐
 │    ESP32 (AsyncWebServer)     │
 │                               │
 │  GET /api/getStatus           │
 │  └─→ handleGetStatus()        │
 │      ├─ Lee currentDistance   │
 │      ├─ Lee lastRFIDCard      │
 │      ├─ Lee slotOccupied[]    │
 │      └─ Retorna JSON          │
 │                               │
 │  GET /api/getParams           │
 │  └─→ handleGetParams()        │
 │      ├─ Lee configs           │
 │      └─ Retorna JSON          │
 │                               │
 │  POST /api/setParams          │
 │  └─→ handleSetParams()        │
 │      ├─ Parsea JSON body      │
 │      ├─ Actualiza variables   │
 │      └─ Retorna confirmación  │
 └──────────────────────────────┘
            ↓ JSON Response
        [Dashboard actualizado]
```

---

## 5. Estructura de Carpetas del Proyecto

```
Estacionamiento/
│
├── platformio.ini                      ← Configuración build (librerías, compilador)
│
├── include/
│   ├── config.h                        ← Centralizado: pins, mensajes, timing
│   └── webserver.h                     ← NEW: Servidor web + API REST
│
├── src/
│   ├── main.cpp                        ← Firmware principal (lógica sensado)
│   └── rfid_register.cpp               ← Utilidad registrar tarjetas RFID
│
├── data/
│   └── www/                            ← Archivos LittleFS (para ESP32)
│       ├── index.html                  ← NEW: Dashboard HTML
│       ├── style.css                   ← NEW: Estilos CSS
│       └── script.js                   ← NEW: JavaScript actualización
│
├── web/                                ← Desarrollo local (espejo)
│   ├── index.html                      ← Editable en VS Code
│   ├── style.css                       ← Editable en VS Code
│   └── script.js                       ← Editable en VS Code
│
├── python/                             ← Futuro subsistema Python
│   ├── collector.py                    ← Recolector de datos TCP
│   ├── main_gui.py                     ← GUI con estadísticas
│   └── database.py                     ← Módulo MySQL (futuro)
│
├── docs/
│   ├── WEB_API_DOCUMENTATION.md        ← NEW: API REST detallada
│   ├── PROYECTO_COMPLETO.md            ← NEW: Guía integral
│   ├── INICIO_RAPIDO_WEB.md            ← NEW: Quick start
│   ├── ARQUITECTURA.md                 ← NEW: Este archivo
│   └── ...otros archivos de documentación...
│
└── test/
    └── (archivos de test futuros)
```

---

## 6. Matriz de Responsabilidades

| Componente | Responsabilidad | Módulo |
|-----------|-----------------|--------|
| **RFID** | Leer tarjeta, validar | main.cpp |
| **Ultrasónica** | Detectar auto, timeout | main.cpp |
| **Servos** | Abrir/cerrar barreras | main.cpp |
| **LEDs** | Indicar ocupancia | main.cpp |
| **OLED** | Mostrar mensajes | main.cpp |
| **DIP Switches** | Confirmar ocupancia | main.cpp |
| **Servidor Web** | Servir HTML/CSS/JS | webserver.h |
| **API REST** | Exponer estado y parámetros | webserver.h |
| **Dashboard** | Interfaz usuario | HTML/CSS/JS |
| **Lógica negocio** | Flujo estacionamiento | main.cpp |

---

## 7. Ciclo de Vida de una Solicitud API

```
Usuario en navegador
        ↓
   [Click "Actualizar Ahora"]
        ↓
script.js ejecuta:
    fetch('/api/getStatus')
        ↓
HTTP GET request viaja a ESP32
        ↓
AsyncWebServer recibe en puerto 80
        ↓
Enruta a handleGetStatus()
        ↓
handleGetStatus() crea JSON:
    {
      "distancia": currentDistance,
      "rfidUID": lastRFIDCard,
      "plumaEntrada": entranceBarrierRaised,
      ...
    }
        ↓
response.send(200, "application/json", json_string)
        ↓
Response viaja al navegador
        ↓
script.js parsea JSON
        ↓
script.js actualiza elementos DOM:
    - document.getElementById('distancia').textContent = "25 cm"
    - document.getElementById('cajon1').className = "ocupado"
    - ... etc ...
        ↓
[Dashboard muestra datos actualizados]
```

---

## 8. Estados del Sistema

```
ESTADO 1: REPOSO (Espera RFID)
┌─────────────────┐
│ • Pluma: DOWN   │
│ • LED: Según    │
│   ocupancia     │
│ • OLED: Mostra  │
│   disponibles   │
│ • Acción: Lee   │
│   RFID          │
└─────────────────┘
        ↓ RFID válida
        
ESTADO 2: ENTRADA ACTIVA
┌──────────────────┐
│ • Pluma: UP      │
│ • Esperando auto │
│ • OLED: Muestra  │
│   "Bienvenido"  │
│ • Acción: Lee   │
│   ultrasónica    │
│ • Timeout: 5s   │
└──────────────────┘
        ↓ Auto detectado
        
ESTADO 3: AUTO PASÓ
┌──────────────────┐
│ • Pluma: Bajando │
│ • Espera 3s      │
│ • OLED: Muestra  │
│   "Pase seguro"  │
└──────────────────┘
        ↓ Terminó bajada
        
ESTADO 1: REPOSO (vuelve)
```

---

## 9. Mapa de Puertos/Pines

```
ESP32 PIN LAYOUT
╔════════════════════════════════════════╗
║              ESP32 DOIT                ║
║  ┌──────────────────────────────────┐  ║
║  │ GPIO 5  │ RFID SS (CS)           │  ║
║  │ GPIO 4  │ RFID RST               │  ║
║  │ GPIO 14 │ Ultrasónica TRIG       │  ║
║  │ GPIO 33 │ Ultrasónica ECHO       │  ║
║  │ GPIO 13 │ Servo Entrada (PWM)    │  ║
║  │ GPIO 15 │ Servo Salida (PWM)     │  ║
║  │ GPIO 25 │ LED Rojo Cajón 1       │  ║
║  │ GPIO 26 │ LED Rojo Cajón 2       │  ║
║  │ GPIO 32 │ Switch Cajón 1 (INPUT) │  ║
║  │ GPIO 27 │ Switch Cajón 2 (INPUT) │  ║
║  │ GPIO 21 │ I2C SDA (OLED)         │  ║
║  │ GPIO 22 │ I2C SCL (OLED)         │  ║
║  │ GPIO 23 │ SPI MOSI (RFID)        │  ║
║  │ GPIO 19 │ SPI MISO (RFID)        │  ║
║  │ GPIO 18 │ SPI CLK (RFID)         │  ║
║  └──────────────────────────────────┘  ║
║  GND, 5V, 3.3V (distribución)          ║
╚════════════════════════════════════════╝

RED (WiFi - Futuro)
PORT 80: HTTP Web Server
PORT 5000: TCP Collector (Python - Futuro)
```

---

**Última actualización:** Diciembre 2025
**Versión:** 1.0.0
