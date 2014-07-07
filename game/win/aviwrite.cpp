#include "..\ht.h"
#include <windowsx.h>
#include <memory.h>
#include <mmsystem.h>
#include <time.h>

AviRecorder *gpavir;

AviRecorder::AviRecorder()
{
	m_pavif = NULL;
	m_pstmVideo = NULL;
	m_pstmAudio = NULL;
	m_pbmFlip = NULL;
	m_ptbmPointer = NULL;
	m_nSample = 0;
	m_fAudioReady = false;
}

AviRecorder::~AviRecorder()
{
	Stop();
}

bool AviRecorder::Start(int cx, int cy, char *pszFn)
{
#ifndef DEV_BUILD
	// Don't let mortals record avis

	return false;
#endif

#if 1
	// Modify cx & cy to be 4 by 3 for NTSC video,
	// which is what Microsoft Movie Maker makes.

	if (cx > cy) {
		int cyT = cx * 3 / 4;
		if (cyT < cy) {
			cx = cy * 4 / 3;
		} else {
			cy = cyT;
		}
	} else {
		int cxT = cy * 4 / 3;
		if (cxT < cx) {
			cy = cx * 3 / 4;
		} else {
			cx = cxT;
		}
	}
#endif

	// Now dword align the dst, for Windows sake

	cx = (cx + 3) & ~3;

	// Create a temp buffer that we'll use for flipping the dib since it needs to
	// be upside down for windows.

	m_pbmFlip = CreateDibBitmap(NULL, cx, cy);
	if (m_pbmFlip == NULL)
		return false;

	// Init

	m_ptbmPointer = LoadTBitmap("arrow5.tbm");
	AVIFileInit();

	// Create filename

	if (pszFn == NULL) {
		char szFn[MAX_PATH];
		strcpy(szFn, "c:\\ht.avi");
		pszFn = szFn;
	}

	// Open AVI file

	HRESULT hr = AVIFileOpen(&m_pavif, pszFn, OF_CREATE | OF_WRITE, NULL);
	if (hr != AVIERR_OK)
		return false;

	// Create the video streams if they don't exist

	AVISTREAMINFO stmInfo;
	memset(&stmInfo, 0, sizeof(stmInfo));
	stmInfo.fccType = streamtypeVIDEO;
	stmInfo.fccHandler = 0; // mmioFOURCC('M','S','V','C');
	stmInfo.dwScale = 2;
	stmInfo.dwRate = 25;
	stmInfo.dwSuggestedBufferSize = cx * cy;
	stmInfo.dwQuality = 0; // 10000;
	SetRect(&stmInfo.rcFrame, 0, 0, cx, cy);

	// Create video stream

	if (m_pstmVideo == NULL) {
		hr = AVIFileCreateStream(m_pavif, &m_pstmVideo, &stmInfo);
		if (hr != AVIERR_OK)
			return false;
	}

	// Set the stream format

	struct Header {
		BITMAPINFOHEADER bih;
		RGBQUAD argbqPal[256];
	};

	Header hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.bih.biSize = sizeof(hdr.bih);
	hdr.bih.biWidth = cx;
	hdr.bih.biHeight = cy;
	hdr.bih.biPlanes = 1;
	hdr.bih.biBitCount = 8;
	hdr.bih.biCompression = BI_RGB;
	hdr.bih.biClrUsed = 256;

	// Get the palette

	gpdisp->GetWindowsPalette(hdr.argbqPal);

	// Set the format

	hr = AVIStreamSetFormat(m_pstmVideo, 0, &hdr, sizeof(hdr));
	if (hr != AVIERR_OK)
		return false;

	// When recording started, useful for keeping video in sync

	m_tStart = HostGetTickCount();

	// Now create the audio stream

	memset(&stmInfo, 0, sizeof(stmInfo));
	stmInfo.fccType = streamtypeAUDIO;
	stmInfo.fccHandler = 0;
	stmInfo.dwScale = 1;
	stmInfo.dwRate = 8000;

	// Create audio stream

	if (m_pstmAudio == NULL) {
		hr = AVIFileCreateStream(m_pavif, &m_pstmAudio, &stmInfo);
		if (hr != AVIERR_OK)
			return false;
	}

	// Set the audio format

	WAVEFORMATEX fmt;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nChannels = 1;
	fmt.nSamplesPerSec = 8000;
	fmt.nAvgBytesPerSec = 8000;
	fmt.nBlockAlign = 1;
	fmt.wBitsPerSample = 8;
	fmt.cbSize = 0;

	// Set the format

	hr = AVIStreamSetFormat(m_pstmAudio, 0, &fmt, sizeof(fmt));
	if (hr != AVIERR_OK)
		return false;
	
	// Now we can allow the audio thread to call

	m_fAudioReady = true;

	return true;
}

void AviRecorder::Stop()
{
	m_fAudioReady = false;

	if (m_pstmVideo != NULL) {
		AVIStreamClose(m_pstmVideo);
		m_pstmVideo = NULL;
	}

	if (m_pstmAudio != NULL) {
		AVIStreamClose(m_pstmAudio);
		m_pstmAudio = NULL;
	}

	if (m_pavif != NULL) {
		AVIFileClose(m_pavif);
		m_pavif = NULL;
	}

	AVIFileExit();

	m_nSample = 0;
	delete m_pbmFlip;
	m_pbmFlip = NULL;
	delete m_ptbmPointer;
	m_ptbmPointer = NULL;
}

void AviRecorder::AddFrame(DibBitmap *pbm)
{
	// Find the center of the flip dib

	Size sizSrc;
	pbm->GetSize(&sizSrc);
	Size sizDst;
	m_pbmFlip->GetSize(&sizDst);
	int xDst = (sizDst.cx - sizSrc.cx) / 2;
	int yDst = (sizDst.cy - sizSrc.cy) / 2;
	int cbRowDst = m_pbmFlip->GetRowBytes();
	int cbRowSrc = pbm->GetRowBytes();

	// Flip the dib for Windows' sake into this spot

	m_pbmFlip->Clear(GetColor(kiclrBlack));
	byte *pbSrc = pbm->GetBits();
	byte *pbDst = m_pbmFlip->GetBits();
	for (int y = 0; y < sizSrc.cy; y++)
		memcpy(&pbDst[(yDst + y) * cbRowDst + xDst], &pbSrc[(sizSrc.cy - y - 1) * cbRowSrc], cbRowSrc);

	// Take out for screenshot purposes
	// Point a pointer in there to represent where the mouse / stylus is

	Size sizT;
	m_ptbmPointer->GetSize(&sizT);
	int xT = xDst + gxPenLast - 5;
	int yT = yDst + sizSrc.cy - gyPenLast - (sizT.cy - 5);
	m_ptbmPointer->BltTo(m_pbmFlip, xT, yT, GetKeyState(VK_LBUTTON) < 0 ? kside2 : ksideNeutral, NULL);

	// Write to the video stream

	int nFrame = (HostGetTickCount() - m_tStart) / 8;
	AVIStreamWrite(m_pstmVideo, nFrame, 1, pbDst, sizDst.cx * sizDst.cy, 0, NULL, NULL);
}

void AviRecorder::AddAudio(byte *pb8Unsigned, dword cb)
{
	if (!m_fAudioReady)
		return;

	// Write to the audio stream

	AVIStreamWrite(m_pstmAudio, m_nSample, cb, pb8Unsigned, cb, 0, NULL, NULL);
	m_nSample += cb;
}