#ifndef _ACTUATION_H
#define _ACTUATION_H

#include <chrono>
#include "IRealtime.h"
#include "Common.h"
#include "StateMachine.h"
#include "kit/meta/meta.h"
#include "kit/freq/freq.h"
#include "kit/reactive/signal.h"

class Actuation:
    public IRealtime
{
    public:
        virtual ~Actuation() {}
        virtual void logic(Freq::Time t) override {
            on_tick(t);
        }
        kit::signal<void(Freq::Time)> on_tick;

        void ensure_event(std::string name);
        void event(std::string name, const std::shared_ptr<Meta>& meta);
        void event(std::string name, std::function<void(std::shared_ptr<Meta>)> func);
        void clear_events();
        bool has_event(std::string name) const;
        bool has_events() const;

        StateMachine& states() { return m_States; }
        const StateMachine& states() const { return m_States; }
        
    private:
        std::unordered_map<std::string, kit::signal<void(std::shared_ptr<Meta>)>> m_Events;
        StateMachine m_States;
};

#endif

