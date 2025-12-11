#!/usr/bin/env python3
import mysql.connector

try:
    conn = mysql.connector.connect(
        host='localhost',
        user='root',
        password='root'
    )
    cursor = conn.cursor()
    cursor.execute("CREATE DATABASE IF NOT EXISTS estacionamiento CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS estacionamiento.lecturas (
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
    print("[OK] Base de datos y tabla creadas correctamente")
    conn.close()
except Exception as e:
    print(f"[ERROR] {e}")
