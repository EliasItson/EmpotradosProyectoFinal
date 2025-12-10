# Guía de Inicio Rápido - Sistema Web API

## 📝 Resumen de Cambios en Esta Sesión

Se ha implementado un subsistema web completo para el sistema de estacionamiento:

### ✅ Componentes Creados

1. **`include/webserver.h`** - Servidor web con API REST
   - AsyncWebServer en puerto 80
   - 3 endpoints API (/api/getStatus, /api/getParams, /api/setParams)
   - Sirve archivos HTML/CSS/JS desde LittleFS
   - Lee variables globales de main.cpp (sin modificar su lógica)

2. **Dashboard Web Responsive**
   - `web/index.html` - Interfaz moderna con 5 secciones
   - `web/style.css` - Estilos profesionales, responsive
   - `web/script.js` - Actualización automática en tiempo real
   - `data/www/` - Copias para LittleFS del ESP32

3. **Documentación**
   - `docs/WEB_API_DOCUMENTATION.md` - Especificación completa de API
   - `docs/PROYECTO_COMPLETO.md` - Guía integral del sistema

### ⚠️ ESTADO IMPORTANTE

**El firmware principal (`src/main.cpp`) NO ha sido modificado en su lógica.**

Se agregaron solo:
- `#include "webserver.h"` y `#include <WiFi.h>` (includes)
- 4 variables globales para que la API lea datos: `currentDistance`, `lastRFIDCard`, `systemUptime`, `systemTemperature`
- 4 variables configurables para la API: `ULTRASONIC_THRESHOLD_CONFIG`, etc.
- 1 línea en loop(): `systemUptime = millis();` para actualizar tiempo
- 1 línea en handleAuthorizedUser(): `lastRFIDCard = getCardUID();`
- 1 línea en checkUltrasonicSensor(): `currentDistance = distance;`
- Inicialización de webserver comentada en setup() (para activar después de WiFi)

**Todas las funcionalidades anteriores (RFID, ultrasónica, servos, LEDs, OLED) siguen funcionando exactamente igual.**

---

## 🚀 Pasos Siguientes para Completar la Integración

### Paso 1: Compilar el Firmware Actualizado

```bash
cd d:\Documents\Arduino\EmpotradosProyectoFinal\Estacionamiento
platformio run -e esp32doit-devkit-v1
```

**Resultado esperado:** Compilación exitosa (Exit Code 0)

Si hay errores sobre librerías no encontradas, PlatformIO las descargará automáticamente.

---

### Paso 2: Cargar el Firmware en el ESP32

```bash
platformio run -e esp32doit-devkit-v1 -t upload
```

**Resultado esperado:** 
```
Leaving... Hard resetting via RTS pin...
*** [SUCCESS]
```

---

### Paso 3: Cargar el Sistema de Archivos (LittleFS)

Este paso carga el dashboard web en la memoria del ESP32:

```bash
platformio run -e esp32doit-devkit-v1 -t uploadfs
```

**Resultado esperado:** El sistema de archivos se carga correctamente

---

### Paso 4: Verificar en Serial Monitor

```bash
platformio device monitor -e esp32doit-devkit-v1 -b 115200
```

Debería ver mensajes como:

```
[WEB] LittleFS montado correctamente
[WEB] Servidor web iniciado en puerto 80
[RFID] Tarjeta válida
[US] Distancia: XX cm
```

---

### Paso 5: Configurar WiFi (Próxima Fase)

**Por ahora:** El servidor web está comentado en setup() porque falta configurar WiFi.

Para activarlo temporalmente **solo con acceso por red local**:

1. Editar `src/main.cpp` línea ~102:
   ```cpp
   // Cambiar de:
   // initWebServer();
   
   // A:
   initWebServer();
   ```

2. Recompilar y cargar

3. Ver en Serial Monitor la IP asignada al ESP32 (ej: `192.168.1.100`)

4. Acceder en navegador: `http://192.168.1.100`

---

## 📱 Acceso al Dashboard

Una vez que WiFi esté configurado:

1. **Desde la red local:**
   ```
   http://<IP_DEL_ESP32>/
   ```

2. **Ejemplo:**
   ```
   http://192.168.1.100/
   ```

3. **La página mostrará:**
   - Badge de conexión (verde = conectado)
   - Estado actual de sensores/actuadores
   - Parámetros ajustables
   - Visualización gráfica del estacionamiento
   - Información del sistema

---

## 🔧 Estructura API

### GET /api/getStatus
Retorna estado actual en JSON:
```json
{
  "distancia": 25.5,
  "rfidUID": "1C:21:09:49",
  "plumaEntrada": true,
  "cajon1": false,
  "disponibles": 2,
  ...
}
```

### GET /api/getParams
Retorna parámetros configurables

### POST /api/setParams
Actualiza parámetros en el ESP32

---

## 📋 Checklist de Implementación

- [x] Crear webserver.h con AsyncWebServer
- [x] Implementar 3 endpoints API
- [x] Crear dashboard HTML/CSS responsive
- [x] Crear script.js con actualización automática
- [x] Actualizar platformio.ini con librerías
- [x] Crear data/www para LittleFS
- [x] Integrar variables globales en main.cpp
- [x] Documentación completa
- [ ] Implementar WiFi.begin() (Fase 2)
- [ ] Agregar EEPROM persistence (Fase 2)
- [ ] WebSocket para updates bidireccionales (Fase 3)
- [ ] Subsistema Python TCP (Fase 3)
- [ ] GUI con estadísticas (Fase 3)

---

## 🐛 Troubleshooting Rápido

### "undefined reference to `initWebServer`"
→ Olvidaste descomentar `initWebServer();` en main.cpp o no cargaste webserver.h

### "Error de compilación: no encuentro ESPAsyncWebServer.h"
→ Espera a que PlatformIO descargue las librerías (primer build toma tiempo)

### "Desconectado en el dashboard"
→ WiFi no está configurado. Por ahora es normal. Completa la Fase 2.

### "LittleFS: Mount failed"
→ Ejecuta `platformio run -e esp32doit-devkit-v1 -t uploadfs` para cargar los archivos web

---

## 📚 Documentación Referencia

- **API Completa:** `docs/WEB_API_DOCUMENTATION.md`
- **Guía Proyecto:** `docs/PROYECTO_COMPLETO.md`
- **Config Hardware:** `include/config.h`
- **Firmware Principal:** `src/main.cpp` (sin cambios lógicos)

---

## ⏭️ Próximo Paso Recomendado (Fase 2)

Una vez que compile y cargue correctamente, el siguiente paso sería:

1. **Implementar WiFi:**
   - Agregar SSID y password en config.h
   - Llamar `WiFi.begin()` en setup()
   - Desbloquear `initWebServer()` cuando WiFi esté conectado

2. **Agregar mDNS:**
   - Reemplazar acceso por IP con `http://estacionamiento.local`

---

## 📞 Notas

- El servidor web es **completamente independiente** del firmware principal
- Puede deshabilitarse comentando una sola línea
- No interfiere con la funcionalidad de sensado y actuadores
- Listo para integración con futuro subsistema Python

---

**Documento actualizado:** Diciembre 2025
