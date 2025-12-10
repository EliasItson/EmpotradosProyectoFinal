# Estacionamiento Inteligente - Subsistema de PC

## Descripción

El subsistema de PC consta de 2 programas Python:

### 1. `collector.py` - Subsistema de Recolección de Datos
- **Función**: Recibe datos del ESP32 y los almacena en MySQL
- **Características**:
  - Servidor TCP en puerto 5000 para recibir datos (opcional)
  - Polling automático del ESP32 cada 2 segundos vía HTTP REST
  - Almacenamiento en BD MySQL local
  - Thread-safe para múltiples conexiones

### 2. `main_gui.py` - Subsistema Principal GUI
- **Función**: Interfaz de usuario para monitoreo y control
- **Características**:
  - Pestaña "Estado y Control": Muestra estado actual desde BD local, representación gráfica del estacionamiento
  - Pestaña "Parámetros": Permite cargar y modificar parámetros del ESP32 vía API REST
  - Pestaña "Estadísticas": Gráficos de ocupación, distancia ultrasónica y estado de plumas en período seleccionable
  - Actualización automática cada 2 segundos
  - Datos consistentes con webserver del ESP32

## Requisitos Previos

### Software
- Python 3.8+
- MySQL Server (local)
- ESP32 ejecutando el firmware actualizado

### Configuración de Base de Datos

```sql
CREATE DATABASE estacionamiento;
USE estacionamiento;

-- Las tablas se crean automáticamente al ejecutar collector.py
```

## Instalación

### 1. Instalar dependencias Python
```bash
pip install -r requirements.txt
```

### 2. Configurar MySQL
- Asegúrate de que MySQL está corriendo
- Crea la base de datos `estacionamiento` (o modifica DB_CONFIG en los scripts)
- Usuario por defecto: `root` sin contraseña (editar en los scripts si es diferente)

### 3. Configurar IPs
Edita los siguientes archivos y actualiza la IP del ESP32:

**collector.py** (línea ~18):
```python
ESP32_IP = "192.168.81.162"  # Cambia por tu IP
```

**main_gui.py** (línea ~19):
```python
ESP32_IP = "192.168.81.162"  # Cambia por tu IP
```

## Uso

### Opción 1: Ejecutar ambos desde terminal

**Terminal 1 - Recolector**:
```bash
python collector.py
```

**Terminal 2 - GUI Principal**:
```bash
python main_gui.py
```

### Opción 2: Crear scripts ejecutables (Windows)

**run_collector.bat**:
```batch
@echo off
python collector.py
pause
```

**run_gui.bat**:
```batch
@echo off
python main_gui.py
pause
```

## Características Detalladas

### Subsistema Recolector
- **Polling HTTP**: Consulta `/api/getStatus` del ESP32 cada 2 segundos
- **Servidor TCP**: Escucha en puerto 5000 para recibir datos opcionales
- **Almacenamiento**: Registra todos los campos en tabla `lecturas`
- **Threading**: Ambas funciones (polling y TCP) corren en threads independientes
- **Logs**: Imprime estado y errores en consola

### Interfaz GUI

#### Tab 1: Estado y Control
- **Panel izquierdo**: 
  - RFID más reciente
  - Distancia ultrasónica
  - Estado de plumas
  - Gráfica visual del estacionamiento (rojo=ocupado, verde=libre)
  
- **Panel derecho**:
  - Información detallada de cada cajón
  - Timestamps de entrada/salida

#### Tab 2: Parámetros
- Cargar parámetros actuales del ESP32
- Modificar:
  - Tiempo de apertura de pluma (ms)
  - Delay de pluma de salida (ms)
- Guardar cambios directamente en ESP32 vía API

#### Tab 3: Estadísticas
- **Filtro**: Seleccionar período en horas (1-168)
- **Gráficos**:
  1. Ocupación de cajones (0/1 en el tiempo)
  2. Distancia ultrasónica
  3. Estado de plumas (entrada/salida)

## Estructura de BD

### Tabla `lecturas`
```
id                 INT PRIMARY KEY AUTO_INCREMENT
timestamp          DATETIME (índice)
rfidUID            VARCHAR(50)
distancia          FLOAT
plumaEntrada       BOOLEAN
plumaSalida        BOOLEAN
cajon1             BOOLEAN (índice)
cajon2             BOOLEAN (índice)
entryTime1         VARCHAR(255)
exitTime1          VARCHAR(255)
entryTime2         VARCHAR(255)
exitTime2          VARCHAR(255)
```

## Solución de Problemas

### "Error al conectar a MySQL"
- Verifica que MySQL está corriendo
- Revisa credenciales en DB_CONFIG
- Asegúrate de que existe la base de datos `estacionamiento`

### "No se conecta al ESP32"
- Verifica la IP en los scripts
- Asegúrate de que el ESP32 está conectado a la red WiFi
- Prueba accediendo a http://{IP_ESP32}/ en navegador

### "No aparecen datos en gráficos"
- Verifica que el `collector.py` está corriendo
- Espera al menos 2 segundos para que se guarden datos
- Revisa la BD manualmente:
  ```sql
  SELECT COUNT(*) FROM lecturas;
  ```

## Notas

- Los datos en la GUI se actualizan automáticamente cada 2 segundos desde BD local
- El `collector.py` consulta el ESP32 cada 2 segundos, asegurando datos frescos
- Los parámetros modificados se guardan en el ESP32 y persisten (almacenado en LittleFS)
- Para máxima consistencia, siempre usa la GUI o el webserver, no ambos simultáneamente en parámetros

## Autor
Proyecto Estacionamiento Inteligente - 2025
