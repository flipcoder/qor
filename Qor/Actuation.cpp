#include "Actuation.h"
using namespace std;
using namespace glm;

void Actuation :: ensure_event(std::string name)
{
    auto itr = m_Events.find(name);
    if(itr == m_Events.end())
        m_Events[name] = kit::signal<void(std::shared_ptr<Meta>)>();
}

void Actuation :: event(std::string name, const std::shared_ptr<Meta>& meta)
{
    auto itr = m_Events.find(name);
    if(itr != m_Events.end())
        itr->second(meta);
}

void Actuation :: event(std::string name, std::function<void(std::shared_ptr<Meta>)> func)
{
    m_Events[name].connect(func);
}

void Actuation :: clear_events()
{
    m_Events.clear();
}

void Actuation :: clear_event(std::string name)
{
    m_Events[name] = kit::signal<void(std::shared_ptr<Meta>)>();
}

bool Actuation :: has_event(std::string name) const
{
    return m_Events.find(name) != m_Events.end();
}

bool Actuation :: has_events() const
{
    return not m_Events.empty();
}

void Actuation :: when(Freq::Time t, Freq::Timeline* timeline, std::function<void()> func)
{
    m_Alarms.emplace_back(t, timeline, func);
}

void Actuation :: logic(Freq::Time t)
{
    StateMachine::logic(t);
    
    for(auto itr = m_Alarms.begin();
        itr != m_Alarms.end();
    ){
        auto&& alarm = *itr;
        if(alarm.poll())
            itr = m_Alarms.erase(itr);
        else
            ++itr;
    }
    
    on_tick(t);
}

