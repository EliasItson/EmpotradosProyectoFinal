#!/usr/bin/env python3
"""
Data Collector for Estacionamiento System
Receives telemetry data from ESP32 via TCP and stores it in MySQL
"""

import socket
import json
import threading
import logging
from datetime import datetime
import mysql.connector
from mysql.connector import Error as MySQLError
import time

# Configuración
TCP_HOST = '0.0.0.0'
TCP_PORT = 5000
BUFFER_SIZE = 4096

# Configuración de MySQL
DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': 'root',  # Cambiar a tu contraseña MySQL
    'database': 'estacionamiento_db'
}

# Logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - [%(levelname)s] - %(message)s',
    handlers=[
        logging.FileHandler('data_collector.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)


class DataCollector:
    def __init__(self):
        self.running = True
        self.db = None
        self.connect_database()
    
    def connect_database(self):
        """Conectar a la base de datos MySQL"""
        try:
            self.db = mysql.connector.connect(**DB_CONFIG)
            logger.info("Conectado a MySQL exitosamente")
        except MySQLError as err:
            logger.error(f"Error al conectar a MySQL: {err}")
            self.db = None
    
    def reconnect_database(self):
        """Reconectar a la base de datos si la conexión se perdió"""
        if self.db and self.db.is_connected():
            return
        logger.warning("Intentando reconectar a MySQL...")
        self.connect_database()
    
    def insert_sensor_reading(self, data):
        """Insertar lectura de sensores"""
        if not self.db or not self.db.is_connected():
            self.reconnect_database()
            if not self.db:
                return
        
        try:
            cursor = self.db.cursor()
            query = """
                INSERT INTO sensor_readings 
                (timestamp, distance, uptime_seconds, available_slots)
                VALUES (%s, %s, %s, %s)
            """
            cursor.execute(query, (
                datetime.fromisoformat(data.get('timestamp', datetime.now().isoformat())),
                data.get('distance', 0),
                data.get('uptime_seconds', 0),
                data.get('available_slots', 0)
            ))
            self.db.commit()
            cursor.close()
            logger.info(f"Lectura de sensor guardada: distancia={data.get('distance')}, temp={data.get('temperature')}")
        except MySQLError as err:
            logger.error(f"Error al insertar sensor reading: {err}")
    
    def insert_barrier_event(self, data):
        """Insertar evento de barrera"""
        if not self.db or not self.db.is_connected():
            self.reconnect_database()
            if not self.db:
                return
        
        try:
            cursor = self.db.cursor()
            query = """
                INSERT INTO barrier_events 
                (timestamp, barrier_type, event_type, reason)
                VALUES (%s, %s, %s, %s)
            """
            cursor.execute(query, (
                datetime.fromisoformat(data.get('timestamp', datetime.now().isoformat())),
                data.get('barrier_type', 'entrance'),
                data.get('event_type', 'raised'),
                data.get('reason', '')
            ))
            self.db.commit()
            cursor.close()
            logger.info(f"Evento de barrera guardado: {data.get('barrier_type')} {data.get('event_type')}")
        except MySQLError as err:
            logger.error(f"Error al insertar barrier event: {err}")
    
    def insert_rfid_access(self, data):
        """Insertar evento de acceso RFID"""
        if not self.db or not self.db.is_connected():
            self.reconnect_database()
            if not self.db:
                return
        
        try:
            cursor = self.db.cursor()
            query = """
                INSERT INTO rfid_access 
                (timestamp, card_uid, authorized, action)
                VALUES (%s, %s, %s, %s)
            """
            cursor.execute(query, (
                datetime.fromisoformat(data.get('timestamp', datetime.now().isoformat())),
                data.get('card_uid', ''),
                data.get('authorized', False),
                data.get('action', '')
            ))
            self.db.commit()
            cursor.close()
            logger.info(f"Acceso RFID guardado: {data.get('card_uid')} - {'Autorizado' if data.get('authorized') else 'Denegado'}")
        except MySQLError as err:
            logger.error(f"Error al insertar rfid access: {err}")
    
    def insert_config_change(self, data):
        """Insertar cambio de configuración"""
        if not self.db or not self.db.is_connected():
            self.reconnect_database()
            if not self.db:
                return
        
        try:
            cursor = self.db.cursor()
            query = """
                INSERT INTO config_changes 
                (timestamp, parameter_name, old_value, new_value, changed_by)
                VALUES (%s, %s, %s, %s, %s)
            """
            cursor.execute(query, (
                datetime.fromisoformat(data.get('timestamp', datetime.now().isoformat())),
                data.get('parameter_name', ''),
                data.get('old_value', 0),
                data.get('new_value', 0),
                data.get('changed_by', 'api')
            ))
            self.db.commit()
            cursor.close()
            logger.info(f"Cambio de configuración guardado: {data.get('parameter_name')} = {data.get('new_value')}")
        except MySQLError as err:
            logger.error(f"Error al insertar config change: {err}")
    
    def insert_slot_occupancy(self, data):
        """Insertar estado de ocupación de espacio"""
        if not self.db or not self.db.is_connected():
            self.reconnect_database()
            if not self.db:
                return
        
        try:
            cursor = self.db.cursor()
            query = """
                INSERT INTO slot_occupancy 
                (timestamp, slot_number, occupied)
                VALUES (%s, %s, %s)
            """
            cursor.execute(query, (
                datetime.fromisoformat(data.get('timestamp', datetime.now().isoformat())),
                data.get('slot_number', 0),
                data.get('occupied', False)
            ))
            self.db.commit()
            cursor.close()
            logger.info(f"Ocupación de espacio guardada: slot {data.get('slot_number')} = {'Ocupado' if data.get('occupied') else 'Libre'}")
        except MySQLError as err:
            logger.error(f"Error al insertar slot occupancy: {err}")
    
    def process_telemetry(self, json_data):
        """Procesar datos de telemetría recibidos"""
        try:
            data = json.loads(json_data)
            event_type = data.get('type', 'unknown')
            
            if event_type == 'sensor_reading':
                self.insert_sensor_reading(data.get('data', {}))
            elif event_type == 'barrier_event':
                self.insert_barrier_event(data.get('data', {}))
            elif event_type == 'rfid_access':
                self.insert_rfid_access(data.get('data', {}))
            elif event_type == 'config_change':
                self.insert_config_change(data.get('data', {}))
            elif event_type == 'slot_occupancy':
                self.insert_slot_occupancy(data.get('data', {}))
            else:
                logger.warning(f"Tipo de evento desconocido: {event_type}")
        
        except json.JSONDecodeError as err:
            logger.error(f"Error al parsear JSON: {err}")
        except Exception as err:
            logger.error(f"Error al procesar telemetría: {err}")
    
    def handle_client(self, client_socket, addr):
        """Manejar conexión de cliente"""
        logger.info(f"Conexión recibida desde {addr}")
        try:
            while self.running:
                data = client_socket.recv(BUFFER_SIZE).decode('utf-8')
                if not data:
                    break
                
                # Procesar cada línea JSON (pueden venir múltiples)
                for line in data.strip().split('\n'):
                    if line:
                        self.process_telemetry(line)
        
        except Exception as err:
            logger.error(f"Error en conexión con {addr}: {err}")
        finally:
            client_socket.close()
            logger.info(f"Conexión cerrada: {addr}")
    
    def start_server(self):
        """Iniciar servidor TCP"""
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            server_socket.bind((TCP_HOST, TCP_PORT))
            server_socket.listen(5)
            logger.info(f"Servidor TCP iniciado en {TCP_HOST}:{TCP_PORT}")
            
            while self.running:
                try:
                    client_socket, addr = server_socket.accept()
                    # Crear thread para cada cliente
                    client_thread = threading.Thread(
                        target=self.handle_client,
                        args=(client_socket, addr),
                        daemon=True
                    )
                    client_thread.start()
                except KeyboardInterrupt:
                    break
        
        except Exception as err:
            logger.error(f"Error en servidor TCP: {err}")
        finally:
            server_socket.close()
            if self.db and self.db.is_connected():
                self.db.close()
            logger.info("Servidor TCP detenido")


def main():
    collector = DataCollector()
    try:
        collector.start_server()
    except KeyboardInterrupt:
        logger.info("Deteniendo data collector...")
        collector.running = False


if __name__ == '__main__':
    main()
