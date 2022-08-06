import sys
import json
import asyncio
from asyncio.subprocess import PIPE, STDOUT
from paho.mqtt import client as mqtt_client

from server_cfg import broker, port, keepalive, username, password

subscribe_topic = "ingchips/iq_report"
publish_topic = "ingchips/direction"
client_id = 'python-mqtt-calc-dir'

def publish_to_topic(client: mqtt_client, topic, msg):
    result = client.publish(topic, msg)
    if result[0] == 0:
        print(f"{msg}")
    else:
        print(f"Failed to send message to topic {topic}")

def publish(client: mqtt_client, msg):
    publish_to_topic(client, publish_topic, msg)

async def connect_aoa():
    global aoa_server
    aoa_server = await asyncio.create_subprocess_exec('alg', '-alg2', stdout=PIPE, stdin=PIPE)

async def exec(client, dec):
    global aoa_server
    addr = dec[0:17]
    # print(addr)
    pack = dec[17:]
    # print(pack)
    try:
        aoa_server.stdin.write(pack.encode())
        result = await asyncio.wait_for(aoa_server.stdout.readline(), 1)
    except asyncio.TimeoutError:
        print("run: except asyncio.TimeoutError:")
        pass
    else:
        if result:
            content = json.loads(result.decode())
            status  = content["status"]

            if status != 'ok':
                return

            id        = content["id"]
            azimuth   = content["azimuth"]
            elevation = content["elevation"]
            item_dict = {
                "status": status,
                "addr": addr,
                "id": id,
                "azimuth": azimuth,
                "elevation": elevation,
            }

            # print(pack)
            publish(client, json.dumps(item_dict))

def subscribe(client: mqtt_client, loop):
    def on_message(client, userdata, msg):
        # pp = msg[0:10]
        # print(pp)
        aa = len(msg.payload)
        bb = msg.payload[aa-1:]
        #print(aa)
        #print(bb)
        if bb != b'\n':
            print('**********')

        if bb.find('\\'.encode()) != -1:
            print('**********')
            return
        # print(msg.payload)
        dec = msg.payload.decode("utf-8","ignore")
        #print("subscribe: ", dec)
        loop.run_until_complete(exec(client, dec))

    client.subscribe(subscribe_topic)
    client.on_message = on_message

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            subscribe(client, loop)
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect

    if sys.platform == "win32":
        loop = asyncio.ProactorEventLoop()  # For subprocess' pipes on Windows
        asyncio.set_event_loop(loop)
    else:
        loop = asyncio.get_event_loop()
    loop.run_until_complete(connect_aoa())

    client.connect(broker, port, keepalive)

    return client

def run():
    client = connect_mqtt()
    client.loop_forever()

if __name__ == '__main__':
    run()
