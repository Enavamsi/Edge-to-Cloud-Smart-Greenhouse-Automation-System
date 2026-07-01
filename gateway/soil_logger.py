import paho.mqtt.client as mqtt
import json
import csv
from datetime import datetime
import os

MQTT_BROKER = "localhost" 
MQTT_TOPIC = "farm/sensors"
CSV_FILE = "soil_nutrient_data.csv"

if not os.path.isfile(CSV_FILE):
    with open(CSV_FILE, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([
            "Timestamp", "Lux", "AirTemp_C", "AirHum_%", 
            "SoilMoist_%", "SoilTemp_C", "pH", "N_mg/kg", "P_mg/kg", "K_mg/kg"
        ])

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        row = [
            ts, data.get("lux"), data.get("air_temp"), data.get("air_humid"),
            data.get("soil_moist"), data.get("soil_temp"), data.get("ph"),
            data.get("n"), data.get("p"), data.get("k")
        ]
        
        with open(CSV_FILE, mode='a', newline='') as f:
            csv.writer(f).writerow(row)
            
        print(f"[{ts}] Saved Locally.")
    except Exception as e:
        print(f"Error: {e}")

client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, 1883, 60)
client.subscribe(MQTT_TOPIC)
client.loop_forever()
