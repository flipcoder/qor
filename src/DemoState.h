#ifndef _DEMOSTATE_H_VZ3QNB09
#define _DEMOSTATE_H_VZ3QNB09

#include "State.h"
#include "Input.h"
#include "TileMap.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Mesh.h"
#include "Scene.h"
#include "Interpreter.h"
#include "Physics.h"
//#include "BasicPhysics.h"
#include "Sprite.h"
#include "ViewModel.h"

class PlayerInterface3D;

class Qor;

class DemoState:
    public State
{
    public:
        
        DemoState(Qor* engine);
        DemoState(Qor* engine, std::string fn);
        virtual ~DemoState();

        virtual void enter() override;
        virtual void preload() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

        virtual Pipeline* pipeline() {
            return m_pPipeline;
        }
        virtual const Pipeline* pipeline() const {
            return m_pPipeline;
        }
        
        virtual std::shared_ptr<Node> root() override {
            return m_pRoot;
        }
        virtual std::shared_ptr<const Node> root() const override {
            return m_pRoot;
        }
        
        virtual std::shared_ptr<Node> camera() override {
            return m_pCamera;
        }
        virtual std::shared_ptr<const Node> camera() const override {
            return m_pCamera;
        }

        virtual void camera(const std::shared_ptr<Node>& camera)override{
            m_pCamera = std::dynamic_pointer_cast<Camera>(camera);
        }
        
    private:
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;
        Pipeline* m_pPipeline = nullptr;

        std::shared_ptr<Node> m_pRoot;
        //Interpreter* m_pInterpreter;
        //std::shared_ptr<Node> m_pTemp;
        //std::shared_ptr<Sprite> m_pSprite;
        std::shared_ptr<PlayerInterface3D> m_pPlayer;
        //std::shared_ptr<TileMap> m_pMap;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Physics> m_pPhysics;
        std::shared_ptr<ViewModel> m_pViewModel;
        std::shared_ptr<Controller> m_pController;
        std::shared_ptr<Scene> m_pScene;

        std::string m_Filename;
};

#endif

