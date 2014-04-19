#include "Canvas.h"
#include "GLTask.h"
#include "kit/log/log.h"
#include "kit/log/errors.h"
using namespace Cairo;
using namespace std;

Canvas :: Canvas(unsigned w, unsigned h):
    m_Surface(ImageSurface::create(FORMAT_ARGB32, w, h)),
    m_Context(Context::create(m_Surface))
{
    int last_id;
    unsigned id;
    
    GL_TASK_START()
        
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_id);
        glGenTextures(1,&id);
        try{
            m_Texture = make_shared<Texture>(id); // take ownership
        }catch(...){
            //GL_TASK_ASYNC_START()
                glDeleteTextures(1,&id);
            //GL_TASK_ASYNC_END()
            throw;
        }
        m_Texture->size(w, h);
        
        glBindTexture(GL_TEXTURE_2D, id);
        BOOST_SCOPE_EXIT_ALL(last_id) {
            glBindTexture(GL_TEXTURE_2D, last_id);
        };
        
        {
            auto err = glGetError();
            if(err != GL_NO_ERROR)
                ERRORf(GENERAL, "OpenGL Error: %s", err);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, 4,
            m_Texture->size().x, m_Texture->size().y,
            0, GL_BGRA, GL_UNSIGNED_BYTE, m_Surface->get_data());

    GL_TASK_END()
}

Canvas :: ~Canvas()
{
    
}

void Canvas :: render(Pass* pass) const
{
    if(!m_Texture || !*m_Texture)
        return;
    
    GL_TASK_START()
        if(m_bDirty)
        {
            m_Texture->bind(pass);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 4,
                m_Texture->size().x, m_Texture->size().y,
                0, GL_BGRA, GL_UNSIGNED_BYTE, m_Surface->get_data());
            m_bDirty = false;
        }
    
        // TODO: render quad

    GL_TASK_END()
}

