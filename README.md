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
    - Partitioner/culling support
    - Vertex Buffer Objects (VBOs)
    - Mesh instance copy-on-write vertex and shader attribute modifications
    - Basic instancing
    - Render-to-Texture
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
    - Static mesh baking (collapse to world space and combine)
    - Tracking/following w/ animation support
    - Node factory supporting user classes
    - Node tags
    - OBJ support
    - Mesh triangulation
- State stack
    - User-defined states
    - Async loading screens
    - Thread-safe OpenGL task handler
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
- [Cairo](http://cairographics.org) 2D primitives w/ Render-To-Texture
- [Pango](http://pango.org) Font and Text Rendering

More features will be listed as I add them...

##Partial/WIP Features 
- Blender exporter (about half way done)
- Particle systems (rewriting this atm)
- Shader selection based on mesh and material properties (almost done)
- Better instancing
- Python bindings are currently very limited
- More documentation and user-friendliness

##Screenshots

[![Screenshot1](http://i.imgur.com/zQxsEfgl.jpg)](http://i.imgur.com/zQxsEfg.jpg)

[![Screenshot2](http://i.imgur.com/XNxLRtKl.jpg)](http://i.imgur.com/XNxLRtK.jpg)

[![Screenshot2](http://i.imgur.com/VPH9ZpCl.png)](http://i.imgur.com/VPH9ZpC.jpg)


##Examples

- [Qorpse (2D Game)](https://github.com/flipcoder/qorpse)
- [C++ Usage Example](https://github.com/flipcoder/qor/blob/master/src/DemoState.cpp)
- [Python Usage Example](https://github.com/flipcoder/qor/blob/master/bin/mods/scriptdemo/__init__.py)


##Credits

All engine source, (and that of prior versions), is my own work.
The siege-tools build system and header-only kit library is also my own original work.
The other libraries used, such as boost, glm, SDL, etc. are all property of their respective owners.
Check the licenses of these libraries for details.

Graphics resources used in demos, screenshots, and videos, were contributed by:
[Mark "Alfred" McDaniel](http://github.com/alfredanonymous)

There are a few minor exceptions, such as the Qor logo, which I did myself.

