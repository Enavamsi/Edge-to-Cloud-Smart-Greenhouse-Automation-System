import paho.mqtt.client as mqtt
import json
from datetime import datetime
import requests

MQTT_BROKER = "localhost" 
MQTT_TOPIC = "farm/sensors"
WEB_API_URL = _______
API_KEY = "YOUR_SECURE_API_KEY"

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        web_payload = {
            "api_key": API_KEY,
            "lux": data.get("lux", 0),
            "air_temp": data.get("air_temp", 0),
            "air_hum": data.get("air_humid", 0),
            "soil_moist": data.get("soil_moist", 0),
            "soil_temp": data.get("soil_temp", 0),
            "ph": data.get("ph", 0),
            "nitrogen": data.get("n", 0),
            "phosphorus": data.get("p", 0),
            "potassium": data.get("k", 0)
        }

        response = requests.post(WEB_API_URL, json=web_payload, timeout=10)
        print(f"[{ts}] Cloud Upload Status: {response.status_code}")

    except Exception as e:
        print(f"Error: {e}")

client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, 1883, 60)
client.subscribe(MQTT_TOPIC)
client.loop_forever()
