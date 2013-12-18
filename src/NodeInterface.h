#ifndef _NODEINTERFACE_H_G4TWQEPP
#define _NODEINTERFACE_H_G4TWQEPP

#include <memory>
#include "Input.h"
#include "Node.h"

/*
 * A basic input interface between an input controller and a node
 *
 * The target can be any type of node, including cameras or trackers
 */
class NodeInterface:
    public IInterface
{
    public:

        NodeInterface(
            const std::shared_ptr<Controller>& input,
            const std::shared_ptr<Node>& node
        ):
            m_pInput(input),
            m_pNode(node)
        {
            assert(input);
            assert(node);
        }

        virtual ~NodeInterface() {}

        virtual void event();
        virtual void logic(Freq::Time t) override;

        std::shared_ptr<Controller> controller() { return m_pInput.lock(); }
        std::shared_ptr<Node> node() { return m_pNode.lock(); }
        std::shared_ptr<const Controller> controller() const { return m_pInput.lock(); }
        std::shared_ptr<const Node> node() const { return m_pNode.lock(); }

    protected:
        std::weak_ptr<Controller> m_pInput;
        std::weak_ptr<Node> m_pNode;
};

#endif

