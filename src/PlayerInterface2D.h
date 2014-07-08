#ifndef _PLAYERINTERFACE_H_W5TRRMBU
#define _PLAYERINTERFACE_H_W5TRRMBU

#include <memory>
#include <cstdint>
#include <boost/optional.hpp>
#include "kit/cache/cache.h"
#include "NodeInterface.h"
#include "Input.h"
#include "Sprite.h"
#include "kit/freq/animation.h"

/*
 * Player Interface
 * This is what allows a Controller to control the Player,
 * and make the Player Sprite "act" like a Player
 * (otherwise it's just a sprite)
 */
class PlayerInterface2D:
    public NodeInterface,
    public std::enable_shared_from_this<PlayerInterface2D>
{
    public:

        enum class Button: unsigned int
        {
            UP,
            DOWN,
            LEFT,
            RIGHT,

            SHOOT,
            SPRINT,
            ACTION,
            STRAFE,

            MAX,

        };

        enum class State: unsigned int
        {
            UP,
            DOWN,
            LEFT,
            RIGHT,

            STAND,
            WALK,

            MAX
        };

        PlayerInterface2D(
            const std::shared_ptr<Controller>& input,
            const std::shared_ptr<Node>& node,
            Freq::Timeline* timeline,
            Cache<Resource, std::string>* resources
        );
        virtual ~PlayerInterface2D() {}

        virtual void event() override;
        virtual void logic(Freq::Time t) override;

        const Input::Switch& button(
            Button btn
        ){
            assert(m_pInput); // should be locked

            try{
                unsigned int idx = m_Buttons.at((unsigned int)btn);
                if(idx != std::numeric_limits<unsigned int>::max())
                    return m_pInput->button(idx);
            }catch(const std::out_of_range&){}

            return m_pInput->input()->dummy_switch();
        }

        void set_state(State state) {
            assert(m_pSprite);
            unsigned int idx = m_States.at((unsigned int)state);
            assert(idx != std::numeric_limits<unsigned int>::max());
            m_pSprite->set_state(idx);
        }

        /*
         * Turns NodeInterface's controller() weak_ptr into m_pInput
         */
        void lock_input() { m_pInput = controller(); }
        void unlock_input() { m_pInput.reset(); }
        void lock_sprite() { m_pSprite = std::static_pointer_cast<Sprite>(node()); }
        void unlock_sprite() { m_pSprite.reset(); }

        /*
         * Make sure Input can call this interface's logic
         *
         * Warning: can't do this in constructor, so we do it on the fly
         */
        void plug() {
            if(!m_InterfaceID)
                m_InterfaceID = controller()->add_interface(
                    std::static_pointer_cast<IInterface>(
                        shared_from_this()
                    )
                );
        }
        /*
         * Manually unplugs interface from input system
         *
         * This does not need to be called since Input system's weak_ptr's
         * will allow Interfaces that go out of scope to auto-unplug
         */
        void unplug() {
            if(m_InterfaceID)
            {
                controller()->remove_interface(
                    *m_InterfaceID
                );
                m_InterfaceID = boost::optional<unsigned int>();
            }
        }

        std::shared_ptr<Sprite> sprite() {
            return std::static_pointer_cast<Sprite>(node());
        }

        std::shared_ptr<Sprite> crosshair() {
            return m_pCrosshair.lock();
        }

    private:

        std::shared_ptr<Controller> m_pInput;
        std::shared_ptr<Sprite> m_pSprite;

        std::array<unsigned int, (unsigned int)Button::MAX> m_Buttons;
        std::array<unsigned int, (unsigned int)State::MAX> m_States;
        std::vector<std::string> m_ButtonNames;
        std::vector<std::string> m_StateNames;

        glm::vec2 m_vMove;
        glm::vec2 m_vDir;

        float m_fSprintMult = 1.5f;
        float m_fWalkSpeed = 100.0f;
        float m_fSpeed = 100.0f;

        Animation<Angle> m_CrosshairEase;
        std::weak_ptr<Sprite> m_pCrosshair;
        boost::optional<unsigned int> m_InterfaceID;
};

#endif

