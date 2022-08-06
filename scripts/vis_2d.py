import json
import math
import glfw
import OpenGL.GL as gl
from queue import Queue
from paho.mqtt import client as mqtt_client

import imgui
from imgui.integrations.glfw import GlfwRenderer

from server_cfg import broker, port, keepalive, username, password

client_id = 'mqttx_sdfxdfxc'              # 客户端id不能重复
subscribe_topic = "ingchips/position_2d"  # 消息主题

scene = {}

client = 0
g_q = Queue()
font_size = 12

def cute_hash(s: str) -> int:
    hash = 5381
    for c in s:
        hash = ((((hash << 5) & 0xffffffff) + hash) + ord(c)) & 0xffffffff
    return (hash & 0xffffff) | 0xff000000

def draw_scene(draw_list, c_x, c_y, scale, scene):

    center_x = scene['center'][0]
    center_y = scene['center'][1]

    def map_x_to_ui(v):
        return (v - center_x) * scale + c_x
    def map_y_to_ui(v):
        return (v - center_y) * scale + c_y

    THICKNESS = 3
    ROUNDING = 8

    for item in scene['entities']:
        if item['type'] == 'rect':
            x0 = map_x_to_ui(item['x'])
            y0 = map_y_to_ui(item['y'])
            if 'fill' in item:
                draw_list.add_rect_filled(
                    x0, y0, x0 + item['w'] * scale, y0 + item['h'] * scale,
                    imgui.get_color_u32_rgba(*item['fill']),
                    rounding = ROUNDING
                )
            if 'border' in item:
                draw_list.add_rect(
                    x0, y0, x0 + item['w'] * scale, y0 + item['h'] * scale,
                    imgui.get_color_u32_rgba(*item['border']),
                    thickness=THICKNESS,
                    rounding = ROUNDING
                )
        if item['type'] == 'circle':
            x0 = map_x_to_ui(item['x'])
            y0 = map_y_to_ui(item['y'])
            if 'fill' in item:
                draw_list.add_circle_filled(
                    x0, y0, item['r'] * scale,
                    imgui.get_color_u32_rgba(*item['fill']),
                    num_segments=50
                )
            if 'border' in item:
                draw_list.add_circle(
                    x0, y0, item['r'] * scale,
                    imgui.get_color_u32_rgba(*item['border']),
                    thickness=THICKNESS,
                    num_segments=50
                )
        if item['type'] == 'line':
            draw_list.add_line(
                map_x_to_ui(item['x0']), map_y_to_ui(item['y0']),
                map_x_to_ui(item['x1']), map_y_to_ui(item['y1']),
                imgui.get_color_u32_rgba(*item['border']), item['thickness'])

def main():
    imgui.create_context()
    window = impl_glfw_init()
    impl = GlfwRenderer(window)

    tags = { }

    while not glfw.window_should_close(window):
        glfw.poll_events()
        impl.process_inputs()

        while g_q.qsize() > 0:
            msg = g_q.get()
            id = msg['id']
            if not (id in tags):
                tags[id] = { "hash": cute_hash(id), "x": 0, "y": 0 }
            tags[id]["x"] = msg['x']
            tags[id]["y"] = msg['y']

        imgui.new_frame()

        imgui.begin("Position", True,
            flags=imgui.WINDOW_NO_TITLE_BAR,
        )
        imgui.set_window_size(1500, 900, condition=imgui.FIRST_USE_EVER)

        w, h = imgui.get_window_size()
        pos = imgui.get_cursor_screen_pos()
        draw_list = imgui.get_window_draw_list()

        c_x = pos.x + w / 2
        c_y = pos.y + h / 2

        center_x = scene['center'][0]
        center_y = scene['center'][1]

        scale = min(w / scene['width'], h / scene['height']) * 0.95
        t_w = scene['width'] * scale
        t_h = scene['height'] * scale

        draw_list.add_rect_filled(
            c_x - t_w / 2,
            c_y - t_h / 2,
            c_x + t_w / 2,
            c_y + t_h / 2,
            imgui.get_color_u32_rgba(*scene['background']),
            rounding = 10
        )

        draw_scene(draw_list, c_x, c_y, scale, scene)

        mark_size = w * 0.01
        mark_color = imgui.get_color_u32_rgba(0.5, 0.8, 0.5, 1.0)
        draw_list.add_line(c_x - mark_size, c_y, c_x + mark_size, c_y, mark_color, 2.0)
        draw_list.add_line(c_x, c_y - mark_size, c_x, c_y + mark_size, mark_color, 2.0)

        R = w * 0.008
        for k, v in tags.items():
            x0 = (v["x"] - center_x) * scale + c_x
            y0 = (v["y"] - center_y) * scale + c_y
            draw_list.add_circle_filled(
                x0, y0, R,
                imgui.get_color_u32(v["hash"])
            )
            draw_list.add_text(
                x0 + R + 5,
                y0 - 5,
                imgui.get_color_u32_rgba(0.8, 0.8, 0.8, 1.0),
                k
            )

        imgui.end()

        gl.glClearColor(0.1, 0.1, 0.1, 1)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

        imgui.render()
        impl.render(imgui.get_draw_data())
        glfw.swap_buffers(window)

    global client
    impl.shutdown()
    glfw.terminate()
    client.loop_stop()

def impl_glfw_init():
    width, height = 1280, 720
    window_name = "Visualization of BLE 2D Positioning"

    if not glfw.init():
        print("Could not initialize OpenGL context")
        exit(1)

    # OS X supports only forward-compatible core profiles from 3.2
    glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
    glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
    glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

    glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, gl.GL_TRUE)

    # Create a windowed mode window and its OpenGL context
    window = glfw.create_window(
        int(width), int(height), window_name, None, None
    )
    glfw.make_context_current(window)

    if not window:
        glfw.terminate()
        print("Could not initialize Window")
        exit(1)

    return window

def connect_mqtt():
    def on_message(client, userdata, msg):
        data = json.loads(msg.payload.decode())
        g_q.put(data)

    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            client.subscribe(subscribe_topic)
            client.on_message = on_message
        else:
            print("Failed to connect, return code %d\n", rc)
    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port, keepalive)
    client.loop_start()
    return client

if __name__ == "__main__":
    with open("scene.json") as f:
        scene = json.loads(f.read())

    client = connect_mqtt()
    main()