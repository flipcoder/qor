# QorBook

This document is incomplete.

## Setup

...

## Main

We must first initialize Qor in our main function.

- Create *Qor* engine instance and pass in our program arguments.
- Register all states (just 1 for now), and give it a name.
- Run the engine and pass in the name of the starting state.

```
int main(int argc, const char** argv)
{
    auto engine = kit::make_unique<Qor>(argc, argv);
    engine->states().register_class<MyState>("mystate");
    engine->run("mystate");
    return 0;
}

```

## States

### The Basics

A Qor *State* must override several methods.

The first method is the *logic* tick, responsible for updating the game every frame.
The value t indicates the amount of time passed since the last frame.
You can get this value in milliseconds as an integer using *t.ms()* or in seconds as a float using *t.s()*.

```
virtual void logic(Freq::Time t) override;
```

The next is the *render* tick, responsible for drawing (not updating) the necessary nodes in the given pass.
Usually this method will contain only one call, which will be to the Qor Pipeline's *render* method.
The purpose of this method is to tell the Qor render pipeline exactly what to draw, and what Camera's perspective
to draw it from.

```
virtual void render() override;
```

The next method is *preload*.  This is what happens in the background when the loading screen is shown.
Do most of the heavy work of loading and caching resources here.

```
virtual void preload() override;
```

The next method is *enter*.  This happens after the loading screen completes on the first frame of our state.

```
virtual void enter() override;
```

### Menus

### Transition States

## Nodes

The following resources should generally be declared and added in *preload*.
If the resource requires explicit state changes (such as the playing of a sound),
then that should _not_ be done in *preload()*, but in *enter* or *logic* instead,
since this *preload* events would happen during the loading screen rather than
while the state itself is running.

### Camera

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

