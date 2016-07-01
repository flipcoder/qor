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
    m_WhenAlarms.emplace_back(t, timeline, func);
}

void Actuation :: until(
    Freq::Time t,
    Freq::Timeline* timeline,
    std::function<void(Freq::Time)> func,
    std::function<void()> end
){
    auto sig = boost::signals2::signal<void(Freq::Time)>();
    sig.connect(func);
    m_UntilAlarms.emplace_back(make_tuple(
        std::move(sig), Freq::Alarm(t, timeline, end)
    ));
}

void Actuation :: logic(Freq::Time t)
{
    StateMachine::logic(t);
    
    for(auto itr = m_UntilAlarms.begin();
        itr != m_UntilAlarms.end();
    ){
        auto&& alarm = std::get<1>(*itr);
        if(alarm.poll())
            itr = m_UntilAlarms.erase(itr);
        else{
            std::get<0>(*itr)(t);
            ++itr;
        }
    }
    
    for(auto itr = m_WhenAlarms.begin();
        itr != m_WhenAlarms.end();
    ){
        auto&& alarm = *itr;
        if(alarm.poll())
            itr = m_WhenAlarms.erase(itr);
        else
            ++itr;
    }
    
    on_tick(t);
}

