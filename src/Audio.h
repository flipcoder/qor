#ifndef _AUDIO_H
#define _AUDIO_H

#include <AL/al.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include <AL/alut.h>
#include <memory>
#include "kit/log/log.h"
#include "Filesystem.h"
#include "Resource.h"
#include <iostream>
#include "kit/math/common.h"
#include "kit/cache/icache.h"
#include <tuple>

#pragma warning(disable:4996)

#define BUFFER_SIZE (4096 * 8)

class Audio
{
public:

    static std::recursive_mutex m_Mutex;
    static std::unique_lock<std::recursive_mutex> lock() {
        return std::unique_lock<std::recursive_mutex>(m_Mutex);
    }
    
    struct Buffer:
        public Resource
    {
        unsigned id = 0;
        
        Buffer();
        Buffer(const std::string& fn, ICache* c);
        Buffer(const std::tuple<std::string, ICache*>& args);
        virtual ~Buffer();
        bool good() const { return id!=0; }
        float length() const;
    };

    struct Source
    {
        mutable unsigned id;
        float pitch = 1.0f;
        float gain = 1.0f;
        float rolloff = 0.0f;
        glm::vec3 pos;
        glm::vec3 vel;
        unsigned int buffer_id = 0;
        enum eFlags {
            F_LOOP = kit::bit(0),
            F_AUTOPLAY = kit::bit(1),
            F_AMBIENT = kit::bit(2)
        };
        unsigned int flags;
        Source(unsigned int _flags = 0);
        virtual ~Source();
        virtual bool update();
        void bind(Buffer* buf);
        virtual void refresh();
        virtual void play();
        bool playing() const;
        bool stopped() const;
        void pause();
        void stop();
        bool good() { return id!=0; }
    };

    struct Stream:
        public Source,
        public Resource
    {       
        public:

            Stream(std::string fn);
            
            Stream(const std::tuple<std::string, ICache*>& args):
                Stream(std::get<0>(args))
            {}
            
            virtual ~Stream();
            virtual bool update() override;
            void clear();
            virtual void refresh() override;
            virtual void play() override;
            bool good() const { return m_bOpen; }
            
        private:
            
            //FILE* m_File;
            OggVorbis_File m_Ogg;
            vorbis_info* m_VorbisInfo;
            vorbis_comment* m_VorbisComment;
            ALenum m_Format;
            ALuint m_Buffers[2];
            bool m_bOpen = false;
            std::string m_Filename;

            bool stream(unsigned int buffer);
    };

    struct Listener
    {
        glm::vec3 pos, vel;
        glm::vec3 at;
        glm::vec3 up;
        float gain;
        
        Listener();
        virtual ~Listener();
        void listen();
    };
    
    Audio();
    virtual ~Audio();
    void set_context();
    void listen(Listener* listener) const;
    bool error() const;
    static void clear_errors();
    static bool check_errors();
    static std::tuple<std::string, std::string> error_string_al(int code);
    static std::tuple<std::string,std::string> error_string_ov(int code);

    static float rolloff() { return s_Rolloff; }
    static float max_distance() { return s_MaxDist; }
    static float reference_distance() { return s_ReferenceDist; }
    static void rolloff(float f) {
        auto l = Audio::lock();
        s_Rolloff = f;
    }
    static void max_distance(float f) {
        auto l = Audio::lock();
        s_MaxDist = f;
    }
    static void reference_distance(float f) {
        auto l = Audio::lock();
        s_ReferenceDist = f;
    }
    
    //K_S_GET_SET(float, rolloff, m_Rolloff);
    //K_S_GET_SET(float, max_distance, m_MaxDist);
    //K_S_GET_SET(float, reference_distance, m_ReferenceDist);

private:
    ALCdevice* m_pDevice = nullptr;
    ALCcontext* m_pContext = nullptr;

    static float s_Rolloff;
    static float s_MaxDist;
    static float s_ReferenceDist;
};

#endif

