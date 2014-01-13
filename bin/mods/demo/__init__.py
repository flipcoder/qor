import qor
import time

class State:
    def __init__(self):
         
        self.started = False
        
        #self.players = [
        #    qor.Sprite("actor.json", "maskKid"),
        #    qor.Sprite("actor.json", "fetusMaximus"),
        #    qor.Sprite("actor.json", "grampire")
        #]

        #self.players[0].position = [-16, 0, -16]
        #self.players[1].position = [0, 0, -32]
        #self.players[2].position = [16, 0, -48]

        #for player in self.players:
        #    player.states([
        #        player.state_id("stand"),
        #        player.state_id("down")
        #    ])
        #    player.state(player.state_id("walk"))
        #    player.spawn()
            
        self.music = qor.Sound("ingame2.ogg")
        self.music.spawn()
        self.sound = qor.Sound("power.wav")
        self.sound.spawn()

        #self.level = qor.Mesh("tantrum.obj")
        self.level = qor.Mesh("level_silentScalpels.obj")
        self.level.spawn()
        
        self.gun = qor.Mesh("gun_bullpup.obj")
        self.gun.rescale(2)
        qor.camera().add(self.gun)
        #self.gun.spawn()

        # control the gun
        #self.player = qor.Player3D(self.gun)
        #self.player.speed = 0.1

        #control the player
        qor.camera().fov = 50.0
        self.player = qor.Player3D(qor.camera())
        self.player.speed = 10.0

        self.dist = 0.25
        self.xoffs = 0
        self.accum = 0.0
        self.ads = False
        
        self.gun.position = [
            0 if self.ads else 0.05,
            -0.04 if self.ads else -0.06,
            -0.05 if self.ads else -0.15
        ]

    def start(self):
        
        #self.music.play()
        #self.sound.play()
        self.started = True

def preload():
    global state
    state = State()

def unload():
    global state

def logic(t):
    global state

    #state.accum += t
    #if state.accum > 2.0:
    #    state.accum = 0
    #    state.ads = not state.ads
    
    print state.gun.position
        
    if not state.started:
        state.start()
    qor.bg_color([1,1,1])
    qor.ortho(False)

def render():
    global state

