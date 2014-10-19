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
#include "kit/async/async.h"
#include <tuple>

#pragma warning(disable:4996)

#define BUFFER_SIZE (4096 * 8)
#define AUDIO_CIRCUIT 1

class Audio
{
public:
    struct Buffer:
        public Resource
    {
        std::shared_future<unsigned> id;
        
        Buffer(){
            id = MX.circuit(AUDIO_CIRCUIT).task<unsigned>([]{
                unsigned id;
                alGenBuffers(1, &id);
                return id;
            });
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
            id = MX.circuit(AUDIO_CIRCUIT).task<unsigned>([fn]{
                return alutCreateBufferFromFile(fn.c_str());
            });
        }
        Buffer(const std::tuple<std::string, ICache*>& args):
            Buffer(std::get<0>(args), std::get<1>(args))
        {}
        virtual ~Buffer() {
            unsigned idt = id.get();
            if(idt)
                MX.circuit(AUDIO_CIRCUIT).task<void>([idt]{
                    alDeleteBuffers(1, &idt);
                });
        }
        bool good() { return id.get()!=0; }
    };

    struct Source
    {
        mutable std::shared_future<unsigned> id;
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
            id = MX.circuit(AUDIO_CIRCUIT).task<unsigned>([]{
                unsigned id;
                alGenSources(1, &id);
                return id;
            });
            if(flags & F_AUTOPLAY){
                refresh();
                play();
            }
        }
        virtual ~Source() {
            unsigned idt = id.get();
            MX.circuit(AUDIO_CIRCUIT).task<void>([idt]{
                alDeleteSources(1, &idt);
            });
        }
        virtual bool update() {
            return false;
        }
        void bind(Buffer* buf) {
            if(buf) {
                unsigned idt = id.get();
                buffer_id = buf->id.get();
                auto buffer_idT = buffer_id;
                MX.circuit(AUDIO_CIRCUIT).task<void>([idt, buffer_idT]{
                    alSourcei(idt, AL_BUFFER, buffer_idT);
                });
            }
        }
        virtual void refresh() {
            //if(!buffer_id)
            //    return;
            unsigned idt = id.get();
            auto pitchT = pitch;
            auto gainT = gain;
            auto posT = pos;
            auto velT = vel;
            auto flagsT = flags;
            MX.circuit(AUDIO_CIRCUIT).task<void>([idt, pitchT, gainT, posT, velT, flagsT]{
                //alSourcei(id, AL_BUFFER, buffer_id);
                alSourcef(idt, AL_PITCH, pitchT);
                alSourcef(idt, AL_GAIN, gainT);
                alSourcefv(idt, AL_POSITION, glm::value_ptr(posT));
                alSourcefv(idt, AL_VELOCITY, glm::value_ptr(velT));
                //alSourcefv(id, AL_ROLLOFF_FACTOR, glm::value_ptr(rolloff));
                alSourcei(idt, AL_LOOPING, (flagsT & F_LOOP) ? AL_TRUE : AL_FALSE);
            });
        }
        virtual void play() {
            unsigned idt = id.get();
            MX.circuit(AUDIO_CIRCUIT).task<void>([idt]{
                alSourcePlay(idt);
            });
        }
        bool playing() const {
            unsigned idt = id.get();
            return MX.circuit(AUDIO_CIRCUIT).task<ALenum>([idt]{
                ALenum state;
                alGetSourcei(idt, AL_SOURCE_STATE, &state);
                return state;
            }).get() == AL_PLAYING;
        }
        void pause() {
            unsigned idt = id.get();
            MX.circuit(AUDIO_CIRCUIT).task<void>([idt]{
                alSourcePause(idt);
            });
        }
        void stop() {
            unsigned idt = id.get();
            MX.circuit(AUDIO_CIRCUIT).task<void>([idt]{
                alSourceStop(idt);
            });
        }

        bool good() { return id.get()!=0; }
    };

    struct Stream:
        public Source,
        public Resource
    {       
        public:

            Stream(std::string fn):
                m_Filename(fn)
            {
                //m_File = fopen(fn.c_str(), "rb");
                //if(!m_File)
                //    break;
                MX.circuit(AUDIO_CIRCUIT).task<void>([this, fn]{
                
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
                });
            }
            
            Stream(const std::tuple<std::string, ICache*>& args):
                Stream(std::get<0>(args))
            {}
            
            virtual ~Stream() {
                stop();
                clear();
                MX.circuit(AUDIO_CIRCUIT).task<void>([this]{
                    alDeleteBuffers(2, m_Buffers);
                    ov_clear(&m_Ogg);
                }).get();
            }

            virtual bool update() override
            {
                return MX.circuit(AUDIO_CIRCUIT).task<bool>([this]{
                    auto idt = id.get();
                    int processed;
                    bool active = true;
                 
                    alGetSourcei(idt, AL_BUFFERS_PROCESSED, &processed);
             
                    while(processed--)
                    {
                        ALuint buffer;
                        
                        alSourceUnqueueBuffers(idt, 1, &buffer);
                        checkErrors();

                        active = stream(buffer);

                        if(active) {
                            alSourceQueueBuffers(idt, 1, &buffer);
                            checkErrors();
                        }
                    }
                    return active;
                }).get();
            }

            void clear()
            {
                MX.circuit(AUDIO_CIRCUIT).task<void>([this]{
                    unsigned idt = id.get();
                    int queued;
                    alGetSourcei(idt, AL_BUFFERS_QUEUED, &queued);
                    while(queued--)
                    {
                        ALuint buffer;
                        alSourceUnqueueBuffers(idt, 1, &buffer);
                        if(checkErrors())
                            break;
                    }
                });
            }

            virtual void refresh() {

                //if(playing())
                //{
                    update();

                    //alSourcei(id, AL_BUFFER, buffer_id);
                    auto pitchT = pitch;
                    auto gainT = gain;
                    auto posT = pos;
                    auto velT = vel;
                    MX.circuit(AUDIO_CIRCUIT).task<void>([this]{
                        unsigned idt = id.get();
                        alSourcef(idt, AL_PITCH, pitch);
                        alSourcef(idt, AL_GAIN, gain);
                        alSourcefv(idt, AL_POSITION, glm::value_ptr(pos));
                        alSourcefv(idt, AL_VELOCITY, glm::value_ptr(vel));
                        alSourcefv(idt, AL_DIRECTION, glm::value_ptr(vel));
                        alSourcef(idt, AL_ROLLOFF_FACTOR,  0.0);
                        alSourcei(idt, AL_SOURCE_RELATIVE, AL_TRUE);
                    });
                    //alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
                //}
            }

            virtual void play() {
                auto idt = id.get();
                if(playing())
                    return;
                MX.circuit(AUDIO_CIRCUIT).task<void>([this, idt]{
                    if(!stream(m_Buffers[0]))
                        return;
                    if(!stream(m_Buffers[1]))
                        return;
                        
                    alSourceQueueBuffers(idt, 2, m_Buffers);
                    alSourcePlay(idt);
                });
            }

            bool good() const { return m_bOpen; }
            
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

        private:
            
            //FILE* m_File;
            OggVorbis_File m_Ogg;
            vorbis_info* m_VorbisInfo;
            vorbis_comment* m_VorbisComment;
            ALenum m_Format;
            ALuint m_Buffers[2];
            std::atomic<bool> m_bOpen = ATOMIC_VAR_INIT(false);
            std::string m_Filename;

            // call internal inside AUDIO circuit
            
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
            
            bool stream(unsigned int buffer)
            {
                //return MX.circuit(AUDIO_CIRCUIT).task<bool>([this, buffer]{
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
                //}).get();
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
            up = glm::vec3(0.0f, -1.0f, 0.0f);
        }
        virtual ~Listener() {}
        void listen() {
            auto posT = pos;
            auto velT = vel;
            auto atT = at;
            auto upT = up;
            MX.circuit(AUDIO_CIRCUIT).task<void>([posT, velT, atT, upT]{
                alListenerfv(AL_POSITION, glm::value_ptr(posT));
                alListenerfv(AL_VELOCITY, glm::value_ptr(velT));
                float ori[6];
                ori[0] = atT.x; ori[1] = atT.y; ori[2] = atT.z;
                ori[3] = upT.x; ori[4] = upT.y; ori[5] = upT.z;
                alListenerfv(AL_ORIENTATION, ori);
            });
        }
    };

public:
    
    Audio(){
        MX.circuit(AUDIO_CIRCUIT).task<void>([]{
            alutInit(0, NULL);
            alGetError();
        });
    }
    virtual ~Audio(){
        MX.circuit(AUDIO_CIRCUIT).task<void>([]{
            alutExit();
        });
    }
    
    void listen(Listener* listener) const {
        if(listener)
            listener->listen();
    }

    bool error() const {
        return MX.circuit(AUDIO_CIRCUIT).task<ALenum>([]{
            return alGetError();
        }).get() != AL_NO_ERROR;
    }
};

#endif

