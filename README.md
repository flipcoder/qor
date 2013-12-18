Qor
===

Copyright (c) 2013 Grady O'Connell

2D/3D OpenGL Game Engine (C++11 w/ Python scripting)

2013 rewrite of my old game engine, found here:
https://github.com/flipcoder/qor-old

##Features

- Cross-platform
- Multi-threaded (more improvements here soon)
- Multi-layer tilemap system for 2D games
- OpenGL ES-compatible shader-based pipeline
    - Normal GL stuff like antialiasing, anisotropic filtering
    - Forward rendering w/ multipass lighting (rewriting still from old engine)
    - Partitioner support
- Raspberry Pi support planned
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
- Scene Graph
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
    - Basic instancing

More features will be listed as I add them...

##Screenshots

(placeholder)

