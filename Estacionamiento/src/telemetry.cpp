#include "telemetry.h"
#include <WiFi.h>
#include <ArduinoJson.h>

static WiFiClient telemetryClient;
static bool telemetryConnected = false;

// IP del data collector (cambiar si es necesario)
static const char* TELEMETRY_HOST = "192.168.100.45";  // IP local de la PC
static const int TELEMETRY_PORT = 5000;

void initTelemetry() {
  Serial.println("[TELEMETRY] Telemetry module initialized");
}

void connectTelemetry() {
  if (telemetryConnected && telemetryClient.connected()) {
    return;
  }

  if (telemetryClient.connect(TELEMETRY_HOST, TELEMETRY_PORT)) {
    telemetryConnected = true;
    Serial.printf("[TELEMETRY] Conectado a %s:%d\n", TELEMETRY_HOST, TELEMETRY_PORT);
  } else {
    telemetryConnected = false;
    Serial.printf("[TELEMETRY] Error al conectar a %s:%d\n", TELEMETRY_HOST, TELEMETRY_PORT);
  }
}

void sendSensorReading(float distance, int uptimeSeconds, int availableSlots) {
  if (!telemetryClient.connected()) {
    connectTelemetry();
    if (!telemetryClient.connected()) {
      return;
    }
  }

  StaticJsonDocument<256> doc;
  doc["type"] = "sensor_reading";
  doc["data"]["timestamp"] = getISO8601Timestamp();
  doc["data"]["distance"] = distance;
  doc["data"]["uptime_seconds"] = uptimeSeconds;
  doc["data"]["available_slots"] = availableSlots;

  String json;
  serializeJson(doc, json);
  telemetryClient.println(json);

  Serial.printf("[TELEMETRY] Lectura de sensor enviada\n");
}

void sendBarrierEvent(const char* barrierType, const char* eventType, const char* reason) {
  if (!telemetryClient.connected()) {
    connectTelemetry();
    if (!telemetryClient.connected()) {
      return;
    }
  }

  StaticJsonDocument<256> doc;
  doc["type"] = "barrier_event";
  doc["data"]["timestamp"] = getISO8601Timestamp();
  doc["data"]["barrier_type"] = barrierType;
  doc["data"]["event_type"] = eventType;
  doc["data"]["reason"] = reason;

  String json;
  serializeJson(doc, json);
  telemetryClient.println(json);

  Serial.printf("[TELEMETRY] Evento de barrera enviado: %s %s\n", barrierType, eventType);
}

void sendRFIDAccess(const char* cardUID, bool authorized, const char* action) {
  if (!telemetryClient.connected()) {
    connectTelemetry();
    if (!telemetryClient.connected()) {
      return;
    }
  }

  StaticJsonDocument<256> doc;
  doc["type"] = "rfid_access";
  doc["data"]["timestamp"] = getISO8601Timestamp();
  doc["data"]["card_uid"] = cardUID;
  doc["data"]["authorized"] = authorized;
  doc["data"]["action"] = action;

  String json;
  serializeJson(doc, json);
  telemetryClient.println(json);

  Serial.printf("[TELEMETRY] Acceso RFID enviado: %s (%s)\n", cardUID, authorized ? "Autorizado" : "Denegado");
}

void sendConfigChange(const char* parameterName, int oldValue, int newValue) {
  if (!telemetryClient.connected()) {
    connectTelemetry();
    if (!telemetryClient.connected()) {
      return;
    }
  }

  StaticJsonDocument<256> doc;
  doc["type"] = "config_change";
  doc["data"]["timestamp"] = getISO8601Timestamp();
  doc["data"]["parameter_name"] = parameterName;
  doc["data"]["old_value"] = oldValue;
  doc["data"]["new_value"] = newValue;
  doc["data"]["changed_by"] = "api";

  String json;
  serializeJson(doc, json);
  telemetryClient.println(json);

  Serial.printf("[TELEMETRY] Cambio de configuración enviado: %s = %d\n", parameterName, newValue);
}

void sendSlotOccupancy(int slotNumber, bool occupied) {
  if (!telemetryClient.connected()) {
    connectTelemetry();
    if (!telemetryClient.connected()) {
      return;
    }
  }

  StaticJsonDocument<256> doc;
  doc["type"] = "slot_occupancy";
  doc["data"]["timestamp"] = getISO8601Timestamp();
  doc["data"]["slot_number"] = slotNumber;
  doc["data"]["occupied"] = occupied;

  String json;
  serializeJson(doc, json);
  telemetryClient.println(json);

  Serial.printf("[TELEMETRY] Ocupación de espacio enviada: slot %d = %s\n", slotNumber, occupied ? "Ocupado" : "Libre");
}

String getISO8601Timestamp() {
  // Retorna timestamp en formato ISO8601 (simplificado sin sincronización NTP)
  // En producción, implementar sincronización con NTP
  time_t now = time(nullptr);
  struct tm timeinfo = *localtime(&now);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buf);
}

void disconnectTelemetry() {
  if (telemetryClient.connected()) {
    telemetryClient.stop();
  }
  telemetryConnected = false;
  Serial.println("[TELEMETRY] Desconectado");
}
