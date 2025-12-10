# Fase 3 - Subsistema Python: Recolección de Datos y GUI

## Resumen de Implementación

Se han creado dos aplicaciones Python que completan el subsistema de PC especificado en los requisitos:

### 1. **Data Collector** (`data_collector.py`)
**Responsabilidad:** Servidor TCP que recibe telemetría del ESP32 y la almacena en MySQL

**Características:**
- Escucha en puerto TCP 5000
- Recibe eventos JSON del ESP32 (sensor_reading, barrier_event, rfid_access, config_change, slot_occupancy)
- Inserta automáticamente en tablas MySQL correspondientes
- Manejo de reconexión automática a MySQL
- Logging detallado a archivo y consola
- Threading para múltiples conexiones simultáneas

**Tablas MySQL creadas:**
- `sensor_readings` — Lecturas periódicas (distancia, temperatura, uptime, espacios disponibles)
- `barrier_events` — Eventos de barrera (entrada/salida, subida/bajada)
- `rfid_access` — Accesos permitidos/denegados con UID de tarjeta
- `config_changes` — Historial de cambios en parámetros
- `slot_occupancy` — Ocupación histórica de espacios
- `system_events` — Alertas y eventos del sistema

---

### 2. **Parking GUI** (`parking_gui.py`)
**Responsabilidad:** Interfaz gráfica de control y monitoreo del estacionamiento

**Características:**
- Interfaz Tkinter con 4 tabs:
  1. **Dashboard**
     - Visualización en tiempo real (distancia, temperatura, uptime, espacios disponibles)
     - IP del ESP32
     - Gráfico de ocupación en tiempo real (últimas 24h)

  2. **Control**
     - Lectura de parámetros actuales via API-REST
     - Edición de 4 parámetros configurables:
       - ULTRASONIC_THRESHOLD
       - ULTRASONIC_TIMEOUT_MS
       - LOWER_BARRIER_WAIT_MS
       - DISPLAY_MESSAGE_MS
     - Guardado via POST /api/setParams

  3. **Estadísticas**
     - Selector de rango de fechas (desde-hasta)
     - Generación de gráficos matplotlib de ocupación
     - Lectura desde MySQL

  4. **Logs**
     - Filtro por tipo de evento (Todos, RFID, Barreras, Sensores, Config)
     - Tabla con 50 eventos más recientes
     - Información: timestamp, tipo, descripción

**Actualización automática:**
- Thread background que actualiza estado cada 2 segundos
- Comunica con ESP32 via API-REST (GET /api/getStatus)

---

### 3. **Schema MySQL** (`schema.sql`)
Define 6 tablas con índices para:
- Búsqueda rápida por timestamp
- Filtrado por tipo de evento
- Historial completo con auditoría

---

### 4. **Setup Assistant** (`setup.py`)
Script de configuración interactivo que:
- Verifica MySQL instalado
- Solicita credenciales (host, user, password)
- Crea base de datos y tablas
- Actualiza archivos .py con configuración
- Instala dependencias Python
- Guía al usuario en próximos pasos

---

### 5. **Documentación** 
- `README.md` — Instrucciones de instalación, uso, troubleshooting
- `requirements.txt` — Dependencias Python (requests, mysql-connector, pandas, matplotlib)

---

## Arquitectura de Integración

```
┌──────────────────────────────────────────────────────────────┐
│                         ESP32                                │
│  ┌────────────────────────────────────────────────────────┐  │
│  │ Loop de sensado (NoDelay, sin delay())                │  │
│  │ - Lectura ultrasónica                                  │  │
│  │ - Detección RFID                                       │  │
│  │ - Control de barreras                                  │  │
│  │ - Persistencia de parámetros (Preferences)            │  │
│  └────────────────────────────────────────────────────────┘  │
│         │                           │                        │
│         ├─ HTTP REST API ──────┐   │                        │
│         │ (Async Webserver)    │   │                        │
│         │                      │   │                        │
│         │ GET /api/getStatus   │   │                        │
│         │ GET /api/getParams   │   │                        │
│         │ POST /api/setParams  │   │                        │
│         │                      │   │                        │
│         └──────────────────────┘   │                        │
│                                    │                        │
│         ┌──────────────────────┐   │                        │
│         │ TCP Client (TELEMETRY)  │ (nuevo)               │
│         │ Envía JSON cada 5s    │  │                        │
│         │ - sensor_reading      │  │                        │
│         │ - barrier_event       │  │                        │
│         │ - rfid_access         │  │                        │
│         │ - config_change       │  │                        │
│         │ - slot_occupancy      │  │                        │
│         └──────────────────────┘  │                        │
└──────────────────────────────────────────────────────────────┘
         │                          │
         │ (REST)                   │ (TCP port 5000)
         │                          │
         ▼                          ▼
┌────────────────────┐   ┌──────────────────────────┐
│  Parking GUI       │   │  Data Collector          │
│  (Tkinter)         │   │  (TCP Server)            │
│                    │   │                          │
│ - Dashboard        │   │ - Recibe JSON            │
│ - Control remoto   │   │ - Parsea eventos        │
│ - Estadísticas     │   │ - Inserta en MySQL      │
│ - Logs             │   │ - Manejo de conexiones  │
│                    │   │ - Logging               │
└────────────────────┘   └──────────────────────────┘
         │                          │
         └──────────┬───────────────┘
                    │ (MySQL read/write)
                    ▼
         ┌──────────────────────┐
         │    MySQL Database    │
         │                      │
         │ sensor_readings      │
         │ barrier_events       │
         │ rfid_access          │
         │ config_changes       │
         │ slot_occupancy       │
         │ system_events        │
         └──────────────────────┘
```

---

## Instalación Rápida

### En Windows (desde la carpeta `python/`):

```powershell
# 1. Ejecutar setup asistente
python setup.py

# 2. En terminal 1: iniciar data collector
python data_collector.py

# 3. En terminal 2: iniciar GUI
python parking_gui.py
```

### Requisitos:
- Python 3.8+
- MySQL 5.7+
- ESP32 conectado a WiFi
- PC conectada a la misma red

---

## Modificaciones al ESP32

Se agregó el módulo de telemetría (`telemetry.h` / `telemetry.cpp`) que:
- Mantiene conexión TCP abierta al data collector
- Envía JSON con eventos del sistema
- Se reconecta automáticamente si se cae la conexión
- No interfiere con el funcionamiento principal (envía en background)

**Nota:** El ESP32 aún no envía datos de telemetría automáticamente. Esto requiere:
1. Incluir `telemetry.h` en `main.cpp`
2. Llamar a `initTelemetry()` en `setup()`
3. Llamar a `connectTelemetry()` después de WiFi conectado
4. Insertar llamadas a `sendSensorReading()`, `sendBarrierEvent()`, etc. en los puntos clave del loop

---

## Próximas Mejoras (Post-Fase 3)

- [ ] Integración de telemetría automática en main.cpp (pull de datos cada 5 segundos)
- [ ] NTP sincronización para timestamps precisos en ESP32
- [ ] Autenticación en GUI (usuario/contraseña)
- [ ] Notificaciones push/email para eventos críticos
- [ ] Dashboard web adicional (Flask/Django)
- [ ] Exportación de reportes (PDF/CSV)
- [ ] Histórico de accesos RFID con foto (si hay cámara integrada)

---

## Especificaciones Cumplidas

✅ **Subsistema principal (GUI)** 
- ✓ Mostrar/modificar parámetros via API-REST
- ✓ Estado de sensores/actuadores desde BD local (actualización en tiempo real)
- ✓ Representación visual del estado del sistema (dashboard)
- ✓ Gráficos de ocupación con rango de fechas

✅ **Subsistema de Recolección de Datos**
- ✓ Ejecuta por separado (proceso independiente)
- ✓ Recibe datos del ESP32 via servidor TCP
- ✓ Almacena en MySQL local

---

Fase 3 completada. El sistema está listo para pruebas end-to-end.
