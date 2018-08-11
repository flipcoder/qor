#include "ResourceCache.h"
using namespace std;

Meta::ptr ResourceCache :: config(std::string fn)
{
    fn = transform(fn);
    auto itr = m_Configs.find(fn);
    if(itr == m_Configs.end())
    {
        auto m = Meta::make(fn);
        m_Configs[fn] = m;
        return m;
    }
    return itr->second;
}

