#include <Preferences.h>
#include <Arduino.h>
#include "config.h"
#include "config_prefs.h"

// Namespace for Preferences
static Preferences prefs;

void loadConfigPrefs() {
  // Try to open Preferences in writeable mode first so we can create defaults if needed.
  bool writeable = prefs.begin("estac", false);
  if (!writeable) {
    // Fallback to read-only if writeable open fails
    if (!prefs.begin("estac", true)) {
      Serial.println("[PREFS] No se pudo iniciar Preferences (ni write ni read-only)");
      return;
    }
    Serial.println("[PREFS] Preferences iniciada en modo read-only");
    writeable = false;
  } else {
    Serial.println("[PREFS] Preferences iniciada en modo writeable");
  }

  // Read or initialize each configuration value
  if (prefs.isKey("ultr_th")) {
    ULTRASONIC_THRESHOLD_CONFIG = prefs.getInt("ultr_th", ULTRASONIC_THRESHOLD);
  } else {
    ULTRASONIC_THRESHOLD_CONFIG = ULTRASONIC_THRESHOLD;
    if (writeable) prefs.putInt("ultr_th", ULTRASONIC_THRESHOLD_CONFIG);
  }

  if (prefs.isKey("ultr_to")) {
    ULTRASONIC_TIMEOUT_CONFIG = prefs.getInt("ultr_to", ULTRASONIC_TIMEOUT_MS);
  } else {
    ULTRASONIC_TIMEOUT_CONFIG = ULTRASONIC_TIMEOUT_MS;
    if (writeable) prefs.putInt("ultr_to", ULTRASONIC_TIMEOUT_CONFIG);
  }

  if (prefs.isKey("lower_wait")) {
    LOWER_BARRIER_WAIT_CONFIG = prefs.getInt("lower_wait", LOWER_BARRIER_WAIT_MS);
  } else {
    LOWER_BARRIER_WAIT_CONFIG = LOWER_BARRIER_WAIT_MS;
    if (writeable) prefs.putInt("lower_wait", LOWER_BARRIER_WAIT_CONFIG);
  }

  if (prefs.isKey("disp_ms")) {
    DISPLAY_MESSAGE_CONFIG = prefs.getInt("disp_ms", DISPLAY_MESSAGE_MS);
  } else {
    DISPLAY_MESSAGE_CONFIG = DISPLAY_MESSAGE_MS;
    if (writeable) prefs.putInt("disp_ms", DISPLAY_MESSAGE_CONFIG);
  }

  Serial.println("[PREFS] Configuración cargada (valores actuales):");
  Serial.printf("[PREFS] ULTRASONIC_THRESHOLD=%d\n", ULTRASONIC_THRESHOLD_CONFIG);
  Serial.printf("[PREFS] ULTRASONIC_TIMEOUT_MS=%d\n", ULTRASONIC_TIMEOUT_CONFIG);
  Serial.printf("[PREFS] LOWER_BARRIER_WAIT_MS=%d\n", LOWER_BARRIER_WAIT_CONFIG);
  Serial.printf("[PREFS] DISPLAY_MESSAGE_MS=%d\n", DISPLAY_MESSAGE_CONFIG);

  prefs.end();
}

void saveConfigPrefs() {
  if (!prefs.begin("estac", false)) {
    Serial.println("[PREFS] No se pudo iniciar Preferences (write)");
    return;
  }

  prefs.putInt("ultr_th", ULTRASONIC_THRESHOLD_CONFIG);
  prefs.putInt("ultr_to", ULTRASONIC_TIMEOUT_CONFIG);
  prefs.putInt("lower_wait", LOWER_BARRIER_WAIT_CONFIG);
  prefs.putInt("disp_ms", DISPLAY_MESSAGE_CONFIG);

  Serial.println("[PREFS] Configuración guardada en Preferences");
  prefs.end();
}
