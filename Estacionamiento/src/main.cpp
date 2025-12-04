#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>
#include <NoDelay.h>

#include "config.h"

// ==================== VARIABLES GLOBALES ====================
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
Servo barrierServoEntry;
Servo barrierServoExit;

bool slotOccupied[SLOTS_COUNT] = {false, false, false, false};
bool entranceBarrierRaised = false;
bool exitBarrierRaised = false;
bool deniedMessageActive = false;
bool authorizedMessageActive = false;

noDelay rfidTimer(RFID_COOLDOWN);
noDelay ultrasonicTimer(ULTRASONIC_CHECK_INTERVAL);
noDelay servoTimer(SERVO_TRANSITION_TIME);
noDelay displayMessageTimer(3000);
noDelay successMessageTimer(2000);
noDelay barrierTimeoutTimer(BARRIER_TIMEOUT);
noDelay buzzerOnTimer(200);
noDelay buzzerOffTimer(100);

// Secuencia de salida (timers)
noDelay exitRaiseTimer(EXIT_RAISE_MS);
noDelay exitLowerTimer(EXIT_LOWER_MS);
bool exitSequenceActive = false;
int exitPhase = 0; // 0 = idle, 1 = waiting to raise, 2 = waiting to lower

bool buzzerPlaying = false;
int buzzerBeepCount = 0;
int buzzerBeepsNeeded = 0;
bool buzzerPhaseOn = false;
int buzzerOnDuration = 200;
int buzzerOffDuration = 100;

int availableSlots = SLOTS_COUNT;
// Reservas temporales por accesos concedidos que aún no se han estacionado
int pendingEntries = 0;

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
void playWelcomeSound();
void playDeniedSound();
void updateBuzzer();
void displayMessage(String line1, String line2 = "");
void clearDisplay();
void updateBarrierLogic();
void updateDisplayLogic();

void setup() {
	Serial.begin(115200);
	setupSensors();
	setupActuators();
	lcd.init();
	lcd.backlight();
	displayMessage("Sistema Listo", "Esperando...");
}

void loop() {
	updateBuzzer();
	updateBarrierLogic();
	updateDisplayLogic();
	checkRFID();
	checkUltrasonicSensor();
	checkParkingSlots();
}

void setupSensors() {
	SPI.begin();
	rfid.PCD_Init();
	pinMode(SENSOR_ULTRASONIC_TRIG, OUTPUT);
	pinMode(SENSOR_ULTRASONIC_ECHO, INPUT);
	pinMode(SWITCH_SLOT1, INPUT_PULLUP);
	pinMode(SWITCH_SLOT2, INPUT_PULLUP);
	pinMode(SWITCH_SLOT3, INPUT_PULLUP);
	pinMode(SWITCH_SLOT4, INPUT_PULLUP);
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
	pinMode(LED_RED_SLOT3, OUTPUT);
	pinMode(LED_RED_SLOT4, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);
	digitalWrite(BUZZER_PIN, LOW);
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
		playDeniedSound();
		displayMessageTimer.start();
		deniedMessageActive = true;
		return;
	}

	// Crear una reserva temporal: decremento real ocurrirá cuando el usuario
	// confirme ocupación presionando el switch del cajón.
	pendingEntries++;
	displayMessage(MSG_WELCOME_1, MSG_WELCOME_2);
	playWelcomeSound();
	raiseEntranceBarrier();
	barrierTimeoutTimer.start();
	successMessageTimer.start();
	authorizedMessageActive = true;
}

void handleUnauthorizedUser() {
	displayMessage("ACCESO", "DENEGADO");
	playDeniedSound();
	displayMessageTimer.start();
	deniedMessageActive = true;
}

void checkUltrasonicSensor() {
	if (!entranceBarrierRaised) return;
	if (!ultrasonicTimer.update()) return;
	unsigned long duration = pulseIn(SENSOR_ULTRASONIC_ECHO, HIGH, 30000);
	float distance = duration * ULTRASONIC_FACTOR / 2;
	if (distance > ULTRASONIC_THRESHOLD) { lowerEntranceBarrier(); displayMessage(MSG_PASS_1, MSG_PASS_2); successMessageTimer.start(); }
}

void raiseEntranceBarrier() { barrierServoEntry.write(SERVO_ANGLE_UP); entranceBarrierRaised = true; servoTimer.start(); }
void lowerEntranceBarrier() { barrierServoEntry.write(SERVO_ANGLE_DOWN); entranceBarrierRaised = false; servoTimer.start(); }

void raiseExitBarrier() { barrierServoExit.write(SERVO_ANGLE_UP); exitBarrierRaised = true; servoTimer.start(); }
void lowerExitBarrier() { barrierServoExit.write(SERVO_ANGLE_DOWN); exitBarrierRaised = false; servoTimer.start(); }

void checkParkingSlots() {
	int slots[SLOTS_COUNT] = {SWITCH_SLOT1, SWITCH_SLOT2, SWITCH_SLOT3, SWITCH_SLOT4};
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
		}
		// Salida del cajón (usuario marcó que desocupó)
		else if (!pressed && slotOccupied[i]) {
			slotOccupied[i] = false;
			updateLED(i, false);
			availableSlots++;
			Serial.printf("Cajon %d - DISPONIBLE. Disponibles: %d\n", i + 1, availableSlots);
			// Iniciar secuencia de salida que levanta la pluma y luego la baja
			exitSequenceActive = true;
			exitPhase = 1;
			exitRaiseTimer.start();
		}
	}
}

void updateLED(int slot, bool occupied) {
	int leds[4] = {LED_RED_SLOT1, LED_RED_SLOT2, LED_RED_SLOT3, LED_RED_SLOT4};
	digitalWrite(leds[slot], occupied ? HIGH : LOW);
}

void displayMessage(String line1, String line2) { lcd.clear(); lcd.setCursor(0,0); lcd.print(line1); if (line2.length()) { lcd.setCursor(0,1); lcd.print(line2); } }
void clearDisplay() { lcd.clear(); }

// Extensión: manejar la secuencia de salida (timers) además del timeout
void updateBarrierLogic() {
	// Lógica para pluma de entrada
	if (entranceBarrierRaised && barrierTimeoutTimer.update()) {
		lowerEntranceBarrier();
		displayMessage("Timeout","Barra bajada");
		successMessageTimer.start();
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
void updateDisplayLogic() { if (deniedMessageActive && displayMessageTimer.update()) { deniedMessageActive = false; displayMessage("Sistema Listo","Esperando..."); } if (authorizedMessageActive && successMessageTimer.update()) { authorizedMessageActive = false; displayMessage("Sistema Listo","Esperando..."); } }

void playWelcomeSound() {
	buzzerBeepsNeeded = WELCOME_BEEPS;
	buzzerBeepCount = 0;
	buzzerPlaying = true;
	buzzerPhaseOn = true;
	buzzerOnDuration = WELCOME_BEEP_DURATION;
	buzzerOffDuration = WELCOME_BEEP_PAUSE;
	buzzerOnTimer.start();
	digitalWrite(BUZZER_PIN, HIGH);
}

void playDeniedSound() {
	buzzerBeepsNeeded = DENIED_BEEPS;
	buzzerBeepCount = 0;
	buzzerPlaying = true;
	buzzerPhaseOn = true;
	buzzerOnDuration = DENIED_BEEP_DURATION;
	buzzerOffDuration = DENIED_BEEP_PAUSE;
	buzzerOnTimer.start();
	digitalWrite(BUZZER_PIN, HIGH);
}

void updateBuzzer() {
	if (!buzzerPlaying) return;
	if (buzzerPhaseOn) {
		if (buzzerOnTimer.update()) { digitalWrite(BUZZER_PIN, LOW); buzzerPhaseOn = false; buzzerOffTimer.start(); }
	} else {
		if (buzzerOffTimer.update()) {
			buzzerBeepCount++;
			if (buzzerBeepCount < buzzerBeepsNeeded) { digitalWrite(BUZZER_PIN, HIGH); buzzerPhaseOn = true; buzzerOnTimer.start(); }
			else { buzzerPlaying = false; digitalWrite(BUZZER_PIN, LOW); }
		}
	}
}

