# 📖 ÍNDICE DE DOCUMENTACIÓN

## 🎯 Comienza Aquí

1. **[INICIO_RAPIDO.md](INICIO_RAPIDO.md)** ← **LEE ESTO PRIMERO**
   - Pasos 1-5 para poner en marcha
   - Guía rápida de personalización
   - Checklist pre-deploy

---

## 📚 Documentación Principal

### Para Entender el Proyecto
- **[README.md](README.md)** - Descripción general, características, requisitos
- **[RESUMEN.md](RESUMEN.md)** - Resumen técnico, capacidades, roadmap futuro

### Para Implementar
- **[ESQUEMA_CONEXION.md](ESQUEMA_CONEXION.md)** - Tabla de pines, conexiones físicas, esquema completo
- **[DIAGRAMAS_FLUJO.md](DIAGRAMAS_FLUJO.md)** - Flujos lógicos, máquinas de estado, ecuaciones

### Para Probar y Debuggear
- **[PRUEBAS_DEBUGGING.md](PRUEBAS_DEBUGGING.md)** - Procedimientos de prueba, troubleshooting, checklist

### Para Extender
- **[EJEMPLOS_EXTENSION.cpp](EJEMPLOS_EXTENSION.cpp)** - 10 ejemplos de código para agregar funcionalidad

---

## 💻 Archivos de Código

### Código Principal
| Archivo | Descripción |
|---------|-------------|
| `src/main.cpp` | **Programa principal** - 540 líneas de código funcional |
| `src/rfid_register.cpp` | Utilidad para registrar tarjetas RFID |
| `include/config.h` | **Configuración centralizada** - Personaliza aquí |
| `platformio.ini` | Configuración del proyecto + librerías |

---

## 🎓 Guía por Objetivo

### "Quiero compilar y cargar el código"
1. Lee: INICIO_RAPIDO.md (Paso 1-2)
2. Ejecuta: `PlatformIO → Build & Upload`
3. Abre: Serial Monitor (115200 baud)

### "Necesito registrar mis tarjetas RFID"
1. Lee: INICIO_RAPIDO.md (Paso 3)
2. Carga: `src/rfid_register.cpp`
3. Sigue: Instrucciones en Serial Monitor
4. Actualiza: `include/config.h` con UIDs

### "Quiero entender cómo funciona"
1. Lee: README.md (Flujo de operación)
2. Mira: DIAGRAMAS_FLUJO.md (Máquinas de estado)
3. Estudia: ESQUEMA_CONEXION.md (Conexiones físicas)

### "Necesito hacer pruebas"
1. Lee: PRUEBAS_DEBUGGING.md (Pruebas secuenciales)
2. Abre: Serial Monitor con baudrate 115200
3. Ejecuta: Cada prueba en orden
4. Revisa: Checklist de verificación

### "Quiero personalizar el sistema"
1. Abre: `include/config.h`
2. Modifica: Pines, tiempos, mensajes, tarjetas
3. Recompila: `PlatformIO → Build`
4. Carga: `PlatformIO → Upload`

### "Quiero agregar más funcionalidad"
1. Lee: EJEMPLOS_EXTENSION.cpp
2. Copia: Código del ejemplo que necesites
3. Descomenta: Quita `/*` y `*/`
4. Personaliza: Según tus necesidades
5. Integra: En `src/main.cpp` según instrucciones

### "Algo no funciona"
1. Abre: Serial Monitor (115200 baud)
2. Lee: Los mensajes de debug
3. Consulta: PRUEBAS_DEBUGGING.md sección "Troubleshooting"
4. Verifica: ESQUEMA_CONEXION.md (conexiones físicas)

---

## 📋 Estructura del Proyecto

```
Estacionamiento/
├── 📖 DOCUMENTACIÓN
│   ├── README.md ...................... Guía general
│   ├── RESUMEN.md ..................... Resumen técnico
│   ├── ESQUEMA_CONEXION.md ............ Conexiones físicas
│   ├── DIAGRAMAS_FLUJO.md ............ Lógica del sistema
│   ├── PRUEBAS_DEBUGGING.md .......... Pruebas y troubleshooting
│   ├── INICIO_RAPIDO.md ............. 5 pasos iniciales
│   ├── EJEMPLOS_EXTENSION.cpp ....... 10 ejemplos de extensión
│   └── INDICE.md ..................... Este archivo
│
├── ⚙️ CONFIGURACIÓN
│   └── platformio.ini ................. Proyecto + librerías
│
├── 💻 CÓDIGO
│   ├── src/
│   │   ├── main.cpp ................. Programa principal (540 líneas)
│   │   └── rfid_register.cpp ........ Registrador de tarjetas
│   ├── include/
│   │   └── config.h ................ Configuración centralizada
│   └── lib/ ......................... Librerías (auto-descargadas)
│
└── 📁 OTROS
    ├── test/ ........................ Para pruebas (opcional)
    └── .vscode/ ..................... Configuración VS Code
```

---

## 🔑 Conceptos Clave

### Hardware Conectado
- **RFID RC522**: Lee tarjetas para validar acceso
- **Ultrasónico HC-SR04**: Detecta cuando pasa el vehículo
- **Servomotor SG90**: Levanta/baja la barra de acceso
- **Display LCD 16x2**: Muestra mensajes al usuario
- **4 Switches**: Marcan ocupación de cajones
- **4 LEDs Rojos**: Indican cajones ocupados
- **Buzzer Piezo**: Emite sonidos de bienvenida/rechazo

### Lógica del Sistema
- **Autorización RFID**: Compara UID con base de datos
- **Control Automático**: Servo sube/baja según eventos
- **Detección de Paso**: Ultrasónico valida que el auto pasó
- **Gestión de Cajones**: Switches marcan ocupación
- **Interfaz Usuario**: Display LCD + Buzzer

### Tiempos Críticos
- Cooldown RFID: 2 segundos
- Chequeo ultrasónico: 100ms
- Transición servo: 500ms
- Timeout barrera: 5 segundos

---

## 🛠️ Herramientas Necesarias

### Software
- VS Code + PlatformIO
- Serial Monitor (incluido en PlatformIO)
- Compilador Arduino/ESP32

### Hardware
- ESP32 DOIT DevKit V1
- Todos los sensores/actuadores listados
- Fuente 5V 3A
- Cable USB para programación

---

## ✅ Checklist de Implementación

- [ ] Leer INICIO_RAPIDO.md
- [ ] Compilar proyecto sin errores
- [ ] Cargar código en ESP32
- [ ] Probar cada componente (PRUEBAS_DEBUGGING.md)
- [ ] Registrar tarjetas RFID
- [ ] Configurar tiempos si es necesario (config.h)
- [ ] Realizar caso de prueba completo
- [ ] Revisar Checklist de verificación
- [ ] Sistema listo para deploy

---

## 📞 Preguntas Frecuentes

**P: ¿Por dónde empiezo?**
R: Lee INICIO_RAPIDO.md (5 pasos)

**P: ¿Cómo registro mis tarjetas?**
R: Lee INICIO_RAPIDO.md Paso 3, luego ESQUEMA_CONEXION.md

**P: ¿Qué pines puedo cambiar?**
R: Todos, edita include/config.h (líneas 1-30)

**P: ¿Cómo agrego más cajones?**
R: Duplica código de cajones en main.cpp y agrega pines en config.h

**P: ¿El servo se mueve al revés?**
R: Intercambia SERVO_ANGLE_UP y SERVO_ANGLE_DOWN en config.h

**P: ¿Cómo veo qué está pasando?**
R: Abre Serial Monitor a 115200 baud (PRUEBAS_DEBUGGING.md)

**P: ¿Puedo agregar WiFi?**
R: Sí, mira EJEMPLOS_EXTENSION.cpp sección 5

---

## 🚀 Próximos Pasos

1. **Ahora**: Abre INICIO_RAPIDO.md
2. **Luego**: Compila y carga (Paso 1-2 INICIO_RAPIDO.md)
3. **Después**: Prueba componentes (PRUEBAS_DEBUGGING.md)
4. **Finalmente**: Registra tarjetas y personaliza

---

## 📬 Información del Proyecto

- **Versión**: 1.0
- **Fecha**: Diciembre 2025
- **Plataforma**: Arduino/ESP32 + PlatformIO
- **Lenguaje**: C++
- **Estado**: Completado y Listo

---

**¿Listo para empezar? → Abre [INICIO_RAPIDO.md](INICIO_RAPIDO.md)**
