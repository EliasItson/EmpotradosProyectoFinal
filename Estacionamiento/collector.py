"""
Subsistema de Recolección de Datos - Estacionamiento Inteligente

Características:
- Servidor TCP en puerto 5000 para recibir datos del ESP32
- Almacenamiento en MySQL local
- Sincronización con BD local en tiempo real
- Manejo de conexiones múltiples (si es necesario)
"""

import socket
import threading
import mysql.connector
import json
import time
from datetime import datetime
import requests

# Configuración
ESP32_IP = "192.168.177.162"  # IP del ESP32
ESP32_PORT = 80  # Puerto del webserver
COLLECTOR_PORT = 5000  # Puerto TCP del collector
POLL_INTERVAL = 2  # segundos entre consultas al ESP32

DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': 'root',
    'database': 'estacionamiento'
}

# Flag global para threads
running = True

def crear_tabla():
    """Crear tabla de lecturas si no existe"""
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        cursor.execute("""
        CREATE TABLE IF NOT EXISTS lecturas (
            id INT AUTO_INCREMENT PRIMARY KEY,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            rfidUID VARCHAR(50),
            distancia FLOAT,
            plumaEntrada BOOLEAN,
            plumaSalida BOOLEAN,
            cajon1 BOOLEAN,
            cajon2 BOOLEAN,
            entryTime1 VARCHAR(255),
            exitTime1 VARCHAR(255),
            entryTime2 VARCHAR(255),
            exitTime2 VARCHAR(255),
            INDEX idx_timestamp (timestamp),
            INDEX idx_cajon1 (cajon1),
            INDEX idx_cajon2 (cajon2)
        )
        """)
        conn.commit()
        print("[DB] Tabla verificada/creada")
        conn.close()
    except Exception as e:
        print(f"[DB] Error al crear tabla: {e}")

def guardar_lectura(data):
    """Guardar lectura en MySQL"""
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        sql = """
        INSERT INTO lecturas 
        (rfidUID, distancia, plumaEntrada, plumaSalida, cajon1, cajon2, entryTime1, exitTime1, entryTime2, exitTime2) 
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """
        cursor.execute(sql, (
            data.get("rfidUID", "--"),
            data.get("distancia", 0.0),
            data.get("plumaEntrada", False),
            data.get("plumaSalida", False),
            data.get("cajon1", False),
            data.get("cajon2", False),
            data.get("entryTime1", "--"),
            data.get("exitTime1", "--"),
            data.get("entryTime2", "--"),
            data.get("exitTime2", "--")
        ))
        conn.commit()
        conn.close()
        print(f"[DB] Lectura guardada a {datetime.now().strftime('%H:%M:%S')}")
        return True
    except Exception as e:
        print(f"[DB] Error al guardar: {e}")
        return False

def handle_client(conn, addr):
    """Manejar conexión TCP de cliente (si la hay)"""
    print(f"[TCP] Cliente conectado: {addr}")
    try:
        while running:
            data = conn.recv(1024)
            if not data:
                break
            print(f"[TCP] Datos recibidos: {data.decode()}")
            # Responder con ACK
            conn.sendall(b"ACK")
    except Exception as e:
        print(f"[TCP] Error: {e}")
    finally:
        conn.close()

def tcp_server():
    """Servidor TCP para recibir datos opcionales"""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", COLLECTOR_PORT))
    server.listen(5)
    print(f"[TCP] Servidor escuchando en puerto {COLLECTOR_PORT}")
    
    while running:
        try:
            conn, addr = server.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.daemon = True
            thread.start()
        except Exception as e:
            if running:
                print(f"[TCP] Error: {e}")
            break
    server.close()

def poll_esp32():
    """Consultar ESP32 periódicamente y guardar datos"""
    print(f"[POLL] Iniciando consulta a ESP32 ({ESP32_IP}) cada {POLL_INTERVAL}s")
    while running:
        try:
            res = requests.get(f"http://{ESP32_IP}/api/getStatus", timeout=5)
            data = res.json()
            guardar_lectura(data)
        except Exception as e:
            print(f"[POLL] Error al consultar ESP32: {e}")
        
        time.sleep(POLL_INTERVAL)

if __name__ == "__main__":
    print("=" * 60)
    print("SUBSISTEMA DE RECOLECCIÓN DE DATOS")
    print("=" * 60)
    
    # Crear tabla
    crear_tabla()
    
    # Iniciar servidor TCP en thread
    tcp_thread = threading.Thread(target=tcp_server)
    tcp_thread.daemon = True
    tcp_thread.start()
    
    # Iniciar polling de ESP32 en thread
    poll_thread = threading.Thread(target=poll_esp32)
    poll_thread.daemon = True
    poll_thread.start()
    
    try:
        print("[MAIN] Sistema en ejecución. Presione Ctrl+C para salir.\n")
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[MAIN] Deteniendo...")
        running = False
        time.sleep(1)
        print("[MAIN] Finalizado.")
