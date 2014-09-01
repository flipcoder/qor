#ifndef _SCENE_H_NACIGDRE
#define _SCENE_H_NACIGDRE

#include <string>
#include "Resource.h"
#include "Node.h"
#include "kit/cache/cache.h"

class Scene:
    public Resource
{
    public:
        
        Scene(
            const std::string& fn,
            Cache<Resource, std::string>* cache
        );
        
        Scene(const std::tuple<std::string, ICache*>& args):
            Scene(
                std::get<0>(args),
                (Cache<Resource, std::string>*)std::get<1>(args)
            )
        {}
        virtual ~Scene() {}
        
        std::shared_ptr<Node> root() {
            return m_pRoot;
        }
        std::shared_ptr<const Node> root() const {
            return m_pRoot;
        }
        
        void iterate_data(const std::shared_ptr<Meta>& data);
        void iterate_nodes(const std::shared_ptr<Meta>& nodes);
        void load();

    private:
        
        std::string m_Filename;
        Cache<Resource, std::string>* m_pCache;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Meta> m_pData;

        glm::vec3 m_Gravity;
};

#endif

