#include "Resource.h"
#include "Filesystem.h"
using namespace std;

Resource :: ~Resource() {}

Resource :: Resource(const std::string& fn):
    m_Filename(fn),
    m_pConfig(std::make_shared<Meta>())
{
    if(Filesystem::getExtension(fn)=="json")
    {
        try {
            m_pConfig = make_shared<Meta>(fn);
        } catch(const Error& e) {}
    }
}

void Resource :: filename(const std::string& fn)
{
    m_Filename = fn;
    if(Filesystem::getExtension(fn)=="json")
    {
        try {
            m_pConfig = make_shared<Meta>(fn);
        } catch(const Error& e) {}
    }
}


