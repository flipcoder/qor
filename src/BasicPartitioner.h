#ifndef _BASICPARTITIONER_H_YPGXO911
#define _BASICPARTITIONER_H_YPGXO911

#include "IPartitioner.h"
#include "kit/kit.h"
#include "Light.h"

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
        
    private:

        std::vector<const Node*> m_Nodes;
        std::vector<const Light*> m_Lights;

        Camera* m_pCamera = nullptr;
};

#endif

