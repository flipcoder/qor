info = {
    "name": "Example",
    "author": "Grady O'Connell",
    "email": "flipcoder@gmail.com"
}

import qor
import time

class State:
    def __init__(self):
         
        self.started = False
        
        self.players = [
            qor.Sprite("actor.json", "maskKid"),
            qor.Sprite("actor.json", "fetusMaximus"),
            qor.Sprite("actor.json", "grampire")
        ]

        self.players[0].position = [-16, 0, -16]
        self.players[1].position = [0, 0, -32]
        self.players[2].position = [16, 0, -48]

        for player in self.players:
            player.states([
                player.state_id("stand"),
                player.state_id("down")
            ])
            player.state(player.state_id("walk"))
            player.spawn()
            
        self.player = qor.Player3D(qor.camera())

    def start(self):
        
        # music
        self.sound = qor.Sound("ingame2.ogg")
        self.sound.spawn()
        self.sound.play()

        # sound effect
        self.sound = qor.Sound("power.wav")
        self.sound.spawn()
        self.sound.play()
        
        self.started = True

def preload():
    global state
    #time.sleep(2)
    state = State()

def unload():
    global state

def logic(t):
    global state
    if not state.started:
        state.start()
    qor.bg_color([0,0,0])
    qor.ortho(False)

def render():
    global state

