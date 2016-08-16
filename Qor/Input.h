#ifndef _INPUT_H_MQD6JJWN
#define _INPUT_H_MQD6JJWN

#include <glm/glm.hpp>
#include <memory>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "IRealtime.h"
#include "kit/log/log.h"
#include <boost/circular_buffer.hpp>
#include <boost/signals2.hpp>
#include <boost/lexical_cast.hpp>

class Window;
class Controller;

class Input:
    public IRealtime
{
    public:

        enum Device{
            KEYBOARD = 0,
            MOUSE,
            GAMEPAD
        };
        
        struct Bind {
            Device type = KEYBOARD;
            unsigned device_id = 0;
            unsigned id = 0;
            Bind(unsigned btn_id):
                id(btn_id)
            {}
            Bind(Device typ, unsigned btn_id):
                type(typ),
                id(btn_id)
            {}
            Bind(Device typ, unsigned dev_id, unsigned btn_id):
                type(typ),
                device_id(dev_id),
                id(btn_id)
            {}
            Bind(const Bind&) = default;
            Bind(Bind&&) = default;
            Bind& operator=(const Bind&) = default;
            Bind& operator=(Bind&&) = default;
        };

        /*
         * Pressure-sensitive switch for an input device
         */
        struct Switch:
            public IRealtime
        {
            public:
                enum When {
                    //NEVER=-1, // switch never pressed
                    NONE=0, // pre-NOW value (automatically switches to NOW)
                    NOW=1,
                    BEFORE=2,
                    MAX
                };

                struct Record {
                    // TODO: move in Switch's methods for testing these fields
                    Freq::Time time = Freq::Time();
                    When when = NONE;
                    float diff = 1.0f; // 1.0f=press, -1.0f=release
                    bool release() const {
                        return diff < 0.0f;
                    }
                    bool press() const {
                        return diff > 0.0f;
                    }
                };

                Switch(bool b = false):
                    m_Pressure(b ? 1.0f : 0.0f)
                {
                    m_Records.set_capacity(1);
                }
                Switch(float p):
                    m_Pressure(kit::saturate(p))
                {
                    m_Records.set_capacity(1);
                }
                //Switch(const Switch&) = default;
                //Switch& operator=(const Switch&) = default;

                bool dummy() { return m_bDummy; }
                void make_dummy() { m_bDummy = true; }

                Switch& operator=(bool b)
                {
                    assert(!m_bDummy);
                    if(m_bDummy)
                        return *this;

                    m_Pressure = b ? 1.0f : 0.0f;
                    trigger();
                    return *this;
                }

                //Switch& operator=(float p)
                //{
                //    assert(!m_bDummy);
                //    if(m_bDummy)
                //        return *this;

                //    float diff = std::fabs(m_Pressure - p);
                //    if(diff  >= m_DeltaThreshold) {
                //        // assign pressure only on trigger
                //        // otherwise, we'd need to accumulate p
                //        m_Pressure = p;
                //        trigger();
                //    }
                //    return *this;
                //}

                bool operator<(float p) const { return m_Pressure < p; }
                bool operator<=(float p) const { return m_Pressure <= p; }
                bool operator>(float p) const { return m_Pressure > p; }
                bool operator>=(float p) const { return m_Pressure >= p; }
                
                operator bool() const {
                    return m_Pressure > m_ActivationThreshold;
                }

                float pressure() const { return m_Pressure; }
                void pressure(float f) {
                    f = kit::saturate(f);
                    float old = m_Pressure;
                    //float diff = std::fabs(m_Pressure - f);
                    m_Pressure = f;
                    //if(diff  >= m_DeltaThreshold) {
                        // assign pressure only on trigger
                        // otherwise, we'd need to accumulate p
                        if(old < m_ActivationThreshold && m_Pressure >=m_ActivationThreshold)
                            trigger();
                        else if(old >= m_ActivationThreshold && m_Pressure < m_ActivationThreshold)
                            trigger();
                    //}
                }
                //std::shared_ptr<Controller> controller() {
                //    return m_pController.lock();
                //}
            
                /*
                 * Just increments the time since the switch was changed beyond
                 *   the delta threshold
                 *
                 * This is called by Controller logic()
                 * Thus, only switches currently assigned to controllers have
                 *   internal ticking timers
                 * Switches no longer assigned to controls have frozen timers
                 */
                virtual void logic(Freq::Time t) override {
                    if(m_bDummy)
                        return;

                    if(!m_Records.empty())
                    {
                        if(record().when < BEFORE)
                            record().when = When((int)record().when + 1);
                    }

                    for(auto& r: m_Records)
                        r.time += t;
                }

                bool pressed() const { return *this; }
                //bool released() const { return *this; }
                bool pressed_now() const {
                    return *this && (!empty() && record().when==NOW);
                }
                bool released_now() const {
                    return (!*this) && (!empty() && record().when==NOW);
                }
                bool now() const {return record().when==NOW;}

                // silence any preceding pressed_now() checks
                bool consume_now() {
                    bool b = pressed_now();
                    if(b)
                        record().when = When((int)record().when + 1);
                    return b;
                }
                bool consume() {
                    bool b = pressed_now();
                    if(b) {
                        record().when = When((int)record().when + 1);
                        m_Pressure = 0.0f;
                    }
                    return b;
                }
                
                Freq::Time time() {
                    return record().time;
                }
                const Switch::Record& history(unsigned int idx = 0) const {
                    if(idx >= m_Records.size())
                        throw std::out_of_range("invalid time index");
                    return m_Records.at(idx);
                }

                void plug(const std::shared_ptr<Controller>& c) {
                    auto weak = std::weak_ptr<Controller>(c);
                    m_Controllers.push_back(weak);
                }

                size_t record_history() const {
                    return m_Records.capacity();
                }
                void record_history(unsigned sz) {
                    m_Records.set_capacity(sz);
                }
                void clear_history() {
                    m_Records.resize(1);
                }

                /*
                 * Number of times key was pressed within given time frame
                 * Maximum will not exceed history capacity
                 */
                int times_pressed(Freq::Time since);

                bool empty() const { return m_Records.empty(); }
 
            private:
                
                Record& record() {
                    assert(!m_Records.empty());
                    return m_Records.front();
                }
                const Record& record() const {
                    assert(!m_Records.empty());
                    return m_Records.front();
                }

                //Record m_Record;
                boost::circular_buffer<Record> m_Records;

                /*
                 * Controller has been updated, so trigger callbacks
                 */
                void trigger();

                float m_Pressure = 0.0f;

                // the pressure point between "active" and "inactive"
                const float m_ActivationThreshold = 0.25f;

                // minimum amount of change to justify an update
                //const float m_DeltaThreshold = 0.1f;

                std::vector<std::weak_ptr<Controller>> m_Controllers;

                // amount of additional timestamps to keep track of
                // TODO: holding all this data again just for a snapshot of the
                // event is useless.  Instead just hold m_When and m_Time
                //boost::circular_buffer<Switch> m_Records;

                /*
                 * Keeps track of "when" the switch was changed in terms of the
                 * frame.
                 */

                bool m_bDummy = false;

                // TODO: callbacks
                //Device* device = nullptr;
        };

        Input(Window*);
        virtual ~Input();

        void logic(Freq::Time t) override;

        /*
         * Returns bind index for key specified as string
         * Pass it to button() to get the switch
         */
        unsigned int bind(
            std::string s,
            const std::shared_ptr<Controller>& controller
        );

        Switch& mouse(unsigned int idx) {
            try{
                return m_Devices[MOUSE][0].at(idx);
            }catch(const std::out_of_range&){
                return m_DummySwitch;
            }
        }
        const Switch& mouse(unsigned int idx) const {
            try{
                return m_Devices[MOUSE][0].at(idx);
            }catch(const std::out_of_range&){
                return m_DummySwitch;
            }
        }
        
        Switch& key(unsigned int idx) {
            try{
                return m_Devices[KEYBOARD][0].at(idx);
            }catch(const std::out_of_range&){
                return m_DummySwitch;
            }
        }
        const Switch& key(unsigned int idx) const {
            try{
                return m_Devices[KEYBOARD][0].at(idx);
            }catch(const std::out_of_range&){
                return m_DummySwitch;
            }
        }
        Switch& key(std::string s) {
            unsigned int id = SDL_GetKeyFromName(s.c_str());
            if(id == SDLK_UNKNOWN)
                return m_DummySwitch;
            return key(id);
        }
        const Switch& key(std::string s) const {
            unsigned int id = SDL_GetKeyFromName(s.c_str());
            if(id == SDLK_UNKNOWN)
                return m_DummySwitch;
            return key(id);
        }
        
        //bool key_once(unsigned int idx) const {
        //    //auto i = m_Devices[KEYBOARD][0].find(idx);
        //    //if(i == m_Devices[KEYBOARD][0].end())
        //    //    return false;
        //    //m_Devices[KEYBOARD][0][idx] = false;
        //    return m_Devices[KEYBOARD][0][idx].once();
        //}
        
        /*
         * Pass in a bound ID, returns the switch
         */
        Switch& button(unsigned int idx) {
            try{
                auto& b = m_Binds.at(idx);
                return m_Devices.at(b.type).at(b.device_id).at(b.id);
            }catch(const std::out_of_range&){
                return m_DummySwitch;
            }
        }
        const Switch& button(unsigned int idx) const {
            try{
                auto& b = m_Binds.at(idx);
                return m_Devices.at(b.type).at(b.device_id).at(b.id);
            }catch(const std::out_of_range&){
                return m_DummySwitch;
            }
        }
        //bool button_once(unsigned int idx) const {
        //    return m_Devices[KEYBOARD][0][m_Binds.at(idx)].once();
        //}

        void quit() { m_bQuit = true; }
        bool quit_flag() const { return m_bQuit; }

        /*
         * Associate a controller to a player
         * Replaces any controller with the same ID
         * Returns true if another controller needed to be removed
         *
         * This function sucks because Controllers can call shared_this()
         * in constructors.  Use new plug(id) instead
         */
        //bool plug(unsigned int id, std::shared_ptr<Controller>& controller) {
        //    bool exists = (m_Controllers.find(id) != m_Controllers.end());
        //    m_Controllers[id] = controller;
        //    return exists; // was controller replaced?
        //}

        std::shared_ptr<Controller> plug(unsigned int id) {
            auto c = std::make_shared<Controller>(id,this);
            m_Controllers[id] = c;
            return c;
        }
        void unplug(
            unsigned int id = std::numeric_limits<unsigned int>::max()
        ){
            if(id == std::numeric_limits<unsigned int>::max())
            {
                m_Controllers.clear();
                return;
            }
            m_Controllers.erase(id);
        }

        Switch& dummy_switch() { return m_DummySwitch; }
        const Switch& dummy_switch() const { return m_DummySwitch; }

        glm::vec2 mouse_rel() const {
            return m_MouseRel;
        }

        glm::vec2 mouse_pos() const {
            return m_MousePos;
        }
        void mouse_pos(glm::vec2 mp) const {
            SDL_WarpMouseInWindow(m_pWindow, std::max(0.0f, mp.x), glm::max(0.0f, mp.y));
        }

        void relative_mouse(bool b) {
            if(b != m_bRelMouse) {
                SDL_SetRelativeMouseMode(b ? SDL_TRUE : SDL_FALSE);
                m_bRelMouse = b;
            }
        }
        
        bool relative_mouse() const {
            return m_bRelMouse;
        }
        
        static unsigned gamepad_hat_id(unsigned id);
        static unsigned gamepad_analog_id(unsigned id);
        
        bool escape() const {
            return m_bEscape;
        }

        enum Listen {
            LISTEN_NONE,
            LISTEN_TEXT,
            LISTEN_KEY
        };

        void listen(
            Listen mode,
            std::shared_ptr<std::string> text = std::shared_ptr<std::string>(),
            std::function<void(bool,bool)> cb = std::function<void(bool,bool)>()
        );
        void abort_listen();
        
        void gamepad_require_focus(bool b);
        void mouse_require_focus(bool b);
        
    private:
        
        bool m_bRelMouse = false;
        int m_Listen = LISTEN_NONE;
        std::weak_ptr<std::string> m_ListenText;
        std::function<void(bool,bool)> m_ListenCallback;

        // These are mutable so methods can return a non-nullable Switch& which
        //   will be blank on creation in the map
        mutable std::unordered_map<unsigned,
            std::unordered_map<unsigned,
                std::unordered_map<unsigned, Switch>
            >
        > m_Devices;
        //mutable std::unordered_map<unsigned int, Switch> m_Mouse;
        //mutable std::unordered_map<unsigned int, Switch> m_Devices[KEYBOARD][0];

        //std::vector<Device> m_Devices;

        // This will have a device index in the future, but for now its just
        std::vector<Bind> m_Binds;

        //std::map<unsigned int, std::weak_ptr<Switch>> m_Switches;

        bool m_bQuit = false;
        std::unordered_map<unsigned int, std::shared_ptr<Controller>> m_Controllers;
        glm::vec2 m_MousePos;
        glm::vec2 m_MouseRel;

        Switch m_DummySwitch;
        Window* m_pWindow;

        std::vector<SDL_Joystick*> m_Joysticks;

        std::atomic<bool> m_bEscape = ATOMIC_VAR_INIT(false);
};

// Inherit from this to make interfaces to controllable objects
class IInterface:
    public IRealtime
{
    public:
        virtual ~IInterface() {}
        virtual void event() = 0;
    private:
};


class Controller:
    public IRealtime,
    public std::enable_shared_from_this<Controller>
{
    public:
        Controller(unsigned int id, Input* input):
            m_ID(id),
            m_pInput(input)
        {
            assert(input);
        }
        virtual ~Controller() {}

        std::map<std::string, std::string> binds() const;
        
        Input::Switch& button(unsigned int idx) {
            return m_pInput->button(m_Binds.at(idx));
        }
        Input::Switch& button(std::string b) {
            try{
                return m_pInput->button(m_Binds.at(button_id(b)));
            }catch(const std::out_of_range&){
                return m_pInput->dummy_switch();
            }
        }
        const Input::Switch& button(unsigned int idx) const {
            try{
                return m_pInput->button(m_Binds.at(idx));
            }catch(const std::out_of_range&){
                return m_pInput->dummy_switch();
            }
        }
        const Input::Switch& button(std::string b) const {
            try{
                return m_pInput->button(m_Binds.at(button_id(b)));
            }catch(const std::out_of_range&){
                return m_pInput->dummy_switch();
            }
        }
        void clear_binds() {
            m_Binds.clear();
        }

        unsigned int bind(
            const std::string& key,
            const std::string& name = std::string()
        ){
            // below call may throw
            m_Binds.push_back(m_pInput->bind(key, shared_from_this()));
            m_BindNames[name].insert(m_Binds.size()-1);
            //m_BindNames.push_back(name);

            //assert(m_Binds.size() == m_BindNames.size());
            return m_Binds.size()-1;
        }

        // FIXME: Only returns single binds
        // How about a composite id for use with button()
        // button(composite_id) -> returns "higher" pressure switch state
        // Also can return empty which is an unbound button and always
        // returns the dummy switch (?)
        unsigned button_id(const std::string& s) const {
            auto bind = m_BindNames.find(s);
            if(bind == m_BindNames.end())
                throw std::out_of_range("button \"" + s + "\"");
            return *bind->second.begin(); // TEMP: first ID assigned to name
        }

        //bool button_once(unsigned int idx) {
        //    try{
        //        return m_pInput->button_once(m_Buttons.at(idx));
        //    }catch(const std::out_of_range&){
        //        return false;
        //    }
        //}

        bool triggered() const { return m_bTrigger; }
        void trigger() { m_bTrigger = true; }
        void untrigger() { m_bTrigger = false; }

        virtual void event() {
            // TODO: call controller update callbacks here
            for(auto itr = m_Interfaces.begin();
                itr != m_Interfaces.end();
            ){
                auto inter = itr->lock();
                if(!inter)
                {
                    itr = m_Interfaces.erase(itr);
                    continue;
                }
                inter->event();
                ++itr;
            }
        }

        virtual void logic(Freq::Time t) override {
            // for each switch, increment the time
            // logic() will bail if switch is inactive

            //for(auto i: m_Binds)
            //    button(i).logic(t);

            for(auto i: m_Interfaces)
            {
                if(i.expired())
                    continue;
                auto s = i.lock();
                if(s)
                    s->logic(t);
                else
                    i.reset();
            }
        }

        void clear_history() {
            // TODO: loop through all switches and clear the history
        }

        virtual void rumble(float magnitude, Freq::Time t);

        unsigned int add_interface(
            const std::shared_ptr<IInterface>& inter
        ){
            m_Interfaces.push_back(inter);
            return m_Interfaces.size()-1;
        }

        void remove_interface(
            unsigned int idx
        ){
            // this is not ideal
            // TODO: use the "unused ID" system for this
            assert(idx < m_Interfaces.size());
            m_Interfaces[idx] = std::weak_ptr<IInterface>();
        }

        Input* input() { return m_pInput; }
        const Input* input() const { return m_pInput; }

    private:

        // Need a pointer to primary input device for rumble support
        //std::weak_ptr<Device> m_pDevice;
        Input* m_pInput;
        unsigned int m_ID;
        bool m_bTrigger = false;

        // ID -> Bind ID in input system
        std::vector<unsigned int> m_Binds;
        std::unordered_map<
            std::string, std::unordered_set<unsigned>
        > m_BindNames;
        //std::vector<std::string> m_BindNames;
        std::vector<std::weak_ptr<IInterface>> m_Interfaces;

        //boost::signals2::signal<void()> 
};


#endif

