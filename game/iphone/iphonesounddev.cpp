#include "../ht.h"

#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioServices.h>
#include "../sounddevice.h"
#include "../mixer.h"
#include "criticalsection.h"
#include "iphone.h"

namespace wi {

#define kcBuffers 4
#define kcbBuffer 256
#define kcChannels 4

extern long HostGetTickCount();
extern void SetSoundServiceDevice(SoundDevice *psndd);
void AudioCallback(void *pvUser, AudioQueueRef haq, AudioQueueBuffer *paqb);

class IPhoneSoundDevice : public SoundDevice // sndd
{
public:
    IPhoneSoundDevice();
    virtual ~IPhoneSoundDevice();

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
    void InitAudioBuffer(AudioQueueBuffer *paqb);
    void InterruptionListener(void *data, UInt32 interruptionState);

    bool m_fEnable;
    long m_tSilence;
    AudioQueueRef m_haq;
    AudioQueueBuffer *m_apaqb[kcBuffers];
    Channel m_achnl[kcChannels];
    base::CriticalSection m_crit;

    friend void AudioCallback(void *pvUser, AudioQueueRef haq,
            AudioQueueBuffer *paqb);
    friend void InterruptionListener(void *data, UInt32 interruptionState);
};

SoundDevice *CreateIPhoneSoundDevice()
{
    IPhoneSoundDevice *psndd = new IPhoneSoundDevice;
    if (psndd == NULL)
        return NULL;
    if (!psndd->Init()) {
        delete psndd;
        return NULL;
    }
    return (SoundDevice *)psndd;
}

void InterruptionListener(void *data, UInt32 interruptionState)
{
    IPhoneSoundDevice *psndd = (IPhoneSoundDevice *)data;
    psndd->InterruptionListener(data, interruptionState);
}

IPhoneSoundDevice::IPhoneSoundDevice()
{
    m_haq = NULL;
    for (int i = 0; i < ARRAYSIZE(m_apaqb); i++) {
        m_apaqb[i] = NULL;
    }
    m_fEnable = false;
    m_tSilence = 0;
}

IPhoneSoundDevice::~IPhoneSoundDevice()
{
    if (m_haq != NULL) {
        AudioQueueDispose(m_haq, true);
    }
}

bool IPhoneSoundDevice::Init()
{
    // Initialize the default audio session object to tell it
    // to allow background music, and to tell us when audio
    // gets resumed (like if a phone call comes in, iphone takes
    // over audio. If the user then ignores the phone call, the
    // audio needs to be turned on again.

    AudioSessionInitialize(NULL, NULL, (AudioSessionInterruptionListener)wi::InterruptionListener, this);
    UInt32 category = kAudioSessionCategory_UserInterfaceSoundEffects;
    AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,
            sizeof(category), &category);
    AudioSessionSetActive(true);

    // Set up streaming

    AudioStreamBasicDescription desc;
    desc.mSampleRate = 8000;
    desc.mFormatID = kAudioFormatLinearPCM;
    desc.mFormatFlags = kAudioFormatFlagIsPacked;
    desc.mBytesPerPacket = 1;
    desc.mFramesPerPacket = 1;
    desc.mBytesPerFrame = 1;
    desc.mChannelsPerFrame = 1;
    desc.mBitsPerChannel = 8;

    OSStatus err = AudioQueueNewOutput(&desc, AudioCallback, this,
            NULL,
            kCFRunLoopCommonModes,
            0, &m_haq);
    if (err != 0) {
        return false;
    }

    for (int i = 0; i < kcBuffers; i++) {
        err = AudioQueueAllocateBuffer(m_haq, kcbBuffer, &m_apaqb[i]);
        if (err != 0) {
           return false;
        }
    }

    return true;
}

void IPhoneSoundDevice::InterruptionListener(void *inClientData, UInt32 interruptionState) {
    switch (interruptionState) {
    case kAudioSessionBeginInterruption:
        AudioSessionSetActive(false);
        break;

    case kAudioSessionEndInterruption:
        AudioSessionSetActive(true);
        break;
    }
}

bool IPhoneSoundDevice::IsEnabled()
{
    return m_fEnable;
}

void IPhoneSoundDevice::Enable(bool fEnable)
{
    base::CritScope cs(&m_crit);

    if (fEnable) {
        if (!m_fEnable) {
            memset(m_achnl, 0, sizeof(m_achnl));
            m_tSilence = 0;
            m_fEnable = true;
            for (int i = 0; i < kcBuffers; i++) {
                InitAudioBuffer(m_apaqb[i]);
            }
            AudioQueuePrime(m_haq, 0, NULL);
            AudioQueueStart(m_haq, NULL);
            SetSoundServiceDevice(this);
        }
    } else {
        if (m_fEnable) {
            m_fEnable = false;
            memset(m_achnl, 0, sizeof(m_achnl));
            m_tSilence = 0;
            AudioQueueStop(m_haq, false);
            SetSoundServiceDevice(NULL);
        }
    }
}

int IPhoneSoundDevice::GetChannelCount()
{
    return kcChannels;
}

bool IPhoneSoundDevice::IsChannelFree(int ichnl)
{
    if (ichnl < 0 || ichnl >= kcChannels)
        return false;
    Channel *pchnl = &m_achnl[ichnl];
    return (pchnl->pb >= pchnl->pbEnd);
}

bool IPhoneSoundDevice::IsSilent()
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

void IPhoneSoundDevice::ServiceProc()
{
}

void IPhoneSoundDevice::PlayAdpcm(int ichnl, byte *pb, word cb)
{
    if (ichnl < 0 || ichnl >= kcChannels)
        return;
    if (!m_fEnable)
        return;

    // Start it up

    {
        base::CritScope cs(&m_crit);

        Channel *pchnl = &m_achnl[ichnl];
        pchnl->pb = pb;
        pchnl->pbEnd = pb + cb;
        pchnl->nStepIndex = 0;
        pchnl->bSampleLast = 128;
        m_tSilence = HostGetTickCount() +
                (cb + kcBuffers * kcbBuffer + 39) / 40 + 1;
    }
}

void IPhoneSoundDevice::SetVolume(int nVolume)
{
    if (nVolume < 0)
        nVolume = 0;
    if (nVolume > 255)
        nVolume = 255;

    AudioQueueSetParameter(m_haq, kAudioQueueParam_Volume,
            (Float32)nVolume / (Float32)255.0);
}

int IPhoneSoundDevice::GetVolume()
{
    AudioQueueParameterValue value;
    AudioQueueGetParameter(m_haq, kAudioQueueParam_Volume,
            &value);
    return (int)(value * 255.0);
}

void AudioCallback(void *pvUser, AudioQueueRef haq, AudioQueueBuffer *paqb)
{
    IPhoneSoundDevice *psndd = (IPhoneSoundDevice *)pvUser;
    psndd->InitAudioBuffer(paqb);
}

void IPhoneSoundDevice::InitAudioBuffer(AudioQueueBuffer *paqb)
{
    base::CritScope cs(&m_crit);

    // Don't send buffers to the device if not enabled

    if (!m_fEnable)
        return;

    // Fill the buffer and queue it for playing

    paqb->mAudioDataByteSize = kcbBuffer;
    MixChannels(m_achnl, ARRAYSIZE(m_achnl), (byte *)paqb->mAudioData,
            kcbBuffer);
    AudioQueueEnqueueBuffer(m_haq, paqb, 0, NULL);
}

} // namespace wi
