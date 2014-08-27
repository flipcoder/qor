#ifndef _IPHYSICSOBJECT_H
#define _IPHYSICSOBJECT_H

#include <memory>
#include <vector>

//#include <newton/Newton.h>
//#include "Physics.h"
class Physics;
#include "kit/math/common.h"
#include "Node.h"

class PhysicsObject
{
public:

    PhysicsObject(){}
    virtual ~PhysicsObject() {}
    
    Physics* physics() { return m_pPhysics; }
    const Physics* physics() const { return m_pPhysics; }
    //btMotionState* getMotionState() { return this; }
    //NewtonBody* getPhysicsBody() { return m_pBody; }
    void* body() { return nullptr; } //placeholder
    void set_physics(Physics* physics) {
        m_pPhysics = sys;
    }
    //void setBody(NewtonBody* obj) {
    //    m_pBody = obj;
    //}
    //NewtonBody* getBody() { return m_pBody; }
    
    virtual float radius() const { return 0.0f; }
    virtual float height() const { return 0.0f; }

    virtual void sync(glm::mat4* m) {}
    
    virtual Type physics_type() const { return m_PhysicsType; }
    void physics_type(Type t) {
        m_PhysicsType = t;
    }
    //virtual unsigned int physicsLogic(float timestep, float mass, glm::vec3& force, glm::vec3& omega, glm::vec3& torque, glm::vec3& velocity);
    virtual float mass() const { return 0.0f; }
    
    //virtual void setWorldTransform(const btTransform& worldTrans) {
    //    Node* node = dynamic_cast<Node*>(this);
    //    glm::mat4 matrix = Physics::fromBulletTransform(worldTrans);
    //    if(!sync(matrix))
    //        *node->matrix() = matrix;
    //    node->pendWorldMatrix();
    //}
    //virtual void getWorldTransform(btTransform& worldTrans) const {
    //    const Node* node = dynamic_cast<const Node*>(this);
    //    worldTrans = Physics::toBulletTransform(*node->matrix_c());
    //}
    
    //// returns true if the object will sync its own properties, otherwise false to autosync
    ////virtual bool sync(glm::mat4& m) {
    ////    return false;
    ////}

    //virtual void setKinematicPos(btTransform &currentPos) {}

    //void addStridingMeshInterface(std::unique_ptr<btStridingMeshInterface>& a) {
    //    m_StridingMeshInterfaces.push_back(std::move(a));
    //}
    //void addCollisionShape(std::unique_ptr<btCollisionShape>& a) {
    //    m_CollisionShapes.push_back(std::move(a));
    //}
    //void setAction(std::unique_ptr<btActionInterface>& a) {
    //    m_ActionInterface = std::move(a);
    //}
    //btActionInterface* getAction() {
    //    return m_ActionInterface.get();
    //}
    //KinematicCharacterController* getCharacter() {
    //    return dynamic_cast<KinematicCharacterController*>(m_ActionInterface.get());
    //}
    //void setGhostPairCallback(std::unique_ptr<btGhostPairCallback>& a) {
    //    m_GhostPairCallback = std::move(a);
    //}
protected:
    //std::unique_ptr<btCollisionObject> m_pBody;
    //std::unique_ptr<btActionInterface> m_ActionInterface;
    //std::unique_ptr<btGhostPairCallback> m_GhostPairCallback;
    //std::vector<std::unique_ptr<btStridingMeshInterface>> m_StridingMeshInterfaces;
    //std::vector<std::unique_ptr<btCollisionShape>> m_CollisionShapes;
    //btMotionState* m_pMotionState;
    
    //NewtonWorld* m_pWorld; // weak
    //std::unique_ptr<NewtonBody> m_pBody;
    //NewtonBody* m_pBody;
    Physics* m_pPhysics;
    Type m_PhysicsType = Type::NONE;
};

#endif

