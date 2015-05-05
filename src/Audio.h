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
        
        Buffer(){
            auto l = Audio::lock();
            alGenBuffers(1, &id);
        }

        Buffer(const std::string& fn, ICache* c) {
            auto l = Audio::lock();
            id = alutCreateBufferFromFile(fn.c_str());
        }
        Buffer(const std::tuple<std::string, ICache*>& args):
            Buffer(std::get<0>(args), std::get<1>(args))
        {}
        virtual ~Buffer() {
            if(id){
                auto l = Audio::lock();
                alDeleteBuffers(1, &id);
            }
        }
        bool good() const {
            return id!=0;
        }
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
        Source(
            unsigned int _flags = 0
        ):
            flags(_flags)
        {
            auto l = Audio::lock();
            alGenSources(1, &id);
            if(flags & F_AUTOPLAY){
                play();
            }
        }
        virtual ~Source() {
            auto l = Audio::lock();
            alSourceStop(id);
            alDeleteSources(1, &id);
        }
        virtual bool update() {
            return false;
            //return kit::make_future<bool>(false);
        }
        void bind(Buffer* buf) {
            auto l = Audio::lock();
            if(buf) {
                alSourcei(id, AL_BUFFER, buf->id);
            }
        }
        virtual void refresh() {
            if(!buffer_id)
                return;
            auto l = Audio::lock();
            alSourcei(id, AL_BUFFER, buffer_id);
            alSourcef(id, AL_PITCH, pitch);
            alSourcef(id, AL_GAIN, kit::clamp<float>(gain, 0.0f, 1.0f - K_EPSILON));
            alSourcefv(id, AL_POSITION, glm::value_ptr(pos));
            alSourcefv(id, AL_VELOCITY, glm::value_ptr(vel));
            alSourcef(id, AL_ROLLOFF_FACTOR, 1.0f);
            alSourcef(id, AL_MAX_DISTANCE, 2048.0f);
            alSourcef(id, AL_REFERENCE_DISTANCE, 256.0f);
            alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
        }
        virtual void play() {
            auto l = Audio::lock();
            refresh();
            alSourcePlay(id);
        }
        bool playing() const {
            auto l = Audio::lock();
            ALenum state;
            alGetSourcei(id, AL_SOURCE_STATE, &state);
            return state == AL_PLAYING;
        }
        void pause() {
            auto l = Audio::lock();
            alSourcePause(id);
        }
        void stop() {
            auto l = Audio::lock();
            alSourceStop(id);
        }

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
            
            virtual ~Stream() {
                auto l = Audio::lock();
                stop();
                clear();
                alDeleteBuffers(2, m_Buffers);
                ov_clear(&m_Ogg);
            }

            virtual bool update() override
            {
                auto l = Audio::lock();
                clear_errors();
                int processed;
                bool active = true;
             
                alGetSourcei(id, AL_BUFFERS_PROCESSED, &processed);
         
                while(processed--)
                {
                    ALuint buffer;
                    
                    alSourceUnqueueBuffers(id, 1, &buffer);
                    Audio::check_errors();

                    active = stream(buffer);

                    if(active) {
                        alSourceQueueBuffers(id, 1, &buffer);
                        Audio::check_errors();
                    }
                }
                return active;
            }

            void clear()
            {
                auto l = Audio::lock();
                Audio::check_errors();
                int queued;
                alGetSourcei(id, AL_BUFFERS_QUEUED, &queued);
                while(queued--)
                {
                    ALuint buffer;
                    alSourceUnqueueBuffers(id, 1, &buffer);
                    if(Audio::check_errors())
                        break;
                }
            }

            virtual void refresh() override {

                //if(playing())
                //{
                auto l = Audio::lock();
                
                    clear_errors();
                
                    update();

                    alSourcei(id, AL_BUFFER, buffer_id);
                    alSourcef(id, AL_PITCH, pitch);
                    alSourcef(id, AL_GAIN, kit::clamp<float>(gain, 0.0f, 1.0f - K_EPSILON));
                    alSourcefv(id, AL_POSITION, glm::value_ptr(pos));
                    alSourcefv(id, AL_VELOCITY, glm::value_ptr(vel));
                    //alSourcefv(id, AL_DIRECTION, glm::value_ptr(velT));
                    alSourcef(id, AL_ROLLOFF_FACTOR, 1.0f);
                    alSourcef(id, AL_MAX_DISTANCE, 2048.0f);
                    alSourcef(id, AL_REFERENCE_DISTANCE, 256.0f);
                    //alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
                //}
            }

            virtual void play() override {
                auto l = Audio::lock();
                if(playing())
                    return;
                if(!stream(m_Buffers[0]))
                    return;
                if(!stream(m_Buffers[1]))
                    return;
                    
                alSourceQueueBuffers(id, 2, m_Buffers);
                alSourcePlay(id);
            }

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

            bool stream(unsigned int buffer)
            {
                auto l = Audio::lock();
                
                char data[BUFFER_SIZE];
                int size = 0;
                int endian = 0;
                int section;
                int result;

                while(size < BUFFER_SIZE)
                {
                    result = ov_read(&m_Ogg, data + size, BUFFER_SIZE - size, endian, 2, 1, &section);
                    
                    if((flags & Source::F_LOOP) && !result)
                        ov_raw_seek(&m_Ogg, 0);
                    
                    if(result > 0)
                        size += result;
                    else
                    {
                        if(result < 0)
                            return false;
                        else
                            break;
                    }
                }

                if(size == 0)
                    return false;

                alBufferData(buffer, m_Format, data, size, m_VorbisInfo->rate);
                return true;
            }
    };

    struct Listener
    {
        glm::vec3 pos, vel;
        glm::vec3 at;
        glm::vec3 up;
        float gain;
        Listener() {
            gain = 1.0f;
            pos = glm::vec3(0.0f, 0.0f, 0.0f);
            vel = glm::vec3(0.0f, 0.0f, 0.0f);
            at = glm::vec3(0.0f, 0.0f, -1.0f);
            up = glm::vec3(0.0f, -1.0f, 0.0f);
        }
        virtual ~Listener() {}
        void listen() {
            auto l = Audio::lock();
            alListenerf(AL_GAIN, kit::clamp<float>(gain, 0.0f, 1.0f - K_EPSILON));
            alListenerfv(AL_POSITION, glm::value_ptr(pos));
            alListenerfv(AL_VELOCITY, glm::value_ptr(vel));
            float ori[6];
            ori[0] = at.x; ori[1] = at.y; ori[2] = at.z;
            ori[3] = up.x; ori[4] = up.y; ori[5] = up.z;
            alListenerfv(AL_ORIENTATION, ori);
        }
    };
    
    Audio();
    virtual ~Audio(){
        auto l = lock();
        alcDestroyContext(m_pContext);
        alcCloseDevice(m_pDevice);
        alutExit();
    }

    void set_context() {
        auto l = Audio::lock();
        if(not alcMakeContextCurrent(m_pContext))
            throw std::runtime_error("failed to set OpenAL Context");
        clear_errors();
    }
    
    void listen(Listener* listener) const {
        if(listener)
            listener->listen();
    }

    static bool error() {
        auto l = Audio::lock();
        return alGetError();
    }
    
    static void clear_errors() {
        auto l = Audio::lock();
        alGetError();
    }
    static bool check_errors() {
        int error = alGetError();
        if(error != AL_NO_ERROR) {
            std::tuple<std::string, std::string> errpair = error_string_al(error);
            WARNINGf("OpenAL Error (%s): %s",
                std::get<0>(errpair) % std::get<1>(errpair)
            );
            return true;
        }
        return false;
    }
    
    static std::tuple<std::string, std::string> error_string_al(int code)
    {
        switch(code)
        {
            case AL_INVALID_NAME:
                return std::make_tuple("AL_INVALID_NAME", "Invalid name.");
            case AL_INVALID_ENUM:
                return std::make_tuple("AL_INVALID_ENUM", "Invalid enum.");
            case AL_INVALID_VALUE:
                return std::make_tuple("AL_INVALID_VALUE", "Invalid value.");
            case AL_INVALID_OPERATION:
                return std::make_tuple("AL_INVALID_OPERATION", "Invalid operation.");
            case AL_OUT_OF_MEMORY:
                return std::make_tuple("AL_OUT_OF_MEMORY", "Out of memory.");
        }
        return (code!=AL_NO_ERROR) ?
            std::tuple<std::string,std::string>(std::string(), std::string("No Error.")):
            std::tuple<std::string,std::string>(
                boost::to_string(code),
                std::string("Unknown Error Code")
            );
    }
    
    static std::tuple<std::string,std::string> error_string_ov(int code)
    {
        switch(code)
        {
            // libvorbis return codes http://www.xiph.org/vorbis/doc/libvorbis/return.html
            case OV_EREAD:
                return std::make_tuple("OC_EREAD","Read from media.");
            case OV_ENOTVORBIS:
                return std::make_tuple("OC_ENOTVORBIS","Not Vorbis data.");
            case OV_EVERSION:
                return std::make_tuple("OV_EVERSION", "Vorbis version mismatch.");
            case OV_EBADHEADER:
                return std::make_tuple("OV_EBADHEADER", "Invalid Vorbis header.");
            case OV_EFAULT:
                return std::make_tuple("OV_EFAULT", "Internal logic fault (bug or heap/stack corruption.");
        }
        return code ?
            std::tuple<std::string,std::string>(
                boost::to_string(code),
                std::string("Unknown Error Code ") + boost::to_string(code)
            ):
                std::tuple<std::string,std::string>("","No Error.");
    }


private:
    ALCdevice* m_pDevice = nullptr;
    ALCcontext* m_pContext = nullptr;
};

#endif

