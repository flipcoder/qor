#ifndef _IPIPELINE_H_Z1WEQDGY
#define _IPIPELINE_H_Z1WEQDGY

#include <glm/glm.hpp>
#include <memory>
#include "Common.h"
#include "Graphics.h"
#include "Shader.h"
#include "ITexture.h"
class Pass;
class Node;

class IPipeline
{
    public:
        virtual ~IPipeline() {}
        virtual void render() {}

        virtual void matrix(Pass* pass, const glm::mat4* m) = 0;
        virtual void texture(
            unsigned id,
            unsigned slot
        ) = 0;

        enum Attribute {
            NONE=0,
            VERTEX,
            //INDEX,
            WRAP, // UV
            NORMAL,
            MAX
        };

        /*
         * Returns a specific attribute location based
         */
        virtual unsigned int layout(Attribute attr) const {
            return (unsigned int)(attr-1);
        }

        virtual void enable_layout(Attribute attr) {
            glEnableVertexAttribArray(layout(attr));
        }
        virtual void disable_layout(Attribute attr) {
            glDisableVertexAttribArray(layout(attr));
        }

        virtual Color bg_color() const { return Color::black(); }
        virtual void bg_color(const Color& c) {}
        virtual void ortho(bool b) {}

        virtual std::shared_ptr<Node> camera() { return std::shared_ptr<Node>(); }
        virtual void camera(const std::shared_ptr<Node>& n) {}

        virtual void shader(std::shared_ptr<Program> p) = 0;
        virtual void shader(std::nullptr_t) = 0;
        virtual std::shared_ptr<Program> shader(unsigned slot) const = 0;
        
    private:
};

#endif

