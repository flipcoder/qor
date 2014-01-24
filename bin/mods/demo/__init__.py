import qor
import time
import random

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
            
        songs = [
            "atmos1.ogg",
            "sh_tribute1.ogg",
            "sh_tribute2.ogg"
        ]
        self.music = qor.Sound(songs[random.randrange(len(songs))])
        self.music.spawn()
        self.sound = qor.Sound("power.wav")
        self.sound.spawn()

        #self.level = qor.Mesh("tantrum.obj")
        #self.level.spawn()
        #self.level = qor.Mesh("pplay_rc1_MAP01.obj") # saltmines
        #self.level.spawn()
        self.level = qor.Mesh("level_silentScalpels.obj")
        self.level.spawn()

        # light test
        self.light = qor.Light()
        self.light.spawn()
        
        #self.gun = qor.Mesh("gun_shotgun_sawnoff.obj")
        #self.gun = qor.Mesh("gun_bullpup.obj")
        #self.gun.rescale(2)
        #self.ads = False
        #self.gun.position = [
        #    0 if self.ads else 0.05,
        #    -0.04 if self.ads else -0.06,
        #    -0.05 if self.ads else -0.15
        #]
        #qor.camera().add(self.gun)

        # control the gun
        #self.player = qor.Player3D(self.gun)
        #self.player.speed = 0.1

        #control the player
        qor.camera().fov = 80
        self.player = qor.Player3D(qor.camera())
        #self.gun.spawn()
        #self.player = qor.Player3D(self.gun)
        #self.player.speed = 0.5
        self.player.speed = 10

    def start(self):
        
        self.music.play()
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
    #    #state.sound.play()
        
    #    state.gun.position = [
    #        0 if state.ads else 0.05,
    #        -0.04 if state.ads else -0.06,
    #        -0.05 if state.ads else -0.15
    #    ]
    #    qor.camera().fov = 60 if state.ads else 80
    #    qor.ortho(False)
    
    #print state.gun.position
        
    if not state.started:
        state.start()
    qor.bg_color([0,0,0])
    qor.ortho(False)

def render():
    global state

