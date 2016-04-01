#include <boost/scope_exit.hpp>
#include "PlayerInterface3D.h"
#include "Sprite.h"
#include <glm/gtx/vector_angle.hpp>
using namespace std;
using namespace glm;

PlayerInterface3D :: PlayerInterface3D(
    const shared_ptr<Controller>& input,
    const shared_ptr<Node>& look_node,
    const shared_ptr<Node>& node,
    const shared_ptr<Meta>& profile,
    std::function<bool()> lock_if
    //const shared_ptr<ResourceCache<Texture>>& textures
):
    NodeInterface(input, node),
    m_wpLookNode(look_node),
    m_Speed(6.0),
    m_Sens(1.0f),
    m_LockIf(lock_if)
{
    TRY(m_Sens = safe_ptr(profile)->at<double>("sensitivity", 1.0));
}

void PlayerInterface3D :: event()
{
    auto n = node();
    auto in = controller();

    if(m_LockIf && m_LockIf())
        return;
    
    m_Move = vec3();
    
    if(in->button("left"))
        m_Move += vec3(-1.0f, 0.0f, 0.0f);
    if(in->button("right"))
        m_Move += vec3(1.0f, 0.0f, 0.0f);
    if(in->button("forward"))
        m_Move += vec3(0.0f, 0.0f, -1.0f);
    if(in->button("back"))
        m_Move += vec3(0.0f, 0.0f, 1.0f);

    if(in->button("jump").pressed_now()) {
        if(m_bFly) {
            m_Move += vec3(0.0f, 1.0f, 0.0f);
        } else {
            m_cbJump();
        }
    }
    if(in->button("crouch")) {
        if(m_bFly){
            m_Move += vec3(0.0f, -1.0f, 0.0f);
        }else{
        }
    }

    //if(in->button(3))
    //    m_Move = vec3(-1.0f, 0.0f, 0.0f);

    if(length(m_Move) > 0.1f)
        m_Move = normalize(m_Move) * m_Speed;
    if(in->button(in->button_id("sprint"))) {
        m_bSprint = true;
        m_Move *= 1.5f;
    }else{
        m_bSprint = false;
    }
}

void PlayerInterface3D :: logic(Freq::Time t)
{
    if(m_LockIf && m_LockIf())
        return;
    
    auto n = node();
    auto in = controller();
    auto m = in->input()->mouse_rel();
    auto ln = look_node();

    const float sens = 0.001f * m_Sens;

    auto p = n->position();
    
    n->position(glm::vec3());
    ln->rotate(m.x * sens, glm::vec3(0.0f, -1.0f, 0.0f), Space::PARENT);
    n->position(p);
    
    if(not m_bLockPitch)
    {
        //float delta;
        //if(in->input()->key(SDLK_i))
        //{
        //    delta = 0.5f * t.s();
        //} else if(in->input()->key(SDLK_k)) {
        //    delta = -0.5f * t.s();
        //}
        auto delta = m.y * sens;
        const float maxpitch = 0.25f - 0.001f;
        delta = std::min(std::max(delta, -maxpitch-m_Pitch), maxpitch-m_Pitch);
        m_Pitch += delta;
        ln->rotate(delta, glm::vec3(-1.0f, 0.0f, 0.0f));
        //LOGf("pitch: %s", m_Pitch);
    }

    auto mag = glm::length(m_Move);
    if(mag > 0.1f) {
        auto vert_movement = m_Move.y;
        auto move = vec3(m_Move.x, 0.0f, m_Move.z);
        auto xz_mag = glm::length(move);
        if(xz_mag > 0.1) {
            move = glm::normalize(move) * mag;
            if(!m_bFly) {
                move = ln->orient_to_world(move);
                move.y = 0.0f;
                move = ln->orient_from_world(move);
                move = glm::normalize(move) * xz_mag;
            }
            move = n->orient_from_world(ln->orient_to_world(move));
            //n->move(move * t.s(), Space::LOCAL);
            //n->velocity(move);
        }
        //n->move(vec3(0.0f, vert_movement, 0.0f) * t.s());
        if(!m_bFly) {
            auto v = n->velocity();
            move.y = v.y;
        }
        n->velocity(move);
    }else{
        auto v = n->velocity();
        if(!m_bFly)
            n->velocity(vec3(0.0f, v.y, 0.0f));
        else
            n->velocity(vec3(0.0f, m_Move.y, 0.0f));
    }
}

