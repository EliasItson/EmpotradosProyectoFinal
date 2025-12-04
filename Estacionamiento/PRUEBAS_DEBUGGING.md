# 🧪 Guía de Pruebas y Debugging

## ✅ Pre-compilación

Antes de cargar en el ESP32:

1. **Compila el proyecto**:
   - PlatformIO → Build
   - Las librerías se descargarán automáticamente
   - Espera a que termine (primera vez toma ~2 min)

2. **Verifica los pines** en tu placa física
   - GPIO 5 (RFID CS)
   - GPIO 27 (RFID RST)
   - GPIO 32, 33 (Ultrasónico)
   - GPIO 13, 12, 14, 26 (Switches)
   - GPIO 15 (Servo)
   - GPIO 21, 22, 23, 25 (LEDs)
   - GPIO 19 (Buzzer)

---

## 📤 Carga y Ejecución

```
PlatformIO → Upload
```

Espera el mensaje `Device ready to receive data` en la consola.

---

## 🔍 Pruebas Secuenciales

### 1. Verificar Serial Monitor
```
baud: 115200
Abre: PlatformIO → Serial Monitor
```

Deberías ver:
```
Sistema de Estacionamiento iniciando...
RFID inicializado
Sistema inicializado correctamente
```

### 2. Probar Display LCD
- Display debe mostrar "Sistema Listo" en línea 1
- Display debe mostrar "Esperando..." en línea 2

**Si no funciona**:
- Verifica dirección I2C (por defecto 0x27)
- Ajusta potenciómetro de contraste
- Ejecuta script `I2C_Scanner` para encontrar dirección correcta

### 3. Probar RFID
1. Abre Serial Monitor (115200 baud)
2. Presenta una tarjeta RFID
3. Deberías ver:
   ```
   Tarjeta detectada: XX:XX:XX:XX
   ```

**Si ves "Usuario NO autorizado"** → Normal, la tarjeta no está en la lista

**Para registrar tarjeta**:
- Carga `src/rfid_register.cpp`
- Presenta tarjeta
- Copia el UID mostrado
- Pégalo en `main.cpp` en `AUTHORIZED_CARDS[]`

### 4. Probar Sensor Ultrasónico
1. Abre Serial Monitor
2. Presenta tarjeta autorizada
3. El servomotor debe subir (escucharás clic)
4. Deberías ver distancias:
   ```
   Distancia: 25.43 cm
   Distancia: 23.12 cm
   ```
5. Pasa la mano frente al sensor
6. Cuando lea > 30cm: baja automáticamente

### 5. Probar Servomotor
- Debe levantar suavemente con ángulo 0°
- Debe bajar suavemente con ángulo 180°
- Ambos movimientos toman ~500ms

**Si no responde**:
- Verifica alimentación 5V
- Prueba con cable directamente a 5V/GND

### 6. Probar Buzzer
- Con usuario autorizado: escucharás 3 beeps
- Con usuario denegado: 2 beeps largos

**Si no suena**:
- Verifica polaridad
- Prueba conectando directamente a 5V

### 7. Probar LEDs de Cajones
1. Presiona Switch 1 → LED 1 debe encenderse
2. Presiona Switch 2 → LED 2 debe encenderse
3. Presiona nuevamente → LED debe apagarse

Serial Monitor mostrará:
```
Cajón 1 - OCUPADO
Cajón 1 - DISPONIBLE
```

---

## 🔧 Caso de Prueba Completo

**Escenario: Usuario Autorizado**

1. Carga tarjeta autorizada en `AUTHORIZED_CARDS[]`
2. Abre Serial Monitor (115200 baud)
3. Presenta tarjeta RFID
4. Observa:
   - Serial: `Usuario autorizado`
   - Display: `Bienvenido! / Acceso concedido`
   - Buzzer: 3 beeps
   - Servo levanta
5. Serial Monitor muestra:
   ```
   Levantando barra...
   Distancia: XX.XX cm
   ```
6. Pasa objeto frente a sensor (distancia > 30cm)
7. Servo baja automáticamente
8. Display: `Pase seguro / Gracias!`

---

## 🔴 Troubleshooting Avanzado

### RFID siempre muestra "Usuario NO autorizado"

**Problema**: UID incorrecto

**Solución**:
```cpp
// Agrega esta línea en checkRFID() para debug:
Serial.print("Buscando: ");
for (int i = 0; i < AUTHORIZED_CARDS_COUNT; i++) {
  Serial.println(AUTHORIZED_CARDS[i]);
}

// Compara con el UID leído
Serial.print("Leído: " + cardUID);
```

### Display muestra caracteres raros

**Problema**: Dirección I2C incorrecta

**Solución**: Crea este archivo de prueba:
```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  for (int i = 0x20; i < 0x27; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Dispositivo encontrado en: 0x");
      Serial.println(i, HEX);
    }
  }
}

void loop() {}
```

Reemplaza `0x27` en `main.cpp` con la dirección encontrada.

### Servo se mueve erráticamente

**Problema**: Interferencia electromagnética o alimentación baja

**Solución**:
- Conecta servo directamente a fuente 5V (no a ESP32)
- Agregacapacitor 100µF entre 5V y GND
- Usa cable corto y grueso para servo

### Sensor ultrasónico mide distancias raras

**Problema**: Reflejo de objetos cercanos o ruido

**Solución**:
- Coloca pequeño cubo a 15cm del sensor
- Serial Monitor debe mostrar ~15cm
- Si mide diferente: ajusta factor `0.0343` en `getUltrasonicDistance()`

```cpp
// Actual:
float distance = duration * 0.0343 / 2;

// Calibración:
// Si mide el doble: 0.0343 / 4
// Si mide la mitad: 0.0343
```

---

## 📊 Monitoreo en Tiempo Real

Abre Serial Monitor y verás:

```
Sistema de Estacionamiento iniciando...
RFID inicializado
Sistema inicializado correctamente
Tarjeta detectada: 12:34:56:78          ← Tarjeta presente
Usuario autorizado                       ← Verificación exitosa
Levantando barra...                      ← Servo activa
Distancia: 42.50 cm                      ← Ultrasónico mide
Distancia: 39.23 cm
Distancia: 35.12 cm
Distancia: 31.45 cm                      ← Cerca de umbral
Distancia: 32.10 cm
Coche detectado pasando barra            ← Umbral > 30cm
Bajando barra...                         ← Servo baja
Cajón 1 - OCUPADO                        ← Switch presionado
Cajón 2 - DISPONIBLE                     ← Switch liberado
```

---

## ⚡ Notas de Debugging

1. **Primero verifica alimentación**: 
   - Voltímetro en 5V/GND: debe haber 5V
   - Voltímetro en 3.3V/GND: debe haber 3.3V

2. **Verifica continuidad de cables**:
   - Multímetro en ohms (Ω)
   - Conecta puntas a ambos extremos del cable
   - Debe leer < 1Ω

3. **Usa Serial Monitor extensivamente**:
   - Agrega `Serial.println("Aquí llego");` en puntos críticos
   - Ayuda a identificar dónde se cuelga el código

4. **Reinicia el ESP32 manualmente si es necesario**:
   - Botón RESET en la placa
   - O conecta pin EN a GND brevemente

---

## 📝 Checklist de Verificación

- [ ] Compilación sin errores
- [ ] Librerías descargadas correctamente
- [ ] Display muestra "Sistema Listo"
- [ ] RFID lee tarjeta (UID en Serial)
- [ ] Servo se mueve suavemente
- [ ] Buzzer emite sonidos
- [ ] LEDs se encienden/apagan
- [ ] Sensor ultrasónico mide distancias
- [ ] Usuario autorizado levanta barra
- [ ] Usuario denegado rechaza acceso
- [ ] Cajones cambian de estado

---

¡Si todo funciona, tu sistema está listo para deployment! 🎉
