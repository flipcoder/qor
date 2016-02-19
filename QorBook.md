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
node->position(vec3(1.0f, 0.0f, 0.0f)); // absolute position setting
node->move(vec3(t.s(), 0.0f, 0.0f)); // relative, manually call every frame of motion
node->velocity(vec3(1.0f, 0.0f, 0.0f)); // relative, automatically applied every frame until changed
```

Python:
```
node.set_position([1.0, 0.0, 0.0]) # changing position
node.move([t, 0.0, 0.0]) # moving, manually call every frame of motion
node.set_velocity([1.0, 0.0, 0.0]) # moving, automatically applied every frame until changed
```

The following sections will look into specific types of Nodes.

If we wish to get the position of a node, we simply call *position* without parameters.
However, an optional parameter may be added to indicate the space in which to derive the position.
To get the position relative to the parent node, simply use *position* or pass *Space::PARENT*.
To get the position in world space, pass in *Space::WORLD*.

To introduce signals, we'll look at the Node's actuation callback, called *on_tick*.
This signal allows for your functions to be called along with the Node's logic.
If you want a Node to rotate every frame, you may consider moving it in your *logic(t)*, however, if this event
only occurs as long as the Node exists, it may be better to attach it as a signal to the Node itself.

Let's rotate the node 1 rotation per second.

C++:
```
Node* n = node.get(); // assuming node is a smart ptr, we want a raw one.
node->on_tick([n](Freq::Time t){
    n->rotate(vec3(t.s(), 0.0f, 1.0f, 0.0f));
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

#### OBJ

Load in an mesh from an OBJ file, and spawn it:

```
auto mesh = m_pQor->make<Mesh>("model.obj");
m_pRoot->add(mesh);
```

Python
```
mesh = qor.Mesh("model.obj")
qor.spawn(mesh)
```

#### Scenes

#### Manually

We can provide the parameters manually for meshes.  This is good for creating
primitives.  Sometimes all you need is a simple shape.

The first parameter of Mesh is the geometry to use, *Prefab::quad()* provides the
geometry of a basic quad.  The 2nd is a vector of modifiers, these are vertex
attributes such as normals, tangents, colors, etc.  We'll use Wrap, which is Qor's
name for UV coordinates.  The prefab function for this is *Prefab::quad_wrap()*.
The next parameter is the material, which we'll load from an image.

C++:
```
auto mesh = m_pQor->make<Mesh>(
    make_shared<MeshGeometry>(Prefab::quad()),
    vector<shared_ptr<IMeshModifier>>{
        make_shared<Wrap>(Prefab::quad_wrap())
    },
    make_shared<MeshMaterial>(
        m_pQor->resources()->cache_cast<ITexture>("myimage.png")
    )
);
m_pRoot->add(mesh);
```

#### Sprites

### Physics

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

Python:
```
music.loop(False)
music.ambient(True)
music.music(False)
```

## Resources

We have worked with resources above without even knowing it yet.  Anything that
can be potentially reused, but not cheaply copied, is backed by a resource.
You usually do not have to deal with resources yourself.

All textures, sounds, and mesh data is considered a resource.  Nodes are different
than resources, but usually use resources internally.  For example, Sound is a node,
but uses the sources, buffers, and streams as resources internally.

The importance of this is to know how cheaply you can reload from the same file
multiple times, without keeping around your own set of resource pointers.

You can access the resource cache through your engine pointer, usually m_pQor or m_pEngine.

At any point during your program you wish to clear out unused resources, simply call:

C++:
```
m_pQor->resources().optimize();
```

Python:
```
qor.optimize()
```

## Timers and Animation

### Timelines

Qor has a number of nifty timer features.  The first type of timing system
we'll look at is called *Timeline*s.

A timeline controls how fast/slow things that are tied to it advance.

Let's assume we wish to gradually slow down a set of objects in the game for a slow-motion
sequence, then gradually resume back to full speed.  But in the middle of all of this,
the user pauses the game to access a menu.  The menu items need to still be functional
and running on the normal speed, yet all the game elements (including those in slow motion and not)
need to be paused.  This might be tricky if it weren't for the concept of timelines.

### Alarms

So, you want to know when a certain amount of time has passed?  One way to do this
would be to have multiple timelines and keep track of elapsed time.  But the easier
way is to use another feature called *Alarm*s.  Alarms are just what they sound like.
They connect to a timeline, and can elapse after a certain amount of time has passed.
They can be stopped or delayed.  You can easily get the amount of time passed as
percentage or in a value.

### Animation

Inside of our timer system is an animation system w/ easing support.
The included FPS demo has some good examples of how useful this system can be.
The sway of the gun during walking, sprinting, shooting, zooming is all done
with animation scripting and is not stored in the model. Every aspect of movement
is its own animation.  It is then summed together to achieve the final effect.

So firstly, let's use an animation to bob a node back and forth smoothly.

First we'll need two keyframes, those are positions in which the node is at rest.

We'll separate the keyframes by 1 second and we'll interpolate.

## Input

Qor's input system is accessible from the engine.

C++:
```
Input* input = m_pQor->input();
```

The Input system can access all player controllers.  Controllers are not
necessarily physical gamepads, but are a set of bindings specific to a player's
profile.  Let's look at a possible json configuration for a player.

```
{
    "name": "Player",
    "sensitivity": 0.5,
    "input": {
        "binds": {
            "e": [
                "up",
                "forward"
            ],
            "up": "up",
            "s": "left",
            "d": [
                "down",
                "back"
            ],
            "f": "right",
            "mouse left": "fire",
            "w": "use",
            "mouse right": "zoom",
            "k": "shoot",
            "j": "strafe",
            "left shift": "sprint",
            "i": "action",
            "t": "reload",
            "y": "chat",
            "return": "select",
            "space": [
                "jump",
                "select"
            ],
            "a": "crouch"
        }
    }
}
```

The action names are all up to you and the needs of your application.

### Basic Input

So now that we know a little bit about controllers, let's return to our
application and make something happen when a key is pressed.

C++:
```
auto controller = m_pQor->session()->profile(0)->controller();

if(controller->button("fire"))
{
    // boom
}
```

But wait, maybe we only want to fire when the button is initially pressed,
and ignore it until it is repressed again. Let's do this instead:

```
if(m_pController->button("fire").pressed_now())
{
    // boom
}
```

But what about those guys with faster fingers and turbo controllers?  If you
considered using an alarm here, you'd be correct.

```
// somewhere else
Freq::Alarm shoot_alarm = Freq::Alarm(m_pQor->timer()->timeline());

if(m_pController->button("fire") && shoot_alarm.elapsed())
{
    // boom
    shoot_alarm.set(Freq::Time::seconds(1.0));
}
```

This allows us to shoot every second.  You'll also notice we removed *pressed_now()*,
since we now want to allow the player to hold down the fire button.

### Text Input

For text input, see *Text Input* under *Text and 2D Primitives*.

### Character Controllers

## Physics

Qor uses Bullet Physics.  It generates the physics objects for you in most
cases, but you are free to access Bullet for additional features.

### Character Controller

### Mesh Collision

### Ray Testing

To do a ray test, simply call *Physics::first_hit* with the ray start and
end as parameters. This method returns a tuple containing the node that was hit
(if any), the position of the intersection, and the normal of the surface at
the point of intersection.

C++:
```
tuple<Node*,vec3,vec3> r = m_pPhysics->first_hit(start, end);
Node* contact = std::get<0>(r);
if(contact)
{
    // hit!
}
```

### Range Query

## Text and 2D Primitives

### Writing Text

### Text Input

## Advanced Nodes

### Hooking

So, you have a bunch of nodes.  What's next?  Well, say you wanted to find all
the nodes of a certain name pattern, of a type, or every one that matches a condition.
This is what hooking is for.  You can search for, and filter nodes using this feature.

First, let's hook by name.

C++:
```
vector<Node*> nodes = m_pRoot->hook("MyNode");
```

Python:
```
node = root.hook("MyNode")
```

That's it.  We now have a list of nodes named MyNode under the root node.
This is a recursive operation, so be careful.

We can also hook by type.

Let's try hooking all the lights and dimming them to 25% intensity in our scene.

C++:
```
vector<Light*> lights = node->hook_type<Light>();
for(auto light: lights)
    light->diffuse(lights.diffuse() * 0.25f);
```

Python:
```
lights = node.hook(type="light")
for light in lights:
    light.diffuse(light.diffuse() * 0.25)
```

Or by tag.  Specify multiple tags one after another without spaces.

C++:
```
vector<Node*> nodes = node->hook("#explosive");
for(auto node: nodes)
{
    // boom
}
```

Python:
```
nodes = node.hook("#explosive")
for node in nodes:
    # boom
```

### Each

Nodes have recursive iteration built in.  You can apply an operation to all the
nodes attached to a given node, by using *Node::each(callback)*.
To make the operation recursive, pass in the additional flag parameter
*Each::RECURSIVE*.

C++:
```
// every child
node->each([](Node* n){
    // do something with `n` here
});

// every descendant
node->each([](Node* n){
    // do something with `n` here
}, Each::RECURSIVE);
```

## Projects

Now that you know the basics, it's time to apply what we know and make something.

### Hello World

Let's make a project base that we can play around with.

We'll start by setting up creating HelloWorldState, and register that in our
main function.

### Basic Controls

### Fly around a 3D model

### Controlling a Player w/ Physics in 3D

