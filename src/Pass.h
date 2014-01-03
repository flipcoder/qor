#ifndef _PASS_H_2JQNSTEX
#define _PASS_H_2JQNSTEX

#include <cstddef>
#include "IPartitioner.h"
#include "IPipeline.h"
#include "kit/math/matrixstack.h"
#include "GLTask.h"

/*
 * Encapsulates a render pass
 *     To be passed through render functions to keep track of partitioning,
 *     flags, and other data needed by objects when rendering
 *     Used by IRenderable, ITexture
 */
class Pass
{
    public:

        enum eFlag
        {
            RECURSIVE = kit::bit(0),
            BASE = kit::bit(1),
            LIGHT = kit::bit(2)
        };

        Pass(IPartitioner* partitioner, IPipeline* pipeline, unsigned flags):
            m_pPartitioner(partitioner),
            m_pPipeline(pipeline),
            m_Flags(flags)
        {}

        IPartitioner* partitioner() { return m_pPartitioner; }
        IPipeline* pipeline() { return m_pPipeline; }
        bool recursive() const { return m_Flags & (unsigned)RECURSIVE; }
        unsigned flags() const { return m_Flags; }

        virtual void matrix(const glm::mat4* m) {
            assert(m_pPipeline);
            m_pPipeline->matrix(this, m);
        }
        MatrixStack* stack() { return &m_Stack; }

        virtual void texture(unsigned id, unsigned slot = 0) {
            m_pPipeline->texture(id, slot);
        }

        void vertex_array(unsigned int id) {
            //if(id!=m_VertexArrayID){
                GL_TASK_START()
                    glBindVertexArray(id);
                GL_TASK_END()
            //}
        }
        
        void vertex_buffer(unsigned int id) {
            //if(id!=m_VertexBufferID) {
                GL_TASK_START()
                    glBindBuffer(GL_ARRAY_BUFFER, id);
                GL_TASK_END()
            //}
        }
        
        void element_buffer(unsigned int id) {
            //if(id!=m_VertexBufferID) {
                GL_TASK_START()
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
                GL_TASK_END()
            //}
        }

        void enable_layout(IPipeline::Attribute attr) {
            m_pPipeline->enable_layout(attr);
        }

        void disable_layout(IPipeline::Attribute attr) {
            m_pPipeline->disable_layout(attr);
        }

        void shader(const std::shared_ptr<Program>& p) {
            m_pPipeline->shader(p);
        }
        void shader(std::nullptr_t n) {
            m_pPipeline->shader(n);
        }

        //virtual void push_layout() {
        //    m_pPipeline->push_layout(this);
        
    private:

        unsigned m_VertexArrayID = 0;
        unsigned m_VertexBufferID = 0;

        IPipeline* m_pPipeline = nullptr;
        IPartitioner* m_pPartitioner = nullptr;
        unsigned m_Flags = 0;

        MatrixStack m_Stack;
};

#endif

