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

Physics::Physics(Node* root, void* userdata):
    m_pRoot(root)
{
    //if(!(m_pFoundation = PxCreateFoundation(
    //    PX_PHYSICS_VERSION,
    //    *m_DefaultAllocatorCallback,
    //    *m_DefaultErrorCallback
    //)))
    //    fail();
    //bool recordMemoryAllocations = true;
    //if(!(m_pProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_pFoundation)))
    //    fail();
    //if(!(m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION,
    //    *m_pFoundation,
    //    PxTolerancesScale(),
    //    recordMemoryAllocations,
    //    m_pProfileZoneManager
    //)))
    //    fail();
    //if(!(m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pFoundation, PxCookingParams())))
    //    fail();
    //if(!PxInitExtensions(*m_pPhysics))
    //    fail();
    //m_pBroadphase = kit::make_unique<btBroadphaseInterface>();
    //m_pWorld = kit::make_unique<
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
        
        ////m_pWorld->stepSimulation(fixed_step, NUM_SUBSTEPS);
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
    unsigned phys = node->physics();
    if(phys)
    {
        //Node* object = dynamic_cast<Node*>(node);
        if(!node->body())
        {
            // Check if there's static geometry in this node, if so let's process it
            switch(phys)
            {
                case (unsigned)PhysicsFlag::STATIC:
                    generate_tree(node, flags, transform.get());
                    break;
                case (unsigned)PhysicsFlag::ACTOR:
                    generate_actor(node, flags, transform.get());
                    break;
                case (unsigned)PhysicsFlag::DYNAMIC:
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
            std::unique_ptr<glm::mat4> transform_copy = kit::make_unique<glm::mat4>(*transform);
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

    Node* physics_object = dynamic_cast<Node*>(node);
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
    if(!node)
        return;
}
//void Physics :: sync(Node* node, unsigned int flags)
//{
//    if(!node)
//        return;

//    // In here, we must read physics instructions from node, and depending on what type it is,
//    // we must deal with it differently
//    if(node->hasAttribute(NodeAttributes::PHYSICS)&&
//        ((dynamic_cast<Node*>(node))->getPhysicsFlag() != Node::STATIC))
//    {
//        Node* po = dynamic_cast<Node*>(node);
//        glm::mat4 body_matrix;
//        NewtonBodyGetMatrix((NewtonBody*)po->getPhysicsBody(), glm::value_ptr(body_matrix));
//        po->sync(&body_matrix);

//        // NOTE: Remember to update the transform from the object side afterwards.
//    }

//    // sync children
//    if(node->hasAttribute(NodeAttributes::CHILDREN)  && (flags & SYNC_RECURSIVE))
//    {
//        auto children = (std::list<shared_ptr<Node>>*)node->getAttribute(NodeAttributes::CHILDREN);
//        for(auto itr = children->begin();
//            itr != children->end();
//            ++itr)
//        {
//            syncBody(itr->get(), flags);
//        }
//        //foreach(Node* child, *children)
//        //    syncBody(child, flags);
//    }
//}

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
    //  so leaving this in world space is fine for now, unless in the future a better constraint system
    //  is integrated
    glm::mat4 m = Matrix::from_array(marray);
    //m.clear(glm::mat4::TRANSPOSE, m);
    //node->sync(&m);
    node->sync(m);
}

