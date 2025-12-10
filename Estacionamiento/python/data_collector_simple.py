#!/usr/bin/env python3
"""
Simple Data Collector (sin MySQL)
Recibe datos del ESP32 y los muestra en consola
Útil para testing mientras se configura MySQL
"""

import socket
import json
import threading
import logging
from datetime import datetime

TCP_HOST = '0.0.0.0'
TCP_PORT = 5000
BUFFER_SIZE = 4096

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - [%(levelname)s] - %(message)s',
    handlers=[
        logging.FileHandler('data_collector_simple.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

class SimpleDataCollector:
    def __init__(self):
        self.running = True
    
    def process_telemetry(self, json_data):
        """Procesar y mostrar telemetría"""
        try:
            data = json.loads(json_data)
            event_type = data.get('type', 'unknown')
            event_data = data.get('data', {})
            
            print(f"\n[{event_type.upper()}]")
            print(f"  Timestamp: {event_data.get('timestamp', 'N/A')}")
            
            if event_type == 'sensor_reading':
                print(f"  Distancia: {event_data.get('distance'):.1f} cm")
                print(f"  Temperatura: {event_data.get('temperature'):.1f} °C")
                print(f"  Uptime: {event_data.get('uptime_seconds')} s")
                print(f"  Espacios disponibles: {event_data.get('available_slots')}")
            
            elif event_type == 'rfid_access':
                auth_str = "AUTORIZADO" if event_data.get('authorized') else "DENEGADO"
                print(f"  UID: {event_data.get('card_uid')}")
                print(f"  Estado: {auth_str}")
                print(f"  Acción: {event_data.get('action')}")
            
            elif event_type == 'barrier_event':
                print(f"  Barrera: {event_data.get('barrier_type')}")
                print(f"  Evento: {event_data.get('event_type')}")
                print(f"  Razón: {event_data.get('reason')}")
            
            elif event_type == 'slot_occupancy':
                occupied_str = "OCUPADO" if event_data.get('occupied') else "LIBRE"
                print(f"  Espacio: {event_data.get('slot_number')}")
                print(f"  Estado: {occupied_str}")
            
            elif event_type == 'config_change':
                print(f"  Parámetro: {event_data.get('parameter_name')}")
                print(f"  Anterior: {event_data.get('old_value')}")
                print(f"  Nuevo: {event_data.get('new_value')}")
        
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
            print(f"\n{'='*70}")
            print(f"Data Collector SIMPLE activo - Esperando conexiones en puerto {TCP_PORT}...")
            print(f"{'='*70}\n")
            
            while self.running:
                try:
                    client_socket, addr = server_socket.accept()
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
            logger.info("Servidor TCP detenido")

def main():
    collector = SimpleDataCollector()
    try:
        collector.start_server()
    except KeyboardInterrupt:
        logger.info("Deteniendo data collector...")
        collector.running = False

if __name__ == '__main__':
    main()
