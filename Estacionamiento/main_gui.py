import tkinter as tk
import requests
from tkinter import messagebox

ESP32_IP = "192.168.1.100"  # Cambia por tu IP

def actualizar_estado():
    try:
        res = requests.get(f"http://{ESP32_IP}/api/getStatus")
        data = res.json()
        lbl_rfid.config(text="RFID: " + data["rfidUID"])
        lbl_distancia.config(text="Distancia: " + str(data["distancia"]) + " cm")
        lbl_plumaEntrada.config(text="Pluma Entrada: " + ("Abierta" if data["plumaEntrada"] else "Cerrada"))
        lbl_plumaSalida.config(text="Pluma Salida: " + ("Abierta" if data["plumaSalida"] else "Cerrada"))
        lbl_cajon1.config(text="Cajón 1: " + ("Ocupado" if data["cajon1"] else "Libre"))
        lbl_cajon2.config(text="Cajón 2: " + ("Ocupado" if data["cajon2"] else "Libre"))
    except:
        pass
    root.after(1000, actualizar_estado)

def guardar_parametros():
    try:
        params = {
            "TIEMPO_APERTURA_MS": int(ent_tiempo.get()),
            "SALIDA_DELAY_MS": int(ent_salida.get())
        }
        res = requests.post(f"http://{ESP32_IP}/api/setParams", json=params)
        messagebox.showinfo("Parámetros", "Guardados correctamente")
    except:
        messagebox.showerror("Error", "No se pudo enviar parámetros")

root = tk.Tk()
root.title("Estacionamiento Inteligente GUI")

# Estado
lbl_rfid = tk.Label(root, text="RFID: --")
lbl_rfid.pack()
lbl_distancia = tk.Label(root, text="Distancia: --")
lbl_distancia.pack()
lbl_plumaEntrada = tk.Label(root, text="Pluma Entrada: --")
lbl_plumaEntrada.pack()
lbl_plumaSalida = tk.Label(root, text="Pluma Salida: --")
lbl_plumaSalida.pack()
lbl_cajon1 = tk.Label(root, text="Cajón 1: --")
lbl_cajon1.pack()
lbl_cajon2 = tk.Label(root, text="Cajón 2: --")
lbl_cajon2.pack()

# Configuración parámetros
tk.Label(root, text="Tiempo apertura pluma (ms):").pack()
ent_tiempo = tk.Entry(root)
ent_tiempo.pack()
tk.Label(root, text="Delay pluma salida (ms):").pack()
ent_salida = tk.Entry(root)
ent_salida.pack()
btn_guardar = tk.Button(root, text="Guardar Parámetros", command=guardar_parametros)
btn_guardar.pack()

actualizar_estado()
root.mainloop()
