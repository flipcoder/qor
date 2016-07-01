#ifndef _ACTUATION_H
#define _ACTUATION_H

#include <chrono>
#include <memory>
#include "IRealtime.h"
#include "Common.h"
#include "StateMachine.h"
#include "kit/meta/meta.h"
#include "kit/freq/freq.h"
#include "kit/reactive/signal.h"
#include <boost/signals2.hpp>

class Actuation:
    public StateMachine
{
    public:
        virtual ~Actuation() {}
        virtual void logic(Freq::Time t) override;
        virtual void lazy_logic(Freq::Time t) override {
            StateMachine::lazy_logic(t);
            on_lazy_tick(t);
        }
        boost::signals2::signal<void(Freq::Time)> on_tick;
        kit::signal<void(Freq::Time)> on_lazy_tick;

        void ensure_event(std::string name);
        void event(std::string name, const std::shared_ptr<Meta>& meta = std::make_shared<Meta>());
        void event(std::string name, std::function<void(std::shared_ptr<Meta>)> func);
        void clear_event(std::string name);
        void clear_events();
        bool has_event(std::string name) const;
        bool has_events() const;

        void when(Freq::Time t, Freq::Timeline* timeline, std::function<void()> func);

        //StateMachine& states() { return m_States; }
        //const StateMachine& states() const { return m_States; }
        
    private:
        std::vector<Freq::Alarm> m_Alarms;
        std::unordered_map<std::string, kit::signal<void(std::shared_ptr<Meta>)>> m_Events;
        //StateMachine m_States;
};

#endif

