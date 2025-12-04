// Archivo para registrar tarjetas RFID autorizadas
// Este sketch permite registrar nuevas tarjetas sin modificar main.cpp

#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#include "config.h"

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// Array para almacenar hasta 10 tarjetas
String registeredCards[10];
int cardCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  SPI.begin();
  rfid.PCD_Init();
  
  Serial.println("\n====================================");
  Serial.println("REGISTRO DE TARJETAS RFID");
  Serial.println("====================================");
  Serial.println("Presenta una tarjeta para registrar su UID");
  Serial.println("Comando 'list' para ver tarjetas registradas");
  Serial.println("====================================\n");
}

void loop() {
  // Buscar tarjeta
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(100);
    return;
  }

  String cardUID = getCardUID();
  
  // Verificar si ya está registrada
  bool alreadyRegistered = false;
  for (int i = 0; i < cardCount; i++) {
    if (registeredCards[i].equalsIgnoreCase(cardUID)) {
      alreadyRegistered = true;
      break;
    }
  }

  if (alreadyRegistered) {
    Serial.println("⚠️  Tarjeta ya registrada: " + cardUID);
  } else if (cardCount < 10) {
    registeredCards[cardCount] = cardUID;
    cardCount++;
    Serial.println("✅ Tarjeta registrada: " + cardUID);
    Serial.println("Copia este código en tu main.cpp:");
    printArrayFormat();
  } else {
    Serial.println("❌ Límite de tarjetas alcanzado (10)");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000);
}

String getCardUID() {
  String uid = "";
  for (int i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0";
    }
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) {
      uid += ":";
    }
  }
  uid.toUpperCase();
  return uid;
}

void printArrayFormat() {
  Serial.println("\nconst char* AUTHORIZED_CARDS[] = {");
  for (int i = 0; i < cardCount; i++) {
    Serial.print("  \"" + registeredCards[i] + "\"");
    if (i < cardCount - 1) {
      Serial.println(",");
    } else {
      Serial.println();
    }
  }
  Serial.println("};");
  Serial.println("const int AUTHORIZED_CARDS_COUNT = " + String(cardCount) + ";\n");
}
