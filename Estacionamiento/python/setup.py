#!/usr/bin/env python3
"""
Setup script for Parking Management System - Python Subsystem
Configures MySQL database and guides user through initial setup
"""

import os
import sys
import subprocess
import mysql.connector
from mysql.connector import Error as MySQLError

def print_banner():
    print("""
    ╔═══════════════════════════════════════════════════════════════╗
    ║    SISTEMA DE ESTACIONAMIENTO - Setup Python Subsystem       ║
    ╚═══════════════════════════════════════════════════════════════╝
    """)

def check_mysql_installed():
    """Verificar si MySQL está instalado y accesible"""
    try:
        subprocess.run(['mysql', '--version'], capture_output=True, check=True)
        print("✓ MySQL detectado")
        return True
    except:
        print("✗ MySQL no encontrado en PATH")
        return False

def create_database(host, user, password):
    """Crear base de datos y tablas"""
    try:
        conn = mysql.connector.connect(
            host=host,
            user=user,
            password=password
        )
        cursor = conn.cursor()
        
        # Leer y ejecutar schema.sql
        schema_path = os.path.join(os.path.dirname(__file__), 'schema.sql')
        if not os.path.exists(schema_path):
            print("✗ schema.sql no encontrado")
            return False
        
        with open(schema_path, 'r') as f:
            sql_content = f.read()
        
        # Ejecutar cada statement
        for statement in sql_content.split(';'):
            statement = statement.strip()
            if statement:
                cursor.execute(statement)
        
        conn.commit()
        cursor.close()
        conn.close()
        
        print("✓ Base de datos y tablas creadas exitosamente")
        return True
    
    except MySQLError as err:
        print(f"✗ Error MySQL: {err}")
        return False

def test_mysql_connection(host, user, password, database):
    """Probar conexión a MySQL"""
    try:
        conn = mysql.connector.connect(
            host=host,
            user=user,
            password=password,
            database=database
        )
        cursor = conn.cursor()
        cursor.execute("SELECT 1")
        cursor.close()
        conn.close()
        print(f"✓ Conexión a MySQL exitosa ({host}:{user}@{database})")
        return True
    except MySQLError as err:
        print(f"✗ Error de conexión: {err}")
        return False

def update_config_files(host, user, password, esp32_url, telemetry_host, telemetry_port):
    """Actualizar archivos de configuración con valores ingresados"""
    
    # Actualizar data_collector.py
    try:
        with open('data_collector.py', 'r') as f:
            content = f.read()
        
        content = content.replace(
            "'password': 'password'",
            f"'password': '{password}'"
        )
        content = content.replace(
            "'host': 'localhost'",
            f"'host': '{host}'"
        )
        content = content.replace(
            "'user': 'root'",
            f"'user': '{user}'"
        )
        
        with open('data_collector.py', 'w') as f:
            f.write(content)
        print("✓ data_collector.py configurado")
    except Exception as e:
        print(f"✗ Error actualizando data_collector.py: {e}")
        return False
    
    # Actualizar parking_gui.py
    try:
        with open('parking_gui.py', 'r') as f:
            content = f.read()
        
        content = content.replace(
            "'password': 'password'",
            f"'password': '{password}'"
        )
        content = content.replace(
            "'host': 'localhost'",
            f"'host': '{host}'"
        )
        content = content.replace(
            "'user': 'root'",
            f"'user': '{user}'"
        )
        content = content.replace(
            'ESP32_URL = "http://estacionamiento.local"',
            f'ESP32_URL = "{esp32_url}"'
        )
        
        with open('parking_gui.py', 'w') as f:
            f.write(content)
        print("✓ parking_gui.py configurado")
    except Exception as e:
        print(f"✗ Error actualizando parking_gui.py: {e}")
        return False
    
    return True

def install_dependencies():
    """Instalar dependencias Python"""
    try:
        print("\nInstalando dependencias Python...")
        subprocess.run(
            [sys.executable, '-m', 'pip', 'install', '-r', 'requirements.txt'],
            check=True
        )
        print("✓ Dependencias instaladas")
        return True
    except Exception as e:
        print(f"✗ Error instalando dependencias: {e}")
        return False

def main():
    print_banner()
    
    print("\n=== CONFIGURACIÓN DE BASE DE DATOS ===")
    host = input("Host MySQL [localhost]: ").strip() or "localhost"
    user = input("Usuario MySQL [root]: ").strip() or "root"
    password = input("Contraseña MySQL: ").strip()
    
    if not test_mysql_connection(host, user, password, "mysql"):
        print("\n✗ No se pudo conectar a MySQL. Verifica credenciales.")
        sys.exit(1)
    
    print("\nCreando base de datos...")
    if not create_database(host, user, password):
        print("\n✗ Error creando base de datos")
        sys.exit(1)
    
    if not test_mysql_connection(host, user, password, "estacionamiento_db"):
        print("\n✗ No se pudo conectar a la base de datos creada")
        sys.exit(1)
    
    print("\n=== CONFIGURACIÓN DE ESP32 ===")
    esp32_url = input("URL/IP del ESP32 [http://estacionamiento.local]: ").strip() or "http://estacionamiento.local"
    
    print("\n=== CONFIGURACIÓN DE TELEMETRÍA ===")
    telemetry_host = input("IP de esta PC (para Data Collector) [192.168.100.1]: ").strip() or "192.168.100.1"
    telemetry_port = input("Puerto Data Collector [5000]: ").strip() or "5000"
    
    print("\n=== ACTUALIZANDO ARCHIVOS DE CONFIGURACIÓN ===")
    if not update_config_files(host, user, password, esp32_url, telemetry_host, telemetry_port):
        print("✗ Error actualizando configuración")
        sys.exit(1)
    
    print("\n=== INSTALANDO DEPENDENCIAS ===")
    if not install_dependencies():
        print("⚠ Algunas dependencias pueden no haberse instalado")
    
    print("""
    ╔═══════════════════════════════════════════════════════════════╗
    ║                    SETUP COMPLETADO                          ║
    ╠═══════════════════════════════════════════════════════════════╣
    ║                                                               ║
    ║  Próximos pasos:                                             ║
    ║                                                               ║
    ║  1. En una terminal, inicia el Data Collector:              ║
    ║     python data_collector.py                                ║
    ║                                                              ║
    ║  2. En otra terminal, inicia la GUI:                        ║
    ║     python parking_gui.py                                   ║
    ║                                                              ║
    ║  3. Asegúrate de que:                                       ║
    ║     - El ESP32 está conectado a WiFi                        ║
    ║     - MySQL está ejecutándose                               ║
    ║     - El Data Collector escucha en puerto 5000              ║
    ║                                                              ║
    ║  Para más información, revisa README.md                      ║
    ║                                                              ║
    ╚═══════════════════════════════════════════════════════════════╝
    """)

if __name__ == '__main__':
    main()
