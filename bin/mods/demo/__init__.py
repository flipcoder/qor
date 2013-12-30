info = {
    "name": "Example",
    "author": "Grady O'Connell",
    "email": "flipcoder@gmail.com"
}

import qor
import time

class State:
    def __init__(self):
        
        self.players = [
            qor.Sprite("data/actors/actor.json", "maskKid"),
            qor.Sprite("data/actors/actor.json", "fetusMaximus"),
            qor.Sprite("data/actors/actor.json", "grampire")
        ]

        self.players[0].position = [-16, 0, -16]
        self.players[1].position = [0, 0, -32]
        self.players[2].position = [16, 0, -48]

        for player in self.players:
            player.states([
                player.state_id("stand"),
                player.state_id("down")
            ])
            #player.state(player.state_id("walk"))
            player.spawn()
            
        self.player = qor.Player3D(qor.camera())
        
        #p = qor.camera().position
        #p[2] = 25.0
        #qor.camera().position = p
        #m = qor.camera().matrix
        #m[5] = -1.0
        #qor.camera().matrix = m
        #qor.camera().update()
        
        #self.player.state(0)
        #self.player.state(1)
        #qor.scene().num_subnodes()
        qor.ortho(False)
        
        self.player_angle = 0.0

def preload():
    global state
    #time.sleep(2)
    state = State()

def unload():
    global state

def logic(t):
    global state
    
    #qor.bg_color(qor.camera().position)
    
    #pos = state.player.position
    #pos[0] += t * 20.0
    #state.player.state(2)
    #speed = t * 0.5
    #state.player_angle += speed
    #for p in state.players:
    #    p.rotate(speed, [0,1,0])

    #while state.player_angle > 1.0:
    #    state.player_angle -= 1.0
        
    #if state.player_angle > 0.25 and state.player_angle < 0.75:
    #    for p in state.players:
    #        p.state(5)
    #else:
    #    for p in state.players:
    #        p.state(0)

    #qor.bg_color([state.player_angle, state.player_angle, state.player_angle])
    
    #qor.bg_color([pos[0], 0, 0])
    #qor.bg_color([pos[0], pos[0], pos[0]])
    #qor.bg_color([pos[0], pos[0], pos[0]])
    #state.player.position = pos

def render():
    global state

