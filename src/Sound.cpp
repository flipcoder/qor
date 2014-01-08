#include "Sound.h"
#include "Resource.h"
using namespace std;

Sound :: Sound(const std::string& fn, Cache<Resource, std::string>* cache):
    Node(fn)
{
    m_pBuffer = cache->cache_as<Audio::Buffer>(fn);
    m_pSource = std::make_shared<Audio::Source>();
    m_pSource->bind(m_pBuffer.get());
    m_pSource->refresh();
}

