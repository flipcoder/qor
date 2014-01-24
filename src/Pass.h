#ifndef _PASS_H_2JQNSTEX
#define _PASS_H_2JQNSTEX

#include <cstddef>
//#include "IPartitioner.h"
//#include "Pipeline.h"
#include "kit/math/matrixstack.h"
#include "GLTask.h"
#include "Graphics.h"
#include "Shader.h"

/*
 * Encapsulates a render pass
 *     To be passed through render functions to keep track of partitioning,
 *     flags, and other data needed by objects when rendering
 *     Used by IRenderable, ITexture
 */
class Node;
class Light;
class Pipeline;
class IPartitioner;
class Pass
{
    public:
        
        enum eFlag
        {
            RECURSIVE = kit::bit(0),
            BASE = kit::bit(1),
            LIGHT = kit::bit(2)
        };

        Pass(IPartitioner* partitioner, Pipeline* pipeline, unsigned flags);

        IPartitioner* partitioner() { return m_pPartitioner; }
        Pipeline* pipeline() { return m_pPipeline; }
        bool recursive() const { return m_Flags & (unsigned)RECURSIVE; }

        virtual void matrix(const glm::mat4* m);
        MatrixStack* stack() { return &m_Stack; }

        virtual void texture(unsigned id, unsigned slot = 0);

        void vertex_array(unsigned int id);
        void vertex_buffer(unsigned int id);
        void element_buffer(unsigned int id);

        void shader(const std::shared_ptr<Program>& p);
        void shader(std::nullptr_t n);
        std::shared_ptr<Program> shader(unsigned slot) const;
        PassType type() const;

        unsigned layout(unsigned attrs);
        void texture_slots(unsigned slot_flags);
        unsigned attribute_id(unsigned id);

        unsigned flags() const {return m_Flags;}
        void flags(unsigned f) {m_Flags = f;}
        
    private:

        unsigned m_VertexArrayID = 0;
        unsigned m_VertexBufferID = 0;

        Pipeline* m_pPipeline = nullptr;
        IPartitioner* m_pPartitioner = nullptr;
        unsigned m_Flags = 0;

        MatrixStack m_Stack;
};

#endif

