info = {
    "name": "Full Game (The Goal Demo of Qor)",
    "author": "Grady O'Connell",
    "email": "flipcoder@gmail.com"
}

import qor
from scripts import ctf

class Mod(qor.Mod):
    def __init__(self):
        # set up specific things that engine needs to know about the mod
        # possibly setting up customized gui, background music, theming, etc.
        # include possible selections of things, and which customizations to
        # enable
        pass

    def validate():
        
        # make sure the game can be run with the current players/settings
        self.map_info = qor.MapInfo(qor.get("map","church"))
        if qor.is_map(self.map_info):
            raise qor.Error

        # checks with ctf script if the given map is okay
        if not ctf.valid_map(self.map_info):
            raise qor.Error
        
    def preload():
        # set up FOV and all that good stuff (possibly read from player config)
        qor.ortho(False)
        
        # set up any mod-specific stuff
    
        # load a specific (or user selected) map
        self.level = qor.Map(self.map_fn)

        self.mode_logic = ctf.logic
    
    def update(t):
        
        # possibly distinguish between client and server somewhere here
        # do player logic (do different classes and characters have diff logic)?
        self.mode_logic(t)
        # update the game, call any modes logic or policies here
    
    def event(ev):
        # called upon specific game events
        # also called if something changes in config that must affect game
        pass
    
    #def stats():
    #    pass

