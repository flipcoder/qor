#ifndef _PHYSICSOBJECT_H
#define _PHYSICSOBJECT_H

#ifdef QOR_NO_PHYSICS
class PhysicsObject {};
#else

#include <memory>
#include <vector>
#include "Physics.h"
#include "kit/math/common.h"
#include "Node.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class PhysicsObject:
    public btMotionState
{
public:

    PhysicsObject(){}
    virtual ~PhysicsObject() {}
    
    Physics* system() { return m_pSystem; }
    const Physics* system() const { return m_pSystem; }
    //NewtonBody* getPhysicsBody() { return m_pBody; }
    std::unique_ptr<btCollisionObject> body() { return nullptr; } //placeholder
    void body(std::unique_ptr<btCollisionObject> p) {
        m_pBody = std::move(p);
    }
    void system(Physics* sys) {
        m_pSystem = sys;
    }
    
    virtual float radius() const { return 0.0f; }
    virtual float height() const { return 0.0f; }

    virtual void sync(glm::mat4* m) {}
    
    virtual Node::Physics physics_type() const { return m_Type; }
    void physics_type(Node::Physics t) {
        m_Type = t;
    }
    //virtual unsigned int physicsLogic(float timestep, float mass, glm::vec3& force, glm::vec3& omega, glm::vec3& torque, glm::vec3& velocity);
    virtual float mass() const { return 0.0f; }
    
    virtual void setWorldTransform(const btTransform& worldTrans) override {
        Node* node = dynamic_cast<Node*>(this);
        glm::mat4 matrix = Physics::fromBulletTransform(worldTrans);
        if(!sync(matrix))
            *node->matrix() = matrix;
        node->pend();
    }
    virtual void getWorldTransform(btTransform& worldTrans) const override {
        const Node* node = dynamic_cast<const Node*>(this);
        worldTrans = Physics::toBulletTransform(*node->matrix_c());
    }
    
    // returns true if the object will sync its own properties, otherwise false to autosync
    virtual bool sync(glm::mat4& m) {
        return false;
    }

    virtual void set_kinematic_pos(btTransform &currentPos) {}

    void add_striding_mesh_interface(std::unique_ptr<btStridingMeshInterface>& a) {
        m_StridingMeshInterfaces.push_back(std::move(a));
    }
    void add_collision_shape(std::unique_ptr<btCollisionShape>& a) {
        m_CollisionShapes.push_back(std::move(a));
    }
    void action(std::unique_ptr<btActionInterface>& a) {
        m_ActionInterface = std::move(a);
    }
    btActionInterface* action() {
        return m_ActionInterface.get();
    }
    btKinematicCharacterController* character() {
        return dynamic_cast<btKinematicCharacterController*>(m_ActionInterface.get());
    }
    void ghost_pair_callback(std::unique_ptr<btGhostPairCallback>& a) {
        m_GhostPairCallback = std::move(a);
    }
protected:
    std::unique_ptr<btCollisionObject> m_pBody;
    std::unique_ptr<btActionInterface> m_ActionInterface;
    std::unique_ptr<btGhostPairCallback> m_GhostPairCallback;
    std::vector<std::unique_ptr<btStridingMeshInterface>> m_StridingMeshInterfaces;
    std::vector<std::unique_ptr<btCollisionShape>> m_CollisionShapes;
    
    //NewtonWorld* m_pWorld; // weak
    //std::unique_ptr<NewtonBody> m_pBody;
    //NewtonBody* m_pBody;
    Physics* m_pSystem = nullptr;
    Node::Physics m_Type = Node::Physics::NONE;
};

#endif // not QOR_NO_PHYSICS 
#endif

