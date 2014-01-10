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

        virtual void reload() {}
        
        virtual ~Resource() = 0;
    private:
        std::string m_Filename;
        std::shared_ptr<Meta> m_pConfig;
};

#endif

