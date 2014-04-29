#include "Scene.h"
#include "Mesh.h"
using namespace std;

Scene :: Scene(const string& fn, Cache<Resource, std::string>* cache):
    //Resource(fn),
    m_Filename(fn),
    m_pCache(cache),
    m_pRoot(make_shared<Node>())
{
    
}

