#include "Canvas.h"
#include "Mesh.h"
#include "GLTask.h"
#include "kit/math/common.h"
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
    if(not Headless::enabled())
    {
        int last_id;
        unsigned id;
        
        GL_TASK_START()
            
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_id);
            glGenTextures(1,&id);
            try{
                m_Texture = make_shared<Texture>(id); // take ownership
            }catch(...){
                glDeleteTextures(1,&id);
                throw;
            }
            m_Texture->size(w, h);
            
            glBindTexture(GL_TEXTURE_2D, id);
            BOOST_SCOPE_EXIT_ALL(last_id) {
                glBindTexture(GL_TEXTURE_2D, last_id);
            };
            
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
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

        auto m = make_shared<Mesh>(
            make_shared<MeshGeometry>(Prefab::quad(
                vec2(1.0f*w, 1.0f*h), vec2(0.0f)
            )),
            vector<shared_ptr<IMeshModifier>>{
                make_shared<Wrap>(Prefab::quad_wrap(
                    glm::vec2(1.0f,0.0f), glm::vec2(0.0f, 1.0f)
                ))
            },
            make_shared<MeshMaterial>(m_Texture)
        );
        add(m);

        // TODO: this could be on_move()
        // keep this box insync with base mesh, since this render_self can't be
        // culled called when child mesh is in frustum (or we miss texture updates)
        on_tick.connect([this, m](Freq::Time) {
            m_Box = m->box();
        });
    }
}

Canvas :: ~Canvas()
{
    
}

void Canvas :: logic_self(Freq::Time t)
{
    if(m_Texture)
        m_Texture->logic(t);
}

void Canvas :: render_self(Pass* pass) const
{
    if(!m_Texture || !*m_Texture)
        return;
    
    if(m_bDirty)
    {
        GL_TASK_START()
        
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
        GL_TASK_END()
        m_bDirty = false;
    }
}

void Canvas :: text(std::string text, Color c, glm::vec2 pos, Canvas::Align align)
{
    if(Headless::enabled())
        return;
    
    m_Context->set_source_rgba(c.r(), c.g(), c.b(), c.a());
    
    if(align == LEFT)
        m_Context->move_to(pos.x, pos.y);
    else
    {
        Cairo::TextExtents extents;
        m_Context->get_text_extents(text, extents);
        if(align == CENTER)
            m_Context->move_to(
                pos.x - (extents.width/2 + extents.x_bearing),
                pos.y
            );
        else if(align == RIGHT)
            m_Context->move_to(
                pos.x - (extents.width + extents.x_bearing),
                pos.y
            );
    }
    m_Context->show_text(text);
    m_bDirty = true;
}

void Canvas :: clear(Color c)
{
    if(Headless::enabled())
        return;

    assert(m_Context);
    m_Context->save();
    m_Context->set_operator(Cairo::OPERATOR_SOURCE);
    m_Context->set_source_rgba(c.r(), c.g(), c.b(), c.a());
    m_Context->paint();
    m_Context->restore();
}

void Canvas :: font(std::string fn, int sz)
{
    if(Headless::enabled())
        return;

    m_Context->select_font_face(
        fn,
        Cairo::FONT_SLANT_NORMAL,
        Cairo::FONT_WEIGHT_NORMAL
    );
    m_Context->set_font_size(sz);
}

void Canvas :: rectangle(
    float x, float y, float width, float height, float radius
){
    if(Headless::enabled())
        return;

    if(radius > K_EPSILON)
    {
        float degrees = K_PI / 180.0;

        m_Context->begin_new_sub_path();
        m_Context->arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
        m_Context->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
        m_Context->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
        m_Context->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
        m_Context->close_path();
    }else{
        m_Context->rectangle(x,y,width,height);
    }
}

void Canvas :: color(Color c)
{
    if(Headless::enabled())
        return;

    m_Context->set_source_rgba(c.r(),c.g(),c.b(),c.a());
}

