# 🅿️ Sistema de Estacionamiento y Caseta con ESP32

Un sistema completo de control de acceso a estacionamiento con caseta automatizada utilizando Arduino/ESP32.

## 🎯 Características

### Caseta de Acceso
- ✅ **Lector RFID**: Verifica autorización de usuarios
- ✅ **Sensor Ultrasónico**: Detecta cuando el vehículo pasa la barrera
- ✅ **Servomotor**: Levanta/baja la barrera automáticamente
- ✅ **Display LCD**: Muestra mensajes (Bienvenido / Acceso Denegado)
- ✅ **Buzzer**: Sonidos de bienvenida y rechazo

### Estacionamiento
- ✅ **4 Switches Manuales**: Uno por cada cajón
- ✅ **4 LEDs Rojos**: Indicadores de cajones ocupados
- ✅ **Lógica de Estados**: Alternancia automática ocupado/disponible

---

## 📋 Requisitos

### Hardware
- 1x ESP32 (DOIT DevKit V1)
- 1x Lector RFID RC522
- 1x Sensor Ultrasónico HC-SR04
- 1x Servomotor SG90 (o similar)
- 1x Display LCD 16x2 con módulo I2C
- 4x Switches de 4 pines
- 4x LEDs rojos (5mm)
- 1x Buzzer piezo (5V)
- Resistencias: 4x 220Ω (LEDs), 10kΩ (opcional para pull-ups)
- Cables de conexión, protoboard, fuente 5V 3A

### Software
- Visual Studio Code
- PlatformIO IDE
- Las librerías se descargarán automáticamente con platformio.ini

---

## 🔌 Configuración Rápida

1. **Copia los archivos en tu carpeta del proyecto**
2. **Abre `platformio.ini` - las dependencias ya están incluidas**
3. **Conecta tu ESP32 por USB**
4. **En PlatformIO: Upload → Compile & Upload**

---

## 📖 Archivos Importantes

| Archivo | Descripción |
|---------|-------------|
| `src/main.cpp` | Código principal del sistema |
| `src/rfid_register.cpp` | Utilidad para registrar UIDs de tarjetas RFID |
| `platformio.ini` | Configuración del proyecto y dependencias |
| `ESQUEMA_CONEXION.md` | Esquema detallado de conexiones y pines |

---

## 🔐 Configurar Tarjetas RFID

### Opción 1: Registrar en tiempo real
1. Carga `src/rfid_register.cpp` en PlatformIO
2. Abre el Serial Monitor (115200 baud)
3. Presenta tarjetas al lector
4. Copia los UIDs generados
5. Pega en `main.cpp` en el array `AUTHORIZED_CARDS`

### Opción 2: Usar UIDs conocidos
```cpp
const String AUTHORIZED_CARDS[] = {
  "12:34:56:78",  // Reemplaza con UID real
  "87:65:43:21"   // Agrega más tarjetas
};
```

---

## 🚗 Flujo de Operación

### 1️⃣ Entrada a la Caseta (Usuario Autorizado)
```
Carro llega → Presenta tarjeta RFID
    ↓
Sistema verifica UID en base de datos
    ↓ ✅ AUTORIZADO
→ Servomotor levanta barra
→ Display: "Bienvenido!"
→ Buzzer: 3 beeps cortos
→ Espera sensor ultrasónico
→ Carro pasa (distancia > 30cm)
→ Servomotor baja barra
→ Display: "Pase seguro"
```

### 2️⃣ Entrada a la Caseta (Usuario No Autorizado)
```
Carro llega → Presenta tarjeta RFID
    ↓
Sistema verifica UID en base de datos
    ↓ ❌ DENEGADO
→ Display: "ACCESO DENEGADO"
→ Buzzer: 2 beeps largos
→ Mensaje por 3 segundos
→ Sistema listo nuevamente
```

### 3️⃣ Estacionamiento
```
Usuario llega a cajón asignado
    ↓
Presiona switch del cajón
    ↓
LED rojo se enciende (cajón ocupado)
    ↓
Cuando se va:
Presiona switch de nuevo o se repone manualmente
    ↓
LED rojo se apaga (cajón disponible)
```

---

## 🎮 Serial Monitor

Abre Serial Monitor a **115200 baud** para ver:
- UIDs de tarjetas detectadas
- Estados de autorización
- Distancias medidas del ultrasónico
- Estados de los 4 cajones

**Ejemplo de salida:**
```
Sistema de Estacionamiento iniciando...
RFID inicializado
Sistema inicializado correctamente
Tarjeta detectada: 12:34:56:78
Usuario autorizado
Levantando barra...
Distancia: 45.23 cm
Distancia: 32.10 cm
Coche detectado pasando barra
Bajando barra...
```

---

## 🔧 Parámetros Personalizables

En `main.cpp`, modifica estos valores según necesites:

```cpp
// Tiempos
const unsigned long RFID_COOLDOWN = 2000;           // ms entre lecturas
const unsigned long BARRIER_LOWER_DELAY = 3000;     // ms de espera
const unsigned long ULTRASONIC_CHECK_INTERVAL = 100; // ms entre mediciones

// Distancia de detección ultrasónica (línea 165)
if (distance > 30) // Cambiar 30 cm según necesidad
```

---

## 📌 Notas Importantes

- **Pull-ups**: Los switches en GPIO 13, 12, 14, 26 usan INPUT_PULLUP (integrado ESP32)
- **Ángulos del Servo**: 0° = barra arriba, 180° = barra abajo (ajusta según tu montaje)
- **Display I2C**: Dirección 0x27 (común), verifica si necesitas otra
- **Alimentación**: Fuente de 3A recomendada para evitar reinicios

---

## 🐛 Solución de Problemas

**RFID no detecta tarjetas**
- Verifica conexión SPI (CLK, MOSI, MISO)
- Comprueba pines CS=5 y RST=27
- Serial Monitor debe mostrar UIDs

**Display en blanco**
- Ajusta potenciómetro de contraste en módulo I2C
- Verifica dirección I2C (comando: `I2CScanner`)
- Revisa conexión SDA/SCL

**Servomotor no responde**
- Verifica alimentación de 5V
- Prueba con ángulos simples (0 y 180)
- Asegúrate de usar GPIO 15

**Sensor ultrasónico sin lecturas**
- Verifica pins TRIG=32 y ECHO=33
- Limpia la lente del sensor
- Prueba con objetos más cercanos

---

## 📦 Dependencias

- `paulstoffregen/Servo@^1.2.1` - Control de servomotor
- `marcoschwartz/LiquidCrystal_I2C@^1.1.4` - Display LCD
- `miguelbalboa/RFID@^1.4.10` - Lector RFID
- `Wire` - Comunicación I2C (incluida ESP32)

---

## 💡 Mejoras Futuras

- [ ] EEPROM para guardar tarjetas registradas
- [ ] Pantalla táctil para admin
- [ ] Historial de accesos (SD card)
- [ ] WiFi para registro remoto
- [ ] Múltiples casetas sincronizadas
- [ ] App móvil para usuarios

---

**Autor**: Tu Nombre  
**Fecha**: Diciembre 2025  
**Versión**: 1.0
