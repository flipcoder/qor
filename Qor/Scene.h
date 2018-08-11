#ifndef _SCENE_H_NACIGDRE
#define _SCENE_H_NACIGDRE

#include <string>
#include "Resource.h"
#include "ResourceCache.h"
#include "Node.h"
#include "kit/cache/cache.h"

class Scene
{
    public:
        
        Scene(
            const std::string& fn,
            ResourceCache* cache
        );
        
        Scene(const std::tuple<std::string, ICache*>& args):
            Scene(
                std::get<0>(args),
                (ResourceCache*)std::get<1>(args)
            )
        {}
        virtual ~Scene() {}
        
        std::shared_ptr<Node> root() {
            return m_pRoot;
        }
        std::shared_ptr<const Node> root() const {
            return m_pRoot;
        }
        
        void iterate_data(const std::shared_ptr<Meta>& doc);
        void iterate_node(const std::shared_ptr<Node>& parent, const std::shared_ptr<Meta>& doc);
        void load();

        Color fog() const { return m_Fog; };
        
    private:
        
        glm::mat4 deserialize_matrix(const std::shared_ptr<Meta>& mat);
        void deserialize_node(std::shared_ptr<Node>& node, const std::shared_ptr<Meta>& doc);
        
        std::string m_Filename;
        ResourceCache* m_pCache;

        std::shared_ptr<Node> m_pRoot;
        std::shared_ptr<Meta> m_pConfig;

        glm::vec3 m_Gravity;
        Color m_Fog;
};

#endif

