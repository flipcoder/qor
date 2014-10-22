#ifndef _TEXT_SCROLLER_H
#define _TEXT_SCROLLER_H

//#include "IRealtime.h"
//#include "IRenderable.h"
#include "Node.h"
#include "Window.h"
#include "Canvas.h"
#include "Input.h"
#include <queue>
#include <boost/signals2.hpp>
#include "kit/freq/animation.h"
#include "Sound.h"
#include "Sprite.h"

class TextScroller:
    public Node
    //public IRealtime,
    //public IPipelineRenderable
{
    public:
        
        enum Mode {
            WAIT = 0,
            TIMED
        };
        
        TextScroller(
            Window* window,
            Controller* ctrl,
            std::string font,
            Cache<Resource, std::string>* resources,
            Mode mode = WAIT
        );
        virtual ~TextScroller () {}

        virtual void logic_self(Freq::Time) override;
        virtual void render_self(Pass* pass) const override;

        void write(
            std::string portrait,
            std::string msg,
            std::function<void()> show =
                std::function<void()>(),
            std::function<void()> end = 
                std::function<void()>(),
            std::function<void(Freq::Time)> tick = 
                std::function<void(Freq::Time)>()
        );
        void write(
            std::string msg,
            std::function<void()> show =
                std::function<void()>(),
            std::function<void()> end = 
                std::function<void()>(),
            std::function<void(Freq::Time)> tick = 
                std::function<void(Freq::Time)>()
        ){
            write("",msg,show,end,tick);
        }
        
        void clear();

        bool empty() const {
            return m_Messages.empty();
        }
        operator bool() const {
            return !m_Messages.empty();
        }

        boost::signals2::signal<void(Freq::Time)> on_clear;
        
        void next_page();
        void load_portrait();
        
        bool active() const {
            return not m_Messages.empty();
        }
        
    private:
        
        Window* m_pWindow = nullptr;
        Controller* m_pController = nullptr;
        std::shared_ptr<Canvas> m_pCanvas;
        std::shared_ptr<Canvas> m_pTextCanvas;
        Cache<Resource, std::string>* m_pResources;

        struct Message {
            std::string portrait;
            std::string msg;
            std::function<void()> on_show;
            std::function<void()> on_end;
            std::function<void(Freq::Time)> on_tick;
        };
        std::queue<Message> m_Messages;
        
        Mode m_Mode = WAIT;
        Freq::Alarm m_AutoSkip;
        Freq::Time m_AutoSkipTime = Freq::Time::seconds(3);
        Freq::Timeline m_Timer;

        bool m_bActive = false;

        Animation<float> m_Drop;
        float m_fInactiveY = 0.0f;
        float m_fActiveY = 0.0f;
        int m_Height;
        //std::map<std::string, std::shared_ptr<Texture>> m_Portraits;
        std::shared_ptr<Sprite> m_pPortraitNode;
        std::string m_PortraitName;

        std::string m_Font;

        //std::unordered_map<std::string, std::shared_ptr<Sound>> m_Sounds;
};

#endif

