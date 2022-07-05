# python 3.6

import random
import time

from paho.mqtt import client as mqtt_client


broker = '172.20.128.40'
port = 1883
topic = "python/mqtt"
# generate client ID with pub prefix randomly
client_id = f'python-mqttsensor-{random.randint(0, 1000)}'
# username = 'emqx'
# password = 'public'

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def on_message(client, data, msg):
    client.publish("reply", msg.payload.decode('utf-8')+";"+str(random.randint(80,100)))



def run():
    client = connect_mqtt()
    client.on_message = on_message
    client.subscribe("request")

    client.loop_forever()
    


run()
