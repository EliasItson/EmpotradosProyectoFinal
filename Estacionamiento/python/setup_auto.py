#!/usr/bin/env python3
"""
Automated setup for Parking Management System
Non-interactive version using default/provided configuration
"""

import os
import sys
import subprocess
import mysql.connector
from mysql.connector import Error as MySQLError

def print_banner():
    print("""
    ╔═══════════════════════════════════════════════════════════════╗
    ║    SISTEMA DE ESTACIONAMIENTO - Auto Setup                   ║
    ╚═══════════════════════════════════════════════════════════════╝
    """)

def create_database(host, user, password):
    """Crear base de datos y tablas"""
    try:
        conn = mysql.connector.connect(
            host=host,
            user=user,
            password=password,
            autocommit=True,
            use_pure=True
        )
        
        # Leer y ejecutar schema.sql
        schema_path = os.path.join(os.path.dirname(__file__), 'schema.sql')
        if not os.path.exists(schema_path):
            print("✗ schema.sql no encontrado")
            return False
        
        with open(schema_path, 'r', encoding='utf-8') as f:
            sql_content = f.read()
        
        # Primero crear la base de datos
        create_db_stmt = "CREATE DATABASE IF NOT EXISTS estacionamiento_db"
        try:
            stmt_cursor = conn.cursor()
            stmt_cursor.execute(create_db_stmt)
            stmt_cursor.fetchall()
            stmt_cursor.close()
        except MySQLError as e:
            print(f"⚠ Error creando BD: {e}")
        
        # Ahora cambiar a la base de datos
        try:
            stmt_cursor = conn.cursor()
            stmt_cursor.execute("USE estacionamiento_db")
            stmt_cursor.fetchall()
            stmt_cursor.close()
        except MySQLError as e:
            print(f"⚠ Error usando BD: {e}")
        
        # Ejecutar el resto de statements
        for statement in sql_content.split(';'):
            statement = statement.strip()
            # Ignorar CREATE DATABASE y USE, que ya se ejecutaron
            if statement and not statement.startswith('--') and \
               'CREATE DATABASE' not in statement and \
               'USE ' not in statement:
                try:
                    stmt_cursor = conn.cursor()
                    stmt_cursor.execute(statement)
                    stmt_cursor.fetchall()
                    stmt_cursor.close()
                except MySQLError as e:
                    # Ignorar errores de "ya existe"
                    if 'already exists' not in str(e):
                        print(f"⚠ {e}")
        
        conn.close()
        
        print("✓ Base de datos y tablas creadas/verificadas")
        return True
    
    except MySQLError as err:
        print(f"✗ Error MySQL: {err}")
        import traceback
        traceback.print_exc()
        return False
    except Exception as err:
        print(f"✗ Error general: {err}")
        import traceback
        traceback.print_exc()
        return False

def test_mysql_connection(host, user, password, database):
    """Probar conexión a MySQL"""
    try:
        conn = mysql.connector.connect(
            host=host,
            user=user,
            password=password,
            database=database,
            autocommit=True,
            use_pure=True
        )
        cursor = conn.cursor()
        cursor.execute("SELECT 1")
        cursor.fetchall()  # Consumir resultado
        cursor.close()
        conn.close()
        print(f"✓ Conexión a MySQL exitosa ({host}:{user}@{database})")
        return True
    except MySQLError as err:
        print(f"✗ Error de conexión: {err}")
        return False

def update_config_files(host, user, password, esp32_url, telemetry_host):
    """Actualizar archivos de configuración"""
    
    # Actualizar data_collector.py
    try:
        with open('data_collector.py', 'r', encoding='utf-8') as f:
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
        
        with open('data_collector.py', 'w', encoding='utf-8') as f:
            f.write(content)
        print("✓ data_collector.py configurado")
    except Exception as e:
        print(f"⚠ Error configurando data_collector.py: {e}")
    
    # Actualizar parking_gui.py
    try:
        with open('parking_gui.py', 'r', encoding='utf-8') as f:
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
        
        with open('parking_gui.py', 'w', encoding='utf-8') as f:
            f.write(content)
        print("✓ parking_gui.py configurado")
    except Exception as e:
        print(f"⚠ Error configurando parking_gui.py: {e}")

def main():
    print_banner()
    
    # Configuración predeterminada
    host = 'localhost'
    user = 'root'
    password = 'root'  # Contraseña de MySQL
    esp32_url = 'http://estacionamiento.local'
    telemetry_host = '192.168.100.45'
    
    print("\n=== CONFIGURACIÓN POR DEFECTO ===")
    print(f"Host MySQL: {host}")
    print(f"Usuario MySQL: {user}")
    print(f"Contraseña MySQL: ****")
    print(f"URL ESP32: {esp32_url}")
    print(f"IP Telemetría: {telemetry_host}")
    
    print("\n=== CREANDO BASE DE DATOS ===")
    if not test_mysql_connection(host, user, password, "mysql"):
        print("\n✗ No se pudo conectar a MySQL.")
        print("Asegúrate de que MySQL está ejecutándose y que las credenciales son correctas.")
        sys.exit(1)
    
    if not create_database(host, user, password):
        print("\n✗ Error creando base de datos")
        sys.exit(1)
    
    if not test_mysql_connection(host, user, password, "estacionamiento_db"):
        print("\n✗ No se pudo conectar a la base de datos creada")
        sys.exit(1)
    
    print("\n=== ACTUALIZANDO ARCHIVOS DE CONFIGURACIÓN ===")
    update_config_files(host, user, password, esp32_url, telemetry_host)
    
    print("""
    ╔═══════════════════════════════════════════════════════════════╗
    ║                    SETUP COMPLETADO                          ║
    ╠═══════════════════════════════════════════════════════════════╣
    ║                                                               ║
    ║  Próximos pasos:                                             ║
    ║                                                              ║
    ║  1. En una terminal, inicia el Data Collector:              ║
    ║     python data_collector.py                                ║
    ║                                                              ║
    ║  2. En otra terminal, inicia la GUI:                        ║
    ║     python parking_gui.py                                   ║
    ║                                                              ║
    ║  El ESP32 debería conectarse automáticamente                ║
    ║  y enviar datos cada 5 segundos.                            ║
    ║                                                              ║
    ╚═══════════════════════════════════════════════════════════════╝
    """)

if __name__ == '__main__':
    main()
