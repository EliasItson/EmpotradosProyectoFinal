// =====================================================================
// ARCHIVO DE CONFIGURACIÓN - Personaliza aquí sin tocar main.cpp
// =====================================================================

#ifndef CONFIG_H
#define CONFIG_H

// ==================== PINES ====================
// No cambies estos a menos que modifiques el hardware físico

// RFID (SPI)
#define RFID_SS_PIN     5
#define RFID_RST_PIN    27

// Sensores
#define SENSOR_ULTRASONIC_TRIG 32
#define SENSOR_ULTRASONIC_ECHO 33

// Switches de cajones
#define SWITCH_SLOT1   13
#define SWITCH_SLOT2   12
#define SWITCH_SLOT3   14
#define SWITCH_SLOT4   26

// Actuadores
#define SERVO_PIN      15
#define LED_RED_SLOT1  21
#define LED_RED_SLOT2  22
#define LED_RED_SLOT3  23
#define LED_RED_SLOT4  25
#define BUZZER_PIN     19

// Display LCD I2C
#define LCD_ADDRESS    0x27
#define LCD_COLS       16
#define LCD_ROWS       2

// ==================== PARÁMETROS DE TIEMPO ====================

// Tiempo mínimo entre lecturas RFID (evita duplicados)
#define RFID_COOLDOWN 2000

// Intervalo de chequeo del sensor ultrasónico
#define ULTRASONIC_CHECK_INTERVAL 100

// Tiempo máximo para que pase el vehículo después de levantar barra
#define BARRIER_TIMEOUT 5000

// ==================== CONFIGURACIÓN DEL SENSOR ULTRASÓNICO ====================

// Distancia en cm por la cual se considera que el vehículo pasó
#define ULTRASONIC_THRESHOLD 30

// Factor de calibración del sensor ultrasónico
// Fórmula: distancia = duration * ULTRASONIC_FACTOR / 2
// Velocidad del sonido: ~343 m/s a 20°C
// Valor estándar: 0.0343
// Si el sensor mide el doble: 0.0172 (0.0343 / 2)
// Si mide la mitad: 0.0686 (0.0343 * 2)
#define ULTRASONIC_FACTOR 0.0343

// ==================== CONFIGURACIÓN DEL SERVOMOTOR ====================

// Ángulo cuando la barra está ABAJO (cerrada)
#define SERVO_ANGLE_DOWN 180

// Ángulo cuando la barra está ARRIBA (abierta)
#define SERVO_ANGLE_UP 0

// Tiempo de transición del servo en ms
#define SERVO_TRANSITION_TIME 500

// ==================== BUZZER - CONFIGURACIÓN DE SONIDOS ====================

// SONIDO DE BIENVENIDA (usuario autorizado)
#define WELCOME_BEEPS 3        // Número de beeps
#define WELCOME_BEEP_DURATION 200   // Duración de cada beep (ms)
#define WELCOME_BEEP_PAUSE 100      // Pausa entre beeps (ms)

// SONIDO DE RECHAZO (usuario denegado)
#define DENIED_BEEPS 2         // Número de beeps
#define DENIED_BEEP_DURATION 500    // Duración de cada beep (ms)
#define DENIED_BEEP_PAUSE 200       // Pausa entre beeps (ms)

// ==================== MENSAJES DEL DISPLAY ====================

// Línea 1 y 2 pueden tener máximo 16 caracteres

// Mensajes de bienvenida
#define MSG_WELCOME_1 "Bienvenido!"
#define MSG_WELCOME_2 "Acceso concedido"

// Mensajes de acceso denegado
#define MSG_DENIED_1 "ACCESO"
#define MSG_DENIED_2 "DENEGADO"

// Mensajes de paso seguro
#define MSG_PASS_1 "Pase seguro"
#define MSG_PASS_2 "Gracias!"

// Mensaje inicial
#define MSG_READY_1 "Sistema Listo"
#define MSG_READY_2 "Esperando..."

// ==================== TARJETAS RFID AUTORIZADAS ====================

// Agrega aquí los UIDs de tus tarjetas
// Formato: "XX:XX:XX:XX"
// Para obtener UIDs, carga rfid_register.cpp

const String AUTHORIZED_CARDS[] = {
  "12:34:56:78",  // Tarjeta 1 - Reemplaza con UID real
  "87:65:43:21"   // Tarjeta 2 - Agrega más si necesitas
};

const int AUTHORIZED_CARDS_COUNT = 2;  // Actualiza este número

// ==================== MODOS DE DEBUG ====================

// Descomenta para activar modo debug (más mensajes en Serial)
// #define DEBUG_MODE

// Descomenta para ver distancias del ultrasónico en tiempo real
// #define DEBUG_ULTRASONIC

// Descomenta para ver estados de switches
// #define DEBUG_SWITCHES

// ==================== CONFIGURACIÓN AVANZADA ====================

// Baudrate del Serial Monitor
#define SERIAL_BAUD 115200

// Inicialización del LCD
// Cambio de dirección I2C común a 0x27, pero algunos módulos usan 0x3F
// Si el LCD no funciona, prueba: 0x20, 0x27, 0x3F, 0x3C

// I2C SDA (Data) - GPIO 21 (por defecto en ESP32)
// I2C SCL (Clock) - GPIO 22 (por defecto en ESP32)

#endif // CONFIG_H

// =====================================================================
// INSTRUCCIONES DE USO:
// 1. Modifica los valores de arriba según tu hardware
// 2. NO toques main.cpp - todos los valores vienen de aquí
// 3. Recompila y sube a ESP32
// =====================================================================
