#include "ResourceCache.h"
using namespace std;

std::shared_ptr<Meta> ResourceCache :: config(std::string fn)
{
    fn = transform(fn);
    auto itr = m_Configs.find(fn);
    if(itr == m_Configs.end())
    {
        auto m = make_shared<Meta>(fn);
        m_Configs[fn] = m;
        return m;
    }
    return itr->second;
}

