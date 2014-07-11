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

class TextScroller:
    public Node
    //public IRealtime,
    //public IPipelineRenderable
{
    public:
        
        TextScroller(
            Window* window,
            Controller* ctrl,
            Cache<Resource, std::string>* resources
        );
        virtual ~TextScroller () {}

        virtual void logic_self(Freq::Time) override;
        virtual void render_self(Pass* pass) const override;

        void write(
            std::string,
            std::function<void(Freq::Time)> show =
                std::function<void(Freq::Time)>(),
            std::function<void(Freq::Time)> end = 
                std::function<void(Freq::Time)>(),
            std::function<void(Freq::Time)> tick = 
                std::function<void(Freq::Time)>()
        );
        void clear();

        bool empty() const {
            return m_Messages.empty();
        }
        operator bool() const {
            return !m_Messages.empty();
        }

        boost::signals2::signal<void(Freq::Time)> on_clear;
        
    private:
        
        Window* m_pWindow = nullptr;
        Controller* m_pController = nullptr;
        std::shared_ptr<Canvas> m_pCanvas;
        std::shared_ptr<Canvas> m_pTextCanvas;

        struct Message {
            std::string msg;
            std::function<void(Freq::Time)> on_show;
            std::function<void(Freq::Time)> on_end;
            std::function<void(Freq::Time)> on_tick;
        };
        std::queue<Message> m_Messages;

        bool m_bActive = false;

        Animation<float> m_Drop;
        float m_fInactiveY = 0.0f;
        float m_fActiveY = 0.0f;

        //std::unordered_map<std::string, std::shared_ptr<Sound>> m_Sounds;
};

#endif

