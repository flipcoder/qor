#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "kit/meta/meta.h"
#include "IRealtime.h"
#include <memory>

class Resource:
    public IRealtime,
    public std::enable_shared_from_this<Resource>
{
    
    public:
        
        Resource():
            m_pConfig(std::make_shared<Meta>())
        {}
        Resource(const std::string& config);
        
        Resource(const Resource&) = default;
        Resource(Resource&&) = default;
        Resource& operator=(const Resource&) = default;
        Resource& operator=(Resource&&) = default;

        virtual void reload() {}
        
        virtual ~Resource() = 0;

        std::string filename() const {
            return m_Filename;
        }
        void filename(const std::string& fn);

        std::shared_ptr<Meta> config() { return m_pConfig; }
        std::shared_ptr<const Meta> config() const { return m_pConfig; }
        
    protected:
        
        std::string m_Filename;
        std::shared_ptr<Meta> m_pConfig;
        
};

#endif

