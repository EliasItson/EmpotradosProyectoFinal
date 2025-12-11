// =====================================================================
// ARCHIVO DE CONFIGURACIÓN - Personaliza aquí sin tocar main.cpp
// =====================================================================

#ifndef CONFIG_H
#define CONFIG_H

// ==================== PINES ====================
//

// RFID (SPI)
#define RFID_SS_PIN 5
#define RFID_RST_PIN 4

// Ultrasonico
#define SENSOR_ULTRASONIC_TRIG 14
#define SENSOR_ULTRASONIC_ECHO 33

// Switches de cajones (2 cajones configurados)
// Las otras posiciones del DIP no están usadas por este proyecto.
#define SWITCH_SLOT1 32
#define SWITCH_SLOT2 27

// Número de cajones configurados
#define SLOTS_COUNT 2

// Actuadores
#define SERVO_PIN 13
// Entrypoint servo (mantener por compatibilidad)
#define SERVO_ENTRY_PIN SERVO_PIN
// Servo para salida (segunda pluma)
#define SERVO_EXIT_PIN 15

// Leds indicadores de estado de cajones
#define LED_RED_SLOT1 25
#define LED_RED_SLOT2 26

// Display I2C (OLED SSD1306)
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// ==================== PARÁMETROS DE TIEMPO ====================

// Duraciones de mensajes en pantalla (ms)
#define DISPLAY_MESSAGE_MS 3000
#define SUCCESS_MESSAGE_MS 3000

// Tiempo mínimo entre lecturas RFID (evita duplicados)
#define RFID_COOLDOWN 2000

// Intervalo de chequeo del sensor ultrasónico
#define ULTRASONIC_CHECK_INTERVAL 100

// Timeout para que el ultrasónico espere que aparezca un auto (ms)
#define ULTRASONIC_TIMEOUT_MS 5000

// Tiempo de espera antes de bajar la barrera tras detectar que el auto se fue (ms)
#define LOWER_BARRIER_WAIT_MS 3000

// Timeout máximo en microsegundos para pulseIn() del ultrasonico
#define ULTRASONIC_PULSE_TIMEOUT_US 30000

// Duraciones para generar pulso TRIG (microsegundos)
#define ULTRASONIC_TRIG_PREP_US 2
#define ULTRASONIC_TRIG_PULSE_US 10

// ==================== CONFIGURACIÓN DEL SENSOR ULTRASÓNICO ====================

// Distancia en cm por la cual se considera que hay un obstáculo (vehículo bloqueando)
// Si distancia < THRESHOLD: hay algo bloqueando (auto presente)
// Si distancia >= THRESHOLD: camino libre (auto pasó o no hay auto)
#define ULTRASONIC_THRESHOLD 30
#define ULTRASONIC_FACTOR 0.0343

// ==================== CONFIGURACIÓN DEL SERVOMOTOR ====================

// Ángulo cuando la barra está ABAJO (cerrada)
#define SERVO_ANGLE_DOWN 0

// Ángulo cuando la barra está ARRIBA (abierta)
#define SERVO_ANGLE_UP 90

// Tiempo de transición del servo en ms
#define SERVO_TRANSITION_TIME 500

// Invertir mecánicamente el servo de salida?
// Si tu servo de salida está montado en sentido inverso respecto al de entrada,
// pon 1 para que el firmware escriba (180 - angle) al servo de salida.
#define SERVO_EXIT_INVERT 0

// ==================== CONFIGURACIÓN DE LA SECUENCIA DE SALIDA ====================

// Tiempo para levantar y bajar la barra de salida en ms
#define EXIT_RAISE_MS 3000
#define EXIT_LOWER_MS 3000

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

// Mensaje cuando no hay disponibilidad
#define MSG_FULL_1 "LLENO"
#define MSG_FULL_2 "Intente luego"

// Mensaje cuando la tarjeta es válida pero no se detecta que pase el carro
#define MSG_TIMEOUT_1 "No se detectó"
#define MSG_TIMEOUT_2 "Intentelo de nuevo"

// ==================== TARJETAS RFID AUTORIZADAS ====================

// Formato: "XX:XX:XX:XX"

static const char *AUTHORIZED_CARDS[] = {
    "1C:21:09:49", // Tarjeta 1
    "43:23:7A:1A"  // Tarjeta 2
};

static const int AUTHORIZED_CARDS_COUNT = 2;

// ==================== CONFIGURACIÓN AVANZADA ====================

// Baudrate del Serial Monitor
#define SERIAL_BAUD 115200

#endif // CONFIG_H

// NTP defaults
#define NTP_SERVER "pool.ntp.org"
#define NTP_TIMEOUT_MS 10000

// =====================================================================
// INSTRUCCIONES DE USO:
// 1. Modifica los valores de arriba según el hardware
// 2. NO toques main.cpp - todos los valores vienen de aquí
// 3. Recompila y sube a ESP32
// =====================================================================
