# Sistema de Estacionamiento - Subsistema Python

## Descripción
Contiene dos aplicaciones:
1. **data_collector.py** — Servidor TCP que recibe telemetría del ESP32 y la almacena en MySQL
2. **parking_gui.py** — GUI de control y monitoreo del estacionamiento

## Requisitos previos

### Sistema
- Python 3.8+
- MySQL 5.7+ (o MariaDB)

### Instalación de dependencias
```bash
pip install -r requirements.txt
```

## Configuración

### Base de Datos
1. Crea la base de datos ejecutando el script SQL:
```bash
mysql -u root -p < schema.sql
```

2. Actualiza las credenciales en ambos scripts (user, password):
```python
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',           # Tu usuario MySQL
    'password': 'password',   # Tu contraseña MySQL
    'database': 'estacionamiento_db'
}
```

### ESP32
Asegúrate de que:
- El ESP32 está conectado a WiFi
- La IP o mDNS name es accesible desde la PC
- Actualiza `ESP32_URL` en `parking_gui.py` si es necesario:
```python
ESP32_URL = "http://estacionamiento.local"  # O usar IP: http://192.168.100.91
```

## Uso

### 1. Iniciar Data Collector (en terminal separada)
```bash
python data_collector.py
```
- Escucha en puerto 5000 (localhost:5000)
- Recibe JSON del ESP32 y lo almacena en MySQL
- Genera log en `data_collector.log`

### 2. Iniciar GUI Principal
```bash
python parking_gui.py
```
Abre una ventana con tabs:
- **Dashboard** — Estado actual (distancia, temperatura, uptime, espacios libres)
- **Control** — Modificar parámetros del ESP32 via API-REST
- **Estadísticas** — Gráficos de ocupación por rango de fechas
- **Logs** — Eventos (RFID, barreras, cambios de config)

## Arquitectura

```
ESP32
  ├─ Loop de sensado (NoDelay, sin delay())
  ├─ Web API (GET /api/getStatus, /api/getParams, POST /api/setParams)
  ├─ Telemetría TCP → Data Collector
  └─ LittleFS (página web + estilos)

Data Collector (TCP Server :5000)
  ├─ Recibe JSON del ESP32 (sensor_reading, barrier_event, rfid_access, etc.)
  └─ Inserta en MySQL (sensor_readings, barrier_events, rfid_access, etc.)

GUI Principal (Tkinter)
  ├─ Consulta ESP32 via API-REST (obtener/cambiar parámetros)
  ├─ Lee MySQL para historial y estadísticas
  └─ Muestra gráficos con Matplotlib
```

## Formato de Telemetría del ESP32 → Data Collector

El ESP32 debe enviar JSON (una línea por evento) al data collector via TCP en puerto 5000:

```json
{"type": "sensor_reading", "data": {"timestamp": "2025-12-10T10:30:00", "distance": 50.5, "temperature": 25.3, "uptime_seconds": 3600, "available_slots": 1}}
{"type": "barrier_event", "data": {"timestamp": "2025-12-10T10:31:00", "barrier_type": "entrance", "event_type": "raised", "reason": "authorized_access"}}
{"type": "rfid_access", "data": {"timestamp": "2025-12-10T10:32:00", "card_uid": "ABC123XYZ", "authorized": true, "action": "entry_granted"}}
{"type": "config_change", "data": {"timestamp": "2025-12-10T10:33:00", "parameter_name": "ULTRASONIC_THRESHOLD", "old_value": 30, "new_value": 35, "changed_by": "api"}}
{"type": "slot_occupancy", "data": {"timestamp": "2025-12-10T10:34:00", "slot_number": 1, "occupied": true}}
```

## Tablas MySQL

- **sensor_readings** — Lecturas periódicas de sensores
- **barrier_events** — Eventos de subida/bajada de barreras
- **rfid_access** — Accesos permitidos/denegados
- **config_changes** — Cambios en parámetros de configuración
- **slot_occupancy** — Histórico de ocupación de espacios
- **system_events** — Alertas y eventos del sistema

## Troubleshooting

### Error: "No se pudo conectar a MySQL"
- Verifica que MySQL está ejecutándose
- Comprueba credenciales en DB_CONFIG
- Asegúrate de que la BD `estacionamiento_db` existe

### Error: "No hay conexión a ESP32"
- Verifica que el ESP32 está en línea (ping a IP o mDNS)
- Comprueba que WiFi está conectado (monitor serie del ESP32)
- Actualiza `ESP32_URL` si es necesario

### Data Collector no recibe datos
- Verifica que el ESP32 está enviando telemetría TCP a localhost:5000
- Revisa `data_collector.log` para más detalles
- Abre puerto 5000 en firewall si es necesario

## Próximas mejoras
- [ ] Autenticación en GUI
- [ ] Notificaciones push/email para eventos críticos
- [ ] Exportar estadísticas a PDF/CSV
- [ ] Dashboard web adicional
- [ ] Histórico de accesos RFID con fotos (si hay cámara)
