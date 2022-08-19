import json
from paho.mqtt import client as mqtt_client
import json
import math

from simplefilters import KalmanFilter
from server_cfg import broker, port, keepalive, username, password

subscribe_topic = "ingchips/direction"  # 消息主题
client_id = 'mqttx_34234xc'             # 客户端id不能重复
publish_topic = "ingchips/position_2d"  # 消息主题

station_def = {}

filters = {}

def degree_to_rad(degree):
    return degree * math.pi / 180

def angle_to_xy(HEIGHT, azimuth, elevation):
    r = HEIGHT * math.tan(degree_to_rad(90 - elevation))
    x = r * math.cos(degree_to_rad(azimuth))
    y = r * math.sin(degree_to_rad(azimuth))
    return x, y

def subscribe(client: mqtt_client):

    def on_message(client, userdata, msg):
        global filters

        data2 = json.loads(msg.payload.decode())

        if data2["status"] != "ok":
            return

        if data2["elevation"] < station_def["min_elevation"]:
            return

        if not (data2['addr'] in station_def):
            return

        id = data2['id']
        if not (id in filters):
            filters[id] = { 'f_x': KalmanFilter(), 'f_y': KalmanFilter()}

        station = station_def[data2['addr']]

        x, y = angle_to_xy(station[2], data2['azimuth'], data2['elevation'])
        print(x, y)
        x = filters[id]['f_x'].filter(x)
        y = filters[id]['f_y'].filter(y)

        pos = {
            'id': id,
            'x': x + station[0],
            'y': y + station[1],
        }

        print(json.dumps(pos))

        client.publish(publish_topic, json.dumps(pos))

    client.subscribe(subscribe_topic)
    client.on_message = on_message

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            subscribe(client)
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port, keepalive)

    return client

def run():
    client = connect_mqtt()
    client.loop_forever()

if __name__ == '__main__':
    with open("station2d.json") as f:
        station_def = json.loads(f.read())

    run()
