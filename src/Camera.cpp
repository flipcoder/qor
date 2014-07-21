#include "Camera.h"
#include "Window.h"
#include <glm/glm.hpp>
using namespace std;

Camera :: Camera(const std::string& fn, IFactory* factory, ICache* cache)
{
    
}

bool Camera :: in_frustum(const Box& box) const
{
    if(m_bOrtho)
    {
        if(not box.quick_valid())
            return false;
        if(box.quick_full())
            return true;
        // TODO: this needs to be in world space
        return (Box(
            glm::vec3(0.0f, 0.0f, -100.0f),
            glm::vec3(m_Size.x, m_Size.y, 100.0f)
        ).collision(box));
    }
    return true;
}

bool Camera :: in_frustum(glm::vec3 point) const
{
    if(m_bOrtho)
        return (Box(
            glm::vec3(0.0f, 0.0f, -100.0f),
            glm::vec3(m_Size.x, m_Size.y, 100.0f)
        ).collision(point));
    return true;
}

const glm::mat4& Camera :: projection() const
{
    return m_ProjectionMatrix;
}

const glm::mat4& Camera :: view() const
{
    m_ViewMatrix = glm::inverse(*matrix_c(Space::WORLD));
    return m_ViewMatrix;
}

void Camera :: window(Window* window)
{
    if(not window)
    {
        m_WindowResize.disconnect();
        return;
    }
    
    m_WindowResize = window->on_resize.connect([this, window](glm::ivec2 w){
        m_Size = w;
        recalculate_projection();
    });
    
    m_Size = window->size();
    recalculate_projection();
}

void Camera :: recalculate_projection()
{
    if(m_bOrtho)
    {
        m_ProjectionMatrix = glm::ortho(
            0.0f,
            static_cast<float>(m_Size.x),
            m_bBottomOrigin ? 0.0f : static_cast<float>(m_Size.y),
            m_bBottomOrigin ? static_cast<float>(m_Size.y) : 0.0f,
            -100.0f,
            100.0f
        );
    }
    else
    {
        float aspect_ratio = (1.0f * m_Size.x) /
            std::max(1.0f, (1.0f * m_Size.y));
        m_ProjectionMatrix = glm::perspective(
            m_FOV,
            aspect_ratio,
            0.01f,
            1000.0f
        );
    }
}

void Camera :: ortho(bool origin_bottom)
{
    m_bOrtho = true;
    m_bBottomOrigin = origin_bottom;
    recalculate_projection();
}

void Camera :: perspective(float fov)
{
    m_bOrtho = false;
    m_FOV = fov;
    recalculate_projection();
}

