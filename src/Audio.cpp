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

