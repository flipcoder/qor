#ifndef _VERTEXBUFFER_H_XI1UX68Y
#define _VERTEXBUFFER_H_XI1UX68Y

#include <vector>

/*
 * Wraps OpenGL's Vertex Buffer Object (VBO) which can send vertex attribute
 * data to the GPU for use in shaders
 */
template<class T, unsigned Count>
class VertexBuffer
{
    public:
        VertexBuffer() = default;
        virtual ~VertexBuffer() { clear_cache(); }


        float* array() { return &m_Buffer[0]; }
        size_t size() { return m_Buffer.size(); }

        void clear_cache()
        {
            if(m_ID)
            {
                glDeleteBuffers(1, &m_ID);
                m_ID = 0;
            }
            m_bNeedsCache = false;
        }

        void cache() const
        {
            if(m_bNeedsCache)
            {
                const_cast<VertexBuffer*>(this)->clear_cache();
                glGenBuffers(1, &m_ID);
                glBindBuffer(GL_ARRAY_BUFFER, m_ID);
                glBufferData(
                    GL_ARRAY_BUFFER,
                    m_Size * Count * sizeof(float),
                    &m_Vertices[0],
                    GL_STATIC_DRAW
                );
                m_bNeedsCache = false;
            }
        }

    private:
        std::vector<T> m_Buffer;
        mutable unsigned int m_ID = 0;
        bool m_bNeedsCache = true;
};

#endif

