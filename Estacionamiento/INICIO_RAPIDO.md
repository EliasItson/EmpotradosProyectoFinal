# 🎯 INICIO RÁPIDO - 5 PASOS

## Paso 1️⃣: Compilar
```
✓ Abre: PlatformIO Home
✓ Selecciona tu proyecto
✓ Clic: Build
✓ Espera a que termine (2-3 minutos la primera vez)
```

## Paso 2️⃣: Cargar en ESP32
```
✓ Conecta ESP32 por USB
✓ PlatformIO: Upload
✓ Espera el mensaje "Done uploading"
✓ Sistema se reinicia automáticamente
```

## Paso 3️⃣: Registrar Tarjetas RFID
```
✓ Carga: src/rfid_register.cpp
✓ PlatformIO: Upload
✓ Abre: Serial Monitor (115200 baud)
✓ Presenta tarjeta RFID
✓ Copia el UID mostrado
✓ Vuelve a cargar: src/main.cpp
✓ En include/config.h pega los UIDs
✓ Compila y carga nuevamente
```

## Paso 4️⃣: Verificar Componentes
```
✓ Serial Monitor debe mostrar "Sistema Listo"
✓ Display LCD debe encenderse
✓ Presenta tarjeta autorizada
✓ Servo debe levantar (escucharás clic)
✓ Buzzer debe sonar (3 beeps)
✓ Display muestra "Bienvenido!"
```

## Paso 5️⃣: Pruebas Finales
```
✓ Prueba todos los 4 switches de cajones
✓ Verifica que LEDs se enciendan/apaguen
✓ Prueba tarjeta no autorizada (debe sonar 2 beeps largos)
✓ Sensor ultrasónico: pasa objeto por la barra
✓ Sistema baja barra automáticamente
```

---

# 📂 Estructura de Archivos

```
📦 Estacionamiento/
├── 📄 README.md                    ← Lee esto primero
├── 📄 RESUMEN.md                   ← Resumen del proyecto
├── 📄 ESQUEMA_CONEXION.md          ← Cómo conectar todo
├── 📄 PRUEBAS_DEBUGGING.md         ← Cómo probar componentes
│
├── ⚙️ platformio.ini               ← Config del proyecto
│
├── 📁 include/
│   └── 🔧 config.h                 ← Personalización (EDITA ESTO)
│
├── 📁 src/
│   ├── 🚀 main.cpp                 ← Programa principal
│   └── 🏷️ rfid_register.cpp        ← Para registrar tarjetas
│
├── 📁 lib/                         ← Librerías (auto-descargadas)
└── 📁 test/                        ← Pruebas (opcional)
```

---

# 🔍 Guía Rápida de Personalización

### Cambiar puertos/pines
**Archivo**: `include/config.h`
```cpp
#define RFID_SS_PIN     5    // ← Cambia aquí
#define SERVO_PIN      15    // ← O aquí
```

### Agregar tarjetas RFID autorizadas
**Archivo**: `include/config.h`
```cpp
const String AUTHORIZED_CARDS[] = {
  "12:34:56:78",  // Tu tarjeta 1
  "87:65:43:21",  // Tu tarjeta 2
  "AA:BB:CC:DD"   // Tu tarjeta 3
};
const int AUTHORIZED_CARDS_COUNT = 3;  // ← Actualiza este número
```

### Cambiar mensajes del display
**Archivo**: `include/config.h`
```cpp
#define MSG_WELCOME_1 "¡Bienvenido!"
#define MSG_WELCOME_2 "Pase al parq."
```

### Ajustar sensibilidad ultrasónica
**Archivo**: `include/config.h`
```cpp
#define ULTRASONIC_THRESHOLD 30  // Cambiar de cm según necesite
```

---

# ⚡ Conexiones Esenciales

```
ESP32 (3.3V side)        ESP32 (5V side)
├─ GND ────────────────────── GND
├─ GPIO 21 ─ I2C SDA ──→ LCD I2C (SDA)
├─ GPIO 22 ─ I2C SCL ──→ LCD I2C (SCL)
├─ GPIO 5 ──────────────→ RFID (CS)
├─ GPIO 27 ─────────────→ RFID (RST)
├─ GPIO 13 ─────────────→ Switch Slot 1
├─ GPIO 12 ─────────────→ Switch Slot 2
├─ GPIO 14 ─────────────→ Switch Slot 3
├─ GPIO 26 ─────────────→ Switch Slot 4
│
├─ GPIO 15 ─────────────→ Servo (Signal)
├─ GPIO 21 ─────────────→ LED Red Slot 1 (anode)
├─ GPIO 22 ─────────────→ LED Red Slot 2 (anode)
├─ GPIO 23 ─────────────→ LED Red Slot 3 (anode)
├─ GPIO 25 ─────────────→ LED Red Slot 4 (anode)
├─ GPIO 19 ─────────────→ Buzzer

5V Power Supply (3A)
├─ +5V ──→ LCD (VCC)
├─ +5V ──→ Servo (VCC)
├─ +5V ──→ Ultrasónico (VCC)
├─ +5V ──→ Buzzer (anode)
└─ GND ──→ Todos (GND común)

RFID (SPI):
├─ CLK  ─ GPIO 18
├─ MOSI ─ GPIO 23
└─ MISO ─ GPIO 19

Ultrasónico:
├─ TRIG ─ GPIO 32
└─ ECHO ─ GPIO 33
```

---

# 🆘 Si Algo Falla

| Problema | Solución |
|----------|----------|
| RFID no detecta tarjetas | Ver PRUEBAS_DEBUGGING.md sección "RFID" |
| Display en blanco | Ajusta potenciómetro de contraste |
| Servo no se mueve | Verifica alimentación 5V |
| Buzzer no suena | Verifica polaridad (+/-) |
| Compilación falla | Ejecuta: PlatformIO → Clean |
| Serial Monitor vacío | Abre: PlatformIO → Serial Monitor |

**Más problemas**: Lee `PRUEBAS_DEBUGGING.md`

---

# ✅ Checklist Pre-Deploy

- [ ] Todos los pines conectados correctamente
- [ ] Fuente 5V con capacidad mínima 3A
- [ ] RFID lee tarjetas (UID aparece en Serial)
- [ ] Servo levanta y baja suavemente
- [ ] Buzzer emite sonidos
- [ ] Display muestra mensajes
- [ ] 4 switches funcionan con LEDs
- [ ] Sensor ultrasónico mide distancias
- [ ] Usuario autorizado abre acceso
- [ ] Usuario no autorizado rechazado
- [ ] Barrera baja automáticamente

---

# 📞 Contacto de Soporte

- 📖 Documentación completa en archivos `.md`
- 🔧 Configuración en `include/config.h`
- 🐛 Debug con Serial Monitor (115200 baud)
- 💾 Código modular y comentado

---

**¡Listo para instalar! 🚀**
