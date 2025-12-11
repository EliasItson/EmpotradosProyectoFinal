"""
Subsistema Principal - GUI Estacionamiento Inteligente

Características:
- Mostrar estado de sensores y actuadores (datos de BD local)
- Modificar parámetros del sistema (comunicación con ESP32)
- Representación gráfica del estacionamiento
- Estadísticas con gráficos por período
- Actualización en tiempo real desde BD local
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import requests
import mysql.connector
import threading
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
from datetime import datetime, timedelta
import time

# Configuración
ESP32_IP = "192.168.100.91"
ESP32_PORT = 80

DB_CONFIG = {
    'host': 'localhost',
    'user': 'root',
    'password': 'root',
    'database': 'estacionamiento'
}

class EstacionamientoGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Estacionamiento Inteligente - Sistema de Control")
        self.root.geometry("1200x700")
        
        self.esp32_url = f"http://{ESP32_IP}:{ESP32_PORT}"
        self.update_running = True
        
        # Crear interfaz
        self.create_widgets()
        
        # Iniciar actualización automática
        self.update_status_thread = threading.Thread(target=self.auto_update_status, daemon=True)
        self.update_status_thread.start()
        
    def create_widgets(self):
        """Crear componentes de la GUI"""
        # Notebook (pestañas)
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Pestaña 1: Estado y Control
        self.tab_estado = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_estado, text="Estado y Control")
        self.create_estado_tab()
        
        # Pestaña 2: Parámetros
        self.tab_params = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_params, text="Parámetros")
        self.create_params_tab()
        
        # Pestaña 3: Estadísticas
        self.tab_stats = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_stats, text="Estadísticas")
        self.create_stats_tab()
        
        # Frame inferior con botones de control
        self.frame_control = ttk.Frame(self.root)
        self.frame_control.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Button(self.frame_control, text="Actualizar Ahora", command=self.actualizar_estado_ahora).pack(side=tk.LEFT, padx=5)
        ttk.Button(self.frame_control, text="Limpiar BD", command=self.limpiar_bd).pack(side=tk.LEFT, padx=5)
        ttk.Button(self.frame_control, text="Salir", command=self.root.quit).pack(side=tk.RIGHT, padx=5)
        
        self.lbl_status = ttk.Label(self.frame_control, text="Estado: Inicializando...", foreground="blue")
        self.lbl_status.pack(side=tk.LEFT, padx=10)
        
    def create_estado_tab(self):
        """Pestaña de estado"""
        # Frame izquierda: estado de sensores/actuadores
        frame_left = ttk.LabelFrame(self.tab_estado, text="Estado Actual (desde BD Local)", padding=10)
        frame_left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Labels de estado
        self.lbl_rfid = ttk.Label(frame_left, text="RFID: --", font=("Arial", 10))
        self.lbl_rfid.pack(anchor=tk.W, pady=5)
        
        self.lbl_distancia = ttk.Label(frame_left, text="Distancia: -- cm", font=("Arial", 10))
        self.lbl_distancia.pack(anchor=tk.W, pady=5)
        
        self.lbl_plumaEntrada = ttk.Label(frame_left, text="Pluma Entrada: --", font=("Arial", 10))
        self.lbl_plumaEntrada.pack(anchor=tk.W, pady=5)
        
        self.lbl_plumaSalida = ttk.Label(frame_left, text="Pluma Salida: --", font=("Arial", 10))
        self.lbl_plumaSalida.pack(anchor=tk.W, pady=5)
        
        ttk.Separator(frame_left, orient=tk.HORIZONTAL).pack(fill=tk.X, pady=10)
        
        # Representación gráfica del estacionamiento
        frame_visual = ttk.LabelFrame(frame_left, text="Representación Gráfica", padding=10)
        frame_visual.pack(fill=tk.BOTH, expand=True, pady=10)
        
        # Canvas para dibujar estacionamiento
        self.canvas = tk.Canvas(frame_visual, width=300, height=200, bg="white", relief=tk.SUNKEN)
        self.canvas.pack(fill=tk.BOTH, expand=True)
        self.draw_parking()
        
        # Frame derecha: info de entrada/salida
        frame_right = ttk.LabelFrame(self.tab_estado, text="Información de Entrada/Salida", padding=10)
        frame_right.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        ttk.Label(frame_right, text="Cajón 1", font=("Arial", 11, "bold")).pack(anchor=tk.W, pady=(10, 5))
        self.lbl_cajon1 = ttk.Label(frame_right, text="Estado: --", font=("Arial", 10))
        self.lbl_cajon1.pack(anchor=tk.W)
        self.lbl_entry1 = ttk.Label(frame_right, text="Entrada: --", font=("Arial", 9))
        self.lbl_entry1.pack(anchor=tk.W)
        self.lbl_exit1 = ttk.Label(frame_right, text="Salida: --", font=("Arial", 9))
        self.lbl_exit1.pack(anchor=tk.W, pady=(0, 15))
        
        ttk.Label(frame_right, text="Cajón 2", font=("Arial", 11, "bold")).pack(anchor=tk.W, pady=(10, 5))
        self.lbl_cajon2 = ttk.Label(frame_right, text="Estado: --", font=("Arial", 10))
        self.lbl_cajon2.pack(anchor=tk.W)
        self.lbl_entry2 = ttk.Label(frame_right, text="Entrada: --", font=("Arial", 9))
        self.lbl_entry2.pack(anchor=tk.W)
        self.lbl_exit2 = ttk.Label(frame_right, text="Salida: --", font=("Arial", 9))
        self.lbl_exit2.pack(anchor=tk.W)
        
    def create_params_tab(self):
        """Pestaña de parámetros"""
        frame = ttk.LabelFrame(self.tab_params, text="Parámetros del Sistema", padding=20)
        frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Delay pluma salida
        ttk.Label(frame, text="Delay pluma salida (ms):", font=("Arial", 10)).grid(row=0, column=0, sticky=tk.W, pady=10)
        self.ent_salida = ttk.Entry(frame, width=15, font=("Arial", 10))
        self.ent_salida.grid(row=0, column=1, sticky=tk.W, padx=10)
        
        # Timeout ultrasonico
        ttk.Label(frame, text="Timeout ultrasonico (ms):", font=("Arial", 10)).grid(row=1, column=0, sticky=tk.W, pady=10)
        self.ent_timeout_us = ttk.Entry(frame, width=15, font=("Arial", 10))
        self.ent_timeout_us.grid(row=1, column=1, sticky=tk.W, padx=10)
        
        # Botones
        ttk.Button(frame, text="Cargar Parámetros", command=self.cargar_parametros).grid(row=2, column=0, pady=20, padx=10)
        ttk.Button(frame, text="Guardar Parámetros", command=self.guardar_parametros).grid(row=2, column=1, padx=10)
        
        self.lbl_params_status = ttk.Label(frame, text="", foreground="green")
        self.lbl_params_status.grid(row=3, column=0, columnspan=2, pady=10)
        
    def create_stats_tab(self):
        """Pestaña de estadísticas"""
        frame_controls = ttk.LabelFrame(self.tab_stats, text="Filtros de Período", padding=10)
        frame_controls.pack(fill=tk.X, padx=10, pady=10)
        
        ttk.Label(frame_controls, text="Últimas N horas:").pack(side=tk.LEFT, padx=5)
        self.var_horas = tk.StringVar(value="24")
        ttk.Spinbox(frame_controls, from_=1, to=168, textvariable=self.var_horas, width=5).pack(side=tk.LEFT, padx=5)
        
        ttk.Button(frame_controls, text="Actualizar Gráficos", command=self.update_stats).pack(side=tk.LEFT, padx=10)
        
        # Canvas para gráficos
        self.canvas_stats = tk.Canvas(self.tab_stats, bg="white")
        self.canvas_stats.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
    def draw_parking(self):
        """Dibujar representación gráfica del estacionamiento"""
        self.canvas.delete("all")
        
        # Obtener estado de cajones
        try:
            res = requests.get(f"{self.esp32_url}/api/getStatus", timeout=2)
            data = res.json()
            cajon1_ocupado = data.get("cajon1", False)
            cajon2_ocupado = data.get("cajon2", False)
        except:
            cajon1_ocupado = False
            cajon2_ocupado = False
        
        # Dibujar cajones
        color1 = "red" if cajon1_ocupado else "green"
        color2 = "red" if cajon2_ocupado else "green"
        
        # Texto
        self.canvas.create_text(150, 20, text="ESTACIONAMIENTO", font=("Arial", 14, "bold"))
        
        # Cajón 1
        self.canvas.create_rectangle(30, 60, 130, 120, fill=color1, outline="black", width=2)
        self.canvas.create_text(80, 90, text="Cajón 1", font=("Arial", 10, "bold"), fill="white")
        
        # Cajón 2
        self.canvas.create_rectangle(170, 60, 270, 120, fill=color2, outline="black", width=2)
        self.canvas.create_text(220, 90, text="Cajón 2", font=("Arial", 10, "bold"), fill="white")
        
        # Leyenda
        self.canvas.create_rectangle(30, 150, 100, 170, fill="green", outline="black")
        self.canvas.create_text(115, 160, text="Libre", font=("Arial", 9), anchor=tk.W)
        
        self.canvas.create_rectangle(180, 150, 250, 170, fill="red", outline="black")
        self.canvas.create_text(265, 160, text="Ocupado", font=("Arial", 9), anchor=tk.W)
        
    def actualizar_estado_ahora(self):
        """Actualizar estado manualmente"""
        self.lbl_status.config(text="Actualizando...", foreground="blue")
        self.root.update()
        self.refresh_estado_from_db()
        self.draw_parking()
        self.lbl_status.config(text="Actualizado a las " + datetime.now().strftime("%H:%M:%S"), foreground="green")
        
    def refresh_estado_from_db(self):
        """Obtener último estado de BD local"""
        try:
            conn = mysql.connector.connect(**DB_CONFIG)
            cursor = conn.cursor(dictionary=True)
            cursor.execute("SELECT * FROM lecturas ORDER BY timestamp DESC LIMIT 1")
            result = cursor.fetchone()
            conn.close()
            
            if result:
                self.lbl_rfid.config(text=f"RFID: {result.get('rfidUID', '--')}")
                self.lbl_distancia.config(text=f"Distancia: {result.get('distancia', '--')} cm")
                self.lbl_plumaEntrada.config(text=f"Pluma Entrada: {'Abierta' if result.get('plumaEntrada') else 'Cerrada'}")
                self.lbl_plumaSalida.config(text=f"Pluma Salida: {'Abierta' if result.get('plumaSalida') else 'Cerrada'}")
                
                self.lbl_cajon1.config(text=f"Estado: {'Ocupado' if result.get('cajon1') else 'Libre'}")
                self.lbl_cajon2.config(text=f"Estado: {'Ocupado' if result.get('cajon2') else 'Libre'}")
                
                self.lbl_entry1.config(text=f"Entrada: {result.get('entryTime1', '--')}")
                self.lbl_exit1.config(text=f"Salida: {result.get('exitTime1', '--')}")
                self.lbl_entry2.config(text=f"Entrada: {result.get('entryTime2', '--')}")
                self.lbl_exit2.config(text=f"Salida: {result.get('exitTime2', '--')}")
        except Exception as e:
            print(f"Error al obtener estado de BD: {e}")
            
    def cargar_parametros(self):
        """Cargar parámetros desde ESP32"""
        try:
            res = requests.get(f"{self.esp32_url}/api/getParams", timeout=5)
            data = res.json()
            self.ent_salida.delete(0, tk.END)
            self.ent_salida.insert(0, str(data.get("SALIDA_DELAY_MS", "")))
            self.ent_timeout_us.delete(0, tk.END)
            self.ent_timeout_us.insert(0, str(data.get("ULTRASONIC_TIMEOUT_MS", "")))
            self.lbl_params_status.config(text="✓ Parámetros cargados correctamente", foreground="green")
        except Exception as e:
            self.lbl_params_status.config(text=f"✗ Error al cargar: {e}", foreground="red")
            
    def guardar_parametros(self):
        """Guardar parámetros en ESP32"""
        try:
            params = {
                "SALIDA_DELAY_MS": int(self.ent_salida.get()),
                "ULTRASONIC_TIMEOUT_MS": int(self.ent_timeout_us.get())
            }
            res = requests.post(f"{self.esp32_url}/api/setParams", json=params, timeout=5)
            self.lbl_params_status.config(text="✓ Parámetros guardados correctamente", foreground="green")
        except ValueError:
            self.lbl_params_status.config(text="✗ Valores inválidos", foreground="red")
        except Exception as e:
            self.lbl_params_status.config(text=f"✗ Error: {e}", foreground="red")
            
    def update_stats(self):
        """Actualizar gráficos de estadísticas"""
        try:
            horas = int(self.var_horas.get())
            fecha_inicio = datetime.now() - timedelta(hours=horas)
            
            conn = mysql.connector.connect(**DB_CONFIG)
            cursor = conn.cursor(dictionary=True)
            cursor.execute("""
                SELECT * FROM lecturas 
                WHERE timestamp >= %s
                ORDER BY timestamp ASC
            """, (fecha_inicio,))
            datos = cursor.fetchall()
            conn.close()
            
            if not datos:
                messagebox.showwarning("Sin datos", f"No hay datos en los últimos {horas} horas")
                return
            
            # Crear gráficos
            fig = Figure(figsize=(12, 4), dpi=100)
            
            # Gráfico 1: Ocupación de cajones
            ax1 = fig.add_subplot(131)
            timestamps = [d['timestamp'].strftime("%H:%M") for d in datos]
            cajon1_data = [d['cajon1'] for d in datos]
            cajon2_data = [d['cajon2'] for d in datos]
            
            ax1.plot(range(len(timestamps)), cajon1_data, label="Cajón 1", marker='o', markersize=3)
            ax1.plot(range(len(timestamps)), cajon2_data, label="Cajón 2", marker='s', markersize=3)
            ax1.set_title("Ocupación de Cajones")
            ax1.set_xlabel("Tiempo")
            ax1.set_ylabel("Ocupado (1=Sí, 0=No)")
            ax1.legend()
            ax1.grid(True, alpha=0.3)
            
            # Gráfico 2: Distancia ultrasónica
            ax2 = fig.add_subplot(132)
            distancias = [d['distancia'] for d in datos]
            ax2.plot(range(len(timestamps)), distancias, color="orange", marker='o', markersize=3)
            ax2.set_title("Distancia Ultrasónica")
            ax2.set_xlabel("Tiempo")
            ax2.set_ylabel("Distancia (cm)")
            ax2.grid(True, alpha=0.3)
            
            # Gráfico 3: Actividad de plumas
            ax3 = fig.add_subplot(133)
            plumaE = [d['plumaEntrada'] for d in datos]
            plumaS = [d['plumaSalida'] for d in datos]
            ax3.plot(range(len(timestamps)), plumaE, label="Pluma Entrada", marker='o', markersize=3)
            ax3.plot(range(len(timestamps)), plumaS, label="Pluma Salida", marker='s', markersize=3)
            ax3.set_title("Estado de Plumas")
            ax3.set_xlabel("Tiempo")
            ax3.set_ylabel("Abierta (1=Sí, 0=No)")
            ax3.legend()
            ax3.grid(True, alpha=0.3)
            
            fig.tight_layout()
            
            # Mostrar en canvas
            self.canvas_stats.delete("all")
            canvas = FigureCanvasTkAgg(fig, master=self.canvas_stats)
            canvas.draw()
            self.canvas_stats.create_window(0, 0, window=canvas.get_tk_widget(), anchor=tk.NW)
            
        except Exception as e:
            messagebox.showerror("Error", f"Error al generar gráficos: {e}")
            
    def auto_update_status(self):
        """Thread de actualización automática cada 2 segundos"""
        while self.update_running:
            try:
                self.refresh_estado_from_db()
                time.sleep(2)
            except:
                time.sleep(2)
                
    def limpiar_bd(self):
        """Limpiar todos los registros de BD"""
        if messagebox.askyesno("Confirmación", "¿Limpiar todos los datos de la BD?"):
            try:
                conn = mysql.connector.connect(**DB_CONFIG)
                cursor = conn.cursor()
                cursor.execute("DELETE FROM lecturas")
                conn.commit()
                conn.close()
                messagebox.showinfo("Éxito", "BD limpiada correctamente")
            except Exception as e:
                messagebox.showerror("Error", f"Error al limpiar: {e}")

if __name__ == "__main__":
    try:
        root = tk.Tk()
        app = EstacionamientoGUI(root)
        print("[GUI] Iniciando mainloop...")
        root.mainloop()
        print("[GUI] Mainloop finalizado")
    except Exception as e:
        print(f"[ERROR] {e}")
        import traceback
        traceback.print_exc()
