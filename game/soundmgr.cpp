#include "ht.h"

namespace wi {

SoundMgr gsndm;

SoundMgr::SoundMgr()
{
	m_apcmh = NULL;
	m_cpcmh = 0;
	m_asfxe = NULL;
	m_csfxe = 0;
	m_afmap = NULL;
	m_psndd = NULL;
	m_cChannels = 0;
	memset(m_anPriorityChannel, 0, sizeof(m_anPriorityChannel));
	m_fStateSaved = false;
}

SoundMgr::~SoundMgr()
{
	Assert(m_psndd == NULL);
}

bool SoundMgr::Init()
{
	// Open the sound device

	m_psndd = HostOpenSoundDevice();
	if (m_psndd == NULL)
		return false;

	FileMap fmap;
	byte *pbFiles = (byte *)gpakr.MapFile("soundfiles", &fmap);
	if (pbFiles == NULL)
		return false;

	// Load sounds - get count and load arrays

	bool fSuccess = true;
	m_cpcmh = BigWord(*(word *)pbFiles);
	pbFiles += 2;
	m_apcmh = new PcmHeader[m_cpcmh];
	if (m_apcmh == NULL) {
		gpakr.UnmapFile(&fmap);
		return false;
	}
	memset(m_apcmh, 0, sizeof(PcmHeader) * m_cpcmh);
	m_afmap = new FileMap[m_cpcmh];
	if (m_afmap == NULL) {
		gpakr.UnmapFile(&fmap);
		return false;
	}
	memset(m_afmap, 0, sizeof(FileMap) * m_cpcmh);

	// Map the sound files

	char *psz = (char *)pbFiles;
	for (int i = 0; i < m_cpcmh; i++) {
		dword cb;
		m_apcmh[i].pb = (byte *)gpakr.MapFile(psz, &m_afmap[i], &cb);
		if (m_apcmh[i].pb == NULL) {
			fSuccess = false;
			break;
		}
		m_apcmh[i].cb = (word)cb;
		if (i < m_cpcmh - 1)
			psz += strlen(psz) + 1;
	}
	gpakr.UnmapFile(&fmap);
	if (!fSuccess)
		return false;

	// Map SfxEntries

	dword cb;
	m_asfxe = (SfxEntry *)gpakr.MapFile("SfxEntries", &m_fmapSfxEntries, &cb);
	if (m_asfxe == NULL)
		return false;
	m_csfxe = cb / kcbSfxEntry;

	// Turn on sound device

	m_cChannels = m_psndd->GetChannelCount();

	return true;
}

void SoundMgr::Exit()
{
	delete m_psndd;
	m_psndd = NULL;

	for (int n = 0; n < m_cpcmh; n++) {
		if (m_apcmh[n].pb != NULL) {
			gpakr.UnmapFile(&m_afmap[n]);
			m_apcmh[n].pb = NULL;
		}
	}
	delete m_apcmh;
	m_apcmh = NULL;
	delete m_afmap;
	m_afmap = NULL;
	m_cpcmh = 0;

	if (m_asfxe != NULL) {
		gpakr.UnmapFile(&m_fmapSfxEntries);
		m_asfxe = NULL;
	}
}

void SoundMgr::Enable(bool fEnable)
{
	if (m_psndd != NULL)
		m_psndd->Enable(fEnable);
}

bool SoundMgr::IsEnabled()
{
	if (m_psndd != NULL)
		return m_psndd->IsEnabled();
	return false;
}

// Our game conflicts with OS audio on some devices like Clie. We try to detect these cases and turn audio
// completely off

void SoundMgr::RestoreState()
{
	if (m_fStateSaved) {
		m_fStateSaved = false;
		if (m_psndd == NULL)
			m_psndd = HostOpenSoundDevice();
		Enable(m_fEnabledSav);
		SetVolume(m_nVolumeSav);
	}
}

bool SoundMgr::SaveStateAndClear()
{
	if (m_fStateSaved)
		return false;
	m_fStateSaved = true;
	m_fEnabledSav = IsEnabled();
	m_nVolumeSav = GetVolume();
	delete m_psndd;
	m_psndd = NULL;
	return true;
}

void SoundMgr::PlaySfx(Sfx sfx)
{
	// Get the sfx entry and channel

	if (m_psndd == NULL)
		return;
	if (!m_psndd->IsEnabled())
		return;
	int nSfx = (int)sfx;
	if (nSfx < 0 || nSfx > m_csfxe)
		return;
	SfxEntry *psfxe = (SfxEntry *)((byte *)m_asfxe + sfx * kcbSfxEntry);
	if (psfxe->nSound == 0xff)
		return;

	// Find a free channel, a channel playing the same priority sound effect,
	// or channel playing a lower priority sound effect

	// First look to see if this category effect is playing

	int ichnlUse = -1;
	byte nPriorityLowest = 0;

#if 0
// Problem: This'll cause one equal priority sound effect to replace another
// of the same priority even if there are free channels to use.

	for (int ichnl = 0; ichnl < m_cChannels; ichnl++) {
		if (m_anPriorityChannel[ichnl] == psfxe->nPriority) {
			nPriorityLowest = 255;
			ichnlUse = ichnl;
			break;
		}
	}
#endif

	// If not playing, see if there is an empty channel. Keep track of lowest
	// priority channel

	if (ichnlUse == -1) {
		for (int ichnl = 0; ichnl < m_cChannels; ichnl++) {
			if (m_psndd->IsChannelFree(ichnl)) {
				nPriorityLowest = 255;
				ichnlUse = ichnl;
				break;
			}
			if (m_anPriorityChannel[ichnl] >= nPriorityLowest) {
				nPriorityLowest = m_anPriorityChannel[ichnl];
				ichnlUse = ichnl;
			}
		}
	}

	// We need to replace a lower priority sound effect
	// (higher numbers are lower priority)

	Assert(ichnlUse != -1);
	if (psfxe->nPriority > nPriorityLowest)
		return;
	m_anPriorityChannel[ichnlUse] = psfxe->nPriority;

	// Play the sound

	PcmHeader *pcmh = &m_apcmh[psfxe->nSound];
	m_psndd->PlayAdpcm(ichnlUse, pcmh->pb, pcmh->cb);
}

void SoundMgr::WaitSilence()
{
	if (m_psndd == NULL)
		return;
	while (!m_psndd->IsSilent()) {
		HostSleep(1);
		HostSoundServiceProc();
	}
}

long SoundMgr::FilterSleepTicks(long ct)
{
	// HACK:
    // On PocketPC sound is processed on an OS callback so we don't need to
    // limit sleeping Further, if we DO limit sleeping we will cause problems
    // for the background Bluetooth communications threads.

#if !defined(CE) && !defined(IPHONE)
	// If we have sound effects to play, don't sleep

	if (m_psndd != NULL) {
		if (!m_psndd->IsSilent())
			return 0;
	}
#endif
	return ct;
}

void SoundMgr::SetVolume(int nVolume)
{
	if (m_psndd != NULL)
		m_psndd->SetVolume(nVolume);
}

int SoundMgr::GetVolume()
{
	if (m_psndd != NULL)
		return m_psndd->GetVolume();
	return -1;
}

} // namespace wi
