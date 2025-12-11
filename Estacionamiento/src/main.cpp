#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>
#include <NoDelay.h>

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <time.h>

// ==================== VARIABLES GLOBALES ====================
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Servo barrierServoEntry;
Servo barrierServoExit;

bool slotOccupied[SLOTS_COUNT] = {false, false};
bool entranceBarrierRaised = false;
bool exitBarrierRaised = false;
bool deniedMessageActive = false;
bool authorizedMessageActive = false;
bool timeoutMessageActive = false;

noDelay rfidTimer(RFID_COOLDOWN);
noDelay ultrasonicTimer(ULTRASONIC_CHECK_INTERVAL);
noDelay servoTimer(SERVO_TRANSITION_TIME);
noDelay displayMessageTimer(DISPLAY_MESSAGE_MS);
noDelay successMessageTimer(SUCCESS_MESSAGE_MS);

noDelay exitRaiseTimer(EXIT_RAISE_MS);
noDelay exitLowerTimer(EXIT_LOWER_MS);
bool exitSequenceActive = false;
int exitPhase = 0; // 0 = INACTIVO, 1 = ESPERANDO PARA SUBIR, 2 = ESPERANDO PARA BAJAR
// Variables para temporización dinámica de la pluma de salida
unsigned long exitLowerStartMillis = 0;
bool exitLowerWaiting = false;
// (entrance timing uses lowerBarrierWaitTimer - no dynamic vars here)

// Timeout: si no detecta auto, baja	barrera y cancela acceso
// NoDelay instance for ultrasonic "no car" timeout. We'll start it with
// a configurable value at runtime so web UI can change the timeout.
noDelay ultrasonicNoCarTimer;
// Esperar antes de bajar
noDelay lowerBarrierWaitTimer(LOWER_BARRIER_WAIT_MS);
// Flag: el auto fue detectado alguna vez
bool carDetectedRecently = false;
// Flag: el auto está siendo detectado ahora
bool carCurrentlyDetected = false;
// 0 = INACTIVO, 1 = ESPERANDO CARRO, 2 = ESPERANDO QUE PASE CARRO, 3 = BAJANDO
int entranceBarrierPhase = 0;

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
void displayMessage(String line1, String line2 = "");
void clearDisplay();
void updateBarrierLogic();
void updateDisplayLogic();
void displayAvailableSlots();

// Web server / FS
WebServer server(80);

// Últimos valores para exponer por la API
String latestRFIDUID = "--";
float lastDistance = 0.0;

// Timestamps de entrada/salida por cajón ("--" si no hay registro)
String lastEntryTime[SLOTS_COUNT];
String lastExitTime[SLOTS_COUNT];

// Parámetros configurables (valores por defecto tomados de config.h)
int SALIDA_DELAY_MS = EXIT_RAISE_MS;
// Tiempo de espera dinámico para que el ultrasonico considere aparecer un auto (ms)
int ULTRASONIC_TIMEOUT_MS_VAR = ULTRASONIC_TIMEOUT_MS;

// Funciones de FS / API
bool initFileSystem();
void setupWebServer();
void handle_getStatus();
void handle_getParams();
void handle_setParams();
void loadParamsFromFS();
void saveParamsToFS();

void setup()
{
	Serial.begin(SERIAL_BAUD);
	setupSensors();
	setupActuators();
	// Inicializar I2C explícitamente con pines definidos en config.h
	Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
	// Inicializar pantalla SSD1306
	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
	{
		Serial.println("SSD1306 allocation failed");
		for (;;)
			delay(10);
	}
	display.clearDisplay();
	display.display();
	// Mostrar estado inicial con contador de espacios disponibles
	displayAvailableSlots();

	// Inicializar WiFi en modo STA (Station)
	Serial.println("Conectando a WiFi...");
	WiFi.mode(WIFI_STA);
	WiFi.begin("Rat World", "274000403");

	// Esperar a que se conecte (máximo 20 segundos)
	int wifiAttempts = 0;
	while (WiFi.status() != WL_CONNECTED && wifiAttempts < 40)
	{
		delay(500);
		Serial.print(".");
		wifiAttempts++;
	}

	if (WiFi.status() == WL_CONNECTED)
	{
		Serial.println();
		Serial.print("WiFi conectado. IP: ");
		Serial.println(WiFi.localIP());

		// Inicializar sincronización NTP
		configTime(0, 0, NTP_SERVER);
		// Intentar obtener hora por hasta NTP_TIMEOUT_MS
		Serial.println("Sincronizando hora NTP...");
		unsigned long start = millis();
		struct tm timeinfo;
		while (millis() - start < NTP_TIMEOUT_MS)
		{
			if (getLocalTime(&timeinfo))
			{
				Serial.println("Hora sincronizada");
				break;
			}
			delay(200);
		}
	}
	else
	{
		Serial.println();
		Serial.println("Error: No se pudo conectar a WiFi. Continuando sin conexión...");
	}

	// Inicializar servidor web
	setupWebServer();
	server.begin();
	Serial.print("Web server iniciado en http://");
	Serial.println(WiFi.localIP());

	// Inicializar timestamps por cajón
	for (int i = 0; i < SLOTS_COUNT; i++)
	{
		lastEntryTime[i] = "--";
		lastExitTime[i] = "--";
	}
}

// Devuelve fecha/hora formateada en ISO-like "YYYY-MM-DD HH:MM:SS" o "--" si no hay hora
String getFormattedTime()
{
	time_t now = time(nullptr);
	struct tm timeinfo;
	if (!localtime_r(&now, &timeinfo))
	{
		return String("--");
	}
	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
	return String(buf);
}

void loop()
{
	server.handleClient();
	updateBarrierLogic();
	updateDisplayLogic();
	checkRFID();
	checkUltrasonicSensor();
	checkParkingSlots();
}

void setupSensors()
{
	SPI.begin();
	rfid.PCD_Init();
	pinMode(SENSOR_ULTRASONIC_TRIG, OUTPUT);
	pinMode(SENSOR_ULTRASONIC_ECHO, INPUT);
	pinMode(SWITCH_SLOT1, INPUT_PULLUP);
	pinMode(SWITCH_SLOT2, INPUT_PULLUP);
}

void setupActuators()
{
	barrierServoEntry.attach(SERVO_ENTRY_PIN);
	barrierServoExit.attach(SERVO_EXIT_PIN);
	barrierServoEntry.write(SERVO_ANGLE_DOWN);
	// Si el servo de salida está invertido mecánicamente, escribir el ángulo invertido
	barrierServoExit.write( (SERVO_EXIT_INVERT) ? (180 - SERVO_ANGLE_DOWN) : (SERVO_ANGLE_DOWN) );
	entranceBarrierRaised = false;
	exitBarrierRaised = false;
	pinMode(LED_RED_SLOT1, OUTPUT);
	pinMode(LED_RED_SLOT2, OUTPUT);
}

void checkRFID()
{
	if (!rfidTimer.update())
		return;
	if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
		return;
	String cardUID = getCardUID();
	latestRFIDUID = cardUID;
	if (isCardAuthorized(cardUID))
		handleAuthorizedUser();
	else
		handleUnauthorizedUser();
	rfid.PICC_HaltA();
	rfid.PCD_StopCrypto1();
}

String getCardUID()
{
	String uid = "";
	for (int i = 0; i < rfid.uid.size; i++)
	{
		if (rfid.uid.uidByte[i] < 0x10)
			uid += "0";
		uid += String(rfid.uid.uidByte[i], HEX);
		if (i < rfid.uid.size - 1)
			uid += ":";
	}
	uid.toUpperCase();
	return uid;
}

bool isCardAuthorized(String cardUID)
{
	for (int i = 0; i < AUTHORIZED_CARDS_COUNT; i++)
	{
		if (cardUID.equalsIgnoreCase(String(AUTHORIZED_CARDS[i])))
			return true;
	}
	return false;
}

void handleAuthorizedUser()
{
	// Verificar disponibilidad antes de conceder acceso
	if (availableSlots <= 0)
	{
		displayMessage(MSG_FULL_1, MSG_FULL_2);
		displayMessageTimer.start();
		deniedMessageActive = true;
		return;
	}

	// Crear una reserva temporal: decremento real ocurrirá cuando el usuario
	// confirme ocupación presionando el switch del cajón.
	pendingEntries++;
	Serial.println("[RFID] Tarjeta válida. Levantando pluma y esperando auto...");
	displayMessage(MSG_WELCOME_1, MSG_WELCOME_2);
	raiseEntranceBarrier();
	// Iniciar la lógica de espera: o detecta auto, o timeout en 10s
	entranceBarrierPhase = 1; // Esperar a que el auto pase
	carDetectedRecently = false;
	carCurrentlyDetected = false;
	// start the ultrasonic "no car" timeout using the runtime-configurable value
	ultrasonicNoCarTimer.setdelay(ULTRASONIC_TIMEOUT_MS_VAR);
	ultrasonicNoCarTimer.start();
	successMessageTimer.start();
	authorizedMessageActive = true;
}

void handleUnauthorizedUser()
{
	displayMessage(MSG_DENIED_1, MSG_DENIED_2);
	displayMessageTimer.start();
	deniedMessageActive = true;
}

void checkUltrasonicSensor()
{
	if (!entranceBarrierRaised || entranceBarrierPhase == 0)
		return;
	if (!ultrasonicTimer.update())
		return;

	// Generar pulso en TRIG
	digitalWrite(SENSOR_ULTRASONIC_TRIG, LOW);
	delayMicroseconds(ULTRASONIC_TRIG_PREP_US);
	digitalWrite(SENSOR_ULTRASONIC_TRIG, HIGH);
	delayMicroseconds(ULTRASONIC_TRIG_PULSE_US);
	digitalWrite(SENSOR_ULTRASONIC_TRIG, LOW);

	// Leer duración del pulso en ECHO
	unsigned long duration = pulseIn(SENSOR_ULTRASONIC_ECHO, HIGH, ULTRASONIC_PULSE_TIMEOUT_US);

	// Convertir duración a distancia (cm)
	float distance = duration * ULTRASONIC_FACTOR / 2;

	Serial.printf("[US] Duration: %lu us | Distancia: %f cm\n", duration, distance);

	lastDistance = distance;

	// Validar que la distancia sea razonable (entre 2cm y 400cm)
	if (distance < 2 || distance > 400)
	{
		Serial.printf("[US] Lectura inválida: %f cm\n", distance);
		return;
	}

	bool carDetected = (distance < ULTRASONIC_THRESHOLD); // < 30cm = bloqueado = hay auto

	Serial.printf("[US] Distancia: %f cm | Detectado: %d | Fase: %d\n", distance, carDetected, entranceBarrierPhase);

	if (entranceBarrierPhase == 1)
	{
		// Esperando a que el auto pase
		if (carDetected)
		{
			// Auto detectado (bloqueando sensor)
			if (!carCurrentlyDetected)
			{
				carCurrentlyDetected = true;
				carDetectedRecently = true;
				Serial.println("[US] Auto detectado: sensor bloqueado");
			}
			// Resetear timeout mientras el auto esté siendo detectado
			ultrasonicNoCarTimer.setdelay(ULTRASONIC_TIMEOUT_MS_VAR);
			ultrasonicNoCarTimer.start();
		}
		if (!carDetected && carCurrentlyDetected && carDetectedRecently)
		{
			// Auto pasó: se detectó antes (estaba bloqueado), ahora se fue (desbloqueado)
			carCurrentlyDetected = false;
			entranceBarrierPhase = 3; // Bajar después de 1s
			lowerBarrierWaitTimer.start();
			Serial.println("[US] Auto pasó: sensor desbloqueado. Bajando pluma...");
		}
	}
}

void raiseEntranceBarrier()
{
		barrierServoEntry.write(SERVO_ANGLE_UP);
	entranceBarrierRaised = true;
	servoTimer.start();
}
void lowerEntranceBarrier()
{
		barrierServoEntry.write(SERVO_ANGLE_DOWN);
	entranceBarrierRaised = false;
	servoTimer.start();
}

void raiseExitBarrier()
{
		barrierServoExit.write( (SERVO_EXIT_INVERT) ? (180 - SERVO_ANGLE_UP) : (SERVO_ANGLE_UP) );
	exitBarrierRaised = true;
	servoTimer.start();
}
void lowerExitBarrier()
{
		barrierServoExit.write( (SERVO_EXIT_INVERT) ? (180 - SERVO_ANGLE_DOWN) : (SERVO_ANGLE_DOWN) );
	exitBarrierRaised = false;
	servoTimer.start();
}

void checkParkingSlots()
{
	int slots[SLOTS_COUNT] = {SWITCH_SLOT1, SWITCH_SLOT2};
	for (int i = 0; i < SLOTS_COUNT; i++)
	{
		bool pressed = (digitalRead(slots[i]) == LOW);
		// Entrada al cajón (usuario acaba de estacionar)
		if (pressed && !slotOccupied[i])
		{
			slotOccupied[i] = true;
			updateLED(i, true);
			// Registrar timestamp de entrada
			lastEntryTime[i] = getFormattedTime();
			// Si hay reservas pendientes, asociar una a esta ocupación.
			if (pendingEntries > 0)
			{
				pendingEntries--;
				if (availableSlots > 0)
					availableSlots--;
			}
			else
			{
				// Ocupación manual sin reserva previa
				if (availableSlots > 0)
					availableSlots--;
			}
			if (availableSlots < 0)
				availableSlots = 0;
			Serial.printf("Cajon %d - OCUPADO. Disponibles: %d\n", i + 1, availableSlots);
			// Actualizar contador en pantalla si no hay mensajes temporales activos
			if (!deniedMessageActive && !authorizedMessageActive && !timeoutMessageActive)
			{
				displayAvailableSlots();
			}
		}
		// Salida del cajón (usuario marcó que desocupó)
		else if (!pressed && slotOccupied[i])
		{
			slotOccupied[i] = false;
			updateLED(i, false);
			// Registrar timestamp de salida
			lastExitTime[i] = getFormattedTime();
			availableSlots++;
			Serial.printf("Cajon %d - DISPONIBLE. Disponibles: %d\n", i + 1, availableSlots);
			// Actualizar contador en pantalla si no hay mensajes temporales activos
			if (!deniedMessageActive && !authorizedMessageActive && !timeoutMessageActive)
			{
				displayAvailableSlots();
			}
			// Iniciar secuencia de salida que levanta la pluma y luego la baja
			exitSequenceActive = true;
			exitPhase = 1;
			exitRaiseTimer.start();
		}
	}
}

void updateLED(int slot, bool occupied)
{
	int leds[SLOTS_COUNT] = {LED_RED_SLOT1, LED_RED_SLOT2};
	digitalWrite(leds[slot], occupied ? HIGH : LOW);
}

void displayMessage(String line1, String line2)
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.print(line1);
	if (line2.length())
	{
		// second line starting a bit lower (10 px)
		display.setCursor(0, 12);
		display.print(line2);
	}
	display.display();
}
void clearDisplay()
{
	display.clearDisplay();
	display.display();
}

// Extensión: manejar la secuencia de salida (timers) además del timeout
void updateBarrierLogic()
{
	// Lógica para pluma de entrada
	if (entranceBarrierRaised && entranceBarrierPhase > 0)
	{
		// Fase 1: esperando a que pase el auto (timeout 10s - esperar más tiempo a detección real)
		if (entranceBarrierPhase == 1 && ultrasonicNoCarTimer.update())
		{
			// Timeout: no se detectó auto en 10s, bajar pluma
			entranceBarrierPhase = 3;
			lowerBarrierWaitTimer.start();
			Serial.println("[TIMEOUT] No se detectó auto. Bajando pluma...");
			// Mostrar mensaje distinto cuando nunca se detectó el auto
			displayMessage(MSG_TIMEOUT_1, MSG_TIMEOUT_2);
			displayMessageTimer.start();
			timeoutMessageActive = true;
		}
		// Fase 3: esperar 1s antes de bajar
		if (entranceBarrierPhase == 3 && lowerBarrierWaitTimer.update())
		{
			lowerEntranceBarrier();
			entranceBarrierPhase = 0;
			// Mostrar mensaje de paso y activar el flag para que se restaure luego
			displayMessage(MSG_PASS_1, MSG_PASS_2);
			authorizedMessageActive = true;
			successMessageTimer.start();
		}
	}

	// Secuencia de salida (usa la pluma de salida)
	if (exitSequenceActive)
	{
		if (exitPhase == 1 && exitRaiseTimer.update())
		{
			raiseExitBarrier();
			// ahora esperamos para bajar
			exitPhase = 2;
			// iniciar temporizador dinámico usando SALIDA_DELAY_MS
			exitLowerStartMillis = millis();
			exitLowerWaiting = true;
		}
		else if (exitPhase == 2 && exitLowerWaiting && (millis() - exitLowerStartMillis >= (unsigned long)SALIDA_DELAY_MS))
		{
			lowerExitBarrier();
			exitSequenceActive = false;
			exitPhase = 0;
			exitLowerWaiting = false;
		}
	}
}
void updateDisplayLogic()
{
	if (deniedMessageActive && displayMessageTimer.update())
	{
		deniedMessageActive = false;
		// Restaurar pantalla por defecto con contador
		displayAvailableSlots();
	}
	if (authorizedMessageActive && successMessageTimer.update())
	{
		authorizedMessageActive = false;
		// Restaurar pantalla por defecto con contador
		displayAvailableSlots();
	}
	if (timeoutMessageActive && displayMessageTimer.update())
	{
		timeoutMessageActive = false;
		// Restaurar pantalla por defecto con contador
		displayAvailableSlots();
	}
}

// Muestra en el OLED la cantidad de espacios disponibles cuando no hay mensajes
void displayAvailableSlots()
{
	String line2 = String("Disp: ") + String(availableSlots);
	// Asegurar que la línea tiene máximo 16 caracteres
	if (line2.length() > 16)
		line2 = line2.substring(0, 16);
	displayMessage(MSG_READY_1, line2);
}

// ------------------------- LittleFS y WebServer -------------------------

bool initFileSystem()
{
	if (!LittleFS.begin())
	{
		Serial.println("LittleFS mount failed, attempting format...");
		if (LittleFS.format())
		{
			Serial.println("LittleFS formatted, attempting mount again...");
			if (!LittleFS.begin())
			{
				Serial.println("LittleFS mount failed after format");
				return false;
			}
		}
		else
		{
			Serial.println("LittleFS format failed");
			return false;
		}
	}
	Serial.println("LittleFS mounted");
	return true;
}

void setupWebServer()
{
	// Servir página principal con HTML/CSS/JS inline
	server.on("/", HTTP_GET, []()
			  {
		String html = "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n";
		html += "  <meta charset=\"UTF-8\">\n";
		html += "  <title>Estacionamiento Inteligente</title>\n";
		html += "  <style>\n";
		html += "    body { font-family: Arial, sans-serif; background-color: #f0f0f0; padding: 20px; }\n";
		html += "    h1, h2 { text-align: center; }\n";
		html += "    #estado, #configuracion { background-color: #fff; padding: 15px; margin: 15px auto; border-radius: 10px; width: 300px; box-shadow: 0 0 5px rgba(0,0,0,0.3); }\n";
		html += "    label { display: block; margin: 10px 0; }\n";
		html += "    input { width: 100%; padding: 5px; }\n";
		html += "    button { width: 100%; padding: 10px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; }\n";
		html += "    button:hover { background: #0056b3; }\n";
		html += "    p { margin: 5px 0; }\n";
		html += "  </style>\n</head>\n<body>\n";
		html += "  <h1>Estacionamiento Inteligente</h1>\n";
		html += "  <div id=\"estado\">\n";
		html += "    <h2>Estado Actual</h2>\n";
		html += "    <p id=\"rfid\">RFID: --</p>\n";
		html += "    <p id=\"distancia\">Distancia: -- cm</p>\n";
		html += "    <p id=\"plumaEntrada\">Pluma Entrada: --</p>\n";
		html += "    <p id=\"plumaSalida\">Pluma Salida: --</p>\n";
		html += "    <p id=\"cajon1\">Cajón 1: --</p>\n";
		html += "    <p id=\"cajon2\">Cajón 2: --</p>\n";
		html += "    <p id=\"entry1\">Entrada 1: --</p>\n";
		html += "    <p id=\"exit1\">Salida 1: --</p>\n";
		html += "    <p id=\"entry2\">Entrada 2: --</p>\n";
		html += "    <p id=\"exit2\">Salida 2: --</p>\n";
		html += "  </div>\n";
		html += "  <div id=\"configuracion\">\n";
	html += "    <h2>Parámetros Configurables</h2>\n";
	html += "    <label>Delay pluma salida (ms):<input type=\"number\" id=\"delaySalida\"></label>\n";
	html += "    <label>Timeout ultrasonico (ms):<input type=\"number\" id=\"timeoutUltrasonico\"></label>\n";
	html += "    <button onclick=\"guardarParametros()\">Guardar</button>\n";
		html += "  </div>\n";
		html += "  <script>\n";
		html += "    let enfoque = false;\n";
		html += "    function actualizarEstado() {\n";
		html += "      fetch(\"/api/getStatus\").then(r=>r.json()).then(d=>{\n";
		html += "        document.getElementById(\"rfid\").innerText=\"RFID: \"+d.rfidUID;\n";
		html += "        document.getElementById(\"distancia\").innerText=\"Distancia: \"+d.distancia.toFixed(2)+\" cm\";\n";
		html += "        document.getElementById(\"plumaEntrada\").innerText=\"Pluma Entrada: \"+(d.plumaEntrada?\"Abierta\":\"Cerrada\");\n";
		html += "        document.getElementById(\"plumaSalida\").innerText=\"Pluma Salida: \"+(d.plumaSalida?\"Abierta\":\"Cerrada\");\n";
		html += "        document.getElementById(\"cajon1\").innerText=\"Cajón 1: \"+(d.cajon1?\"Ocupado\":\"Libre\");\n";
		html += "        document.getElementById(\"cajon2\").innerText=\"Cajón 2: \"+(d.cajon2?\"Ocupado\":\"Libre\");\n";
		html += "        document.getElementById(\"entry1\").innerText=\"Entrada 1: \"+ (d.entryTime1?d.entryTime1:\"--\");\n";
		html += "        document.getElementById(\"exit1\").innerText=\"Salida 1: \"+ (d.exitTime1?d.exitTime1:\"--\");\n";
		html += "        document.getElementById(\"entry2\").innerText=\"Entrada 2: \"+ (d.entryTime2?d.entryTime2:\"--\");\n";
		html += "        document.getElementById(\"exit2\").innerText=\"Salida 2: \"+ (d.exitTime2?d.exitTime2:\"--\");\n";
		html += "      }).catch(e=>console.error(\"Error:\",e));\n";
		html += "    }\n";
		html += "    function actualizarParametros() {\n";
		html += "      if(!enfoque) {\n";
		html += "        fetch(\"/api/getParams\").then(r=>r.json()).then(d=>{\n";
		html += "          document.getElementById(\"delaySalida\").value=d.SALIDA_DELAY_MS;\n";
		html += "          document.getElementById(\"timeoutUltrasonico\").value=d.ULTRASONIC_TIMEOUT_MS;\n";
		html += "        }).catch(e=>console.error(\"Error:\",e));\n";
		html += "      }\n";
		html += "    }\n";
		html += "    function guardarParametros() {\n";
		html += "      let p={SALIDA_DELAY_MS:parseInt(document.getElementById(\"delaySalida\").value),ULTRASONIC_TIMEOUT_MS:parseInt(document.getElementById(\"timeoutUltrasonico\").value)};\n";
		html += "      fetch(\"/api/setParams\",{method:\"POST\",headers:{\"Content-Type\":\"application/json\"},body:JSON.stringify(p)}).then(()=>{alert(\"Guardado\");enfoque=false;actualizarParametros();}).catch(e=>console.error(\"Error:\",e));\n";
		html += "    }\n";
		html += "    document.getElementById(\"delaySalida\").addEventListener(\"focus\",()=>{enfoque=true;});\n";
		html += "    document.getElementById(\"delaySalida\").addEventListener(\"blur\",()=>{enfoque=false;});\n";
		html += "    document.getElementById(\"timeoutUltrasonico\").addEventListener(\"focus\",()=>{enfoque=true;});\n";
		html += "    document.getElementById(\"timeoutUltrasonico\").addEventListener(\"blur\",()=>{enfoque=false;});\n";
		html += "    setInterval(()=>{actualizarEstado();actualizarParametros();},1000);\n";
		html += "    actualizarEstado();actualizarParametros();\n";
		html += "  </script>\n</body>\n</html>";
		server.send(200, "text/html", html); });

	// API endpoints
	server.on("/api/getStatus", HTTP_GET, handle_getStatus);
	server.on("/api/getParams", HTTP_GET, handle_getParams);
	server.on("/api/setParams", HTTP_POST, handle_setParams);
}

void handle_getStatus()
{
	DynamicJsonDocument doc(512);
	doc["rfidUID"] = latestRFIDUID;
	doc["distancia"] = lastDistance;
	doc["plumaEntrada"] = entranceBarrierRaised;
	doc["plumaSalida"] = exitBarrierRaised;
	for (int i = 0; i < SLOTS_COUNT; i++)
	{
		String key = String("cajon") + String(i + 1);
		doc[key] = slotOccupied[i];
	}
	// Agregar timestamps de entrada/salida
	for (int i = 0; i < SLOTS_COUNT; i++)
	{
		String ekey = String("entryTime") + String(i + 1);
		String xkey = String("exitTime") + String(i + 1);
		doc[ekey] = lastEntryTime[i];
		doc[xkey] = lastExitTime[i];
	}
	String out;
	serializeJson(doc, out);
	server.send(200, "application/json", out);
}

void handle_getParams()
{
	DynamicJsonDocument doc(256);
	doc["SALIDA_DELAY_MS"] = SALIDA_DELAY_MS;
	doc["ULTRASONIC_TIMEOUT_MS"] = ULTRASONIC_TIMEOUT_MS_VAR;
	String out;
	serializeJson(doc, out);
	server.send(200, "application/json", out);
}

void handle_setParams()
{
	if (!server.hasArg("plain"))
	{
		server.send(400, "application/json", "{\"error\":\"no body\"}");
		return;
	}
	String body = server.arg("plain");
	DynamicJsonDocument doc(256);
	DeserializationError err = deserializeJson(doc, body);
	if (err)
	{
		server.send(400, "application/json", "{\"error\":\"invalid json\"}");
		return;
	}
	if (doc.containsKey("SALIDA_DELAY_MS"))
		SALIDA_DELAY_MS = doc["SALIDA_DELAY_MS"];
	if (doc.containsKey("ULTRASONIC_TIMEOUT_MS"))
		ULTRASONIC_TIMEOUT_MS_VAR = doc["ULTRASONIC_TIMEOUT_MS"];
	// Ensure current timer uses latest value when changed via API
	if (doc.containsKey("ULTRASONIC_TIMEOUT_MS"))
		ultrasonicNoCarTimer.setdelay(ULTRASONIC_TIMEOUT_MS_VAR);
	saveParamsToFS();
	server.send(200, "application/json", "{\"ok\":true}");
}

void loadParamsFromFS()
{
	Serial.println("Listing LittleFS files:");
	File root = LittleFS.open("/");
	File file = root.openNextFile();
	while (file)
	{
		Serial.print("  ");
		Serial.println(file.name());
		file = root.openNextFile();
	}

	if (!LittleFS.exists("/config.json"))
	{
		Serial.println("Config file not found, using defaults");
		return;
	}
	File f = LittleFS.open("/config.json", "r");
	if (!f)
	{
		Serial.println("Failed to open config file");
		return;
	}
	size_t size = f.size();
	std::unique_ptr<char[]> buf(new char[size + 1]);
	f.readBytes(buf.get(), size);
	buf[size] = '\0';
	f.close();
	DynamicJsonDocument doc(256);
	DeserializationError err = deserializeJson(doc, buf.get());
	if (err)
	{
		Serial.println("Failed to parse config JSON");
		return;
	}
	if (doc.containsKey("SALIDA_DELAY_MS"))
		SALIDA_DELAY_MS = doc["SALIDA_DELAY_MS"];
	if (doc.containsKey("ULTRASONIC_TIMEOUT_MS"))
		ULTRASONIC_TIMEOUT_MS_VAR = doc["ULTRASONIC_TIMEOUT_MS"];
	// Apply the loaded value to the running timer
	ultrasonicNoCarTimer.setdelay(ULTRASONIC_TIMEOUT_MS_VAR);
	Serial.println("Config loaded from FS");
}

void saveParamsToFS()
{
	DynamicJsonDocument doc(256);
	doc["SALIDA_DELAY_MS"] = SALIDA_DELAY_MS;
	doc["ULTRASONIC_TIMEOUT_MS"] = ULTRASONIC_TIMEOUT_MS_VAR;
	String out;
	serializeJson(doc, out);
	File f = LittleFS.open("/config.json", "w");
	if (!f)
		return;
	f.print(out);
	f.close();
}
