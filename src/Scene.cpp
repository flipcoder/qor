#include "Scene.h"
#include "Mesh.h"
#include "Sound.h"
#include "Light.h"
#include "Material.h"
#include "kit/meta/meta.h"
using namespace std;

Scene :: Scene(const string& fn, Cache<Resource, std::string>* cache):
    //Resource(fn),
    m_pConfig(make_shared<Meta>(fn)),
    m_Filename(fn),
    m_pCache(cache),
    m_pRoot(make_shared<Node>())
    //m_pData(make_shared<Meta>(fn))
{
    load();
}

void Scene :: iterate_data(const std::shared_ptr<Meta>& doc)
{
    string name = doc->at<string>("name", string());
    string type = doc->at<string>("type", string());
    LOGf("data: %s: %s", name % type);
    try{
        if(type == "mesh")
            m_pCache->cache_as<Mesh::Data>(m_Filename + ":" + doc->at<string>("name"));
        else if(type == "material")
            m_pCache->cache_as<Material>(doc->at<string>("image"));
    }catch(const Error& e){
    }catch(const std::exception& e){
        WARNING(e.what());
    }
}

void Scene :: iterate_node(const std::shared_ptr<Node>& parent, const std::shared_ptr<Meta>& doc)
{
    shared_ptr<Node> node;
    string name = doc->at<string>("name", string());
    string type = doc->at<string>("type", string());
    LOGf("node: %s: %s", name % type);
    
    // TODO: use node factory instead
    
    if(type == "empty")
        node = make_shared<Node>();
    else if(type == "mesh")
        node = make_shared<Mesh>();
    else if(type == "sound")
    {
        LOGf("sound: %s", doc->at<string>("sound"));
        auto snd = make_shared<Sound>(
            doc->at<string>("sound"),
            m_pCache
        );
        snd->source()->play();
        snd->source()->update();
        node = snd;
    }
    else if(type == "light")
        node = make_shared<Light>(doc);
    
    if(not node)
        node = make_shared<Node>();
    
    parent->add(node);
    
    try{
        for(auto& e: *doc->meta("nodes"))
        {
            try{
                iterate_node(node, e.as<std::shared_ptr<Meta>>());
            }catch(const boost::bad_any_cast&){}
        }
    }catch(const out_of_range&){}
}


void Scene :: load()
{
    LOGf("%s", m_pConfig->size());
    auto grav = m_pConfig->meta("gravity", make_shared<Meta>(
        MetaFormat::JSON, "[0.0, -9.8, 0.0]"
    ));
    m_Gravity = glm::vec3(
        grav->at<double>(0),
        grav->at<double>(1),
        grav->at<double>(2)
    );
    for(auto& e: *m_pConfig->meta("data"))
    {
        try{
            iterate_data(e.as<std::shared_ptr<Meta>>());
        }catch(const boost::bad_any_cast&){}
    }
    m_pRoot = make_shared<Node>();
    for(auto& e: *m_pConfig->meta("nodes"))
    {
        try{
            iterate_node(m_pRoot, e.as<std::shared_ptr<Meta>>());
        }catch(const boost::bad_any_cast&){}
    }
}

