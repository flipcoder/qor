#include "GUI.h"
#include "Window.h"
#include "kit/freq/freq.h"

GUI :: GUI(Freq* timer, Window* window, Cache<Resource, std::string>* cache):
    m_pTimer(timer),
    m_pWindow(window),
    m_pCache(cache)
{
}

GUI :: ~GUI()
{
    m_pContext->RemoveReference();
    Rocket::Core::Shutdown();
}

void GUI :: init()
{
    Rocket::Core::SetRenderInterface((Rocket::Core::RenderInterface*)this);
    Rocket::Core::SetSystemInterface((Rocket::Core::SystemInterface*)this);
    
    Rocket::Core::Initialise();
    m_pContext = Rocket::Core::CreateContext(
        "default",
        Rocket::Core::Vector2i(
            m_pWindow->size().x, m_pWindow->size().y
        )
    );
    if(!m_pContext)
        ERROR(LIBRARY, "libRocket");
}

float GUI :: GetElapsedTime()
{
    return m_pTimer->get_seconds();
}

void GUI :: RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
{
    
}

Rocket::Core::CompiledGeometryHandle GUI :: CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture)
{
    return (Rocket::Core::CompiledGeometryHandle) NULL;
}

void GUI :: RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
{
    
}

void GUI :: ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
{
    
}

void GUI :: EnableScissorRegion(bool enable)
{
    if(enable)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

void GUI :: SetScissorRegion(int x, int y, int width, int height)
{
    glScissor(x, y, width, height);
}

bool GUI :: LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
{
    return false;
}


bool GUI :: GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
{
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    if(!texture_id)
    {
        LOG("Could not generate textures\n");
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    texture_handle = (Rocket::Core::TextureHandle) texture_id;
    
    return true;
}


void GUI :: ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
{
    glDeleteTextures(1, (GLuint*) &texture_handle);
}

GUI::Font :: Font(const std::string& fn):
    Resource(fn)
{
    Rocket::Core::FontDatabase::LoadFontFace(Rocket::Core::String(fn.c_str()));
}

GUI::Font :: ~Font()
{
    
}

