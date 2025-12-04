#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>
#include <NoDelay.h>

// ==================== CONFIGURACIÓN DE PINES ====================
// RFID Reader (SPI)
#define RFID_SS_PIN     5    // Chip Select
#define RFID_RST_PIN    27   // Reset

// Sensores
#define SENSOR_ULTRASONIC_TRIG 32  // Ultrasónico - Trigger
#define SENSOR_ULTRASONIC_ECHO 33  // Ultrasónico - Echo
#define SWITCH_SLOT1   13   // Botón para cajón 1
#define SWITCH_SLOT2   12   // Botón para cajón 2
#define SWITCH_SLOT3   14   // Botón para cajón 3
#define SWITCH_SLOT4   26   // Botón para cajón 4

// Actuadores
#define SERVO_PIN      15   // Servomotor
#define LED_RED_SLOT1  21   // LED rojo cajón 1
#define LED_RED_SLOT2  22   // LED rojo cajón 2
#define LED_RED_SLOT3  23   // LED rojo cajón 3
#define LED_RED_SLOT4  25   // LED rojo cajón 4
#define BUZZER_PIN     19   // Buzzer
#define LCD_ADDRESS    0x27 // Dirección I2C del LCD (16x2)

// ==================== VARIABLES GLOBALES ====================
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
Servo barrierServo;

const String AUTHORIZED_CARDS[] = {"12:34:56:78", "87:65:43:21"};
const int AUTHORIZED_CARDS_COUNT = 2;

bool slotOccupied[4] = {false, false, false, false};
bool barrierRaised = false;
bool deniedMessageActive = false;
bool authorizedMessageActive = false;

noDelay rfidTimer(2000);
noDelay ultrasonicTimer(100);
noDelay servoTimer(500);
noDelay displayMessageTimer(3000);
noDelay successMessageTimer(2000);
noDelay barrierTimeoutTimer(5000);
noDelay buzzerOnTimer(200);
noDelay buzzerOffTimer(100);

bool buzzerPlaying = false;
int buzzerBeepCount = 0;
int buzzerBeepsNeeded = 0;
bool buzzerPhaseOn = false;
int buzzerOnDuration = 200;
int buzzerOffDuration = 100;

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
void raiseBarrier();
void lowerBarrier();
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
	barrierServo.attach(SERVO_PIN);
	barrierServo.write(180);
	barrierRaised = false;
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
	for (int i = 0; i < AUTHORIZED_CARDS_COUNT; i++) if (cardUID.equalsIgnoreCase(AUTHORIZED_CARDS[i])) return true;
	return false;
}

void handleAuthorizedUser() {
	displayMessage("Bienvenido!", "Acceso concedido");
	playWelcomeSound();
	raiseBarrier();
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
	if (!barrierRaised) return;
	if (!ultrasonicTimer.update()) return;
	unsigned long duration = pulseIn(SENSOR_ULTRASONIC_ECHO, HIGH, 30000);
	float distance = duration * 0.0343 / 2;
	if (distance > 30) { lowerBarrier(); displayMessage("Pase seguro", "Gracias!"); successMessageTimer.start(); }
}

void raiseBarrier() { barrierServo.write(0); barrierRaised = true; servoTimer.start(); }
void lowerBarrier() { barrierServo.write(180); barrierRaised = false; servoTimer.start(); }

void checkParkingSlots() {
	int slots[4] = {SWITCH_SLOT1, SWITCH_SLOT2, SWITCH_SLOT3, SWITCH_SLOT4};
	for (int i = 0; i < 4; i++) {
		bool pressed = (digitalRead(slots[i]) == LOW);
		if (pressed && !slotOccupied[i]) { slotOccupied[i] = true; updateLED(i, true); }
		else if (!pressed && slotOccupied[i]) { slotOccupied[i] = false; updateLED(i, false); }
	}
}

void updateLED(int slot, bool occupied) {
	int leds[4] = {LED_RED_SLOT1, LED_RED_SLOT2, LED_RED_SLOT3, LED_RED_SLOT4};
	digitalWrite(leds[slot], occupied ? HIGH : LOW);
}

void displayMessage(String line1, String line2) { lcd.clear(); lcd.setCursor(0,0); lcd.print(line1); if (line2.length()) { lcd.setCursor(0,1); lcd.print(line2); } }
void clearDisplay() { lcd.clear(); }

void updateBarrierLogic() { if (barrierRaised && barrierTimeoutTimer.update()) { lowerBarrier(); displayMessage("Timeout","Barra bajada"); successMessageTimer.start(); } }
void updateDisplayLogic() { if (deniedMessageActive && displayMessageTimer.update()) { deniedMessageActive = false; displayMessage("Sistema Listo","Esperando..."); } if (authorizedMessageActive && successMessageTimer.update()) { authorizedMessageActive = false; displayMessage("Sistema Listo","Esperando..."); } }

void playWelcomeSound() { buzzerBeepsNeeded = 3; buzzerBeepCount = 0; buzzerPlaying = true; buzzerPhaseOn = true; buzzerOnDuration = 200; buzzerOffDuration = 100; buzzerOnTimer.start(); digitalWrite(BUZZER_PIN, HIGH); }
void playDeniedSound() { buzzerBeepsNeeded = 2; buzzerBeepCount = 0; buzzerPlaying = true; buzzerPhaseOn = true; buzzerOnDuration = 500; buzzerOffDuration = 200; buzzerOnTimer.start(); digitalWrite(BUZZER_PIN, HIGH); }

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

