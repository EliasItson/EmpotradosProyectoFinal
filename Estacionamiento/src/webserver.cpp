#include "webserver.h"

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Definición de la instancia del servidor
AsyncWebServer webServer(WEB_SERVER_PORT);

// Buffer para recibir POST
static String postBuffer = "";

bool initWebServer() {
  // Iniciar LittleFS
  if (!LittleFS.begin()) {
    Serial.println("[WEB] Error: No se pudo montar LittleFS");
    return false;
  }
  Serial.println("[WEB] LittleFS montado correctamente");

  // Servir archivos estáticos desde LittleFS
  webServer.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

  // Ruta para obtener estado actual del sistema
  webServer.on("/api/getStatus", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGetStatus(request);
  });

  // Ruta para obtener parámetros configurables
  webServer.on("/api/getParams", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGetParams(request);
  });

  // Ruta para establecer parámetros (soporta body stream)
  webServer.on("/api/setParams", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleSetParams(request);
  }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    handleSetParamsBody(request, data, len, index, total);
  });

  // Iniciar servidor
  webServer.begin();
  Serial.println("[WEB] Servidor web iniciado en puerto " + String(WEB_SERVER_PORT));
  return true;
}

void handleGetStatus(AsyncWebServerRequest *request) {
  StaticJsonDocument<512> doc;

  doc["distancia"] = (int)currentDistance;
  doc["rfidUID"] = lastRFIDCard.isEmpty() ? "--" : lastRFIDCard;
  doc["plumaEntrada"] = entranceBarrierRaised;
  doc["plumaSalida"] = exitBarrierRaised;
  doc["cajon1"] = slotOccupied[0];
  doc["cajon2"] = slotOccupied[1];
  doc["disponibles"] = availableSlots;
  doc["uptime"] = (int)(systemUptime / 1000);
  doc["temp"] = (int)systemTemperature;
  doc["firmware"] = "v1.0.0";
  doc["ip"] = WiFi.localIP().toString();

  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleGetParams(AsyncWebServerRequest *request) {
  StaticJsonDocument<256> doc;

  doc["ULTRASONIC_THRESHOLD"] = ULTRASONIC_THRESHOLD_CONFIG;
  doc["ULTRASONIC_TIMEOUT_MS"] = ULTRASONIC_TIMEOUT_CONFIG;
  doc["LOWER_BARRIER_WAIT_MS"] = LOWER_BARRIER_WAIT_CONFIG;
  doc["DISPLAY_MESSAGE_MS"] = DISPLAY_MESSAGE_CONFIG;

  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleSetParamsBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (index == 0) {
    postBuffer = "";
  }
  postBuffer += String((char*)data).substring(0, len);

  if (index + len == total) {
    handleSetParams(request);
  }
}

void handleSetParams(AsyncWebServerRequest *request) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, postBuffer);

  if (error) {
    request->send(400, "application/json", "{\"error\":\"JSON inválido\"}");
    return;
  }

  if (doc.containsKey("ULTRASONIC_THRESHOLD")) {
    ULTRASONIC_THRESHOLD_CONFIG = doc["ULTRASONIC_THRESHOLD"];
  }
  if (doc.containsKey("ULTRASONIC_TIMEOUT_MS")) {
    ULTRASONIC_TIMEOUT_CONFIG = doc["ULTRASONIC_TIMEOUT_MS"];
  }
  if (doc.containsKey("LOWER_BARRIER_WAIT_MS")) {
    LOWER_BARRIER_WAIT_CONFIG = doc["LOWER_BARRIER_WAIT_MS"];
  }
  if (doc.containsKey("DISPLAY_MESSAGE_MS")) {
    DISPLAY_MESSAGE_CONFIG = doc["DISPLAY_MESSAGE_MS"];
  }

  // Guardar en persistencia
  saveConfigPrefs();

  StaticJsonDocument<64> response;
  response["status"] = "success";
  response["message"] = "Parámetros actualizados";

  String respStr;
  serializeJson(response, respStr);
  request->send(200, "application/json", respStr);

  Serial.println("[WEB] Parámetros actualizados vía API");
  postBuffer = "";
}
