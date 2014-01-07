#ifndef _SOUND_H
#define _SOUND_H

#include <memory>
#include "Node.h"
#include "Audio.h"

class Sound:
    public Node
{
    public:
        Sound(const std::string& fn, IFactory* factory, ICache* cache);
        Sound(const std::tuple<std::string, IFactory*, ICache*>& args):
            Sound(std::get<0>(args), std::get<1>(args), std::get<2>(args))
        {}
        virtual ~Sound() {}
    private:
        std::shared_ptr<Audio::Source> m_pSource;
        std::shared_ptr<Audio::Buffer> m_pBuffer;
};

#endif

