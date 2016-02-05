#ifndef _SOUND_H
#define _SOUND_H

#include <boost/signals2.hpp>
#include <memory>
#include "Node.h"
#include "Audio.h"

class Sound:
    public Node
{
    public:
        Sound(const std::string& fn, Cache<Resource, std::string>* cache);
        Sound(const std::tuple<std::string, IFactory*, ICache*>& args):
            Sound(
                std::get<0>(args),
                //std::get<1>(args), // don't need factory
                (Cache<Resource, std::string>*) std::get<2>(args)
            )
        {}
        virtual ~Sound();

        Audio::Buffer* buffer() {
            return m_pBuffer.get();
        };
        
        Audio::Source* source() {
            return m_pSource.get();
        };

        virtual void logic_self(Freq::Time t);
        
        void play();
        void pause();
        void stop();
        void detach_on_done();
        
        static std::shared_ptr<Sound> play(
            Node* parent,
            std::string fn,
            Cache<Resource, std::string>* resources
        );

        template<class T>
        boost::signals2::connection on_done(T t){
            return m_onDone.connect(t);
        }
        
    private:
        
        bool m_bStream = false;
        bool m_bPlayed = false;
        
        std::shared_ptr<Audio::Source> m_pSource;
        std::shared_ptr<Audio::Buffer> m_pBuffer;
        Cache<Resource, std::string>* m_pResources;

        // master volume
        boost::signals2::scoped_connection m_MasterVolCon;
        boost::signals2::scoped_connection m_VolCon;
        // sound or music volume (depending on what this is)
        boost::signals2::signal<void()> m_onDone;

        //m_VolumeCon;
        //m_GlobalVolumeCon;
};

#endif

