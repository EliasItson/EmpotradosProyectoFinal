#ifndef ESTACIONAMIENTO_WEBSERVER_H
#define ESTACIONAMIENTO_WEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Variables externas del main.cpp que monitorearemos
extern float currentDistance;
extern String lastRFIDCard;
extern bool entranceBarrierRaised;
extern bool exitBarrierRaised;
extern bool slotOccupied[];
extern int availableSlots;
extern unsigned long systemUptime;
// systemTemperature removed (no physical temperature sensor configured)

// Variables de configuración (pueden ser modificadas)
extern int ULTRASONIC_THRESHOLD_CONFIG;
extern int ULTRASONIC_TIMEOUT_CONFIG;
extern int LOWER_BARRIER_WAIT_CONFIG;
extern int DISPLAY_MESSAGE_CONFIG;

// Funciones para persistencia (implementadas en src/config_prefs.cpp)
void saveConfigPrefs();
void loadConfigPrefs();

// (Handlers declarados más abajo)

// Puerto del servidor
#define WEB_SERVER_PORT 80

// Declaración de la instancia del servidor (definida en src/webserver.cpp)
extern AsyncWebServer webServer;

// Inicialización y handlers (implementados en src/webserver.cpp)
bool initWebServer();
void handleGetStatus(AsyncWebServerRequest *request);
void handleGetParams(AsyncWebServerRequest *request);
void handleSetParams(AsyncWebServerRequest *request);
void handleSetParamsBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

#endif // ESTACIONAMIENTO_WEBSERVER_H
