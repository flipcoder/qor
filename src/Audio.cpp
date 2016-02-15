#include "Audio.h"

std::recursive_mutex Audio :: m_Mutex;

float Audio :: s_Rolloff = 1.0f;
float Audio :: s_MaxDist = 2048.0f;
float Audio :: s_ReferenceDist = 256.0f;

Audio::Buffer :: Buffer(){
    auto l = Audio::lock();
    alGenBuffers(1, &id);
}

Audio::Buffer :: Buffer(const std::string& fn, ICache* c) {
    auto l = Audio::lock();
    Audio::check_errors();
    id = alutCreateBufferFromFile(fn.c_str());
    Audio::check_errors();
}
Audio::Buffer :: Buffer(const std::tuple<std::string, ICache*>& args):
    Buffer(std::get<0>(args), std::get<1>(args))
{}
Audio::Buffer :: ~Buffer() {
    if(id){
        auto l = Audio::lock();
        Audio::check_errors();
        alDeleteBuffers(1, &id);
        Audio::check_errors();
    }
}
float Audio::Buffer :: length() const
{
    assert(id > 0);

    ALint sz;
    ALint channels;
    ALint bits;
    ALint freq;

    alGetBufferi(id, AL_SIZE, &sz);
    alGetBufferi(id, AL_CHANNELS, &channels);
    alGetBufferi(id, AL_BITS, &bits);
    alGetBufferi(id, AL_FREQUENCY, &freq);
    
    unsigned samples = sz * 8 / (channels * bits);
    return (float)samples / (float)freq;
}

Audio::Source :: Source(
    unsigned int _flags
):
    flags(_flags)
{
    auto l = Audio::lock();
    alGenSources(1, &id);
    if(flags & F_AUTOPLAY){
        play();
    }
}
Audio::Source :: ~Source() {
    auto l = Audio::lock();
    stop();
    Audio::check_errors();
    alDeleteSources(1, &id);
    Audio::check_errors();
}
bool Audio::Source :: update() {
    return false;
    //return kit::make_future<bool>(false);
}
void Audio::Source :: bind(Buffer* buf) {
    auto l = Audio::lock();
    alSourcei(id, AL_BUFFER, buf ? buf->id : 0);
}
void Audio::Source :: refresh() {
    if(!buffer_id)
        return;
    auto l = Audio::lock();
    alSourcei(id, AL_BUFFER, buffer_id);
    alSourcef(id, AL_PITCH, pitch);
    alSourcef(id, AL_GAIN, kit::clamp<float>(gain, 0.0f, 1.0f - K_EPSILON));
    alSourcei(id, AL_SOURCE_RELATIVE, (flags & F_AMBIENT) ? AL_TRUE : AL_FALSE);
    alSourcefv(id, AL_POSITION, glm::value_ptr(pos));
    alSourcefv(id, AL_VELOCITY, glm::value_ptr(vel));
    alSourcef(id, AL_ROLLOFF_FACTOR, 1.0f);
    alSourcef(id, AL_MAX_DISTANCE, 2048.0f);
    alSourcef(id, AL_REFERENCE_DISTANCE, 256.0f);
    alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
}
void Audio::Source :: play() {
    auto l = Audio::lock();
    refresh();
    alSourcePlay(id);
}
bool Audio::Source :: playing() const {
    auto l = Audio::lock();
    ALint state;
    alGetSourcei(id, AL_SOURCE_STATE, &state);
    //LOGf("state: %s", state)
    return state == AL_PLAYING;
}
bool Audio::Source :: stopped() const {
    auto l = Audio::lock();
    ALint state;
    alGetSourcei(id, AL_SOURCE_STATE, &state);
    //LOGf("state: %s", state)
    return state == AL_STOPPED;
}
//bool initial() const {
//    auto l = Audio::lock();
//    ALint state;
//    alGetSourcei(id, AL_SOURCE_STATE, &state);
//    return state == AL_INITIAL;
//}
void Audio::Source :: pause() {
    auto l = Audio::lock();
    alSourcePause(id);
}
void Audio::Source :: stop() {
    auto l = Audio::lock();
    if(playing())
        alSourceStop(id);
}

Audio::Stream :: ~Stream()
{
    auto l = Audio::lock();
    stop();
    clear();
    alDeleteBuffers(2, m_Buffers);
    ov_clear(&m_Ogg);
}

bool Audio::Stream :: update()
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

void Audio::Stream :: clear()
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

void Audio::Stream :: refresh()
{

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
        alSourcef(id, AL_ROLLOFF_FACTOR, s_Rolloff);
        alSourcef(id, AL_MAX_DISTANCE, s_MaxDist);
        alSourcef(id, AL_REFERENCE_DISTANCE, s_ReferenceDist);
        //alSourcei(id, AL_LOOPING, (flags & F_LOOP) ? AL_TRUE : AL_FALSE);
    //}
}

void Audio::Stream :: play()
{
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

bool Audio::Stream :: stream(unsigned int buffer)
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

Audio::Listener :: Listener()
{
    gain = 1.0f;
    pos = glm::vec3(0.0f, 0.0f, 0.0f);
    vel = glm::vec3(0.0f, 0.0f, 0.0f);
    at = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, -1.0f, 0.0f);
}

Audio::Listener :: ~Listener() {}

void Audio::Listener :: listen()
{
    auto l = Audio::lock();
    alListenerf(AL_GAIN, kit::clamp<float>(gain, 0.0f, 1.0f - K_EPSILON));
    alListenerfv(AL_POSITION, glm::value_ptr(pos));
    alListenerfv(AL_VELOCITY, glm::value_ptr(vel));
    float ori[6];
    ori[0] = at.x; ori[1] = at.y; ori[2] = at.z;
    ori[3] = up.x; ori[4] = up.y; ori[5] = up.z;
    alListenerfv(AL_ORIENTATION, ori);
}

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

Audio :: ~Audio()
{
    auto l = lock();
    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);
    alutExit();
}

void Audio :: set_context()
{
    auto l = Audio::lock();
    if(not alcMakeContextCurrent(m_pContext))
        throw std::runtime_error("failed to set OpenAL Context");
    clear_errors();
}

void Audio :: listen(Listener* listener) const
{
    if(listener)
        listener->listen();
}

bool Audio :: error() const
{
    auto l = Audio::lock();
    return alGetError();
}

void Audio :: clear_errors()
{
    auto l = Audio::lock();
    alGetError();
}
bool Audio :: check_errors()
{
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

std::tuple<std::string, std::string> Audio :: error_string_al(int code)
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

std::tuple<std::string,std::string> Audio :: error_string_ov(int code)
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


