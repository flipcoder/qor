#ifndef _IRENDERABLE_H
#define _IRENDERABLE_H

class Pass;

class IRenderable{
    public:
        virtual ~IRenderable() {}
        virtual void render() const = 0;
};

class PipelineRenderable {
    public:
        virtual ~PipelineRenderable() {}
        virtual void render(Pass* pass) const = 0;
};

#endif

