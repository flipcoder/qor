#include "Audio.h"

std::recursive_mutex Audio :: m_Mutex;

Audio :: Audio()
{
    auto l = lock();
    //alutInit(0, NULL);
    alutInitWithoutContext(0, NULL);
    m_pDevice = alcOpenDevice(NULL);
    if(not m_pDevice)
        throw std::runtime_error("failed to open OpenAL audio device");
    m_pContext = alcCreateContext(m_pDevice, NULL);
    if(not m_pContext)
        alcCloseDevice(m_pDevice);
    try{
        set_context();
    }catch(...){
        alcDestroyContext(m_pContext);
        alcCloseDevice(m_pDevice);
        throw;
    }
}

Audio::Stream :: Stream(std::string fn):
    m_Filename(fn)
{
    auto l = Audio::lock();
    
    // clear errors
    clear_errors();
    
    int r;
    if((r = ov_fopen((char*)&fn[0], &m_Ogg)) < 0)
        ERROR(READ, Filesystem::getFileName(fn));
    
    if(check_errors())
        ERROR(READ, Filesystem::getFileName(fn));

    m_VorbisInfo = ov_info(&m_Ogg, -1);
    m_VorbisComment = ov_comment(&m_Ogg, -1);
    
    if(check_errors())
        ERROR(READ, Filesystem::getFileName(fn));
 
    if(m_VorbisInfo->channels == 1)
        m_Format = AL_FORMAT_MONO16;
    else
        m_Format = AL_FORMAT_STEREO16;
    
    alGenBuffers(2, m_Buffers);

    if(check_errors())
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
