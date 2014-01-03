#ifndef _GAMESTATE_H_VZ3QNB09
#define _GAMESTATE_H_VZ3QNB09

#include "State.h"
#include "Input.h"
#include "TileMap.h"
#include "Camera.h"
#include "BasicPipeline.h"
#include "Mesh.h"
#include "BasicPhysics.h"
#include "Sprite.h"
#include "PlayerInterface2D.h"
#include "kit/freq/animation.h"

class Qor;

class LoadingState:
    public State
{
    public:
        
        LoadingState(Qor* qor);
        virtual ~LoadingState();

        virtual void logic(Freq::Time t) override;
        virtual void render() const override;

        virtual bool is_loader() const override {
            return true;
        }
    
    private:
        
        void fade_to(const Color& c, float t);
        
        Qor* m_pQor = nullptr;
        Window* m_pWindow = nullptr;
        Input* m_pInput = nullptr;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Mesh> m_pWaitIcon;
        std::shared_ptr<BasicPipeline> m_pPipeline;
        std::shared_ptr<Camera> m_pCamera;

        //Animation<Color> m_Animation;
        Freq::Timed<Color> m_Fade;
        float m_FadeTime;
        
        //std::shared_ptr<Node> m_pTemp;
        //std::shared_ptr<Sprite> m_pSprite;
        //std::shared_ptr<PlayerInterface2D> m_pPlayer;
        //std::shared_ptr<TileMap> m_pMap;
        //std::shared_ptr<Camera> m_pCamera;
        //std::shared_ptr<BasicPhysics> m_pPhysics;
};

#endif

