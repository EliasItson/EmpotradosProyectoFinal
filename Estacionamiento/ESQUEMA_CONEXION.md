# Sistema de Estacionamiento y Caseta - Esquema de Conexión

## Resumen del Sistema
- **Controlador**: ESP32 (DOIT DevKit V1)
- **Sensores**: RFID, Ultrasónico, 4 Switches
- **Actuadores**: Servomotor, 4 LEDs rojos, Buzzer, Display LCD (I2C)

---

## 📌 Asignación de Pines

### **RFID Reader (SPI)**
| Componente | Pin ESP32 |
|-----------|-----------|
| CS (SS)   | GPIO 5    |
| RST       | GPIO 27   |
| SPI CLK   | GPIO 18   |
| SPI MOSI  | GPIO 23   |
| SPI MISO  | GPIO 19   |
| GND       | GND       |
| VCC       | 3.3V      |

### **Sensor Ultrasónico**
| Componente | Pin ESP32 |
|-----------|-----------|
| TRIG      | GPIO 32   |
| ECHO      | GPIO 33   |
| GND       | GND       |
| VCC       | 5V (con resistencia de 5V)      |

### **Switches de Cajones**
| Cajón | Pin ESP32 |
|-------|-----------|
| 1     | GPIO 13   |
| 2     | GPIO 12   |
| 3     | GPIO 14   |
| 4     | GPIO 26   |
| GND   | GND       |
| VCC   | 3.3V (con resistencias pull-up) |

### **Actuadores**
| Componente | Pin ESP32 |
|-----------|-----------|
| Servomotor | GPIO 15   |
| LED Rojo 1 | GPIO 21   |
| LED Rojo 2 | GPIO 22   |
| LED Rojo 3 | GPIO 23   |
| LED Rojo 4 | GPIO 25   |
| Buzzer    | GPIO 19   |

### **Display LCD 16x2 (I2C)**
| Componente | Pin ESP32 |
|-----------|-----------|
| SDA       | GPIO 21   |
| SCL       | GPIO 22   |
| GND       | GND       |
| VCC       | 5V        |
| Dirección I2C | 0x27  |

---

## 🔐 Tarjetas RFID Autorizadas

Edita el array `AUTHORIZED_CARDS` en `main.cpp` con los UIDs de tus tarjetas:

```cpp
const String AUTHORIZED_CARDS[] = {
  "12:34:56:78",  // Tarjeta 1
  "87:65:43:21"   // Tarjeta 2
};
```

Para obtener el UID de una tarjeta, ejecuta el programa y lee la tarjeta. El UID aparecerá en el Serial Monitor.

---

## 🔧 Configuración Importante

### Tiempos
- **RFID Cooldown**: 2 segundos entre lecturas
- **Barrier Lower Delay**: 3 segundos para que pase el coche
- **Ultrasonic Check Interval**: 100ms entre mediciones

### Lógica de Cajones
- Switch presionado (LOW) = Cajón ocupado → LED rojo ON
- Switch no presionado (HIGH) = Cajón disponible → LED rojo OFF

### Distancia Ultrasónica
- Si distancia > 30cm: Se considera que el coche pasó la barra
- Se baja automáticamente la barrera

---

## 🚗 Flujo de Operación

### Caseta - Usuario Autorizado
1. Carro llega a caseta
2. Usuario presenta tarjeta RFID
3. Sistema verifica tarjeta
4. ✅ Autorizado:
   - Servomotor levanta barra (0°)
   - Display muestra "Bienvenido!"
   - Buzzer: 3 beeps cortos
   - Espera detección ultrasónica
   - Carro pasa (distancia > 30cm)
   - Servomotor baja barra (180°)
   - Display muestra "Pase seguro"

### Caseta - Usuario No Autorizado
1. Carro llega a caseta
2. Usuario presenta tarjeta RFID
3. Sistema verifica tarjeta
4. ❌ Denegado:
   - Display muestra "ACCESO DENEGADO"
   - Buzzer: 2 beeps largos
   - Mensaje se muestra 3 segundos

### Estacionamiento
1. Usuario presiona switch según cajón asignado
2. LED rojo del cajón se enciende
3. Cuando se desocupa, presiona de nuevo (o se repone manualmente)
4. LED rojo se apaga

---

## 📊 Monitoreo Serial

Abre el Serial Monitor a **115200 baud** para ver:
- UIDs de tarjetas detectadas
- Estados de autorización
- Distancias medidas
- Estados de cajones

---

## ⚡ Requerimientos de Energía

- **ESP32**: 500mA
- **RFID Reader**: 100mA
- **Servomotor**: 500-800mA (peak)
- **LEDs**: 20mA c/u (80mA total)
- **Buzzer**: 50mA
- **Display LCD**: 50mA

**Total recomendado**: Fuente de 3A @ 5V para operación estable

---

## 🐛 Troubleshooting

### RFID no funciona
- Verificar conexión SPI
- Probar UID en Serial Monitor
- Asegurar que tarjeta esté en `AUTHORIZED_CARDS`

### Sensor ultrasónico sin lecturas
- Verificar pines TRIG y ECHO
- Revisar cables de alimentación (5V)
- Limpiar sensor

### Display en blanco
- Verificar dirección I2C (0x27)
- Ajustar potenciómetro de contraste
- Verificar conexión SDA/SCL

### Servomotor no responde
- Verificar alimentación de 5V
- Revisar conexión de señal
- Probar con ángulos simples (0 y 180)
