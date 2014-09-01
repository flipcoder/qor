#include "Scene.h"
#include "Mesh.h"
#include "kit/meta/meta.h"
using namespace std;

Scene :: Scene(const string& fn, Cache<Resource, std::string>* cache):
    Resource(fn),
    m_Filename(fn),
    m_pCache(cache),
    m_pRoot(make_shared<Node>())
    //m_pData(make_shared<Meta>(fn))
{
    load();
}

void Scene :: iterate_data(const std::shared_ptr<Meta>& data)
{
    
}

void Scene :: iterate_nodes(const std::shared_ptr<Meta>& nodes)
{
}

void Scene :: load()
{
    auto grav = m_pConfig->meta("gravity", make_shared<Meta>(
        MetaFormat::JSON, "[0.0, -9.8, 0.0]"
    ));
    m_Gravity = glm::vec3(
        grav->at<float>(0),
        grav->at<float>(1),
        grav->at<float>(2)
    );
    iterate_data(m_pConfig->meta("data"));
    iterate_nodes(m_pConfig->meta("nodes"));

    // read into scene.data[]
    
    //std::deque<std::tuple<
    //    std::shared_ptr<Meta>,
    //    std::unique_lock<kit::dummy_mutex>,
    //    std::string
    //>> metastack;

    //m_pConfig->each([this, &metastack](
    //    const std::shared_ptr<Meta>& parent,
    //    MetaElement& e,
    //    unsigned level
    //){
    //    std::vector<std::string> path;
    //    for(auto&& d: metastack) {
    //        std::string key = std::get<2>(d);
    //        if(!key.empty())
    //            path.push_back(std::get<2>(d));
    //    }
    //    path.push_back(e.key);
    //    //LOGf("path element: %s", boost::algorithm::join(path,"/"));

    //    return MetaLoop::STEP;
    //},
    //    (unsigned)Meta::EachFlag::DEFAULTS |
    //        (unsigned)Meta::EachFlag::RECURSIVE,
    //    &metastack
    //);

    //m_pConfig->parse();
}

