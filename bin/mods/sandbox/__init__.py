import qor
import time
import random

class State:
    def __init__(self):
        qor.camera().fov = 80
        self.player = qor.Player3D(qor.camera())
        self.player.speed = 10
    
    def enter(self):
        qor.relative_mouse(True)

def preload():
    global state
    state = State()

def enter():
    global state
    state.enter()

def unload():
    global state

def logic(t):
    global state
    qor.bg_color([0,0,0])
    qor.perspective()

def render():
    global state

