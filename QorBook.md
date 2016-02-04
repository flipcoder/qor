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

```
virtual void enter() override;
```

Python:
```
def enter()
```

### Menus

### Transition States

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
To get the position in world space, pass in *Space::WORLD*.

### Camera

To see your scene, you'll need at least one camera.  Create a camera the same way you create a node.

C++:
```
auto camera = m_pQor->make<Camera>();
```

Python:
```
camera = qor.Camera()
```

#### Ortho 2D

#### Perspective

### Mesh

#### Sprites

#### OBJ

Load in an OBJ mesh:

```
auto mesh = m_pQor->make<Mesh>("model.obj");
```

Spawn it:

```
m_pRoot->add(mesh);
```

#### Scenes

#### Physics

#### Zones

### Light

Create a light:

```
auto light = m_pQor->make<Light>();
```

### Sound/Music

#### Sound

Load and play a sound once:

```
auto sound = m_pQor->make<Sound>("sound.wav");
m_pRoot->add(sound);
sound->play();
sound->detach_on_done(); // get rid of it when done
```

#### Music (stream)

Play some background music:

```
auto music = m_pQor->make<Sound>("music.ogg");
m_pRoot->add(music);
music->play();
```

