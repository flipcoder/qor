#ifndef _IRENDERABLE_H
#define _IRENDERABLE_H

class Pass;

class IRenderable {
    public:
        virtual ~IRenderable() {}
        virtual void render() const = 0;
};

class IPipelineRenderable {
    public:
        virtual ~IPipelineRenderable() {}
        virtual void render(Pass* pass) const = 0;
};

#endif

