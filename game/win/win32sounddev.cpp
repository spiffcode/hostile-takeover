#include "..\ht.h"

#define kcChannels 4

class Win32SoundDevice : public SoundDevice // sndd
{
public:
	Win32SoundDevice();
	virtual ~Win32SoundDevice();

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
	void InitNextBufferForPlayback(int nBuffer);

	int m_cBuffers;
	int m_cbBuffer;
	HWAVEOUT m_hwavo;
	WAVEHDR *m_awavHeader;
	byte *m_abWaveData;
	Channel m_achnl[kcChannels];
	bool m_fEnable;
	long m_tSilence;
	int m_nVolumeLast;
	bool m_fVolumeFixed;
	CRITICAL_SECTION m_crit;

	friend void CALLBACK WaveOutProc(HWAVEOUT hwavo, UINT msg, dword dwInstance, dword wp, dword lp);
};

SoundDevice *CreateWin32SoundDevice()
{
	Win32SoundDevice *pwinsndd = new Win32SoundDevice;
	if (pwinsndd == NULL)
		return NULL;
	if (!pwinsndd->Init()) {
		delete pwinsndd;
		return NULL;
	}

	return (SoundDevice *)pwinsndd;
}

Win32SoundDevice::Win32SoundDevice()
{
	m_tSilence = 0;
	m_fEnable = false;
	m_hwavo = NULL;
	m_awavHeader = NULL;
	m_abWaveData = NULL;
	m_cBuffers = 0;
	m_cbBuffer = 0;
	m_fVolumeFixed = false;
    InitializeCriticalSection(&m_crit); 
}

Win32SoundDevice::~Win32SoundDevice()
{
	EnterCriticalSection(&m_crit);
	Enable(false);
	if (m_hwavo != NULL) {
		waveOutReset(m_hwavo);
		for (int n = 0; n < m_cBuffers; n++)
			waveOutUnprepareHeader(m_hwavo, &m_awavHeader[n], sizeof(WAVEHDR));
		if (m_hwavo != NULL) {
			waveOutClose(m_hwavo);
			m_hwavo = NULL;
		}
	}
	LeaveCriticalSection(&m_crit);
    DeleteCriticalSection(&m_crit);

	delete m_awavHeader;
	m_awavHeader = NULL;
	delete m_abWaveData;
	m_abWaveData = NULL;
}

bool Win32SoundDevice::Init()
{
#ifdef CE
	// Get version information. Buffer size will be based on this

	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx(&osver);
	
#define kdwBuildPocketPC 9348
#define kdwBuildPocketPC2002 11171

#define kcBuffersSlow 6
#define kcbBufferSlow 512

#define kcBuffersFast 6
#define kcbBufferFast 512

	m_cBuffers = kcBuffersFast;
	m_cbBuffer = kcbBufferFast;

	// Below PocketPC2002 latency performance sucks.

	if (osver.dwMajorVersion == 3) {
		// Is PocketPC but less than 2002?

		if (osver.dwMinorVersion == 0 && osver.dwBuildNumber < kdwBuildPocketPC2002) {
			m_cBuffers = kcBuffersSlow;
			m_cbBuffer = kcbBufferSlow;
		}
	}

#else
	// Windows
	
	m_cBuffers = 3;
	m_cbBuffer = 256;
#endif

	m_awavHeader = new WAVEHDR[m_cBuffers];
	if (m_awavHeader == NULL)
		return false;
	m_abWaveData = new byte[m_cBuffers * m_cbBuffer];
	if (m_abWaveData == NULL)
		return false;

	EnterCriticalSection(&m_crit);

	// Open sound device
	
	WAVEFORMATEX fmt;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = 1;
	fmt.nSamplesPerSec = 8000;
	fmt.nAvgBytesPerSec = 8000;
	fmt.nBlockAlign = 1;
	fmt.wBitsPerSample = 8;
	fmt.cbSize = 0;
	MMRESULT res = waveOutOpen(&m_hwavo, WAVE_MAPPER, &fmt,
			(dword)WaveOutProc, (dword)this, CALLBACK_FUNCTION);
	if (res != MMSYSERR_NOERROR) {
		LeaveCriticalSection(&m_crit);
		return false;
	}

	// Remember what the initial volume is

	m_nVolumeLast = GetVolume();

	// Init headers

	for (int n = 0; n < m_cBuffers; n++) {
		memset(&m_awavHeader[n], 0, sizeof(WAVEHDR));
		m_awavHeader[n].lpData = (char *)&m_abWaveData[n * m_cbBuffer];
		m_awavHeader[n].dwBufferLength = m_cbBuffer;
		waveOutPrepareHeader(m_hwavo, &m_awavHeader[n], sizeof(WAVEHDR));
	}

	// Some PocketPC devices disallow devices to change the volume. Detect if we're running on
	// one of these types of devices.

	int nVolumeSave = GetVolume();
	if (nVolumeSave >= 0x80) {
		SetVolume(nVolumeSave - 0x20);
	} else {
		SetVolume(nVolumeSave + 0x20);
	}
	int nVolumeNew = GetVolume();
	SetVolume(nVolumeSave);
	if (nVolumeNew == nVolumeSave)
		m_fVolumeFixed = true;

	LeaveCriticalSection(&m_crit);
	return true;
}

bool Win32SoundDevice::IsEnabled()
{
	return m_fEnable;
}

void Win32SoundDevice::Enable(bool fEnable)
{
	EnterCriticalSection(&m_crit);
	if (fEnable) {
		if (!m_fEnable) {
			memset(m_achnl, 0, sizeof(m_achnl));
			m_tSilence = 0;
			m_fEnable = true;
			for (int nBuffer = 0; nBuffer < m_cBuffers; nBuffer++) {
				m_awavHeader[nBuffer].dwFlags |= WHDR_DONE;
				InitNextBufferForPlayback(nBuffer);
			}
			SetSoundServiceDevice(this);
		}
	} else {
		if (m_fEnable) {
			memset(m_achnl, 0, sizeof(m_achnl));
			m_tSilence = 0;
			m_fEnable = false;
			SetSoundServiceDevice(NULL);
		}
	}
	LeaveCriticalSection(&m_crit);
}

int Win32SoundDevice::GetChannelCount()
{
	return kcChannels;
}

bool Win32SoundDevice::IsChannelFree(int ichnl)
{
	if (ichnl < 0 || ichnl >= kcChannels)
		return false;
	Channel *pchnl = &m_achnl[ichnl];
	return (pchnl->pb >= pchnl->pbEnd);
}

bool Win32SoundDevice::IsSilent()
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

void Win32SoundDevice::ServiceProc()
{
#ifdef CHECK_OLD_ALLOCS
	my_alloccheck();
#endif
}

void Win32SoundDevice::PlayAdpcm(int ichnl, byte *pb, word cb)
{
	if (ichnl < 0 || ichnl >= kcChannels)
		return;
	if (!m_fEnable)
		return;

	// Some PocketPC's reset the volume to the "system setting" when the
	// unit is turned off then turned back on. Reset the volume to our
	// setting in this case

	int nVolume = GetVolume();
	if (nVolume != m_nVolumeLast)
		SetVolume(m_nVolumeLast);

	// Start it up

	Channel *pchnl = &m_achnl[ichnl];
	pchnl->pb = pb;
	pchnl->pbEnd = pb + cb;
	pchnl->nStepIndex = 0;
	pchnl->bSampleLast = 128;
	m_tSilence = HostGetTickCount() + (cb + m_cBuffers * m_cbBuffer + 39) / 40 + 1;
}

void Win32SoundDevice::SetVolume(int nVolume)
{
	if (m_fVolumeFixed)
		return;

	if (nVolume < 0)
		nVolume = 0;
	if (nVolume > 255)
		nVolume = 255;
	dword dwVolume = nVolume | (nVolume << 8) | (nVolume << 16) | (nVolume << 24);
	EnterCriticalSection(&m_crit);
	waveOutSetVolume(m_hwavo, dwVolume);
	m_nVolumeLast = GetVolume();
	LeaveCriticalSection(&m_crit);
}

int Win32SoundDevice::GetVolume()
{
	if (m_fVolumeFixed)
		return -1;

	dword dwVolume;
	EnterCriticalSection(&m_crit);
	waveOutGetVolume(m_hwavo, &dwVolume);
	LeaveCriticalSection(&m_crit);
	return (int)(byte)dwVolume;
}

void CALLBACK WaveOutProc(HWAVEOUT hwavo, UINT msg, dword dwInstance, dword wp, dword lp)
{
	if (msg != WOM_DONE)
		return;
	Win32SoundDevice *pwinsndd = (Win32SoundDevice *)dwInstance;
	WAVEHDR *pwavHeader = (WAVEHDR *)wp;
	int nBuffer = pwavHeader - pwinsndd->m_awavHeader;
	pwinsndd->InitNextBufferForPlayback(nBuffer);
}

void Win32SoundDevice::InitNextBufferForPlayback(int nBuffer)
{
	// Don't send buffers to the device if not enabled

	if (!m_fEnable)
		return;

	// Fill the chunk and queue it for playing

	Assert(nBuffer >= 0 && nBuffer < m_cBuffers);
	if (nBuffer < 0 || nBuffer >= m_cBuffers)
		return;

	// This is protected by critical sections! The second thread and the UI thread
	// can both hang on waveOutWrite unless access is serialized.

	EnterCriticalSection(&m_crit);
	Assert(m_awavHeader[nBuffer].dwFlags & WHDR_DONE);
	m_awavHeader[nBuffer].dwFlags &= ~WHDR_DONE;
	MixChannels(m_achnl, ARRAYSIZE(m_achnl), &m_abWaveData[nBuffer * m_cbBuffer], m_cbBuffer);
	waveOutWrite(m_hwavo, &m_awavHeader[nBuffer], sizeof(WAVEHDR));

	// For AVI recording

#ifndef CE
	if (gpavir != NULL)
		gpavir->AddAudio(&m_abWaveData[nBuffer * m_cbBuffer], m_cbBuffer);
#endif

	LeaveCriticalSection(&m_crit);
}
