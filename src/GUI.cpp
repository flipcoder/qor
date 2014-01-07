#include "GUI.h"
#include "Window.h"
#include "kit/freq/freq.h"

GUI :: GUI(Freq* timer, Window* window, Cache<Resource, std::string>* cache):
    m_pTimer(timer),
    m_pWindow(window),
    m_pCache(cache)
{
    Rocket::Core::SetSystemInterface((Rocket::Core::SystemInterface*)this);
    Rocket::Core::SetRenderInterface((Rocket::Core::RenderInterface*)this);
    
    m_pContext = Rocket::Core::CreateContext(
        "default",
        Rocket::Core::Vector2i(
            m_pWindow->size().x, m_pWindow->size().y
        )
    );

    //Rocket::Core::Initialise();
    //Rocket::Controls::Initialise();
}

GUI :: ~GUI()
{
    m_pContext->RemoveReference();
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
    return false;
}


void GUI :: ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
{
    
}

