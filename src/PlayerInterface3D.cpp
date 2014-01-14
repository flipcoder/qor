#include <boost/scope_exit.hpp>
#include "PlayerInterface3D.h"
#include "Sprite.h"
#include <glm/gtx/vector_angle.hpp>
using namespace std;
using namespace glm;

PlayerInterface3D :: PlayerInterface3D(
    const shared_ptr<Controller>& input,
    const shared_ptr<Node>& node
    //const shared_ptr<ResourceCache<Texture>>& textures
):
    NodeInterface(input, node),
    m_Speed(5.0)
{
}

void PlayerInterface3D :: event()
{
    auto n = node();
    auto in = controller();

    m_Move = vec3();
    
    if(in->button(in->button_id("left")))
        m_Move += vec3(-1.0f, 0.0f, 0.0f);
    if(in->button(in->button_id("right")))
        m_Move += vec3(1.0f, 0.0f, 0.0f);
    if(in->button(in->button_id("forward")))
        m_Move += vec3(0.0f, 0.0f, -1.0f);
    if(in->button(in->button_id("back")))
        m_Move += vec3(0.0f, 0.0f, 1.0f);

    if(in->button(in->button_id("jump")))
        m_Move += vec3(0.0f, 1.0f, 0.0f);
    if(in->button(in->button_id("crouch")))
        m_Move += vec3(0.0f, -1.0f, 0.0f);

    //if(in->button(3))
    //    m_Move = vec3(-1.0f, 0.0f, 0.0f);

    if(length(m_Move) > 0.1f)
        m_Move = normalize(m_Move) * m_Speed;
    if(in->button(in->button_id("sprint")))
        m_Move *= 2.0f;
}

void PlayerInterface3D :: logic(Freq::Time t)
{
    auto n = node();
    auto in = controller();
    auto m = in->input()->mouse_rel();

    float mouse_sens = 0.001f;

    auto p = n->position();
    n->position(glm::vec3());
    n->rotate(m.x * mouse_sens, glm::vec3(0.0f, -1.0f, 0.0f), Space::PARENT);
    n->position(p);
    
    n->rotate(m.y * mouse_sens, glm::vec3(-1.0f, 0.0f, 0.0f));

    auto mag = glm::length(m_Move);
    if(mag > 0.1f) {
        auto vert_movement = m_Move.y;
        auto move = vec3(m_Move.x, 0.0f, m_Move.z);
        auto xz_mag = glm::length(move);
        if(xz_mag > 0.1) {
            
            move = glm::normalize(move) * mag;
            if(!m_bFly) {
                auto old_y = n->position().y;
                n->move(move * t.s(), Space::LOCAL);
                n->position(vec3(n->position().x, old_y, n->position().z));
            }
            else
                n->move(move * t.s(), Space::LOCAL);
        }
        n->move(vec3(0.0f, vert_movement, 0.0f) * t.s());
    }
}

