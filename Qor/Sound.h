#ifndef _SOUND_H
#define _SOUND_H
#ifndef QOR_NO_AUDIO

#include <boost/signals2.hpp>
#include <memory>
#include "Node.h"
#include "Audio.h"
#include "ResourceCache.h"

class Sound:
    public Node
{
    public:

        static float GAIN;
        static float SOUND_GAIN;
        static float MUSIC_GAIN;
        
        Sound(ResourceCache* cache);
        Sound(const std::string& fn, ResourceCache* cache);
        Sound(const std::tuple<std::string, IFactory*, ICache*>& args):
            Sound(
                std::get<0>(args),
                //std::get<1>(args), // don't need factory
                (ResourceCache*) std::get<2>(args)
            )
        {}
        
        static std::shared_ptr<Sound> raw(
            std::function<int(char*,int)> func,
            ResourceCache* cache
        );

        virtual ~Sound();

        Audio::Buffer* buffer() {
            return m_pBuffer.get();
        };
        
        Audio::Source* source() {
            return m_pSource.get();
        };

        virtual void logic_self(Freq::Time t) override;
        
        void update_signals();
        void play();
        bool playing() const;
        bool played() const { return m_bPlayed; }
        void pause();
        void stop();
        void detach_on_done();
        
        static std::shared_ptr<Sound> play(
            Node* parent,
            std::string fn,
            ResourceCache* resources
        );

        template<class T>
        boost::signals2::connection on_done(T t){
            return m_onDone.connect(t);
        }

        void ambient(bool a = true) {
            if(a != m_bAmbient) {
                m_bAmbient = a;
                update_signals();
            }
        }
        
        virtual std::string type() const override { return "sound"; }
        void loop(bool b);

        void gain(float g);
        bool music() const { return m_bMusic; }
        
    private:
        
        bool m_bStream = false;
        bool m_bMusic = false;
        bool m_bAmbient = false;
        bool m_bPlayed = false;
        bool m_bLoop = false;
        float m_Gain = 1.0f;
        
        std::shared_ptr<Audio::Buffer> m_pBuffer = nullptr;
        std::shared_ptr<Audio::Source> m_pSource = nullptr;
        
        ResourceCache* m_pResources = nullptr;

        // master volume
        boost::signals2::scoped_connection m_MasterVolCon;
        // sound or music volume (depending on what this is)
        boost::signals2::scoped_connection m_VolCon;
        boost::signals2::signal<void()> m_onDone;

        //m_VolumeCon;
        //m_GlobalVolumeCon;
};

#endif
#endif

