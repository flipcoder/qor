#include "GUI.h"
#include "Window.h"
#include "kit/freq/freq.h"

GUI :: GUI(Freq* timer, Window* window):
    m_pTimer(timer),
    m_pWindow(window)
{
    Rocket::Core::SetSystemInterface((Rocket::Core::SystemInterface*)this);
    m_pContext = Rocket::Core::CreateContext(
        "default",
        Rocket::Core::Vector2i(
            m_pWindow->size().x, m_pWindow->size().y
        )
    );
}

float GUI :: GetElapsedTime()
{
    return m_pTimer->get_seconds();
}

