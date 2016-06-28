Qor
===

[![Join the chat at https://gitter.im/flipcoder/qor](https://badges.gitter.im/flipcoder/qor.svg)](https://gitter.im/flipcoder/qor?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

2D/3D OpenGL Game Engine (C++11 w/ Python scripting)

Copyright (c) 2011-2016 Grady O'Connell
See CREDITS for a list of contributers.
    
Status: Hobby project, still tinkering

This is a rewrite of my [older game engine](https://github.com/flipcoder/qor-old).

##Instructions

- Initialize the kit submodule and checkout the master branch:

```
git submodule update --init --recursive
git submodule foreach --recursive git checkout master
```

- Get the libraries:

```
Vorbis, Boost, Jsoncpp, Pthread, Bullet, Sdl2, Raknet, Glew, Assimp, Freeimage, Glm, Freealut, Rapidxml, Openal, Cairomm, Ogg, Pangomm, Catch, Backward-cpp
```

- Build Qor with [premake4](http://industriousone.com/premake/download):

```
premake4 gmake
make -j`nproc`
```

- [siege-tools](https://github.com/flipcoder/siege-tools) is recommended.
- Read the documentation: [QorBook](https://github.com/flipcoder/qor/blob/master/QorBook.md)

##Features

- Cross-platform
- Multi-threaded (more improvements here soon)
- Multi-layer tilemap system for 2D games
    - Compatible with [Tiled](http://mapeditor.org) map editor
- OpenGL ES-compatible shader-based pipeline
    - Forward rendering w/ multipass lighting
    - Several shaders, including tangent-space bumpmapping
    - Partitioner/culling support
    - Automatic VBO baking of visible areas (10x framerate in many cases)
    - Mesh instance copy-on-write vertex and shader attribute modifications
    - Basic instancing
    - Render-to-Texture
    - Thread-safe OpenGL task handler
- C++ and Python support
- Python Console
- Shared resource management
    - Animating Sprites
    - Resource management and multiple path search
    - Resource caching
    - Basic create() function for all types
    - Json config for more complex resources
        - Config-based type-resolution and subresources
    - Composite/nested resource suport
    - User-defined resources
- 3D Scene graph
    - World matrix transform caching
    - Static mesh baking (collapse to world space and combine)
    - Tracking/following w/ animation support
    - Node factory supporting user classes
    - Node tags
    - Many 3D model formats supported
    - Blender scene exporter
- Reactive Subsystems
    - Communicate with/between nodes easily by sending/receiving data through named metaobject/json events
    - Each node has a lightweight state machine with signal support
    - User-defined events and states
- State stack
    - User-defined engine states
    - Async loading screens
- Input
    - Multiple controllers
    - Bindings specific to player profiles
    - Button/switch-associated events
    - Switch history for timer-based events and multiple fast press events
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
- Headless mode for dedicated servers and unit testing

More features will be listed as I add them...

##Partial/WIP Features 
- Level-of-detail
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

- [FRAG.EXE (First person shooter)](https://github.com/flipcoder/FRAG.EXE)
- [Micro Army (2d platformer)](https://github.com/flipcoder/microarmy)
- [Qorpse (2D shooter)](https://github.com/flipcoder/qorpse)
- [Skyroads-inspired racer](https://github.com/flipcoder/bouncingship)
- [C++ Usage Example](https://github.com/flipcoder/qor/blob/master/src/DemoState.cpp)
- [Python Usage Example](https://github.com/flipcoder/qor/blob/master/bin/mods/scriptdemo/__init__.py)

