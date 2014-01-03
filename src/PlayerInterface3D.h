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
 * This is what allows a Controller to control the Node representing the player
 */
class PlayerInterface3D:
    public NodeInterface,
    public std::enable_shared_from_this<PlayerInterface3D>
{
    public:

        //enum class Button: unsigned int
        //{
        //    UP,
        //    DOWN,
        //    LEFT,
        //    RIGHT,

        //    SHOOT,
        //    SPRINT,
        //    ACTION,
        //    STRAFE,

        //    MAX,

        //};

        PlayerInterface3D(
            const std::shared_ptr<Controller>& input,
            const std::shared_ptr<Node>& node
        );
        virtual ~PlayerInterface3D() {}

        virtual void event() override;
        virtual void logic(Freq::Time t) override;

        /*
         * Turns NodeInterface's controller() weak_ptr into m_pInput
         */
        void lock_input() { m_pInput = controller(); }
        void unlock_input() { m_pInput.reset(); }
        void lock_node() { m_pNode = node(); }
        void unlock_node() { m_pNode.reset(); }
        //void lock_sprite() { m_pSprite = std::static_pointer_cast<Sprite>(node()); }
        //void unlock_sprite() { m_pSprite.reset(); }

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

    private:

        std::shared_ptr<Node> m_pNode;
        std::shared_ptr<Controller> m_pInput;

        //std::array<unsigned int, (unsigned int)Button::MAX> m_Buttons;
        //std::vector<std::string> m_ButtonNames;

        glm::vec3 m_Move;
        glm::vec3 m_Dir;

        boost::optional<unsigned int> m_InterfaceID;

        // allow pitch vector to influence move delta (spectator-style fly mode)
        bool m_bFly = false;
};

#endif

