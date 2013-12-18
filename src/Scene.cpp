#include "Scene.h"
#include "Mesh.h"
using namespace std;

Scene :: Scene(const string& fn, Cache<IResource, std::string>* cache):
    m_Filename(fn),
    m_pCache(cache),
    m_pRoot(make_shared<Node>())
{
    
}

