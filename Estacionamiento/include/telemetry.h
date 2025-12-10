#ifndef ESTACIONAMIENTO_TELEMETRY_H
#define ESTACIONAMIENTO_TELEMETRY_H

#include <Arduino.h>

// Inicializar módulo de telemetría
void initTelemetry();

// Conectar al servidor de telemetría
void connectTelemetry();

// Enviar lectura de sensores
void sendSensorReading(float distance, int uptimeSeconds, int availableSlots);

// Enviar evento de barrera
void sendBarrierEvent(const char* barrierType, const char* eventType, const char* reason);

// Enviar acceso RFID
void sendRFIDAccess(const char* cardUID, bool authorized, const char* action);

// Enviar cambio de configuración
void sendConfigChange(const char* parameterName, int oldValue, int newValue);

// Enviar ocupación de espacio
void sendSlotOccupancy(int slotNumber, bool occupied);

// Obtener timestamp en formato ISO8601
String getISO8601Timestamp();

// Desconectar del servidor de telemetría
void disconnectTelemetry();

#endif // ESTACIONAMIENTO_TELEMETRY_H
