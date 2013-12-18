#ifndef _GUI_H
#define _GUI_H

#include <Rocket/Core.h>
#include <memory>

class Freq;
class Window;

class GUI:
    public Rocket::Core::SystemInterface
{
    public:
        
        GUI(Freq* timer, Window* window);
        virtual ~GUI() {}

        virtual float GetElapsedTime() override;
        
    private:
        
        Freq* m_pTimer;
        Window* m_pWindow;
        Rocket::Core::Context* m_pContext;
};

#endif

