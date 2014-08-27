#include "Common.h"
#include "Input.h"
#include "Window.h"
//#include <CEGUI/System.h>
//#include <CEGUI/GUIContext.h>
//#include <CEGUI/InjectedInputReceiver.h>

Input :: Input(Window* window):
    m_pWindow(window)
{
    m_DummySwitch.make_dummy();
    
    m_bRelMouse = !m_bRelMouse; // undo initial state
    relative_mouse(!m_bRelMouse); // redo initial state change
    
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    //SDL_SetWindowGrab(SDL_GL_GetCurrentWindow(), SDL_TRUE);
}

void Input :: Switch :: trigger()
{
    // TODO: add flag to method for overwriting history or modifying front?
    m_Records.push_back(Record());
    //m_Records.emplace_back();

    for(auto& c: m_Controllers)
        if(!c.expired())
        {
            auto s = c.lock();
            if(s)
                s->trigger();
            else
                c.reset();
        }
}

void Input :: logic(Freq::Time t)
{
    SDL_Event ev;
    //auto& gui = CEGUI::System::getSingleton().getDefaultGUIContext();
    //CEGUI::System::getSingleton().injectTimePulse(t.s());
    //gui.injectTimePulse(t.s());
    m_MouseRel = glm::ivec2();
    
    while(SDL_PollEvent(&ev))
    {
        switch(ev.type)
        {
            case SDL_QUIT:
                m_bQuit = true;
                break;

            case SDL_WINDOWEVENT:
                if(ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    m_pWindow->on_resize(glm::ivec2(ev.window.data1, ev.window.data2));
                break;

            case SDL_KEYDOWN:
                m_Devices[KEYBOARD][0][ev.key.keysym.sym] = true;
                //gui.injectKeyDown((CEGUI::Key::Scan)ev.key.keysym.scancode);
                break;

            case SDL_KEYUP:
                m_Devices[KEYBOARD][0][ev.key.keysym.sym] = false;
                //gui.injectKeyUp((CEGUI::Key::Scan)ev.key.keysym.scancode);
                break;

            case SDL_TEXTINPUT:
            {
                unsigned idx=0;
                //while(ev.text.text[idx++])
                    //gui.injectChar(ev.text.text[idx]);
                break;
            }
            case SDL_MOUSEMOTION:
                if(m_bRelMouse)
                    m_MouseRel += glm::vec2(ev.motion.xrel, ev.motion.yrel);
                else
                    m_MouseRel = m_MousePos;
                
                m_MousePos = glm::vec2(ev.motion.x, -ev.motion.y);
                
                if(!m_bRelMouse)
                    m_MouseRel = m_MousePos - m_MouseRel; // simulate mouse rel
                
                //gui.injectMousePosition(
                //    static_cast<float>(ev.motion.x),
                //    static_cast<float>(ev.motion.y)
                //);
                break;

            case SDL_MOUSEBUTTONDOWN:
            {
                if(ev.button.button == SDL_BUTTON_LEFT) {
                    m_Devices[MOUSE][0][0] = true;
                    //gui.injectMouseButtonDown(CEGUI::MouseButton::LeftButton);
                }
                else if(ev.button.button == SDL_BUTTON_RIGHT){
                    m_Devices[MOUSE][0][1] = true;
                    //gui.injectMouseButtonDown(CEGUI::MouseButton::RightButton);
                }
                else if(ev.button.button == SDL_BUTTON_MIDDLE){
                    m_Devices[MOUSE][0][2] = true;
                    //gui.injectMouseButtonDown(CEGUI::MouseButton::MiddleButton);
                }

                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(ev.button.button == SDL_BUTTON_LEFT){
                    m_Devices[MOUSE][0][0] = false;
                    //gui.injectMouseButtonUp(CEGUI::MouseButton::LeftButton);
                }
                else if(ev.button.button == SDL_BUTTON_RIGHT){
                    m_Devices[MOUSE][0][1] = false;
                    //gui.injectMouseButtonUp(CEGUI::MouseButton::RightButton);
                }
                else if(ev.button.button == SDL_BUTTON_MIDDLE){
                    m_Devices[MOUSE][0][2] = false;
                    //gui.injectMouseButtonUp(CEGUI::MouseButton::MiddleButton);
                }
                break;
            }
            //case SDL_WINDOWEVENT_RESIZED:
            //    break;
        }
    }

    for(auto& types: m_Devices)
        for(auto& devs: types.second)
            for(auto& c: devs.second)
                c.second.logic(t);
    
    //for(auto& c: m_Mouse)
    //    c.second.logic(t);
    //for(auto& c: m_Devices[KEYBOARD][0])
    //    c.second.logic(t);
    
    for(auto& c: m_Controllers)
    {
        if(c.second->triggered())
        {
            c.second->event();
            c.second->untrigger();
        }
        c.second->logic(t);
    }

    if(m_bRelMouse)
        SDL_WarpMouseInWindow(
            m_pWindow->sdl_window(),
            m_pWindow->center().x/2,
            m_pWindow->center().y/2
        );
}

unsigned int Input :: bind(
    std::string s,
    const std::shared_ptr<Controller>& controller
){
    // first token might be device name
    // if not, device is keyboard
    std::string device = s.substr(0,s.find(' ')+1);
    if(s.substr(0,s.find(' ')) == "mouse")
    {
        std::string device = s.substr(0,s.find(' '));
        std::string button = s.substr(device.length()+1);
        button = button.substr(0, button.find(' '));
        unsigned id = 0;
        if(button == "left")
            id = 0;
        else if(button == "right")
            id = 1;
        else if(button == "middle")
            id = 2;
        else
        {
            // use number
            id = boost::lexical_cast<unsigned>(button);
        }
        m_Binds.push_back(Bind(MOUSE, 0, id));
        m_Devices[MOUSE][0][id].plug(controller);
    }
    else
    {
        unsigned int id = SDL_GetKeyFromName(s.c_str());
        if(id == SDLK_UNKNOWN)
            ERRORf(ACTION, "bind key %s", s)
        m_Binds.push_back(id);
        m_Devices[KEYBOARD][0][id].plug(controller);
    }
    return m_Binds.size()-1;
}


void Controller :: rumble(float magnitude, Freq::Time t)
{
    // TODO: add rumble support
    // http://wiki.libsdl.org/moin.cgi/CategoryForceFeedback

    //SDL_HapticRumblePlay(m_pHaptic, magnitude, t.ms());
}

