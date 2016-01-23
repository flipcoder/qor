#ifndef QOR_NO_PHYSICS

#include "Physics.h"
#include <glm/glm.hpp>
#include "Node.h"
//#include "IMeshContainer.h"
//#include "NodeAttributes.h"
#include "Node.h"
#include "Graphics.h"
#include "PhysicsObject.h"
#include "Mesh.h"
#include <iostream>
#include <memory>
#include "kit/math/vectorops.h"
using namespace std;

Physics::Physics(Node* root, void* userdata):
    m_pRoot(root)
{
    m_pCollisionConfig = kit::make_unique<btDefaultCollisionConfiguration>();
    m_pDispatcher = kit::make_unique<btCollisionDispatcher>(m_pCollisionConfig.get());
    //m_pBroadphase = kit::make_unique<btDbvtBroadphase>();
    btVector3 worldMin(-1000,-1000,-1000);
    btVector3 worldMax(1000,1000,1000);
    m_pBroadphase = kit::make_unique<btAxisSweep3>(worldMin, worldMax);
    m_pSolver = kit::make_unique<btSequentialImpulseConstraintSolver>();
    m_pWorld = kit::make_unique<btDiscreteDynamicsWorld>(
        m_pDispatcher.get(),
        m_pBroadphase.get(),
        m_pSolver.get(),
        m_pCollisionConfig.get()
    );
    m_pWorld->setGravity(btVector3(0.0, -9.8, 0.0));
}

Physics :: ~Physics() {
    if(m_pWorld)
    {
        //NewtonDestroyAllBodies(m_pWorld);
        //NewtonDestroy(m_pWorld);
    }
}

void Physics :: logic(Freq::Time advance)
{
    static float accum = 0.0f;
    const float fixed_step = 1/60.0f;
    float timestep = advance.s();
    m_pWorld->stepSimulation(timestep, NUM_SUBSTEPS, fixed_step);

//    accum += timestep;
//    while(accum >= fixed_step)
//    {
        
//        m_pWorld->stepSimulation(fixed_step, NUM_SUBSTEPS, fixed_step);
//        //NewtonUpdate(m_pWorld, fixed_step);
//        sync(m_pRoot, SYNC_RECURSIVE);
////#ifdef _NEWTON_VISUAL_DEBUGGER
////        NewtonDebuggerServe(m_pDebugger, m_pWorld);
////#endif
//        accum -= fixed_step;
//    }
}

void Physics :: generate(Node* node, unsigned flags, std::unique_ptr<glm::mat4> transform)
{
    if(!node)
        return;

    // TODO: If no transform is given, derive world space transform from node
    if(!transform)
        transform = kit::make_unique<glm::mat4>();

    // apply transformation of node so the mesh vertices are correct
    *transform *= *node->matrix_c();
    //assert(transform->isIdentity());
    
    // Are there physics instructions?
    if(node->physics())
    {
        if(not node->body())
        {
            // Check if there's static geometry in this node, if so let's process it
            switch(node->physics())
            {
                case Node::Physics::STATIC:
                    generate_tree(node, flags, transform.get());
                    break;
                case Node::Physics::ACTOR:
                    generate_actor(node, flags, transform.get());
                    break;
                case Node::Physics::DYNAMIC:
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

    //Actor* actor = dynamic_cast<Actor*>(node);
    
    // TODO: generate code
}

void Physics :: generate_tree(Node* node, unsigned int flags, glm::mat4* transform)
{
    assert(node);
    assert(transform);
    if(node->physics() != Node::STATIC)
        return;
    assert(node->physics_shape() == Node::MESH);
    //assert(node->physics() == Node::STATIC);
    //assert(node->physics_shape() == Node::MESH);

    //std::vector<shared_ptr<Node>> meshes = node->children();
    //for(auto&& c: meshes)
    //{
        Mesh* mesh = dynamic_cast<Mesh*>(node);
        if(not mesh)
            return;
        //NewtonCollision* collision = NewtonCreateTreeCollision(m_pWorld, 0);
        //NewtonTreeCollisionBeginBuild(collision);
        //try{
        if(not mesh->internals())
            return;
        if(not mesh->internals()->geometry)
            return;
        //}catch(const exception& e){
        //    WARNING(e.what());
        //}
        
        if(mesh->internals()->geometry->ordered_verts().empty())
            return;
        
        auto triangles = kit::make_unique<btTriangleMesh>();
        //mesh->each([&triangles](Node* n){
        //    Mesh* mesh = dynamic_cast<Mesh*>(node);
            auto verts = mesh->internals()->geometry->ordered_verts();
            LOGf("verts size %s", verts.size());
            for(int i = 0; i < verts.size(); i += 3)
            {
                triangles->addTriangle(
                    btVector3(verts[i].x, verts[i].y,  verts[i].z),
                    btVector3(verts[i+1].x, verts[i+1].y,  verts[i+1].z),
                    btVector3(verts[i+2].x, verts[i+2].y,  verts[i+2].z)
                );
            }
        //}, Node::Each::INCLUDE_SELF);
        
        node->reset_body();
        auto physics_object = node->body();
        assert(physics_object.get());
        unique_ptr<btCollisionShape> shape = kit::make_unique<btBvhTriangleMeshShape>(
            triangles.get(), true, true
        );
        btRigidBody::btRigidBodyConstructionInfo info(
            0.0f,
            physics_object.get(), // inherits btMotionState
            shape.get()
        );
        auto body = kit::make_unique<btRigidBody>(info);
        auto interface = unique_ptr<btStridingMeshInterface>(std::move(triangles));
        physics_object->add_striding_mesh_interface(interface);
        physics_object->add_collision_shape(shape);
        physics_object->body(std::move(body));
        physics_object->system(this);
        m_pWorld->addRigidBody((btRigidBody*)physics_object->body());
        
        //NewtonTreeCollisionEndBuild(collision, 0);
        //add_body(collision, node, transform);
        //NewtonReleaseCollision(m_pWorld, collision);
    //}
    //if(meshes.empty())
    //    return;
    //Node* physics_object = dynamic_cast<Node*>(node);
}

void Physics :: generate_dynamic(Node* node, unsigned int flags, glm::mat4* transform)
{
    assert(node);
    assert(transform);
    assert(node->physics());
    assert(node->physics() == Node::DYNAMIC);

    Mesh* mesh = dynamic_cast<Mesh*>(node);
    if(not mesh)
        return;

    switch(node->physics_shape())
    {
        case Node::HULL:
            break;
        case Node::BOX:
        {
        
            //std::vector<glm::vec3> verts;
            ////try{
            //if(not mesh->internals())
            //    return;
            //if(not mesh->internals()->geometry)
            //    return;
            //verts = mesh->internals()->geometry->ordered_verts();
            ////}catch(const exception& e){
            ////    WARNING(e.what());
            ////}
            //auto triangles = kit::make_unique<btTriangleMesh>();
            
            //for(int i = 0; i < verts.size(); i += 3)
            //{
            //    //NewtonTreeCollisionAddFace(
            //    //    collision, 3, glm::value_ptr(verts[i]),
            //    //    sizeof(glm::vec3), 0
            //    //); 
            //    triangles->addTriangle(
            //        btVector3(verts[0].x, verts[0].y,  verts[0].z),
            //        btVector3(verts[1].x, verts[1].y,  verts[1].z),
            //        btVector3(verts[2].x, verts[2].y,  verts[2].z)
            //    );
            //}
            
            mesh->each([](Node* n){
                auto m = std::dynamic_pointer_cast<Mesh>(n->as_node());
                assert(m);
                m->set_physics(Node::NO_PHYSICS);
            });
            
            node->reset_body();
            auto physics_object = node->body();
            assert(physics_object.get());
            unique_ptr<btCollisionShape> shape = kit::make_unique<btBoxShape>(
                btVector3(1.0f,1.0f,1.0f)
            );
            btRigidBody::btRigidBodyConstructionInfo info(
                mesh->mass(),
                physics_object.get(), // inherits btMotionState
                shape.get()
            );
            auto body = kit::make_unique<btRigidBody>(info);
            //auto interface = unique_ptr<btStridingMeshInterface>(std::move(triangles));
            //physics_object->add_striding_mesh_interface(interface);
            physics_object->add_collision_shape(shape);
            physics_object->body(std::move(body));
            physics_object->system(this);
            m_pWorld->addRigidBody((btRigidBody*)physics_object->body());
            return;
            //break;
        }
        default:
            assert(false);
    };
    //std::vector<shared_ptr<Mesh>> meshes = node->children<Mesh>();
    //if(meshes.empty())
    //    return;
    //Node* physics_object = dynamic_cast<Node*>(node);
}

// syncBody gets the data out of physics subsystem, reports it back to each node
void Physics :: sync(Node* node, unsigned int flags)
{
    if(!node)
        return;
    if(!node->physics())
        return;
    
    if(node->physics() != Node::Physics::STATIC)
    {
        glm::mat4 body_matrix;
        //NewtonBodyGetMatrix((NewtonBody*)node->body(), glm::value_ptr(body_matrix));
        node->sync(body_matrix);

        // NOTE: Remember to update the transform from the object side afterwards.
    }

    if(flags & SYNC_RECURSIVE)
        for(auto&& child: *node)
            sync(child.get(), flags);
}

//NewtonBody* Physics :: add_body(NewtonCollision* nc, Node* node, glm::mat4* transform)
//{
//    return nullptr;
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
    m_pWorld->removeCollisionObject((btRigidBody*)obj);
    delete (btRigidBody*)obj;
    return true;
}

//void Physics :: cb_force_torque(const NewtonBody* body, float timestep, int threadIndex)
//{
//    float mass, ix, iy, iz;
//    //NewtonBodyGetMassMatrix(body, &mass, &ix, &iy, &iz);
//    glm::vec3 force(0.0f, mass * -9.8f, 0.0f);
//    glm::vec3 omega(0.0f);
//    glm::vec3 velocity(0.0f);
//    glm::vec3 torque(0.0f);
//    //NewtonBodyGetVelocity(body, glm::value_ptr(velocity));

//    //Node* node = (Node*)NewtonBodyGetUserData(body);
//    unsigned int userflags = 0;
//    //node->on_physics_tick(
//    //    Freq::Time::seconds(timestep), mass, force, omega, torque, velocity, &userflags
//    //);
    
//    //if(userflags & USER_FORCE)
//    //    NewtonBodyAddForce(body, glm::value_ptr(force));
//    //if(userflags & USER_OMEGA)
//    //    NewtonBodySetOmega(body, glm::value_ptr(omega));
//    //if(userflags & USER_TORQUE)
//    //    NewtonBodySetTorque(body, glm::value_ptr(torque));
//    //if(userflags & USER_VELOCITY)
//    //    NewtonBodySetVelocity(body, glm::value_ptr(velocity));
//}

//void Physics :: cb_transform(const NewtonBody* body)
//{
//    //Node* node = (Node*)NewtonBodyGetUserData(body);
    
//    //float marray[16];
//    //NewtonBodyGetMatrix(body, &marray[0]);
    
//    //glm::mat4 m = Matrix::from_array(marray);
//    //node->sync(m);
//}

#endif

