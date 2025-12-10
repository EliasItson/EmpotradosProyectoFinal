// Persistencia de parámetros en ESP32 (Preferences)
#ifndef CONFIG_PREFS_H
#define CONFIG_PREFS_H

// Cargar configuración desde memoria persistente (EEPROM/Preferences)
void loadConfigPrefs();

// Guardar configuración actual en memoria persistente
void saveConfigPrefs();

// Variables de configuración declaradas en main.cpp (acceso externo)
extern int ULTRASONIC_THRESHOLD_CONFIG;
extern int ULTRASONIC_TIMEOUT_CONFIG;
extern int LOWER_BARRIER_WAIT_CONFIG;
extern int DISPLAY_MESSAGE_CONFIG;

#endif // CONFIG_PREFS_H
