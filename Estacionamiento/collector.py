import requests
import mysql.connector
import time

# Configuración de ESP32 y MySQL
ESP32_IP = "192.168.1.100"  # cambia por tu IP
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': '',
    'database': 'estacionamiento'
}

# Crear tabla en MySQL (solo la primera vez)
def crear_tabla():
    conn = mysql.connector.connect(**DB_CONFIG)
    cursor = conn.cursor()
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS lecturas (
        id INT AUTO_INCREMENT PRIMARY KEY,
        timestamp DATETIME,
        rfidUID VARCHAR(50),
        distancia FLOAT,
        plumaEntrada BOOLEAN,
        plumaSalida BOOLEAN,
        cajon1 BOOLEAN,
        cajon2 BOOLEAN
    )
    """)
    conn.commit()
    conn.close()

# Guardar lectura en MySQL
def guardar_lectura(data):
    conn = mysql.connector.connect(**DB_CONFIG)
    cursor = conn.cursor()
    sql = "INSERT INTO lecturas (timestamp, rfidUID, distancia, plumaEntrada, plumaSalida, cajon1, cajon2) VALUES (NOW(), %s, %s, %s, %s, %s, %s)"
    cursor.execute(sql, (data["rfidUID"], data["distancia"], data["plumaEntrada"], data["plumaSalida"], data["cajon1"], data["cajon2"]))
    conn.commit()
    conn.close()

# Loop principal
crear_tabla()
while True:
    try:
        res = requests.get(f"http://{ESP32_IP}/api/getStatus")
        data = res.json()
        guardar_lectura(data)
        print("Lectura guardada:", data)
    except Exception as e:
        print("Error:", e)
    time.sleep(1)  # cada segundo
