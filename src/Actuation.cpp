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
        m_Events[name](meta);
}

void Actuation :: on_event(std::string name, std::function<void(std::shared_ptr<Meta>)> func)
{
    m_Events[name].connect(func);
}

void Actuation :: clear_events()
{
    m_Events.clear();
}

bool Actuation :: has_events() const
{
    return not m_Events.empty();
}

