#!/usr/bin/env python3
"""
Parking Management GUI - Main Application
Displays sensor data, barrier status, RFID access, and allows remote parameter control
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import requests
import json
import threading
import logging
from datetime import datetime, timedelta
import mysql.connector
from mysql.connector import Error as MySQLError
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import pandas as pd

# Configuración
ESP32_URL = "http://estacionamiento.local"  # O usar IP: http://192.168.100.91
UPDATE_INTERVAL = 2000  # 2 segundos

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
    format='%(asctime)s - [%(levelname)s] - %(message)s'
)
logger = logging.getLogger(__name__)


class ParkingGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sistema de Estacionamiento")
        self.root.geometry("1200x800")
        
        self.db = None
        self.esp32_status = {"distance": 0, "temp": 0, "uptime": 0, "available": 0, "ip": ""}
        self.esp32_params = {}
        
        self.setup_database()
        self.create_gui()
        self.start_update_thread()
    
    def setup_database(self):
        """Conectar a la base de datos MySQL"""
        try:
            self.db = mysql.connector.connect(**DB_CONFIG)
            logger.info("Conectado a MySQL")
        except MySQLError as err:
            logger.error(f"Error al conectar a MySQL: {err}")
            messagebox.showerror("Error", f"No se pudo conectar a MySQL: {err}")
    
    def create_gui(self):
        """Crear la interfaz gráfica"""
        # Crear notebook (tabs)
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Tab 1: Dashboard
        self.dashboard_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.dashboard_frame, text="Dashboard")
        self.create_dashboard()
        
        # Tab 2: Control de Parámetros
        self.control_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.control_frame, text="Control")
        self.create_control_panel()
        
        # Tab 3: Estadísticas
        self.stats_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.stats_frame, text="Estadísticas")
        self.create_stats_panel()
        
        # Tab 4: Logs
        self.logs_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.logs_frame, text="Logs")
        self.create_logs_panel()
    
    def create_dashboard(self):
        """Crear tab de dashboard con estado del sistema"""
        # Frame superior con información del sistema
        top_frame = ttk.LabelFrame(self.dashboard_frame, text="Estado del Sistema")
        top_frame.pack(fill=tk.BOTH, expand=False, padx=10, pady=10)
        
        # Grid de información
        info_grid = ttk.Frame(top_frame)
        info_grid.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Distancia ultrasónica
        ttk.Label(info_grid, text="Distancia (cm):").grid(row=0, column=0, sticky=tk.W, padx=5, pady=5)
        self.label_distance = ttk.Label(info_grid, text="--", font=("Arial", 14, "bold"))
        self.label_distance.grid(row=0, column=1, sticky=tk.W, padx=5, pady=5)
        
        # (Temperatura eliminada - dato no disponible)
        
        # Uptime
        ttk.Label(info_grid, text="Uptime (seg):").grid(row=1, column=0, sticky=tk.W, padx=5, pady=5)
        self.label_uptime = ttk.Label(info_grid, text="--", font=("Arial", 14, "bold"))
        self.label_uptime.grid(row=1, column=1, sticky=tk.W, padx=5, pady=5)
        
        # Espacios disponibles
        ttk.Label(info_grid, text="Espacios disponibles:").grid(row=1, column=2, sticky=tk.W, padx=5, pady=5)
        self.label_available = tk.Label(info_grid, text="--", font=("Arial", 14, "bold"), fg="green")
        self.label_available.grid(row=1, column=3, sticky=tk.W, padx=5, pady=5)
        
        # IP del ESP32
        ttk.Label(info_grid, text="IP ESP32:").grid(row=2, column=0, sticky=tk.W, padx=5, pady=5)
        self.label_ip = ttk.Label(info_grid, text="--")
        self.label_ip.grid(row=2, column=1, columnspan=3, sticky=tk.W, padx=5, pady=5)
        
        # (Se eliminó el segmento "Ocupación en Tiempo Real" por diseño)
        self.canvas_dashboard = None
    
    def create_control_panel(self):
        """Crear panel de control de parámetros"""
        control_lf = ttk.LabelFrame(self.control_frame, text="Parámetros Configurables")
        control_lf.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # ULTRASONIC_THRESHOLD
        ttk.Label(control_lf, text="Umbral Ultrasónico (cm):").grid(row=0, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_threshold = ttk.Entry(control_lf, width=20)
        self.entry_threshold.grid(row=0, column=1, padx=10, pady=5)
        
        # ULTRASONIC_TIMEOUT
        ttk.Label(control_lf, text="Timeout Ultrasónico (ms):").grid(row=1, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_timeout = ttk.Entry(control_lf, width=20)
        self.entry_timeout.grid(row=1, column=1, padx=10, pady=5)
        
        # LOWER_BARRIER_WAIT
        ttk.Label(control_lf, text="Tiempo Espera Barrera (ms):").grid(row=2, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_barrier_wait = ttk.Entry(control_lf, width=20)
        self.entry_barrier_wait.grid(row=2, column=1, padx=10, pady=5)
        
        # DISPLAY_MESSAGE
        ttk.Label(control_lf, text="Tiempo Mensaje Display (ms):").grid(row=3, column=0, sticky=tk.W, padx=10, pady=5)
        self.entry_message_time = ttk.Entry(control_lf, width=20)
        self.entry_message_time.grid(row=3, column=1, padx=10, pady=5)
        
        # Botones
        button_frame = ttk.Frame(control_lf)
        button_frame.grid(row=4, column=0, columnspan=2, pady=20)
        
        ttk.Button(button_frame, text="Obtener Parámetros", command=self.fetch_params).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Guardar Parámetros", command=self.save_params).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Restaurar Valores", command=self.reset_params).pack(side=tk.LEFT, padx=5)
    
    def create_stats_panel(self):
        """Crear panel de estadísticas"""
        stats_lf = ttk.LabelFrame(self.stats_frame, text="Estadísticas de Ocupación")
        stats_lf.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Selector de rango de fechas
        filter_frame = ttk.Frame(stats_lf)
        filter_frame.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Label(filter_frame, text="Desde:").pack(side=tk.LEFT, padx=5)
        self.entry_date_from = ttk.Entry(filter_frame, width=15)
        self.entry_date_from.pack(side=tk.LEFT, padx=5)
        self.entry_date_from.insert(0, (datetime.now() - timedelta(days=7)).strftime("%Y-%m-%d"))
        
        ttk.Label(filter_frame, text="Hasta:").pack(side=tk.LEFT, padx=5)
        self.entry_date_to = ttk.Entry(filter_frame, width=15)
        self.entry_date_to.pack(side=tk.LEFT, padx=5)
        self.entry_date_to.insert(0, datetime.now().strftime("%Y-%m-%d"))
        
        ttk.Button(filter_frame, text="Generar Gráfico", command=self.generate_chart).pack(side=tk.LEFT, padx=5)
        
        # Canvas para gráfico
        self.canvas_stats = tk.Canvas(stats_lf, bg='white')
        self.canvas_stats.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
    
    def create_logs_panel(self):
        """Crear panel de logs y eventos"""
        logs_lf = ttk.LabelFrame(self.logs_frame, text="Eventos Recientes")
        logs_lf.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Selector de tipo de evento
        filter_frame = ttk.Frame(logs_lf)
        filter_frame.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Label(filter_frame, text="Filtrar por:").pack(side=tk.LEFT, padx=5)
        self.combo_event_type = ttk.Combobox(
            filter_frame,
            values=["Todos", "Acceso RFID", "Eventos Barrera", "Lecturas Sensores", "Cambios Config"],
            state="readonly"
        )
        self.combo_event_type.set("Todos")
        self.combo_event_type.pack(side=tk.LEFT, padx=5)
        
        ttk.Button(filter_frame, text="Actualizar", command=self.update_logs).pack(side=tk.LEFT, padx=5)
        
        # Treeview para logs
        self.logs_tree = ttk.Treeview(
            logs_lf,
            columns=("timestamp", "type", "description"),
            height=20
        )
        self.logs_tree.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        self.logs_tree.heading("#0", text="ID")
        self.logs_tree.heading("timestamp", text="Timestamp")
        self.logs_tree.heading("type", text="Tipo")
        self.logs_tree.heading("description", text="Descripción")
        
        self.logs_tree.column("#0", width=50)
        self.logs_tree.column("timestamp", width=180)
        self.logs_tree.column("type", width=150)
        self.logs_tree.column("description", width=400)
    
    def fetch_params(self):
        """Obtener parámetros del ESP32"""
        try:
            response = requests.get(f"{ESP32_URL}/api/getParams", timeout=5)
            if response.status_code == 200:
                self.esp32_params = response.json()
                self.entry_threshold.delete(0, tk.END)
                self.entry_threshold.insert(0, str(self.esp32_params.get("ULTRASONIC_THRESHOLD", "")))
                
                self.entry_timeout.delete(0, tk.END)
                self.entry_timeout.insert(0, str(self.esp32_params.get("ULTRASONIC_TIMEOUT_MS", "")))
                
                self.entry_barrier_wait.delete(0, tk.END)
                self.entry_barrier_wait.insert(0, str(self.esp32_params.get("LOWER_BARRIER_WAIT_MS", "")))
                
                self.entry_message_time.delete(0, tk.END)
                self.entry_message_time.insert(0, str(self.esp32_params.get("DISPLAY_MESSAGE_MS", "")))
                
                messagebox.showinfo("Éxito", "Parámetros obtenidos correctamente")
            else:
                messagebox.showerror("Error", f"Error al obtener parámetros: {response.status_code}")
        except Exception as e:
            messagebox.showerror("Error", f"Error de conexión: {e}")
    
    def save_params(self):
        """Guardar parámetros en el ESP32"""
        try:
            params = {
                "ULTRASONIC_THRESHOLD": int(self.entry_threshold.get()),
                "ULTRASONIC_TIMEOUT_MS": int(self.entry_timeout.get()),
                "LOWER_BARRIER_WAIT_MS": int(self.entry_barrier_wait.get()),
                "DISPLAY_MESSAGE_MS": int(self.entry_message_time.get())
            }
            response = requests.post(
                f"{ESP32_URL}/api/setParams",
                json=params,
                timeout=5
            )
            if response.status_code == 200:
                messagebox.showinfo("Éxito", "Parámetros guardados correctamente")
            else:
                    # Mostrar cuerpo de respuesta si existe (ayuda a depurar)
                    body = response.text if response.text else str(response.status_code)
                    messagebox.showerror("Error", f"Error al guardar: {body}")
        except ValueError:
            messagebox.showerror("Error", "Por favor ingrese números válidos")
        except Exception as e:
            messagebox.showerror("Error", f"Error de conexión: {e}")
    
    def reset_params(self):
        """Resetear parámetros a valores por defecto"""
        if messagebox.askyesno("Confirmar", "¿Resetear parámetros a valores por defecto?"):
            # Aquí irían los valores por defecto
            messagebox.showinfo("Info", "Funcionalidad aún no implementada")
    
    def update_status(self):
        """Actualizar estado del sistema desde ESP32"""
        try:
            response = requests.get(f"{ESP32_URL}/api/getStatus", timeout=5)
            if response.status_code == 200:
                self.esp32_status = response.json()
                self.label_distance.config(text=f"{self.esp32_status.get('distancia', 0):.1f}")
                self.label_uptime.config(text=f"{self.esp32_status.get('uptime', 0)}")
                self.label_available.config(text=f"{self.esp32_status.get('disponibles', 0)}/2")
                self.label_ip.config(text=self.esp32_status.get('ip', '--'))
        except Exception as e:
            logger.warning(f"Error al actualizar estado: {e}")
    
    def generate_chart(self):
        """Generar gráfico de ocupación por rango de fechas"""
        if not self.db or not self.db.is_connected():
            messagebox.showerror("Error", "No hay conexión a MySQL")
            return
        
        try:
            date_from = self.entry_date_from.get()
            date_to = self.entry_date_to.get()
            
            query = """
                SELECT DATE(timestamp) AS day, COUNT(*) as occupied_count
                FROM slot_occupancy
                WHERE occupied = 1 AND timestamp BETWEEN %s AND %s
                GROUP BY day
                ORDER BY day
            """
            
            cursor = self.db.cursor(dictionary=True)
            cursor.execute(query, (date_from, date_to))
            results = cursor.fetchall()
            cursor.close()
            
            if not results:
                messagebox.showinfo("Info", "No hay datos para mostrar")
                return
            
            df = pd.DataFrame(results)
            df['day'] = pd.to_datetime(df['timestamp']).dt.date
            
            fig = Figure(figsize=(10, 4), dpi=100)
            ax = fig.add_subplot(111)
            ax.plot(df['day'], df['occupied_count'], marker='o')
            ax.set_xlabel("Fecha")
            ax.set_ylabel("Espacios Ocupados")
            ax.set_title("Ocupación de Estacionamiento")
            fig.tight_layout()
            
            # Mostrar en canvas
            canvas = FigureCanvasTkAgg(fig, master=self.canvas_stats)
            canvas.draw()
            canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
        except Exception as e:
            messagebox.showerror("Error", f"Error al generar gráfico: {e}")
    
    def update_logs(self):
        """Actualizar tabla de logs desde MySQL"""
        if not self.db or not self.db.is_connected():
            messagebox.showerror("Error", "No hay conexión a MySQL")
            return
        
        try:
            # Limpiar tabla
            for item in self.logs_tree.get_children():
                self.logs_tree.delete(item)
            
            # Obtener eventos
            query = """
                SELECT id, timestamp, 'RFID' as type, CONCAT('UID: ', card_uid, ' - ', IF(authorized, 'Autorizado', 'Denegado')) as description
                FROM rfid_access
                UNION ALL
                SELECT id, timestamp, 'BARRERA' as type, CONCAT(barrier_type, ' - ', event_type) as description
                FROM barrier_events
                UNION ALL
                SELECT id, timestamp, 'CONFIG' as type, CONCAT(parameter_name, ' = ', new_value) as description
                FROM config_changes
                ORDER BY timestamp DESC
                LIMIT 50
            """
            
            cursor = self.db.cursor(dictionary=True)
            cursor.execute(query)
            results = cursor.fetchall()
            cursor.close()
            
            for i, row in enumerate(results):
                self.logs_tree.insert("", "end", text=str(row['id']), values=(
                    row['timestamp'],
                    row['type'],
                    row['description']
                ))
        
        except Exception as e:
            logger.error(f"Error al actualizar logs: {e}")
    
    def start_update_thread(self):
        """Iniciar thread para actualizar datos periódicamente"""
        def update_loop():
            while True:
                self.update_status()
                self.root.after(UPDATE_INTERVAL, lambda: None)
                import time
                time.sleep(UPDATE_INTERVAL / 1000)
        
        thread = threading.Thread(target=update_loop, daemon=True)
        thread.start()


def main():
    root = tk.Tk()
    app = ParkingGUI(root)
    root.mainloop()


if __name__ == '__main__':
    main()
