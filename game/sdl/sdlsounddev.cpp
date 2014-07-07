#include "../ht.h"
#include <SDL.h>

#include "../sounddevice.h"
#include "../mixer.h"

namespace wi {

#define kcBuffers 4
#define kcbBuffer 256
#define kcChannels 4

extern long HostGetTickCount();
void AudioCallback(void *pvUser, Uint8 *stream, int len);

class SdlSoundDevice : public SoundDevice // sndd
{
public:
    SdlSoundDevice();
    virtual ~SdlSoundDevice();

    bool Init();
    virtual void Enable(bool fEnable);
    virtual bool IsEnabled();
    virtual void PlayAdpcm(int ichnl, byte *pb, word cb);
    virtual int GetChannelCount();
    virtual bool IsChannelFree(int ichnl);
    virtual void ServiceProc();
    virtual bool IsSilent();
    virtual void SetVolume(int nVolume);
    virtual int GetVolume();

private:
    void InitAudioBuffer(byte *pbBuffer);

    bool m_fEnable;
    long m_tSilence;
    byte *m_apbBuffers[kcBuffers];
    Channel m_achnl[kcChannels];
	SDL_AudioSpec m_spec;

    friend void AudioCallback(void *pvUser, Uint8 *stream, int len);
};

SoundDevice *CreateSdlSoundDevice()
{
    SdlSoundDevice *psndd = new SdlSoundDevice;
    if (psndd == NULL)
        return NULL;
    if (!psndd->Init()) {
        delete psndd;
        return NULL;
    }
    return (SoundDevice *)psndd;
}

SdlSoundDevice::SdlSoundDevice()
{
    for (int i = 0; i < ARRAYSIZE(m_apbBuffers); i++)
        m_apbBuffers[i] = NULL;

    m_fEnable = false;
    m_tSilence = 0;
}

SdlSoundDevice::~SdlSoundDevice()
{
	SDL_CloseAudio();
	
	// TODO(darrinm): and free buffers?
}

bool SdlSoundDevice::Init()
{
    // Set up streaming
	m_spec.freq = 8000;
	m_spec.format = AUDIO_U8;
	m_spec.channels = 1;
	m_spec.samples = kcbBuffer;
	m_spec.callback = AudioCallback;
	m_spec.userdata = this;
	
	if (SDL_OpenAudio(&m_spec, NULL) != 0)
		return false;
		
    for (int i = 0; i < kcBuffers; i++) {
        m_apbBuffers[i] = (byte *)malloc(m_spec.size);
        if (m_apbBuffers[i] == NULL) {
           return false;
        }
    }

    return true;
}

bool SdlSoundDevice::IsEnabled()
{
    return m_fEnable;
}

void SdlSoundDevice::Enable(bool fEnable)
{
	SDL_LockAudio();
	
    if (fEnable) {
        if (!m_fEnable) {
            memset(m_achnl, 0, sizeof(m_achnl));
            m_tSilence = 0;
            m_fEnable = true;
			#if 0
            for (int i = 0; i < kcBuffers; i++) {
                InitAudioBuffer(m_apbBuffers[i]);
            }
			#endif
			SDL_PauseAudio(0);
        }
    } else {
        if (m_fEnable) {
            m_fEnable = false;
            memset(m_achnl, 0, sizeof(m_achnl));
            m_tSilence = 0;
			SDL_PauseAudio(1);
        }
    }
	
	SDL_UnlockAudio();
}

int SdlSoundDevice::GetChannelCount()
{
    return kcChannels;
}

bool SdlSoundDevice::IsChannelFree(int ichnl)
{
    if (ichnl < 0 || ichnl >= kcChannels)
        return false;
    Channel *pchnl = &m_achnl[ichnl];
    return (pchnl->pb >= pchnl->pbEnd);
}

bool SdlSoundDevice::IsSilent()
{
    if (!m_fEnable)
        return true;
    if (m_tSilence == 0)
        return true;
    long tCurrent = HostGetTickCount();
    if (tCurrent < 0 && m_tSilence >= 0)
        return true;
    return tCurrent >= m_tSilence;
}

void SdlSoundDevice::ServiceProc()
{
}

void SdlSoundDevice::PlayAdpcm(int ichnl, byte *pb, word cb)
{
    if (ichnl < 0 || ichnl >= kcChannels)
        return;
    if (!m_fEnable)
        return;

    // Start it up

	SDL_LockAudio();
	
	Channel *pchnl = &m_achnl[ichnl];
	pchnl->pb = pb;
	pchnl->pbEnd = pb + cb;
	pchnl->nStepIndex = 0;
	pchnl->bSampleLast = 128;
	m_tSilence = HostGetTickCount() +
			(cb + kcBuffers * m_spec.size + 39) / 40 + 1;
	
	SDL_UnlockAudio();
}

void SdlSoundDevice::SetVolume(int nVolume)
{
    if (nVolume < 0)
        nVolume = 0;
    if (nVolume > 255)
        nVolume = 255;
#if 0 // TODO(darrinm): SDL has no volume control?
    AudioQueueSetParameter(m_haq, kAudioQueueParam_Volume,
            (Float32)nVolume / (Float32)255.0);
#endif
}

int SdlSoundDevice::GetVolume()
{
#if 0 // TODO(darrinm): SDL has no volume control?
    AudioQueueParameterValue value;
    AudioQueueGetParameter(m_haq, kAudioQueueParam_Volume,
            &value);
    return (int)(value * 255.0);
#else
	return 255;
#endif
}

void AudioCallback(void *pvUser, Uint8* stream, int len)
{
    SdlSoundDevice *psndd = (SdlSoundDevice *)pvUser;
    psndd->InitAudioBuffer(stream);
}

void SdlSoundDevice::InitAudioBuffer(byte *pbBuffer)
{
    // Don't send buffers to the device if not enabled

    if (!m_fEnable)
        return;

	SDL_LockAudio();

    // Fill the buffer and queue it for playing

    MixChannels(m_achnl, ARRAYSIZE(m_achnl), pbBuffer, m_spec.size);

	SDL_UnlockAudio();
}

} // namespace wi
