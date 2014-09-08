#ifndef _BASICPARTITIONER_H_YPGXO911
#define _BASICPARTITIONER_H_YPGXO911

#include "IPartitioner.h"
#include "kit/kit.h"
#include "Light.h"
#include <boost/signals2.hpp>

class BasicPartitioner:
    public IPartitioner
{
    public:
        BasicPartitioner();
        virtual ~BasicPartitioner() {}

        virtual void partition(const Node* root) override;
        virtual const std::vector<
            const Light*
        >& visible_lights() const override {
            return m_Lights;
        }
        virtual const std::vector<const Node*>& visible_nodes() const override{
            return m_Nodes;
        }
        virtual const std::vector<const Node*>& visible_nodes_from(
            const Light* light
        ) const override {
            return m_Nodes;
        }

        virtual void camera(Camera* camera) override {
            m_pCamera = camera;
        }
        virtual const Camera* camera() const override {
            return m_pCamera;
        }
        virtual Camera* camera() override {
            return m_pCamera;
        }
        
        virtual void logic(Freq::Time t) override;
        
        virtual boost::signals2::connection on_collision(
            const std::shared_ptr<Node>& a,
            const std::shared_ptr<Node>& b,
            std::function<void(Node*, Node*)> cb
        ) override;
        
    private:

        std::vector<const Node*> m_Nodes;
        std::vector<const Light*> m_Lights;

        std::vector<
            std::tuple<
                std::weak_ptr<Node>,
                std::weak_ptr<Node>,
                std::unique_ptr<boost::signals2::signal<
                    void(Node*, Node*)
                >>
            >
        > m_Collisions;

        //std::map<
        //    std::tuple<Node*, Node*>,
        //    boost::signals2::signal<
        //        std::function<void(Node*, Node*)>(Node*, Node*),
        //        kit::push_back_values<std::vector<
        //            std::function<void(Node*, Node*)>(Node*, Node*)
        //        >>
        //    >
        //> m_Collisions;
        
        Camera* m_pCamera = nullptr;
};

#endif

