# python3.6
from time import time
import random
import socket
from paho.mqtt import client as mqtt_client

# Telemetry
rttList = list()
openPackets = list()

# Broker connection
broker = '172.20.128.40'
port = 1883
# generate client ID with pub prefix randomly
client_id = f'python-mqttadapter-{random.randint(0, 100)}'

# UDP connection
SELF_IP = '' #  gateway
UDP_PORT = 8080 # iot port
MCU_IP = "172.20.128.4"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_addr = (SELF_IP, UDP_PORT)
sock.bind(server_addr)
dataArr = list()

# Returns the current timestamp s
def getcurrtimestamp():
    return int(time())

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print("onmsg")
        content = msg.payload.decode('utf-8')
        # split to extract packet id
        #1923849203840;id;data
        splitContent = content.split(';')
        
        for packet in openPackets:
            if(packet['packetid'] == splitContent[1]):
                # Same ID
                rttList.append(getcurrtimestamp() - int(packet['timestamp']))
        # set content to fit original form timestamp;data
        content = splitContent[0]+";"+splitContent[2]
        print("Appended " + content+ " to dataArr")
        dataArr.append(content)


    client.subscribe("reply")
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_start()

    packetID = 1
    while True:
        data, address = sock.recvfrom(4096)

        #print("Received: " +str(data.decode('utf-8')))

        prep_message = data.decode('utf-8')
        prep_message += ";"+str(packetID) ## append ID

        mapArgs = prep_message.split(';')

        openPackets.append(dict(timestamp=mapArgs[0], packetid=mapArgs[1]))

        packetID += 1
        client.publish("request",prep_message) # send to topic for request
        
        while(len(dataArr) == 0):
            sus = 1 

        prep_message = dataArr.pop()
        print(prep_message)
  
        sock.sendto(prep_message.encode('utf-8'),address)


run()
