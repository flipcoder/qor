#include "Canvas.h"
#include "Mesh.h"
#include "GLTask.h"
#include "kit/log/log.h"
#include "kit/log/errors.h"
using namespace std;
using namespace glm;

Canvas :: Canvas(unsigned w, unsigned h):
    m_Surface(Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, w, h)),
    m_Context(Cairo::Context::create(m_Surface)),
    //m_PangoContext(Glib::wrap(pango_x_get_context())),
    m_Layout(Pango::Layout::create(m_Context))
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
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        {
            auto err = glGetError();
            if(err != GL_NO_ERROR)
                ERRORf(GENERAL, "OpenGL Error: %s", err);
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, 4,
            m_Texture->size().x, m_Texture->size().y,
            0, GL_BGRA, GL_UNSIGNED_BYTE, m_Surface->get_data());
        
        {
            auto err = glGetError();
            if(err != GL_NO_ERROR)
                ERRORf(GENERAL, "OpenGL Error: %s", err);
        }


    GL_TASK_END()

    add(make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(vec2(1.0f*w, 1.0f*h))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(
                glm::vec2(1.0f,1.0f), glm::vec2()
            ))
        },
        make_shared<MeshMaterial>(m_Texture)
    ));

}

Canvas :: ~Canvas()
{
    
}

void Canvas :: render_self(Pass* pass) const
{
    if(!m_Texture || !*m_Texture)
        return;
    
    //GL_TASK_START()
        //if(m_bDirty)
        //{
            //if(pass->flags() & Pass::BASE)
            //{
                m_Texture->bind(pass);
                glTexImage2D(GL_TEXTURE_2D, 0, 4,
                    m_Texture->size().x, m_Texture->size().y,
                    0, GL_BGRA, GL_UNSIGNED_BYTE, m_Surface->get_data());

                //glTexSubImage2D(GL_TEXTURE_2D, 0, 4,
                //    m_Texture->size().x, m_Texture->size().y,
                //    0, GL_BGRA, GL_UNSIGNED_BYTE, m_Surface->get_data());
                //m_bDirty = false;
                {
                    auto err = glGetError();
                    if(err != GL_NO_ERROR)
                        ERRORf(GENERAL, "OpenGL Error: %s", err);
                }
            //}

        //}
    
        // TODO: render quad

    //GL_TASK_END()
}

