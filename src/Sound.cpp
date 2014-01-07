#include "Sound.h"
#include "Resource.h"
using namespace std;

Sound :: Sound(const std::string& fn, IFactory* factory, ICache* cache):
    Node(fn)
{
    auto resources = (Cache<Resource, std::string>*)cache;
    m_pBuffer = resources->cache_as<Audio::Buffer>(fn);
    m_pSource = std::make_shared<Audio::Source>();
    m_pSource->bind(m_pBuffer.get());
}

