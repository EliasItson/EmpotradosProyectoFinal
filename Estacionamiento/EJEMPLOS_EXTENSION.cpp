// ============================================================================
// EJEMPLOS DE CÓDIGO - Extensiones y Modificaciones
// ============================================================================

// Estos son ejemplos que puedes agregar a main.cpp para ampliar funcionalidad

// ============================================================================
// 1. GUARDAR TARJETAS EN EEPROM (No volátil)
// ============================================================================

/*
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define EEPROM_OFFSET_CARD_COUNT 0
#define EEPROM_OFFSET_CARDS 4

// Leer tarjetas guardadas de EEPROM al iniciar
void loadAuthorizedCardsFromEEPROM() {
  int cardCount = EEPROM.read(EEPROM_OFFSET_CARD_COUNT);
  Serial.print("Tarjetas guardadas: ");
  Serial.println(cardCount);
  
  for (int i = 0; i < cardCount && i < 10; i++) {
    int offset = EEPROM_OFFSET_CARDS + (i * 12);
    // Leer UID (12 bytes: XX:XX:XX:XX)
    for (int j = 0; j < 12; j++) {
      // authorizedCards[i][j] = EEPROM.read(offset + j);
    }
  }
}

// Guardar nueva tarjeta en EEPROM
void saveCardToEEPROM(String cardUID, int index) {
  if (index >= 10) return; // Máximo 10 tarjetas
  
  int offset = EEPROM_OFFSET_CARDS + (index * 12);
  for (int i = 0; i < cardUID.length() && i < 12; i++) {
    EEPROM.write(offset + i, cardUID[i]);
  }
  
  int cardCount = EEPROM.read(EEPROM_OFFSET_CARD_COUNT);
  EEPROM.write(EEPROM_OFFSET_CARD_COUNT, cardCount + 1);
  EEPROM.commit();
  
  Serial.println("Tarjeta guardada en EEPROM");
}
*/

// ============================================================================
// 2. HISTORIAL DE ACCESOS (con Timestamp)
// ============================================================================

/*
#include <time.h>

typedef struct {
  unsigned long timestamp;
  String cardUID;
  bool authorized;
  char message[50];
} AccessLog;

AccessLog accessHistory[100];
int logIndex = 0;

void logAccess(String cardUID, bool authorized, const char* message) {
  if (logIndex >= 100) logIndex = 0; // Circular buffer
  
  accessHistory[logIndex].timestamp = millis();
  accessHistory[logIndex].cardUID = cardUID;
  accessHistory[logIndex].authorized = authorized;
  strcpy(accessHistory[logIndex].message, message);
  
  logIndex++;
  
  Serial.print("LOG: ");
  Serial.print(accessHistory[logIndex-1].timestamp);
  Serial.print(" - ");
  Serial.print(cardUID);
  Serial.print(" - ");
  Serial.println(authorized ? "✅" : "❌");
}

// Imprimir últimos N accesos
void printLastAccessLog(int count) {
  Serial.println("\n=== ÚLTIMOS ACCESOS ===");
  int start = (logIndex - count > 0) ? logIndex - count : 0;
  
  for (int i = start; i < logIndex; i++) {
    Serial.print(accessHistory[i].timestamp);
    Serial.print(" - ");
    Serial.print(accessHistory[i].cardUID);
    Serial.print(" - ");
    Serial.println(accessHistory[i].authorized ? "✅" : "❌");
  }
}
*/

// ============================================================================
// 3. INTERFAZ SERIAL PARA AGREGAR TARJETAS EN TIEMPO REAL
// ============================================================================

/*
void handleSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "help") {
      Serial.println("Comandos disponibles:");
      Serial.println("  list - Ver tarjetas autorizadas");
      Serial.println("  add <UID> - Agregar tarjeta");
      Serial.println("  remove <UID> - Remover tarjeta");
      Serial.println("  status - Estado del sistema");
      Serial.println("  restart - Reiniciar ESP32");
    }
    else if (command == "list") {
      for (int i = 0; i < AUTHORIZED_CARDS_COUNT; i++) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(AUTHORIZED_CARDS[i]);
      }
    }
    else if (command.startsWith("add ")) {
      String uid = command.substring(4);
      // Agregar dinámicamente (requiere modificar array)
      Serial.print("Agregando: ");
      Serial.println(uid);
    }
    else if (command == "status") {
      Serial.println("=== ESTADO DEL SISTEMA ===");
      Serial.print("Barrera: ");
      Serial.println(barrierRaised ? "ARRIBA" : "ABAJO");
      Serial.print("Slot 1: ");
      Serial.println(slotOccupied[0] ? "OCUPADO" : "LIBRE");
      Serial.print("Slot 2: ");
      Serial.println(slotOccupied[1] ? "OCUPADO" : "LIBRE");
      Serial.print("Slot 3: ");
      Serial.println(slotOccupied[2] ? "OCUPADO" : "LIBRE");
      Serial.print("Slot 4: ");
      Serial.println(slotOccupied[3] ? "OCUPADO" : "LIBRE");
    }
  }
}

// Agregar a loop():
// handleSerialCommand();
*/

// ============================================================================
// 4. TONOS PERSONALIZADOS PARA BUZZER
// ============================================================================

/*
// Tonos pre-programados
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440

// Tocar melodía
void playMelody(int* notes, int* durations, int length) {
  for (int i = 0; i < length; i++) {
    tone(BUZZER_PIN, notes[i]);
    delay(durations[i]);
    noTone(BUZZER_PIN);
    delay(50);
  }
}

// Ejemplo: Melodía de bienvenida
void playWelcomeMelody() {
  int notes[] = {NOTE_C4, NOTE_E4, NOTE_G4};
  int durations[] = {500, 500, 1000};
  playMelody(notes, durations, 3);
}

// Ejemplo: Melodía de rechazo (descending)
void playDeniedMelody() {
  int notes[] = {NOTE_G4, NOTE_E4, NOTE_C4};
  int durations[] = {500, 500, 1000};
  playMelody(notes, durations, 3);
}

// Reemplazar en main.cpp:
// playWelcomeMelody() en lugar de playWelcomeSound()
// playDeniedMelody() en lugar de playDeniedSound()
*/

// ============================================================================
// 5. NOTIFICACIÓN POR WiFi (ESP32 + WiFi)
// ============================================================================

/*
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "TuWiFi";
const char* password = "TuPassword";
const char* webhookURL = "https://tu-servidor.com/access";

void setupWiFi() {
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado");
    Serial.println(WiFi.localIP());
  }
}

void sendAccessNotification(String cardUID, bool authorized) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String payload = "{\"uid\":\"" + cardUID + "\",\"authorized\":" 
                     + (authorized ? "true" : "false") + "}";
    
    http.begin(webhookURL);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(payload);
    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);
    
    http.end();
  }
}

// Agregar al handleAuthorizedUser():
// sendAccessNotification(cardUID, true);
*/

// ============================================================================
// 6. SENSOR DE OCUPACIÓN AUTOMÁTICA (Infrarrojo)
// ============================================================================

/*
#define IR_SLOT1 35  // GPIO 35 (entrada analógica)
#define IR_SLOT2 34

void checkAutomaticOccupancy() {
  // Lecturas analógicas
  int ir1 = analogRead(IR_SLOT1);
  int ir2 = analogRead(IR_SLOT2);
  
  // Si IR detecta objeto: < 1000 (ajustar según sensor)
  bool occupied1 = ir1 < 1000;
  bool occupied2 = ir2 < 1000;
  
  if (occupied1 != slotOccupied[0]) {
    slotOccupied[0] = occupied1;
    updateLED(0, occupied1);
    Serial.print("Slot 1 auto: ");
    Serial.println(occupied1 ? "OCUPADO" : "LIBRE");
  }
  
  if (occupied2 != slotOccupied[1]) {
    slotOccupied[1] = occupied2;
    updateLED(1, occupied2);
    Serial.print("Slot 2 auto: ");
    Serial.println(occupied2 ? "OCUPADO" : "LIBRE");
  }
}

// Agregar a loop():
// checkAutomaticOccupancy();
*/

// ============================================================================
// 7. MODO MANTENIMIENTO (Abrir barra manualmente)
// ============================================================================

/*
#define MAINTENANCE_BUTTON 36  // GPIO con botón de mantenimiento

void checkMaintenanceMode() {
  if (digitalRead(MAINTENANCE_BUTTON) == LOW) {
    raiseBarrier();
    Serial.println("MODO MANTENIMIENTO: Barra levantada");
    
    // Mantener arriba mientras se presione
    while (digitalRead(MAINTENANCE_BUTTON) == LOW) {
      delay(100);
    }
    
    delay(1000);
    lowerBarrier();
    Serial.println("MODO MANTENIMIENTO: Barra bajada");
  }
}

// Agregar a setup():
// pinMode(MAINTENANCE_BUTTON, INPUT_PULLUP);

// Agregar a loop():
// checkMaintenanceMode();
*/

// ============================================================================
// 8. ESTADÍSTICAS EN TIEMPO REAL
// ============================================================================

/*
unsigned long accessCount = 0;
unsigned long deniedCount = 0;
unsigned long totalDistance = 0;
int distanceMeasurements = 0;

void recordStatistics(bool authorized) {
  if (authorized) {
    accessCount++;
  } else {
    deniedCount++;
  }
}

void printStatistics() {
  Serial.println("\n=== ESTADÍSTICAS ===");
  Serial.print("Accesos autorizados: ");
  Serial.println(accessCount);
  Serial.print("Accesos denegados: ");
  Serial.println(deniedCount);
  
  if (distanceMeasurements > 0) {
    float avgDistance = (float)totalDistance / distanceMeasurements;
    Serial.print("Distancia promedio: ");
    Serial.println(avgDistance);
  }
  
  float denialRate = (float)deniedCount / (accessCount + deniedCount) * 100;
  Serial.print("Tasa de rechazo: ");
  Serial.print(denialRate);
  Serial.println("%");
}

// En handleAuthorizedUser() y handleUnauthorizedUser():
// recordStatistics(true/false);

// En checkUltrasonicSensor():
// totalDistance += distance;
// distanceMeasurements++;
*/

// ============================================================================
// 9. PANTALLA LCD CON INFORMACIÓN DINÁMICA
// ============================================================================

/*
void displayDetailedStatus() {
  lcd.clear();
  
  // Línea 1: Información del estado
  lcd.setCursor(0, 0);
  if (barrierRaised) {
    lcd.print("Barra: ABIERTA ");
  } else {
    lcd.print("Barra: CERRADA ");
  }
  
  // Línea 2: Información de cajones
  lcd.setCursor(0, 1);
  int occupied = 0;
  for (int i = 0; i < 4; i++) {
    if (slotOccupied[i]) occupied++;
  }
  lcd.print("Ocupados: ");
  lcd.print(occupied);
  lcd.print("/4");
}

void displayWaitingForCar() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Esperando...");
  lcd.setCursor(0, 1);
  lcd.print("Distancia: ");
  
  float distance = getUltrasonicDistance();
  lcd.print(distance);
  lcd.print("cm");
}

// Llamar en checkUltrasonicSensor():
// displayWaitingForCar();
*/

// ============================================================================
// 10. ALERTAS DE ERROR Y DIAGNÓSTICO
// ============================================================================

/*
typedef enum {
  RFID_ERROR = 1,
  SERVO_ERROR = 2,
  ULTRASONIC_ERROR = 3,
  LCD_ERROR = 4,
  EEPROM_ERROR = 5
} ErrorCode;

void reportError(ErrorCode error) {
  Serial.print("❌ ERROR #");
  Serial.println(error);
  
  displayMessage("ERROR", "Revisar sistema");
  
  // Beep de error: 5 beeps rápidos
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void selfDiagnostics() {
  Serial.println("\n=== AUTO DIAGNÓSTICO ===");
  
  // Probar RFID
  if (!rfid.PCD_PeripheryRawRegisterRead(MFRC522::VersionReg)) {
    reportError(RFID_ERROR);
  }
  
  // Probar Servo
  barrierServo.write(90);
  delay(200);
  barrierServo.write(180);
  
  // Probar LCD
  displayMessage("Test", "OK");
  delay(500);
  
  Serial.println("Auto diagnóstico completado");
}

// Llamar en setup() o con botón:
// selfDiagnostics();
*/

// ============================================================================
// Fin de ejemplos
// ============================================================================

/*
INSTRUCCIONES PARA USAR ESTOS EJEMPLOS:

1. Descomenta el código que quieras usar
2. Agrega cualquier #include faltante en la sección de includes
3. Modifica las líneas indicadas con "Agregar a..."
4. Recompila y carga en ESP32
5. Prueba en Serial Monitor

NOTA: Algunos ejemplos requieren hardware adicional o conexión WiFi.
Asegúrate de tener todo configurado antes de usar.
*/
