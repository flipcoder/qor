# QorBook

This document is incomplete.

## Setup

...

## Main

We must first initialize Qor in our main function.

- Create *Qor* engine instance and pass in our program arguments.
- Register all states (just 1 for now), and give it a name.
- Run the engine and pass in the name of the starting state.

C++:
```
int main(int argc, const char** argv)
{
    auto engine = kit::make_unique<Qor>(argc, argv);
    engine->states().register_class<MyState>("mystate");
    engine->run("mystate");
    return 0;
}

```

This step is unnecessary for python users.

## States

### The Basics

A Qor *State* must override several methods.

The first method is the *logic* tick, responsible for updating the game every frame.
The value t indicates the amount of time passed since the last frame.
You can get this value in milliseconds as an integer using *t.ms()* or in seconds as a float using *t.s()*.

C++:
```
virtual void logic(Freq::Time t) override;
```

Python:
```
def logic(t)
```

The next is the *render* tick, responsible for drawing (not updating) the necessary nodes in the given pass.
Usually this method will contain only one call, which will be to the Qor Pipeline's *render* method.
The purpose of this method is to tell the Qor render pipeline exactly what to draw, and what Camera's perspective
to draw it from.

C++:
```
virtual void render() override;
```

Python:
```
def render()
```

The next method is *preload*.  This is what happens in the background when the loading screen is shown.
Do most of the heavy work of loading and caching resources here.

C++:
```
virtual void preload() override;
```

Python:
```
def preload()
```

The next method is *enter*.  This happens after the loading screen completes on the first frame of our state.
This is specifically for things that we can't do during the loading screen, such as starting music, and altering
the graphics pipeline.

```
virtual void enter() override;
```

Python:
```
def enter()
```

## Nodes

### Basics

The following resources should generally be declared and added in *preload*.
If the resource requires explicit state changes (such as the playing of a sound),
then that should _not_ be done in *preload()*, but in *enter* or *logic* instead,
since this *preload* events would happen during the loading screen rather than
while the state itself is running.

To make an empty node, use the following code.

C++:
```
auto node = m_pQor->make<Node>();
```

Python:
```
node = qor.Node()
```

Nodes form the basis of everything in our scene and how it is all related to one another.
They have positions, orientations, names, properties, layers, and signals.

Nodes can have both children and parents.  If a parent moves, all the children move along with it.

To add a node as a parent of another node, use *add*.

C++:
```
auto a = m_pQor->make<Node>();
auto b = m_pQor->make<Node>();
a->add(b);
```
Python:
```
a = qor.Node()
b = qor.Node()
a.add(b)
```

Next we will change the position of a node.

To change the position of a node, pass a 3D vector to *position()* or for relative changes, use *move()*.

We use *t* in this example to indicate the time passed since last frame.  This will be available in your *logic(t)*.
If you want movement to be the same on every system, you need to multiply it by *t* and continously apply it every
frame that you want it to continue to move.
For basic movement, you may instead consider using velocity which applies automatically until changed, and will
automatically use the right *t* value for each frame, so you don't need to.

Here are some basic examples with explanation for each use case.

C++:
```
node->position(glm::vec3(1.0f, 0.0f, 0.0f)); // absolute position setting
node->move(glm::vec3(1.0f * t.s(), 0.0f, 0.0f)); // relative, manually call every frame of motion
node->velocity(glm::vec3(1.0f, 0.0f, 0.0f)); // relative, automatically applied every frame until changed
```

Python:
```
node.set_position([1.0, 0.0, 0.0]) # changing position
node.move([1.0 * t, 0.0, 0.0]) # moving, manually call every frame of motion
node.set_velocity([1.0, 0.0, 0.0]) # moving, automatically applied every frame until changed
```

The following sections will look into specific types of Nodes.

If we wish to get the position of a node, we simply call *position* without parameters.
However, an optional parameter may be added to indicate the space in which to derive the position.
To get the position relative to the parent node, simply use *position* or pass *Space::PARENT*.
To get the position in world space or parent space, pass in *Space::WORLD* or *Space::PARENT* respectively.

To introduce signals, we'll look at the Node's actuation callback, called *on_tick*.
This signal allows for your functions to be called along with the Node's logic.
If you want a Node to rotate every frame, you may consider moving it in your *logic(t)*, however, if this event
only occurs as long as the Node exists, it may be better to attach it as a signal to the Node itself.

Let's rotate the node 1 rotation per second.

C++:
```
Node* n = node.get(); // assuming node is a smart ptr, we want a raw one.
node->on_tick([n](Freq::Time t){
    n->rotate(glm::vec3(t.s(), 0.0f, 1.0f, 0.0f));
})
```

Python:
```
n = weakref.ref(node)
node.on_tick(lambda t:
    n.rotate(t, [0.0, 1.0, 0.0])
);
```

We pass *t*, which is the fraction of time passed since last frame in seconds,
as the fraction of the rotation we want to make each frame.  We are rotating
around the y axis.  Qor's rotate() function is a little different from other engines,
in that its units are in *turns*. If you're used to degrees, simply divide the parameter
you want by 360. (90 degrees is 90/360th of a turn, get it?)  If you want to use
radians, simply divide by 2 PI. I find turns to be more intuitive in most cases.

### Camera

To actually see your scene, you'll need at least one camera.  Create a camera the same way you create a node.

C++:
```
auto camera = m_pQor->make<Camera>();
```

Python:
```
camera = qor.Camera()
```

#### Ortho 2D

For 2D games, you'll want a 2D camera.  Use Camera's *ortho()* function to put the camera in 2D ortho mode.

For 3D games, you'll want to use *perspective(fov)*.

#### Perspective

### Mesh

And finally, in order to view what is occuring in your scene, your nodes need to be visible objects.
So instead of simply declaring an empty node, which is more of a placeholder, we will declare other types of nodes,
such as sprites, meshes, lights, sounds, etc.

#### Sprites

#### OBJ

Load in an OBJ mesh:

```
auto mesh = m_pQor->make<Mesh>("model.obj");
```

Spawn it by attaching to your root node:

C++:
```
m_pRoot->add(mesh);
```

Python
```
qor.spawn(mesh)
```

#### Scenes

#### Physics

#### Zones

### Light

Create a light, and set some values to it:

C++:
```
auto light = m_pQor->make<Light>();
light->diffuse(Color(1.0f, 1.0f, 1.0f)); // rgb
light->specular(Color(0.1f, 0.1f, 0.1f));
```

### Sound/Music

#### Sound

Load and play a sound once:

C++
```
auto sound = m_pQor->make<Sound>("sound.wav");
m_pRoot->add(sound);
sound->play();
sound->detach_on_done(); // get rid of it when done playing
```

Sounds, like other resources, are cached.  They are only loaded the first time
you use them and stay in memory until you explicit clean up unused resources.

Tip: Instead of the above, there is a oneliner function for playing a sound once in Qor. It is Sound::play.

Here's an example of it:
```
Sound::play(m_pCamera, "coin.wav", m_pQor->resources());
```

#### Music (stream)

Play some background music:

```
auto music = m_pQor->make<Sound>("music.ogg");
m_pRoot->add(music);
music->play();
```
When loaded, Ogg files are automatically set to stream and loop (after you call *play()*).
You can of course change this behavior by using a json file as the resource instead.

Instead of loading music.ogg, let's load music.json, and fill it with this data:

```
{
    "filename": "music.ogg",
    "loop": false, // don't loop
    "ambient": true, // hear sound wherever you are
    "music": false // sound is not considered music, but just sound fx
}
```

If you don't want to use an external file, you can set these properties manually as well.

C++:
```
music->loop(false);
music->ambient(true);
music->music(false);
```

## Resources

We have worked with resources above without even knowing it yet.  Anything that
can be potentially reused, but not cheaply copied, is backed by a resource.
You usually do not have to deal with resources yourself.

All textures, sounds, and mesh data is considered a resource.  Nodes are different
than resources, but usually use resources internally.  For example, Sound is a node,
but uses the sources, buffers, and streams as resources internally.

The importance of this is to know how cheaply you can reload from the same file
multiple times.

You can access the resource cache through your engine pointer, usually m_pQor or m_pEngine.

At any point during your program you wish to clear out unused resources, simply call:

C++:
```
m_pQor->resources().optimize();
```

Python
```
qor.optimize()
```

## Timers and Animation



## Creating the Game

Now that you know the basics, it's time to apply what we know and make something basic.

