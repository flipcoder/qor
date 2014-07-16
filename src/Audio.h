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
    struct Buffer:
        public Resource
    {
        unsigned int id;
        
        Buffer(){
            alGenBuffers(1, &id);
        }

        Buffer(const std::string& fn, ICache* c) {
            //id = 0;
            //ALenum format;
            //ALsizei size;
            //ALvoid* data;
            //ALsizei freq;
            //ALboolean loop;
            
            //alGenBuffers(1, &id);
            //if(alGetError() != AL_NO_ERROR) {
            //    id = 0;
            //    break;
            //}
            //alutLoadWAVFile((ALbyte*)fn.c_str(), &format, &data, &size, &freq, &loop);
            //alBufferData(id, format, data, size, freq);
            //alutUnloadWAV(format,data,size,freq);
            id = alutCreateBufferFromFile(fn.c_str());
        }
        Buffer(const std::tuple<std::string, ICache*>& args):
            Buffer(std::get<0>(args), std::get<1>(args))
        {}
        virtual ~Buffer() {
            if(id)
                alDeleteBuffers(1, &id);
        }
        bool good() { return id!=0; }
    };

    struct Source
    {
        unsigned int id = 0;
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
            alGenSources(1, &id);
            if(flags & F_AUTOPLAY){
                refresh();
                play();
            }
        }
        virtual ~Source() {
            alDeleteSources(1, &id);
        }
        virtual bool update() {
            return false;
        }
        void bind(Buffer* buf) {
            if(buf) {
                buffer_id = buf->id;
                alSourcei(id, AL_BUFFER, buffer_id);
            }
        }
        virtual void refresh() {
            //if(!buffer_id)
            //    return;
            //alSourcei(id, AL_BUFFER, buffer_id);
            alSourcef(id, AL_PITCH, pitch);
            alSourcef(id, AL_GAIN, gain);
            alSourcefv(id, AL_POSITION, glm::value_ptr(pos));
            alSourcefv(id, AL_VELOCITY, glm::value_ptr(vel));
            //alSourcefv(id, AL_ROLLOFF_FACTOR, glm::value_ptr(rolloff));
            alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
        }
        virtual void play() {
            alSourcePlay(id);
        }
        bool playing() const {
            ALenum state;
            alGetSourcei(id, AL_SOURCE_STATE, &state);
            return state == AL_PLAYING;
        }
        void pause() {
            alSourcePause(id);
        }
        void stop() {
            alSourceStop(id);
        }

        bool good() { return id!=0; }
    };

    struct Stream:
        public Source,
        public Resource
    {
        private:
            //FILE* m_File;
            OggVorbis_File m_Ogg;
            vorbis_info* m_VorbisInfo;
            vorbis_comment* m_VorbisComment;
            ALenum m_Format;
            ALuint m_Buffers[2];
            bool m_bOpen = false;
            std::string m_Filename;

        public:

            Stream(std::string fn):
                m_Filename(fn)
            {
                //m_File = fopen(fn.c_str(), "rb");
                //if(!m_File)
                //    break;
                
                int r;
                if((r = ov_fopen((char*)&fn[0], &m_Ogg)) < 0)
                    ERROR(READ, Filesystem::getFileName(fn));

                if(checkErrors())
                    ERROR(READ, Filesystem::getFileName(fn));

                m_VorbisInfo = ov_info(&m_Ogg, -1);
                m_VorbisComment = ov_comment(&m_Ogg, -1);
                
                if(checkErrors())
                    ERROR(READ, Filesystem::getFileName(fn));
             
                if(m_VorbisInfo->channels == 1)
                    m_Format = AL_FORMAT_MONO16;
                else
                    m_Format = AL_FORMAT_STEREO16;
                
                alGenBuffers(2, m_Buffers);

                if(checkErrors())
                    ERROR(READ, Filesystem::getFileName(fn));

                flags |= Source::F_LOOP;

                //std::cout
                //    << "version         " << m_VorbisInfo->version         << "\n"
                //    << "channels        " << m_VorbisInfo->channels        << "\n"
                //    << "rate (hz)       " << m_VorbisInfo->rate            << "\n"
                //    << "bitrate upper   " << m_VorbisInfo->bitrate_upper   << "\n"
                //    << "bitrate nominal " << m_VorbisInfo->bitrate_nominal << "\n"
                //    << "bitrate lower   " << m_VorbisInfo->bitrate_lower   << "\n"
                //    << "bitrate window  " << m_VorbisInfo->bitrate_window  << "\n"
                //    << "\n"
                //    << "vendor " << m_VorbisComment->vendor << "\n";
                    
                //for(int i = 0; i < m_VorbisComment->comments; i++)
                //    std::cout << "   " << m_VorbisComment->user_comments[i] << "\n";
                    
                //std::cout << std::endl;

                m_bOpen = true;
            }
            
            Stream(const std::tuple<std::string, ICache*>& args):
                Stream(std::get<0>(args))
            {}
            
            virtual ~Stream() {
                stop();
                clear();
                alDeleteBuffers(2, m_Buffers);
                ov_clear(&m_Ogg);
            }

            virtual bool update() override
            {
                int processed;
                bool active = true;
             
                alGetSourcei(id, AL_BUFFERS_PROCESSED, &processed);
         
                while(processed--)
                {
                    ALuint buffer;
                    
                    alSourceUnqueueBuffers(id, 1, &buffer);
                    checkErrors();

                    active = stream(buffer);

                    if(active) {
                        alSourceQueueBuffers(id, 1, &buffer);
                        checkErrors();
                    }
                }
             
                return active;
            }

            void clear()
            {
                int queued;
                alGetSourcei(id, AL_BUFFERS_QUEUED, &queued);
                while(queued--)
                {
                    ALuint buffer;
                    alSourceUnqueueBuffers(id, 1, &buffer);
                    if(checkErrors())
                        break;
                }
            }

            bool stream(unsigned int buffer)
            {
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

            virtual void refresh() {

                //if(playing())
                //{
                    update();

                    //alSourcei(id, AL_BUFFER, buffer_id);
                    alSourcef(id, AL_PITCH, pitch);
                    alSourcef(id, AL_GAIN, gain);
                    alSourcefv(id, AL_POSITION, glm::value_ptr(pos));
                    alSourcefv(id, AL_VELOCITY, glm::value_ptr(vel));
                    alSourcefv(id, AL_DIRECTION, glm::value_ptr(vel));
                    alSourcef(id, AL_ROLLOFF_FACTOR,  0.0);
                    alSourcei(id, AL_SOURCE_RELATIVE, AL_TRUE);
                    //alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
                //}
            }

            virtual void play() {
                if(playing())
                    return;
                if(!stream(m_Buffers[0]))
                    return;
                if(!stream(m_Buffers[1]))
                    return;
                    
                alSourceQueueBuffers(id, 2, m_Buffers);
                alSourcePlay(id);
            }

            bool good() { return m_bOpen; }

            bool checkErrors() {
                int error = alGetError();
                if(error != AL_NO_ERROR) {
                    std::tuple<std::string, std::string> errpair = errorStringAL(error);
                    WARNINGf("OpenAL Error (%s): %s",
                        std::get<0>(errpair) % std::get<1>(errpair)
                    );
                    return true;
                }
                return false;
            }
            
            static std::tuple<std::string, std::string> errorStringAL(int code)
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
            
            static std::tuple<std::string,std::string> errorStringOV(int code)
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
    };

    struct Listener
    {
        glm::vec3 pos, vel;
        glm::vec3 at;
        glm::vec3 up;
        Listener() {
            pos = glm::vec3(0.0f, 0.0f, 0.0f);
            vel = glm::vec3(0.0f, 0.0f, 0.0f);
            at = glm::vec3(0.0f, 0.0f, -1.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        virtual ~Listener() {}
        void listen() {
            alListenerfv(AL_POSITION, glm::value_ptr(pos));
            alListenerfv(AL_VELOCITY, glm::value_ptr(vel));
            float ori[6];
            ori[0] = at.x; ori[1] = at.y; ori[2] = at.z;
            ori[3] = up.x; ori[4] = up.y; ori[5] = up.z;
            alListenerfv(AL_ORIENTATION, ori);
        }
    };

public:
    
    Audio(){
        alutInit(0, NULL);
        alGetError();
    }
    virtual ~Audio(){
        alutExit();
    }
    
    void listen(Listener* listener) const {
        if(listener)
            listener->listen();
    }

    bool error() const { return alGetError() != AL_NO_ERROR; }
};

#endif

