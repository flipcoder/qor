#include "Common.h"
#include "Input.h"
#include "Window.h"
#include <memory>
using namespace std;

Input :: Input(Window* window):
    m_pWindow(window)
{
    m_DummySwitch.make_dummy();
    
    m_bRelMouse = !m_bRelMouse; // undo initial state
    relative_mouse(!m_bRelMouse); // redo initial state change
    
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    //SDL_SetWindowGrab(SDL_GL_GetCurrentWindow(), SDL_TRUE);
    for(unsigned i=0; i<SDL_NumJoysticks(); ++i)
    {
        m_Joysticks.push_back(SDL_JoystickOpen(i));
    }
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_StopTextInput();
}

Input :: ~Input()
{
    for(auto*& j: m_Joysticks)
    {
        SDL_JoystickClose(j);
        j = nullptr;
    }
    m_Joysticks.clear();
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

    if(m_Listen && m_ListenText.expired())
    {
        kit::clear(m_ListenCallback); // probably no longer safe
        listen(LISTEN_NONE);
    }
    
    while(SDL_PollEvent(&ev))
    {
        switch(ev.type)
        {
            case SDL_QUIT:
                m_bQuit = true;
                break;

            case SDL_WINDOWEVENT:
                //if(ev.window.event == SDL_WINDOWEVENT_RESIZED)
                //    m_pWindow->resize(glm::ivec2(ev.window.data1, ev.window.data2));
                break;

            case SDL_KEYDOWN:
                
                m_Devices[KEYBOARD][0][ev.key.keysym.sym] = true;
                if(ev.key.keysym.sym == SDLK_ESCAPE)
                {
                    if(m_Listen)
                        listen(LISTEN_NONE);
                    else
                        m_bEscape = true;
                }
                else if(ev.key.keysym.sym == SDLK_RETURN)
                {
                    if(m_Listen == LISTEN_TEXT)
                    {
                        listen(LISTEN_NONE);
                        m_Devices[KEYBOARD][0][ev.key.keysym.sym] = false;
                    }
                }
                else if(ev.key.keysym.sym == SDLK_BACKSPACE)
                {
                    if(m_Listen == LISTEN_TEXT)
                    {
                        auto lt = m_ListenText.lock();
                        if(not lt->empty())
                            *lt = lt->substr(0,lt->size()-1);
                        if(m_ListenCallback)
                            m_ListenCallback(false);
                    }
                }
                else if(m_Listen == LISTEN_KEY)
                {
                    auto lt = m_ListenText.lock();
                    *lt  = SDL_GetKeyName(ev.key.keysym.sym);
                    listen(LISTEN_NONE);
                    m_Devices[KEYBOARD][0][ev.key.keysym.sym] = false;
                }
                
                //gui.injectKeyDown((CEGUI::Key::Scan)ev.key.keysym.scancode);
                break;

            case SDL_KEYUP:
                m_Devices[KEYBOARD][0][ev.key.keysym.sym] = false;
                if(ev.key.keysym.sym == SDLK_ESCAPE)
                    m_bEscape = false;
                //gui.injectKeyUp((CEGUI::Key::Scan)ev.key.keysym.scancode);
                break;

            case SDL_DROPFILE:
                // string fn = ev.drop.file;
                // SDL_free(ev.drop.file);
                break;
                
            case SDL_JOYHATMOTION:
            {
                // 8 bits for hat id, 4 bits for each direction
                // yes, we could store dirs in just 2 bits, but they wouldn't
                //   have button-compatible mappings
                // direction bits are stored in this order:
                //   0 - left
                //   1 - right
                //   2 - up
                //   3 - down
                //unsigned id = (1<<12) + (unsigned(ev.jhat.hat) << 4);
                unsigned id = gamepad_hat_id(ev.jhat.hat << 4);
                //if(ev.jhat.value == SDL_HAT_CENTERED)
                //{
                    //// centering invalidates all other directions
                    //for(unsigned i=0; i<4; ++i) {
                    //    auto& dir = m_Devices[GAMEPAD][ev.jhat.which][id+i];
                    //    if(dir) dir = false;
                    //}
                //}
                //else 
                //{
                if(m_Listen == LISTEN_NONE)
                {
                    auto& left = m_Devices[GAMEPAD][ev.jhat.which][id];
                    auto& right = m_Devices[GAMEPAD][ev.jhat.which][id+1];
                    auto& up = m_Devices[GAMEPAD][ev.jhat.which][id+2];
                    auto& down = m_Devices[GAMEPAD][ev.jhat.which][id+3];
                    if(ev.jhat.value & SDL_HAT_LEFT)
                        left = true;
                    else if(left)
                        left = false;
                    if(ev.jhat.value & SDL_HAT_RIGHT)
                        right = true;
                    else if(right)
                        right = false;
                    
                    if(ev.jhat.value & SDL_HAT_UP)
                        up = true;
                    else if(up)
                        up = false;
                    if(ev.jhat.value & SDL_HAT_DOWN)
                        down = true;
                    else if(down)
                        down = false;
                } else if (m_Listen == LISTEN_KEY) {
                    auto lt = m_ListenText.lock();
                    *lt = string("gamepad ") +
                        to_string(ev.jhat.which) +
                        " hat " + to_string(id);
                    listen(LISTEN_NONE);
                }
                //}
                
                break;
            }

            case SDL_JOYAXISMOTION:
            {
                float val = (((int)ev.jaxis.value + 32768) + 0.5f) / 32767.0f - 1.0f;
                unsigned id = gamepad_analog_id(ev.jaxis.axis << 1);
                if(val >= 0.0f)
                {
                    //LOGf("pressure: %s", val);
                    m_Devices[GAMEPAD][ev.jaxis.which][id] = false;
                    m_Devices[GAMEPAD][ev.jaxis.which][id+1].pressure(val);
                }
                else
                {
                    //LOGf("pressure: %s", -val);
                    m_Devices[GAMEPAD][ev.jaxis.which][id].pressure(-val);
                    m_Devices[GAMEPAD][ev.jaxis.which][id+1] = false;
                }
                break;
            }
            case SDL_JOYBUTTONDOWN:
                //LOGf("gamepad%s %s", int(ev.jbutton.which) % int(ev.jbutton.button))
                m_Devices[GAMEPAD][ev.jbutton.which][ev.jbutton.button] = true;
                break;
                
            case SDL_JOYBUTTONUP:
                m_Devices[GAMEPAD][ev.jbutton.which][ev.jbutton.button] = false;
                break;

            case SDL_TEXTINPUT:
            {
                assert(SDL_IsTextInputActive());
                auto lt = m_ListenText.lock();
                *lt += ev.text.text;
                //LOG(ev.text.text);
                if(m_ListenCallback)
                    m_ListenCallback(false);
                break;
            }
            case SDL_TEXTEDITING:
            {
                assert(SDL_IsTextInputActive());
                //auto lt = m_ListenText.lock();
                //*lt = ev.edit.text;
                //LOGf("text: %s", ev.edit.text);
                //LOGf("length: %s", to_string(ev.edit.length));
                //LOGf("start: %s", to_string(ev.edit.start));
                //if(m_ListenCallback)
                //    m_ListenCallback(false);
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
    std::string device = s.substr(0,s.find(' '));
    if(device == "mouse")
    {
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
    else if(device == "gamepad")
    {
        //LOG("gamepad")
        bool analog = false;
        bool hat = false;
        std::string button = s.substr(device.length()+1);
        if(boost::starts_with(button,"analog")){
            //LOG("analog");
            analog = true;
            button = button.substr(strlen("analog")+1);
        }else if(boost::starts_with(button,"hat")){
            //LOG("hat");
            hat = true;
            button = button.substr(strlen("hat")+1);
        }

        //LOGf("button %s", button);
        unsigned id = boost::lexical_cast<unsigned>(button);
        if(analog) id = Input::gamepad_analog_id(id);
        if(hat) id = Input::gamepad_hat_id(id);
        m_Binds.push_back(Bind(GAMEPAD, 0, id));
        m_Devices[GAMEPAD][0][id].plug(controller);
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

unsigned Input :: gamepad_analog_id(unsigned id)
{
    return (1<<12) + id;
}

unsigned Input :: gamepad_hat_id(unsigned id)
{
    return (1<<16) + id;
}

void Input :: listen(
    Listen mode,
    std::shared_ptr<std::string> text,
    std::function<void(bool)> cb
){
    if(mode == m_Listen)
        return;
    
    if(mode == LISTEN_KEY) {
        m_ListenText = weak_ptr<string>(text);
        m_ListenCallback = std::move(cb);
    } else if(mode == LISTEN_TEXT) {
        SDL_StartTextInput();
        m_ListenText = weak_ptr<string>(text);
        m_ListenCallback = std::move(cb);
    } else if(mode == LISTEN_NONE) {
        if(m_Listen == LISTEN_TEXT) {
            SDL_StopTextInput();
            if(m_ListenCallback)
                m_ListenCallback(true);
            m_ListenText = weak_ptr<string>();
        } else if(m_Listen == LISTEN_KEY) {
            if(m_ListenCallback)
                m_ListenCallback(true);
            m_ListenText = weak_ptr<string>();
        }
    }
    m_Listen = mode;
}

void Controller :: rumble(float magnitude, Freq::Time t)
{
    // TODO: add rumble support
    // http://wiki.libsdl.org/moin.cgi/CategoryForceFeedback

    //SDL_HapticRumblePlay(m_pHaptic, magnitude, t.ms());
}

