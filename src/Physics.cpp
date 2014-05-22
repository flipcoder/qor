#include "Physics.h"
#include <glm/glm.hpp>
#include "Node.h"
//#include "IMeshContainer.h"
//#include "NodeAttributes.h"
#include "Node.h"
#include "Graphics.h"
#include "Node.h"
#include <iostream>
#include <memory>
using namespace std;
//using namespace physx;

#include "IPhysicsObject.h"

Physics::Physics(Node* root, void* userdata):
    m_pRoot(root)
{
    m_pWorld = NewtonCreate();
    if(userdata)
        NewtonWorldSetUserData(m_pWorld, userdata);
}

Physics :: ~Physics() {
    if(m_pWorld)
        NewtonDestroy(m_pWorld);
}

void Physics :: logic(Freq::Time advance)
{
    static float accum = 0.0f;
    const float fixed_step = 1/60.0f;
    float timestep = advance.s();

    accum += timestep;
    while(accum >= fixed_step)
    {
        
        NewtonUpdate(m_pWorld, fixed_step);
        sync(m_pRoot, SYNC_RECURSIVE);
//#ifdef _NEWTON_VISUAL_DEBUGGER
//        NewtonDebuggerServe(m_pDebugger, m_pWorld);
//#endif
        accum -= fixed_step;
    }
}

void Physics :: generate(Node* node, unsigned int flags, std::unique_ptr<glm::mat4> transform)
{
    if(!node)
        return;

    // generate identity matrix if transform was passed in as null
    // could use an auto_ptr in these situations but meh

    // TODO: If no transform is given, derive world space transform from node
    if(!transform)
        transform = kit::make_unique<glm::mat4>();

    // apply transformation of node so the mesh vertices are correct
    *transform *= *node->matrix_c();
    //assert(transform->isIdentity());
    
    // Are there physics instructions?
    if(node->physics())
    {
        IPhysicsObject* pobj = dynamic_cast<IPhysicsObject*>(node);
        //IPhysicsObject* pobj = (IPhysicsObject*)node;
        if(!pobj->body())
        {
            // Check if there's static geometry in this node, if so let's process it
            switch(pobj->physics_type())
            {
                case IPhysicsObject::Type::STATIC:
                    generate_tree(node, flags, transform.get());
                    break;
                case IPhysicsObject::Type::ACTOR:
                    generate_actor(node, flags, transform.get());
                    break;
                case IPhysicsObject::Type::DYNAMIC:
                    generate_dynamic(node, flags, transform.get());
                    break;
                default:
                    //assert(false);
                    break;
            }
        }
    }

    // generate children
    if(node->has_children() && (flags & (unsigned)GenerateFlag::RECURSIVE))
    {
        for(auto&& child: node->subnodes())
        {
            // copy current node's transform so it can be modified by child
            std::unique_ptr<glm::mat4> transform_copy =
                kit::make_unique<glm::mat4>(*transform);
            generate(child.get(), flags, std::move(transform_copy));
        }
    }
    
    // delete generated identity matrix for those who passed in null matrix pointers
    //if(created_transform)
    //    delete transform;
}

void Physics :: generate_actor(Node* node, unsigned int flags, glm::mat4* transform)
{
    assert(node);
    assert(transform);
    assert(node->physics());

    IPhysicsObject* pobj = dynamic_cast<IPhysicsObject*>(node);
    //Actor* actor = dynamic_cast<Actor*>(node);
    
    // TODO: generate code
}

void Physics :: generate_tree(Node* node, unsigned int flags, glm::mat4* transform)
{
    assert(node);
    assert(transform);
    assert(node->physics());

    // TODO: btBvhTriangleMeshShape or btMultiMaterialTriangleMeshShape

    //std::vector<shared_ptr<Mesh>> meshes = node->children<Mesh>();
    //if(meshes.empty())
    //    return;
    //Node* physics_object = dynamic_cast<Node*>(node);
    
    // TODO: generate code    
}

void Physics :: generate_dynamic(Node* node, unsigned int flags, glm::mat4* transform)
{
    assert(node);
    assert(transform);
    assert(node->physics());

    //std::vector<shared_ptr<Mesh>> meshes = node->children<Mesh>();
    //if(meshes.empty())
    //    return;
    //Node* physics_object = dynamic_cast<Node*>(node);

    // TODO: generate code
}

// syncBody gets the data out of physics subsystem, reports it back to each node
void Physics :: sync(Node* node, unsigned int flags)
{
    return;
    
    if(!node)
        return;
    if(!node->physics())
        return;
    
    auto pobj = dynamic_cast<IPhysicsObject*>(node);
    assert(pobj);
    //if(!pobj)
    //    return;
    
    if(pobj->physics_type() != IPhysicsObject::Type::STATIC)
    {
        glm::mat4 body_matrix;
        NewtonBodyGetMatrix((NewtonBody*)node->body(), glm::value_ptr(body_matrix));
        node->sync(body_matrix);

        // NOTE: Remember to update the transform from the object side afterwards.
    }

    if(flags & SYNC_RECURSIVE)
        for(auto&& child: *node)
            sync(child.get(), flags);
}

//btRigidBody* Physics :: add_body(btCollisionObject* obj, Node* node, glm::mat4* transform, btVector3* inertia)
//{
    //float mass = node->mass();

    //glm::vec3 inertia, origin;
    //NewtonBody* body = NewtonCreateBody(m_pWorld, nc, glm::value_ptr(*transform));
    //NewtonBodySetUserData(body, node);
    
    //btTransform btt;
    //btt.setFromOpenGLMatrix(glm::value_ptr(transform));
    //btRigidBody* body = new bt

    //node->setPhysicsBody(this, (void*)body, (void*)motion);
    //m_pWorld->addRigidBody(body);
    
    //if(mass > EPSILON)
    //{
    //    NewtonConvexCollisionCalculateInertialMatrix(nc, glm::value_ptr(inertia), glm::value_ptr(origin));
    //    NewtonBodySetMassMatrix(body, mass, inertia.x * mass, inertia.y * mass, inertia.z * mass);
    //    NewtonBodySetCentreOfMass(body, glm::value_ptr(origin));
    //    NewtonBodySetForceAndTorqueCallback(body, (NewtonApplyForceAndTorque)&cbForceTorque);
    //    NewtonBodySetTransformCallback(body, (NewtonSetTransform)&cbTransform);
    //}

//    return nullptr;
//}

bool Physics :: delete_body(void* obj)
{
    if(!obj)
        return false;
//    //m_pWorld->removeCollisionObject(obj);
//    //delete obj;

    //NewtonDestroyBody(m_pWorld, (NewtonBody*)obj);
    NewtonDestroyBody((NewtonBody*)obj);
    return true;
}

void Physics :: cbForceTorque(const NewtonBody* body, float timestep, int threadIndex)
{
    return; // TEMP
    
    float mass, ix, iy, iz;
    NewtonBodyGetMassMatrix(body, &mass, &ix, &iy, &iz);
    glm::vec3 force(0.0f, mass * -9.8f, 0.0f);
    glm::vec3 omega(0.0f);
    glm::vec3 velocity(0.0f);
    glm::vec3 torque(0.0f);
    NewtonBodyGetVelocity(body, glm::value_ptr(velocity));

    Node* node = (Node*)NewtonBodyGetUserData(body);
    //unsigned int userflags = node->physics_logic(timestep, mass, force, omega, torque, velocity);
    unsigned userflags = 0;
    
    if(userflags & USER_FORCE)
        NewtonBodyAddForce(body, glm::value_ptr(force));
    if(userflags & USER_OMEGA)
        NewtonBodySetOmega(body, glm::value_ptr(omega));
    if(userflags & USER_TORQUE)
        NewtonBodySetTorque(body, glm::value_ptr(torque));
    if(userflags & USER_VELOCITY)
        NewtonBodySetVelocity(body, glm::value_ptr(velocity));
}

void Physics :: cbTransform(const NewtonBody* body)
{
    Node* node = (Node*)NewtonBodyGetUserData(body);
    
    float marray[16];
    NewtonBodyGetMatrix(body, &marray[0]);
    
    // Note: All physics nodes should be collapsed (node transform == world transform)
    //  so leaving this in world space is fine for now, unless in the future a better system
    //  is integrated
    glm::mat4 m = Matrix::from_array(marray);
    //m.clear(glm::mat4::TRANSPOSE, m);
    //node->sync(&m);
    node->sync(m);
}

