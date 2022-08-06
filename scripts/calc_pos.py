
#!/usr/bin/env python
#coding:utf-8
import numpy as np
import os
import time
from numpy.linalg import inv
import json
import threading

import random
from paho.mqtt import client as mqtt_client
import json
import math
import sys

from simplefilters import KalmanFilter
from server_cfg import broker, port, keepalive, username, password

topic = "ingchips/direction"  # 消息主题
client_id = 'mqttx_7bsdfjl'   # 客户端id不能重复

pubtopic = "ingchips/position_3d"  # 消息主题

myid = []
# tup1 = (0,0,0)
list = []
tup1 = []
tup2 = []
tup3 = []
start1 = []
start2 = []
start3 = []

# 设置递归深度
sys.setrecursionlimit(100000000)

f_x = KalmanFilter()
f_y = KalmanFilter()
f_z = KalmanFilter()

def connect_mqtt():
    '''连接mqtt代理服务器'''
    def on_connect(client, userdata, flags, rc):
        '''连接回调函数'''
        # 响应状态码为0表示连接成功
        if rc == 0:
            print("Connected to MQTT SUB OK!")

        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set('yangfan', '123456')
    client.on_connect = on_connect
    client.connect(broker, port, keepalive )

    return client

def subscribe(client: mqtt_client):
    '''订阅主题并接收消息'''
    def on_message(client, userdata, msg):
        '''订阅消息回调函数'''
        # print("====",msg)
        # print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

        data1 = msg.payload.decode()
        # print("data1：", data1)

        # # # 编码
        # json_str = json.dumps(data1)
        # print("JSON 对象：", json_str)
        # # # 解码
        data2 = json.loads(data1)

        if len(myid) != 0:
            for i in range(0, len(myid)):
                myid.pop()
        myid.append(data2["id"])
        print("角度数据是：------------", data2)

        if data2["status"] == "ok":
            print("status：", data2["status"])

            getresult(data2,client)


    # 订阅指定消息主题
    client.subscribe(topic)
    client.on_message = on_message

###获取最短距离
def intersection_of_multi_lines(strt_points, directions):
    '''
    strt_points: line start points; numpy array, nxdim
    directions: list dierctions; numpy array, nxdim

    return: the nearest points to n lines
    '''

    n, dim = strt_points.shape

    G_left = np.tile(np.eye(dim), (n, 1))
    G_right = np.zeros((dim * n, n))

    for i in range(n):
        G_right[i * dim:(i + 1) * dim, i] = -directions[i, :]

    G = np.concatenate([G_left, G_right], axis=1)
    d = strt_points.reshape((-1, 1))

    m = np.linalg.inv(np.dot(G.T, G)).dot(G.T).dot(d)

    # return m[0:dim]
    return m


def getresult(dire,client):
    # print("par====", dire)

    if dire["addr"] == "D1:29:F6:BE:F3:26":
        a = dire["azimuth"]
        e = dire["elevation"]
        # 角度为0的过滤
        # 偏离较大的过滤
        # 1度=pai/180 弧度
        # 下面将 y 和 z 坐标颠倒，以符合右手坐标系
        if e != 0:
            w = math.pi / 180
            x1 = math.cos(e * w) * math.cos(a * w)
            y1 = math.sin(e * w)
            z1 = math.cos(e * w) * math.sin(a * w)
            # print("math.cos(e) == ", math.cos(e * w))
            # print("math.cos(a) == ", math.cos(a * w))
            # print("x1 == ", x1)
            # print("y1 == ", y1)
            # print("z1 == ", z1)

            # tup1 = [x1, y1, z1]
            # print("tup1 == ", tup1)
            # 删除 列表中的数据重新添加
            if len(tup1) != 0:
                for i in range(0, len(tup1)):
                    tup1.pop()
                # print("tup1 == ", tup1)

            tup1.append(x1)
            tup1.append(y1)
            tup1.append(z1)
            print("射线tup1 == ", tup1)

            # list.append(tup1)
            # print("list == ", list)

    if dire["addr"] == "D2:29:F6:BE:F3:26":
        a = dire["azimuth"]
        e = dire["elevation"]

        if e != 0:
            w = math.pi / 180
            x2 = math.cos(e * w) * math.cos(a * w)
            y2 = math.sin(e * w)
            # print("y2 == ", y2)
            z2 = math.cos(e * w) * math.sin(a * w)
            # print("z2 == ", z2)

            # tup2 = [x2, y2, z2]
            if len(tup2) != 0:
                for i in range(0, len(tup2)):
                    tup2.pop()
                # print("tup2 == ", tup2)
            tup2.append(x2)
            tup2.append(y2)
            tup2.append(z2)
            print("射线tup2 == ", tup2)
            # list.append(tup2)


    if dire["addr"] == "D3:29:F6:BE:F3:26":
        a = dire["azimuth"]
        e = dire["elevation"]
        # 角度为0的过滤
        if e != 0:
            w = math.pi / 180
            x3 = math.cos(e*w) * math.cos(a*w)
            y3 = math.cos(e*w) * math.sin(a*w)
            # print("y3 == ", y3)
            z3 = math.sin(e*w)
            # print("z3 == ", z3)

            # tup3 = [x3, y3, z3]
            if len(tup3) != 0:
                for i in range(0, len(tup3)):
                    tup3.pop()
                # print("tup3 == ", tup3)
            tup3.append(x3)
            tup3.append(y3)
            tup3.append(z3)
            # print("tup3 == ", tup3)
            # list.append(tup3)

    ##########################################################################################
    # #test case

    # 如果三个坐标都不为空，执行下面
#        if len(tup1) == 3 & len(tup2) == 3 & len(tup3) == 3:
#     print("len(tup1)======== ", len(tup1))
#     print("len(tup2)======== ", len(tup2))
    if len(tup1) == 3 & len(tup2) == 3:
        # print("有三个板子解算后了------------------------")
        # print("角度三角函数解算后tup1 == ", tup1)
        # print("角度三角函数解算后tup2 == ", tup2)
#        print("角度三角函数解算后tup3 == ", tup3)
        print("天线板坐标start1 == ", start1[0])
        print("天线板坐标start2 == ", start2[0])
#        print("天线板坐标start3 == ", start3[0])
        strt_point = np.zeros((2, 3))
        strt_point[0, :] = np.array(start1[0])
        strt_point[1, :] = np.array(start2[0])
#        strt_point[2, :] = np.array(start3[0])

        directions = np.zeros((2, 3))
        directions[0, :] = np.array(tup1)
        directions[1, :] = np.array(tup2)
#        directions[2, :] = np.array(tup3)

        inters = intersection_of_multi_lines(strt_point, directions)
        # print('strt_point', strt_point)
        # print('directions', directions)
        # print('[DEBUG] intersection {}'.format(inters))

        coordinate = inters[:3]
        print('===============================坐标是：')
        print(coordinate)
        # 过滤差异较大的坐标


        coordinate2 = []
        # if len(coordinate2) != 0:
        #     for i in range(0, len(coordinate2)):
        #         coordinate2.pop()

        global f_x
        global f_y
        global f_z
        x_1 = f_x.filter((coordinate[0])[0])
        y_1 = f_y.filter((coordinate[1])[0])
        z_1 = f_z.filter((coordinate[2])[0])
        # print((coordinate[0])[0])
        # print(x_1)

        coordinate2.append(x_1)
        coordinate2.append(y_1)
        coordinate2.append(z_1)

        print('--------------------------------修改后坐标是：')
        print(coordinate2)

        # 得到坐标后，发布消息
        # runmypub(coordinate2,client)

        publish(client, coordinate2)

# def runmypub(coordinate2,client):
#     # print('传参1：------------', coordinate)
#     '''运行发布者'''
#     client = connect_mqtt_pub()
#     # 运行一个线程来自动调用loop()处理网络事件, 非阻塞
#     client.loop_start()
#     publish(coordinate2, client)


# def connect_mqtt_pub():
#     '''连接mqtt代理服务器'''
#
#     def on_connect(client, userdata, flags, rc):
#         '''连接回调函数'''
#         # 响应状态码为0表示连接成功
#         if rc == 0:
#             print("Connected to MQTT PUB OK!")
#         else:
#             print("Failed to connect, return code %d\n", rc)
#
#     # 连接mqtt代理服务器，并获取连接引用
#
#     client = mqtt_client.Client(client_id)
#     client.username_pw_set('yangfan', '123456')
#     client.on_connect = on_connect
#     client.connect(broker, port, keepalive)
#     return client

def publish(client, coordinate2):
    # print('传参2：------------', coordinate2)
    '''发布消息'''
    '''每隔1秒发布一次服务器信息'''

    msg = get_info(coordinate2)
    result = client.publish(pubtopic, msg)
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{pubtopic}`")

        # 这里是否有问题
        # client.disconnect()
        # client.loop_stop()

        # 再次运行 请求角度
        # client = connect_mqtt()
        subscribe(client)
        #  运行一个线程来自动调用loop()处理网络事件, 阻塞模式loop_forever()
        # client.loop_forever()

    else:
        print(f"Failed to send message to topic {pubtopic}")



# time.sleep(0.3)
def get_info(coordinate2):
    # print('传参3：------------', coordinate)
    # print("id是：------------", myid[0])

    info = {
        'id': myid,
        'x': coordinate2[0],
        'y': coordinate2[1],
        'z': coordinate2[2]
    }
    # print('info：------------', info)
    # mqtt只能传输字符串数据
    return json.dumps(info)

def run():
    # filepath = loadfile()
    # print('filepath:' + filepath)

    f = open("station.json")
    lines = f.read()
    #print('天线板位置坐标:------------', lines)
    # print(type(lines))
    f.close()

    data3 = json.loads(lines)
    print('天线板位置坐标:------------', data3)
    # {"D1:29:F6:BE:F3:26": [0, 0, 0], "D2:29:F6:BE:F3:26": [0, 1, 0], "D3:29:F6:BE:F3:26": [0, 0, 0]}
    # {"D1:29:F6:BE:F3:26": [0, 0, 0], "D2:29:F6:BE:F3:26": [0, 1, 0]}

    start1.append(data3["D1:29:F6:BE:F3:26"])
    start2.append(data3["D2:29:F6:BE:F3:26"])
#    start3.append(data3["D3:29:F6:BE:F3:26"])

    print("天线板坐标start1：", start1)
    print("天线板坐标start2：", start2)
#    print("天线板坐标start3：", start3)


    # 运行订阅者
    client = connect_mqtt()
    subscribe(client)
    #  运行一个线程来自动调用loop()处理网络事件, 阻塞模式loop_forever()
    client.loop_forever()
    # client.loop_start()

if __name__ == '__main__':
    run()
