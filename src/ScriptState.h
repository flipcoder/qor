#ifndef _SCRIPTSTATE_H_VZ3QNB09
#define _SCRIPTSTATE_H_VZ3QNB09

#include "State.h"
#include "Input.h"
#include "TileMap.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Mesh.h"
#include "Interpreter.h"
#include "Physics.h"
//#include "BasicPhysics.h"
#include "Sprite.h"
#include "PlayerInterface2D.h"

class Qor;

class ScriptState:
    public State
{
    public:
        
        ScriptState(Qor* engine);
        ScriptState(Qor* engine, std::string fn);
        virtual ~ScriptState();

        virtual void preload() override;
        virtual void logic(Freq::Time t) override;
        virtual void render() const override;
        virtual bool needs_load() const override {
            return true;
        }

        virtual std::shared_ptr<Pipeline> pipeline() {
            return m_pPipeline;
        }
        virtual std::shared_ptr<const Pipeline> pipeline() const {
            return m_pPipeline;
        }
        
        virtual std::shared_ptr<Node> root() override {
            return m_pRoot;
        }
        virtual std::shared_ptr<const Node> root() const override {
            return m_pRoot;
        }
        
    private:
        
        Qor* m_pQor = nullptr;
        Input* m_pInput = nullptr;

        std::shared_ptr<Node> m_pRoot;
        Interpreter* m_pInterpreter;
        std::shared_ptr<Interpreter::Context> m_pScript;
        //std::shared_ptr<Node> m_pTemp;
        //std::shared_ptr<Sprite> m_pSprite;
        //std::shared_ptr<PlayerInterface2D> m_pPlayer;
        //std::shared_ptr<TileMap> m_pMap;
        std::shared_ptr<Camera> m_pCamera;
        std::shared_ptr<Pipeline> m_pPipeline;
        std::shared_ptr<Physics> m_pPhysics;

        std::string m_Filename;
};

#endif

