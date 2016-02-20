#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H

#include <boost/signals2.hpp>
#include "IRealtime.h"

class StateMachine:
    public IRealtime
{
    public:
        struct StateMachineState
        {
            boost::signals2::signal<void()> on_enter;
            boost::signals2::signal<void()> on_leave;
            boost::signals2::signal<void(Freq::Time)> on_tick;
            std::function<bool()> on_attempt;
        };

        StateMachine() = default;
        StateMachine(std::string initial){
            change(initial);
        }
        StateMachine(StateMachine&&) = default;
        StateMachine(const StateMachine&) = default;
        StateMachine& operator=(const StateMachine&) = default;
        StateMachine& operator=(StateMachine&&) = default;
        ~StateMachine() {}

        void change(std::string state);
        virtual void logic(Freq::Time t) override;
        
        boost::signals2::connection on_tick(std::string state, std::function<void(Freq::Time)> cb){
            return m_States.at(state).on_tick.connect(cb);
        }
        boost::signals2::connection on_enter(std::string state, std::function<void()> cb) {
            return m_States.at(state).on_enter.connect(cb);
        }
        boost::signals2::connection on_leave(std::string state, std::function<void()> cb) {
            return m_States.at(state).on_leave.connect(cb);
        }
        void on_attempt(std::string state, std::function<bool()> cb) {
            m_States.at(state).on_attempt = cb;
        }

        void clear();
        size_t size() const { return m_States.size(); }
        bool empty() const { return m_States.empty(); }
        std::string state() const { return m_Current; }
        
    private:
        std::string m_Current;
        std::unordered_map<std::string, StateMachineState> m_States;
};

#endif

