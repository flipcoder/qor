#include "Resource.h"
#include "Filesystem.h"

Resource :: Resource(const std::string& fn):
    m_Filename(fn),
    m_pConfig(std::make_shared<Meta>(fn))
{
    if(Filesystem::getExtension(fn)=="json")
    {
        try {
            m_pConfig->deserialize();
        } catch(const Error& e) {}
    }
}

