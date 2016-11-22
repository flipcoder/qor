#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H

#include <boost/signals2.hpp>
#include "IRealtime.h"

class StateMachine:
    virtual public IRealtime
{
    public:
        struct StateMachineState
        {
            boost::signals2::signal<void()> on_enter;
            boost::signals2::signal<void()> on_leave;
            boost::signals2::signal<void(Freq::Time)> on_tick;
            boost::signals2::signal<void(Freq::Time)> on_lazy_tick;
            std::function<bool(std::string)> on_attempt;
            boost::signals2::signal<void(std::string)> on_reject;
        };

        struct StateMachineSlot
        {
            std::string current;
            std::unordered_map<std::string, StateMachineState> states;
        };

        StateMachine() = default;
        StateMachine(StateMachine&&) = default;
        StateMachine& operator=(StateMachine&&) = default;
        //StateMachine(const StateMachine&) = default;
        //StateMachine& operator=(const StateMachine&) = default;
        ~StateMachine() {}

        void operator()(std::string slot, bool state);
        void operator()(std::string slot, int state);
        void operator()(std::string slot, std::string state);
        std::string operator()(std::string slot) const;
        void state(std::string slot, bool state);
        void state(std::string slot, int state);
        void state(std::string slot, std::string state);
        bool is_state(std::string slot) const;
        std::string state(std::string slot) const;
        
        //void change(std::string slot, std::string state);
        virtual void logic(Freq::Time t) override;
        virtual void lazy_logic(Freq::Time t) override;
        
        boost::signals2::connection on_tick(std::string slot, std::string state, std::function<void(Freq::Time)> cb){
            return m_Slots[slot].states[state].on_tick.connect(cb);
        }
        boost::signals2::connection on_lazy_tick(std::string slot, std::string state, std::function<void(Freq::Time)> cb){
            return m_Slots[slot].states[state].on_lazy_tick.connect(cb);
        }
        boost::signals2::connection on_enter(std::string slot, std::string state, std::function<void()> cb) {
            return m_Slots[slot].states[state].on_enter.connect(cb);
        }
        boost::signals2::connection on_leave(std::string slot, std::string state, std::function<void()> cb) {
            return m_Slots[slot].states[state].on_leave.connect(cb);
        }
        boost::signals2::connection on_reject(std::string slot, std::string state, std::function<void(std::string)> cb) {
            return m_Slots[slot].states[state].on_reject.connect(cb);
        }
        void on_attempt(std::string slot, std::string state, std::function<bool(std::string)> cb) {
            m_Slots[slot].states[state].on_attempt = cb;
        }

        void clear();
        void clear(std::string slot);
        size_t size() const { return m_Slots.size(); }
        bool empty() const { return m_Slots.empty(); }
        bool empty(std::string slot) const { return m_Slots[slot].states.empty(); }
        //std::string state(std::string slot) const;
            
    private:
        mutable std::unordered_map<std::string, StateMachineSlot> m_Slots;
};

#endif

