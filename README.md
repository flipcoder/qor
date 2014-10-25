Qor
===

Copyright (c) 2014 Grady O'Connell

2D/3D OpenGL Game Engine (C++11 w/ Python scripting)
    
Status: Hobby project, still tinkering

This is a rewrite of my [older game engine](https://github.com/flipcoder/qor-old).

##Instructions
- My C++11 common toolset, [kit](http://github.com/flipcoder/kit), is required.
- *premake4.lua* contains a list of libraries you will need.
- Build Qor with [premake4](http://industriousone.com/premake/download).
- [siege-tools](https://github.com/flipcoder/siege-tools) is recommended.
- Example mods and scripts will be available soon in a separate repository.

##Features

- Cross-platform
- Multi-threaded (more improvements here soon)
- Multi-layer tilemap system for 2D games
    - Compatible with [Tiled](http://mapeditor.org) map editor
- OpenGL ES-compatible shader-based pipeline
    - Normal GL stuff like antialiasing, anisotropic filtering
    - Forward rendering w/ multipass lighting
    - Partitioner support (WIP)
- C++ and Python mod support
- Shared resource management
    - Animating Sprites
    - Resource path finding management
    - Basic create() function for all types
    - Json config for more complex resources
        - Config-based type-resolution and subresources
    - Cache garbage collection
    - Composite resource suport (resources loading other resources through cache)
    - User-defined resources
    - Texture streaming support (Not in yet, but supported by resource cache)
- 3D Scene graph
    - World matrix transform caching
    - Tracking/following w/ animation support
    - Node factory supporting user classes
    - Node tags
- State stack
    - User-defined states
    - Async loading screens
        - thread-safe OpenGL task handler
- Mesh support
    - Mesh copy-on-write vertex and shader attribute modifications
    - Basic instancing (WIP)
- Input
    - Multiple controllers
    - Bindings specific to player profile
    - Button/switch-associated events
    - Switch history for timer-based events and multiple fast presses
    - Activation thresholds
- Animation (code moved to [kit](https://github.com/flipcoder/kit))
    - Timer-based keyframes
    - Interpolation/Easing
    - Callback scheduling
- Music and 3D Sound
    - Powered by OpenAL
    - Wav and Ogg support
    - Thread-safe
- [Cairo](http://cairographics.org) Render-To-Texture
- [Pango](http://pango.org) Font and Text Rendering

More features will be listed as I add them...

##Screenshots

[![Screenshot1](http://i.imgur.com/zQxsEfgl.jpg)](http://i.imgur.com/zQxsEfg.jpg)

[![Screenshot2](http://i.imgur.com/XNxLRtKl.jpg)](http://i.imgur.com/XNxLRtK.jpg)

[![Screenshot2](http://i.imgur.com/VPH9ZpCl.png)](http://i.imgur.com/VPH9ZpC.jpg)

##Videos

[![Qorpse 2D Game Jam Progress, built with Qor](http://img.youtube.com/vi/5Fyw-sdIT_8/0.jpg)](http://www.youtube.com/watch?v=5Fyw-sdIT_8)

##Examples

Some rough examples:

- [C++ Usage Example](https://github.com/flipcoder/qor/blob/master/src/DemoState.cpp)
- [Python Usage Example](https://github.com/flipcoder/qor/blob/master/bin/mods/scriptdemo/__init__.py)

