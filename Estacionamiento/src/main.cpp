#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>
#include <NoDelay.h>
#include <WiFi.h>

#include "config.h"
#include "webserver.h"
#include "config_prefs.h"
#include "telemetry.h"
#include <ESPmDNS.h>

// ==================== VARIABLES GLOBALES ====================
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Servo barrierServoEntry;
Servo barrierServoExit;

bool slotOccupied[SLOTS_COUNT] = {false, false};
bool entranceBarrierRaised = false;
bool exitBarrierRaised = false;
bool deniedMessageActive = false;
bool authorizedMessageActive = false;
bool timeoutMessageActive = false;

noDelay rfidTimer(RFID_COOLDOWN);
noDelay ultrasonicTimer(ULTRASONIC_CHECK_INTERVAL);
noDelay servoTimer(SERVO_TRANSITION_TIME);
noDelay displayMessageTimer(DISPLAY_MESSAGE_MS);
noDelay successMessageTimer(SUCCESS_MESSAGE_MS);
noDelay telemetryTimer(5000);  // Enviar telemetria cada 5 segundos

noDelay exitRaiseTimer(EXIT_RAISE_MS);
noDelay exitLowerTimer(EXIT_LOWER_MS);
bool exitSequenceActive = false;
int exitPhase = 0; // 0 = INACTIVO, 1 = ESPERANDO PARA SUBIR, 2 = ESPERANDO PARA BAJAR

// Timeout: si no detecta auto, baja	barrera y cancela acceso
noDelay ultrasonicNoCarTimer(ULTRASONIC_TIMEOUT_MS); 
// Esperar antes de bajar
noDelay lowerBarrierWaitTimer(LOWER_BARRIER_WAIT_MS); 
// Flag: el auto fue detectado alguna vez
bool carDetectedRecently = false; 
// Flag: el auto está siendo detectado ahora
bool carCurrentlyDetected = false; 
// 0 = INACTIVO, 1 = ESPERANDO CARRO, 2 = ESPERANDO QUE PASE CARRO, 3 = BAJANDO
int entranceBarrierPhase = 0; 

int availableSlots = SLOTS_COUNT;
// Reservas temporales por accesos concedidos que aún no se han estacionado
int pendingEntries = 0;

// ==================== VARIABLES PARA WEB SERVER ====================
// Datos que expone la API para monitoreo
float currentDistance = 0.0;
String lastRFIDCard = "";
unsigned long systemUptime = 0;

// Variables configurables desde la API (inicialmente de config.h)
int ULTRASONIC_THRESHOLD_CONFIG = ULTRASONIC_THRESHOLD;
int ULTRASONIC_TIMEOUT_CONFIG = ULTRASONIC_TIMEOUT_MS;
int LOWER_BARRIER_WAIT_CONFIG = LOWER_BARRIER_WAIT_MS;
int DISPLAY_MESSAGE_CONFIG = DISPLAY_MESSAGE_MS;

// Declaraciones
void setupSensors();
void setupActuators();
void checkRFID();
String getCardUID();
bool isCardAuthorized(String cardUID);
void handleAuthorizedUser();
void handleUnauthorizedUser();
void checkUltrasonicSensor();
float getUltrasonicDistance();
void raiseEntranceBarrier();
void lowerEntranceBarrier();
void raiseExitBarrier();
void lowerExitBarrier();
void checkParkingSlots();
void updateLED(int slot, bool occupied);
void displayMessage(String line1, String line2 = "");
void clearDisplay();
void updateBarrierLogic();
void updateDisplayLogic();
void displayAvailableSlots();

void setup() {
	Serial.begin(SERIAL_BAUD);
	setupSensors();
	setupActuators();
	// Inicializar I2C explícitamente con pines definidos en config.h
	Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
	// Inicializar pantalla SSD1306
	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
		Serial.println("SSD1306 allocation failed");
		for (;;) delay(10);
	}
	display.clearDisplay();
	display.display();
	// Mostrar estado inicial con contador de espacios disponibles

	// Cargar configuracion persistente (si existe)
	loadConfigPrefs();

	// Inicializar telemetria
	initTelemetry();

	// Mostrar estado inicial con contador de espacios disponibles
	displayAvailableSlots();

	// Conectar WiFi y arrancar servidor web si está habilitado
#if WIFI_AUTO_START
	Serial.printf("[WIFI] Iniciando conexión a SSID: %s\n", WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	unsigned long start = millis();
	const unsigned long timeout = 15000; // 15s
	while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
		delay(200);
		Serial.print('.');
	}
	if (WiFi.status() == WL_CONNECTED) {
		Serial.println();
		Serial.print("[WIFI] Conectado. IP: ");
		Serial.println(WiFi.localIP());
		// iniciar mDNS
		if (MDNS.begin(MDNS_NAME)) {
			Serial.printf("[MDNS] Resuelto como http://%s.local\n", MDNS_NAME);
		} else {
			Serial.println("[MDNS] No se pudo iniciar mDNS");
		}
		// Inicializar servidor web
		initWebServer();
		// Conectar a servidor de telemetria
		connectTelemetry();
	} else {
		Serial.println();
		Serial.println("[WIFI] No se pudo conectar dentro del timeout. Web server no iniciado.");
	}
#else
	Serial.println("[WIFI] Auto-start deshabilitado. Para habilitar, configurar WIFI_AUTO_START en config.h");
#endif
}

void loop() {
	// Actualizar uptime (en milisegundos, la API lo convertira a segundos)
	systemUptime = millis();
	
	updateBarrierLogic();
	updateDisplayLogic();
	checkRFID();
	checkUltrasonicSensor();
	checkParkingSlots();
	
	// Enviar telemetria periodicamente (cada 5s)
	if (telemetryTimer.update()) {
		sendSensorReading(currentDistance, systemUptime / 1000, availableSlots);
	}
}

void setupSensors() {
	SPI.begin();
	rfid.PCD_Init();
	pinMode(SENSOR_ULTRASONIC_TRIG, OUTPUT);
	pinMode(SENSOR_ULTRASONIC_ECHO, INPUT);
	pinMode(SWITCH_SLOT1, INPUT_PULLUP);
	pinMode(SWITCH_SLOT2, INPUT_PULLUP);
}

void setupActuators() {
	barrierServoEntry.attach(SERVO_ENTRY_PIN);
	barrierServoExit.attach(SERVO_EXIT_PIN);
	barrierServoEntry.write(SERVO_ANGLE_DOWN);
	barrierServoExit.write(SERVO_ANGLE_DOWN);
	entranceBarrierRaised = false;
	exitBarrierRaised = false;
	pinMode(LED_RED_SLOT1, OUTPUT);
	pinMode(LED_RED_SLOT2, OUTPUT);
}

void checkRFID() {
	if (!rfidTimer.update()) return;
	if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
	String cardUID = getCardUID();
	if (isCardAuthorized(cardUID)) handleAuthorizedUser(); else handleUnauthorizedUser();
	rfid.PICC_HaltA();
	rfid.PCD_StopCrypto1();
}

String getCardUID() {
	String uid = "";
	for (int i = 0; i < rfid.uid.size; i++) {
		if (rfid.uid.uidByte[i] < 0x10) uid += "0";
		uid += String(rfid.uid.uidByte[i], HEX);
		if (i < rfid.uid.size - 1) uid += ":";
	}
	uid.toUpperCase();
	return uid;
}

bool isCardAuthorized(String cardUID) {
	for (int i = 0; i < AUTHORIZED_CARDS_COUNT; i++) {
		if (cardUID.equalsIgnoreCase(String(AUTHORIZED_CARDS[i]))) return true;
	}
	return false;
}

void handleAuthorizedUser() {
	// Verificar disponibilidad antes de conceder acceso
	if (availableSlots <= 0) {
		displayMessage(MSG_FULL_1, MSG_FULL_2);
		displayMessageTimer.start();
		deniedMessageActive = true;
		return;
	}

	// Actualizar variable global de última tarjeta RFID
	lastRFIDCard = getCardUID();

	// Crear una reserva temporal: decremento real ocurrirá cuando el usuario
	// confirme ocupación presionando el switch del cajón.
	pendingEntries++;
	Serial.println("[RFID] Tarjeta válida. Levantando pluma y esperando auto...");
	displayMessage(MSG_WELCOME_1, MSG_WELCOME_2);
	raiseEntranceBarrier();
	// Iniciar la lógica de espera: o detecta auto, o timeout en 10s
	entranceBarrierPhase = 1; // Esperar a que el auto pase
	carDetectedRecently = false;
	carCurrentlyDetected = false;
	ultrasonicNoCarTimer.start();
	successMessageTimer.start();
	authorizedMessageActive = true;
}

void handleUnauthorizedUser() {
	displayMessage(MSG_DENIED_1, MSG_DENIED_2);
	displayMessageTimer.start();
	deniedMessageActive = true;
	// Enviar evento RFID denegado
	String cardUID = getCardUID();
	sendRFIDAccess(cardUID.c_str(), false, "unauthorized");
}

void checkUltrasonicSensor() {
	if (!entranceBarrierRaised || entranceBarrierPhase == 0) return;
	if (!ultrasonicTimer.update()) return;
	
	// Generar pulso en TRIG
	digitalWrite(SENSOR_ULTRASONIC_TRIG, LOW);
	delayMicroseconds(ULTRASONIC_TRIG_PREP_US);
	digitalWrite(SENSOR_ULTRASONIC_TRIG, HIGH);
	delayMicroseconds(ULTRASONIC_TRIG_PULSE_US);
	digitalWrite(SENSOR_ULTRASONIC_TRIG, LOW);

	// Leer duración del pulso en ECHO
	unsigned long duration = pulseIn(SENSOR_ULTRASONIC_ECHO, HIGH, ULTRASONIC_PULSE_TIMEOUT_US);
	
	// Convertir duración a distancia (cm)
	float distance = duration * ULTRASONIC_FACTOR / 2;
	
	// Actualizar variable global para la API web
	currentDistance = distance;
	
	Serial.printf("[US] Duration: %lu us | Distancia: %f cm\n", duration, distance);
	
	// Validar que la distancia sea razonable (entre 2cm y 400cm)
	if (distance < 2 || distance > 400) {
		Serial.printf("[US] Lectura inválida: %f cm\n", distance);
		return;
	}
	
	bool carDetected = (distance < ULTRASONIC_THRESHOLD); // < 30cm = bloqueado = hay auto
	
	Serial.printf("[US] Distancia: %f cm | Detectado: %d | Fase: %d\n", distance, carDetected, entranceBarrierPhase);
	
	if (entranceBarrierPhase == 1) {
		// Esperando a que el auto pase
		if (carDetected) {
			// Auto detectado (bloqueando sensor)
			if (!carCurrentlyDetected) {
				carCurrentlyDetected = true;
				carDetectedRecently = true;
				Serial.println("[US] Auto detectado: sensor bloqueado");
			}
			// Resetear timeout mientras el auto esté siendo detectado
			ultrasonicNoCarTimer.start();
		}
		if (!carDetected && carCurrentlyDetected && carDetectedRecently) {
			// Auto pasó: se detectó antes (estaba bloqueado), ahora se fue (desbloqueado)
			carCurrentlyDetected = false;
			entranceBarrierPhase = 3; // Bajar después de 1s
			lowerBarrierWaitTimer.start();
			Serial.println("[US] Auto pasó: sensor desbloqueado. Bajando pluma...");
		}
	}
}

void raiseEntranceBarrier() { barrierServoEntry.write(SERVO_ANGLE_UP); entranceBarrierRaised = true; servoTimer.start(); }
void lowerEntranceBarrier() { barrierServoEntry.write(SERVO_ANGLE_DOWN); entranceBarrierRaised = false; servoTimer.start(); }

void raiseExitBarrier() { barrierServoExit.write(SERVO_ANGLE_UP); exitBarrierRaised = true; servoTimer.start(); }
void lowerExitBarrier() { barrierServoExit.write(SERVO_ANGLE_DOWN); exitBarrierRaised = false; servoTimer.start(); }

void checkParkingSlots() {
	int slots[SLOTS_COUNT] = {SWITCH_SLOT1, SWITCH_SLOT2};
	for (int i = 0; i < SLOTS_COUNT; i++) {
		bool pressed = (digitalRead(slots[i]) == LOW);
		// Entrada al cajón (usuario acaba de estacionar)
		if (pressed && !slotOccupied[i]) {
			slotOccupied[i] = true;
			updateLED(i, true);
			// Si hay reservas pendientes, asociar una a esta ocupación.
			if (pendingEntries > 0) {
				pendingEntries--;
				if (availableSlots > 0) availableSlots--;
			} else {
				// Ocupación manual sin reserva previa
				if (availableSlots > 0) availableSlots--;
			}
			if (availableSlots < 0) availableSlots = 0;
			Serial.printf("Cajon %d - OCUPADO. Disponibles: %d\n", i + 1, availableSlots);
			// Enviar telemetria de ocupacion
			sendSlotOccupancy(i + 1, true);
			// Actualizar contador en pantalla si no hay mensajes temporales activos
			if (!deniedMessageActive && !authorizedMessageActive && !timeoutMessageActive) {
				displayAvailableSlots();
			}
		}
		// Salida del cajón (usuario marcó que desocupó)
		else if (!pressed && slotOccupied[i]) {
			slotOccupied[i] = false;
			updateLED(i, false);
			availableSlots++;
			Serial.printf("Cajon %d - DISPONIBLE. Disponibles: %d\n", i + 1, availableSlots);
			// Enviar telemetria de ocupacion
			sendSlotOccupancy(i + 1, false);
			// Actualizar contador en pantalla si no hay mensajes temporales activos
			if (!deniedMessageActive && !authorizedMessageActive && !timeoutMessageActive) {
				displayAvailableSlots();
			}
			// Iniciar secuencia de salida que levanta la pluma y luego la baja
			exitSequenceActive = true;
			exitPhase = 1;
			exitRaiseTimer.start();
		}
	}
}

void updateLED(int slot, bool occupied) {
	int leds[SLOTS_COUNT] = {LED_RED_SLOT1, LED_RED_SLOT2};
	digitalWrite(leds[slot], occupied ? HIGH : LOW);
}

void displayMessage(String line1, String line2) {
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.print(line1);
	if (line2.length()) {
		// second line starting a bit lower (10 px)
		display.setCursor(0, 12);
		display.print(line2);
	}
	display.display();
}
void clearDisplay() { display.clearDisplay(); display.display(); }

// Extensión: manejar la secuencia de salida (timers) además del timeout
void updateBarrierLogic() {
	// Lógica para pluma de entrada
	if (entranceBarrierRaised && entranceBarrierPhase > 0) {
		// Fase 1: esperando a que pase el auto (timeout 10s - esperar más tiempo a detección real)
		if (entranceBarrierPhase == 1 && ultrasonicNoCarTimer.update()) {
			// Timeout: no se detectó auto en 10s, bajar pluma
			entranceBarrierPhase = 3;
			lowerBarrierWaitTimer.start();
			Serial.println("[TIMEOUT] No se detectó auto. Bajando pluma...");
			// Mostrar mensaje distinto cuando nunca se detectó el auto
			displayMessage(MSG_TIMEOUT_1, MSG_TIMEOUT_2);
			displayMessageTimer.start();
			timeoutMessageActive = true;
		}
		// Fase 3: esperar 1s antes de bajar
		if (entranceBarrierPhase == 3 && lowerBarrierWaitTimer.update()) {
			lowerEntranceBarrier();
			entranceBarrierPhase = 0;
			// Mostrar mensaje de paso y activar el flag para que se restaure luego
			displayMessage(MSG_PASS_1, MSG_PASS_2);
			authorizedMessageActive = true;
			successMessageTimer.start();
		}
	}

	// Secuencia de salida (usa la pluma de salida)
	if (exitSequenceActive) {
		if (exitPhase == 1 && exitRaiseTimer.update()) {
			raiseExitBarrier();
			// ahora esperamos para bajar
			exitPhase = 2;
			exitLowerTimer.start();
		} else if (exitPhase == 2 && exitLowerTimer.update()) {
			lowerExitBarrier();
			exitSequenceActive = false;
			exitPhase = 0;
		}
	}
}
void updateDisplayLogic() {
	if (deniedMessageActive && displayMessageTimer.update()) {
		deniedMessageActive = false;
		// Restaurar pantalla por defecto con contador
		displayAvailableSlots();
	}
	if (authorizedMessageActive && successMessageTimer.update()) {
		authorizedMessageActive = false;
		// Restaurar pantalla por defecto con contador
		displayAvailableSlots();
	}
	if (timeoutMessageActive && displayMessageTimer.update()) {
		timeoutMessageActive = false;
		// Restaurar pantalla por defecto con contador
		displayAvailableSlots();
	}
}

// Muestra en el OLED la cantidad de espacios disponibles cuando no hay mensajes
void displayAvailableSlots() {
	String line2 = String("Disp: ") + String(availableSlots);
	// Asegurar que la línea tiene máximo 16 caracteres
	if (line2.length() > 16) line2 = line2.substring(0, 16);
	displayMessage(MSG_READY_1, line2);
}


