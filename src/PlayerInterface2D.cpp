#include <boost/scope_exit.hpp>
#include "PlayerInterface2D.h"
#include "Sprite.h"
#include <glm/gtx/vector_angle.hpp>
using namespace std;
using namespace glm;

PlayerInterface2D :: PlayerInterface2D(
    const shared_ptr<Controller>& input,
    const shared_ptr<Node>& node,
    Cache<IResource, std::string>* resources
):
    NodeInterface(input, node)
{
    lock_sprite();
    BOOST_SCOPE_EXIT(this_) {
        this_->unlock_sprite();
    } BOOST_SCOPE_EXIT_END

    m_ButtonNames = {
        "up",
        "down",
        "left",
        "right",

        "shoot",
        "sprint",
        "action",
        "strafe"
    };

    m_StateNames = {
        "up",
        "down",
        "left",
        "right",

        "stand",
        "walk",
    };

    for(unsigned int i=0; i<(unsigned int)Button::MAX; ++i)
        try{
            m_Buttons[i] = input->button_id(m_ButtonNames[i]);
        }catch(const out_of_range&){
            m_Buttons[i] = numeric_limits<unsigned int>::max();
        }
    for(unsigned int i=0; i<(unsigned int)State::MAX; ++i)
        try{
            m_States[i] = m_pSprite->state_id(m_StateNames[i]);
        }catch(const out_of_range&){
            m_States[i] = numeric_limits<unsigned int>::max();
        }
    
    set_state(State::STAND);
    set_state(State::DOWN);

    auto crosshair = make_shared<Sprite>(
        "data/hud/arrow.png",
        resources,
        "", // no skin
        vec3(0.0f, 0.0f, 99.0f)
    );
    crosshair->mesh()->offset(vec3(0.0f, -32.0f, 0.0f));
    m_pSprite->add(crosshair);
    m_pCrosshair = crosshair;

    m_vDir = vec2(0.0f, 1.0f);
    m_CrosshairEase.stop(Angle::degrees(180));
}

void PlayerInterface2D :: event()
{
    lock_input();
    BOOST_SCOPE_EXIT(this_) {
        this_->unlock_input();
    } BOOST_SCOPE_EXIT_END
    lock_sprite();
    BOOST_SCOPE_EXIT(this_) {
        this_->unlock_sprite();
    } BOOST_SCOPE_EXIT_END
    auto crosshair = m_pCrosshair.lock();
    assert(crosshair);

    const bool strafe = button(Button::STRAFE);

    m_vMove = vec2(0.0f);
    if(button(Button::LEFT))
        m_vMove += vec2(-1.0f, 0.0f);
    if(button(Button::RIGHT))
        m_vMove += vec2(1.0f, 0.0f);
    if(button(Button::UP))
        m_vMove += vec2(0.0f, -1.0f);
    if(button(Button::DOWN))
        m_vMove += vec2(0.0f, 1.0f);

    if(!strafe)
    {
        m_CrosshairEase.resume();

        if(m_vMove != vec2())
        {
            vec2 old_dir = m_vDir;
            m_vDir = normalize(m_vMove);

            Angle a = angle(old_dir, m_vDir);
            if(fabs(a.degrees()) > EPSILON)
            {
                m_CrosshairEase.stop(
                    Angle::degrees(
                        orientedAngle(
                            vec2(m_vDir.x, -m_vDir.y),
                            vec2(0.0f, 1.0f)
                        )
                    ),
                    Freq::Time(100),
                    //Freq::Time((fabs(a.degrees())  > 90.0f + EPSILON) ? 0 : 100),
                    [](const Angle& a, const Angle& b, float t){
                        return a + (b-a)*t;
                    }
                );
            }
        }

        // TODO: get direction from crosshair angle
        if(fabs(m_vDir.y) > EPSILON)
            set_state(m_vDir.y > 0.0f ? State::DOWN : State::UP);
        else if(fabs(m_vDir.x) > EPSILON)
            set_state(m_vDir.x > 0.0f ? State::RIGHT: State::LEFT);
    }
    else
    {
        m_CrosshairEase.pause();
    }

    //crosshair->reset_orientation();
    //*crosshair->matrix() = rotate(mat4(),
    //    orientedAngle(
    //        vec2(m_vDir.x, -m_vDir.y), vec2(0.0f, 1.0f)
    //    ),
    //    Axis::Z
    //);

    m_fSpeed = m_fWalkSpeed;
    if(button(Button::SPRINT) && !strafe)
    {
        m_fSpeed = m_fWalkSpeed * m_fSprintMult;
        m_pSprite->speed(m_fSprintMult);
    }
    else
    {
        m_pSprite->resume();
    }

    set_state(
        length(m_vMove) > EPSILON ?
        State::WALK :
        State::STAND
    );
}

void PlayerInterface2D :: logic(Freq::Time t)
{
    lock_sprite();
    BOOST_SCOPE_EXIT(this_) {
        this_->unlock_sprite();
    } BOOST_SCOPE_EXIT_END

    m_CrosshairEase.logic(t);
    auto crosshair = m_pCrosshair.lock();
    assert(crosshair);
    //*crosshair->matrix() = m_CrosshairEase.get();

    crosshair->reset_orientation();
    glm::vec3 pos = Matrix::translation(*crosshair->matrix());
    *crosshair->matrix() = rotate(
        m_CrosshairEase.get().degrees(),
        Axis::Z
    );
    crosshair->position(pos);
    crosshair->pend();

    if(length(m_vMove) > EPSILON)
        m_pSprite->move(vec3(m_vMove * m_fSpeed, 0.0f) * t.s());
}

