#include "ht.h"

namespace wi {

// OPT: dynamically build an index of pointers to each StripData, FrameData if perf critical

#define GetStripDataPtr(nStrip) ((StripData *)(((byte *)m_panih) + BigDword(m_panih->aoffStpd[nStrip])))
#define GetFrameDataPtr(nStrip, nFrame) (GetStripDataPtr(nStrip)->GetFrameData(nFrame))

AnimationData *LoadAnimationData(const char *pszAniName)
{
	AnimationData *panid = new AnimationData();
	Assert(panid != NULL, "out of memory!");
	if (panid == NULL)
		return NULL;
		
	if (!panid->Init(pszAniName)) {
		delete panid;
		return NULL;
	}

	return panid;
}

AnimationData::AnimationData()
{
	m_panih = NULL;
	m_aptbm = NULL;
    m_ctbm = 0;
}

AnimationData::~AnimationData()
{
	if (m_panih != NULL)
		gpakr.UnmapFile(&m_fmap);
    for (int n = 0; n < m_ctbm; n++) {
        if (m_aptbm[n] != NULL)
            delete m_aptbm[n];
    }
	delete[] m_aptbm;
}

bool AnimationData::Init(const char *pszAniName)
{
	m_panih = (AnimationFileHeader *)gpakr.MapFile((char *)pszAniName, &m_fmap);
	if (m_panih == NULL)
		return false;

#if 0
	// UNDONE: incorporate the bitmaps directly inside the animation file?

	char szTbmName[kcbFilename];
	int cch = (int)strlen(pszAniName);

	// Animation files must end with ".anir" for this to work

	Assert((pszAniName[cch - 4] | 0x20) == 'a' && (pszAniName[cch - 3] | 0x20) == 'n' &&
			(pszAniName[cch - 2] | 0x20) == 'i' && (pszAniName[cch - 1] | 0x20) == 'r', 
			"Animation files must end with extension \".anir\"");
	strncpy(szTbmName, pszAniName, cch - 4);
	szTbmName[cch - 4] = 0;
	strcat(szTbmName, "tbm");
	
	m_ptbm = LoadHTBitmap(szTbmName);
	if (m_ptbm == NULL) {
		Assert("unable to load HTBitmap %s", szTbmName);
		return false;
	}
#endif

    // Iterate over all the strips and frames to get a total count
    // of images necessary

    for (int nStrip = 0; nStrip < GetStripCount(); nStrip++) {
        int nStripFrames = GetStripDataPtr(nStrip)->GetFrameCount();

        for (int nFrame = 0; nFrame < nStripFrames; nFrame++) {

            FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);

            if (BigWord(pfrmd->ibm) != 65535)
                m_ctbm++;

            if (BigWord(pfrmd->ibm2) != 65535)
                m_ctbm++;
        }
    }

    m_aptbm = new TBitmap*[m_ctbm]();
    if (m_aptbm == NULL)
        return false;

    for (int nStrip = 0; nStrip < GetStripCount(); nStrip++) {
        int nStripFrames = GetStripDataPtr(nStrip)->GetFrameCount();

        for (int nFrame = 0; nFrame < nStripFrames; nFrame++) {
            FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);

            word ibm = BigWord(pfrmd->ibm);
            word ibm2 = BigWord(pfrmd->ibm2);

            if (ibm != 65535) {
                m_aptbm[ibm] = CreateTBitmap(pfrmd->szName);
                if (m_aptbm[ibm] == NULL)
                    return false;
            }

            if (ibm2 != 65535) {
                m_aptbm[ibm2] = CreateTBitmap(pfrmd->szName2);
                if (m_aptbm[ibm2] == NULL)
                    return false;
            }
        }
    }

	return true;
}

int AnimationData::GetStripCount()
{
	return BigDword(m_panih->cstpd);
}

int AnimationData::GetFrameCount(int nStrip)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());

	return GetStripDataPtr(nStrip)->GetFrameCount();
}

void AnimationData::GetFrameOrigin(int nStrip, int nFrame, Point *pptOrigin)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());
	Assert(nFrame >= 0 && nFrame < GetStripDataPtr(nStrip)->GetFrameCount());

	FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);
	pptOrigin->x = pfrmd->xOrigin;
	pptOrigin->y = pfrmd->yOrigin;
}

void AnimationData::GetSpecialPoint(int nStrip, int nFrame, Point *pptSpecial)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());
	Assert(nFrame >= 0 && nFrame < GetStripDataPtr(nStrip)->GetFrameCount());

	FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);
	pptSpecial->x = (char)pfrmd->bCustomData1;
	pptSpecial->y = (char)pfrmd->bCustomData2;
}

int AnimationData::GetFrameDelay(int nStrip, int nFrame)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());
	Assert(nFrame >= 0 && nFrame < GetStripDataPtr(nStrip)->GetFrameCount());

	FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);
	return pfrmd->cHold;
}

void AnimationData::GetBounds(int nStrip, int nFrame, Rect *prc)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());
	Assert(nFrame >= 0 && nFrame < GetStripDataPtr(nStrip)->GetFrameCount());

	FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);
	Size siz;

	// If there is no first bitmap (e.g., delay-only frame) set the bounds
	// to empty.

	if (BigWord(pfrmd->ibm) == 65535) {
		prc->SetEmpty();
	} else {
        m_aptbm[BigWord(pfrmd->ibm)]->GetSize(&siz);
		prc->left = -pfrmd->xOrigin;
		prc->right = prc->left + siz.cx;
		prc->top = -pfrmd->yOrigin;
		prc->bottom = prc->top + siz.cy;
	}

	// If there is a second bitmap return the union of its bounds and
	// the bounds of the first bitmap.

	if (BigWord(pfrmd->ibm2) != 65535) {
        m_aptbm[BigWord(pfrmd->ibm2)]->GetSize(&siz);
		int xL = -pfrmd->xOrigin2;
		if (prc->left > xL)
			prc->left = xL;
		int yT = -pfrmd->yOrigin2;
		if (prc->top > yT)
			prc->top = yT;
		int xR = xL + siz.cx;
		if (prc->right < xR)
			prc->right = xR;
		int yB = yT + siz.cy;
		if (prc->bottom < yB)
			prc->bottom = yB;
	}
}

void AnimationData::DrawFrame(int nStrip, int nFrame, DibBitmap *pbm, int x, int y, Side side, Rect *prcSrc)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());
	Assert(nFrame >= 0 && nFrame < GetStripDataPtr(nStrip)->GetFrameCount());

	FrameData *pfrmd = GetFrameDataPtr(nStrip, nFrame);
	if (BigWord(pfrmd->ibm2) != 65535)
        m_aptbm[BigWord(pfrmd->ibm2)]->BltTo(pbm, x - pfrmd->xOrigin2, y - pfrmd->yOrigin2, side, prcSrc);
	if (BigWord(pfrmd->ibm) != 65535)
        m_aptbm[BigWord(pfrmd->ibm)]->BltTo(pbm, x - pfrmd->xOrigin, y - pfrmd->yOrigin, side, prcSrc);
}

int AnimationData::GetStripIndex(const char *pszStripName)
{
	// OPT: Can switch to a binary search if needed
	dword *poffStpd = m_panih->aoffStpd;
	int cstpd = GetStripCount();
	for (int i = 0; i < cstpd; i++, poffStpd++) {
		StripData *pstpd = (StripData *)(((byte *)m_panih) + BigDword(*poffStpd));
		if (stricmp(pszStripName, pstpd->GetName()) == 0)
			return i;
	}

	Assert("Strip not found");
	return -1;
}

int AnimationData::GetStripDelay(int nStrip)
{
	Assert(nStrip >= 0 && nStrip < GetStripCount());

	return GetStripDataPtr(nStrip)->GetDefaultDelay();
}

//
// Animation class implementation
//

void Animation::Init(AnimationData *panid)
{
	m_panid = panid;
	m_nStrip = 0;
	m_nFrame = 0;
	m_cDelay = m_panid->GetStripDelay(m_nStrip);
}

#define knVerAnimationState 1
bool Animation::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerAnimationState)
		return false;
	m_nStrip = pstm->ReadByte();
	m_nFrame = pstm->ReadByte();

	m_cDelay = pstm->ReadByte();
	m_cCountdown = pstm->ReadByte();
	m_wf = pstm->ReadWord();

	// This hack deals with the fact that different resolution versions
	// of the same animation may have different numbers of frames. When an
	// animation is saved it may include a frame index that exceeds what
	// is allowed when it is reloaded at a different resolution. In this
	// case we force the frame within the range and presume that nobody
	// will notice.

	int cfrm = m_panid->GetFrameCount(m_nStrip);
	if (m_nFrame >= cfrm) {
		m_nFrame = cfrm - 1;
		m_cCountdown = m_cDelay + m_panid->GetFrameDelay(m_nStrip, m_nFrame);
	}

	return pstm->IsSuccess();
}

bool Animation::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerAnimationState);
	pstm->WriteByte(m_nStrip);
	pstm->WriteByte(m_nFrame);
	pstm->WriteByte(m_cDelay);
	pstm->WriteByte(m_cCountdown);
	pstm->WriteWord(m_wf);
	return pstm->IsSuccess();
}

void Animation::SetStrip(const char *pszStripName)
{
	m_nStrip = m_panid->GetStripIndex(pszStripName);
	m_cCountdown = m_panid->GetStripDelay(m_nStrip) + m_panid->GetFrameDelay(m_nStrip, 0);
}

bool Animation::Start(const char *pszStripName, word wf)
{
	return Start(m_panid->GetStripIndex(pszStripName), 0, wf);
}

bool Animation::Start(int nStrip, int nFrame, word wf) 
{
	Assert(nStrip >= 0 && nStrip < m_panid->GetStripCount());

	m_nStrip = nStrip;
	if (nFrame == -1)
		m_nFrame = m_panid->GetFrameCount(m_nStrip) - 1;
	else
		m_nFrame = nFrame;

	Assert(m_nFrame >= 0 && m_nFrame < m_panid->GetFrameCount(m_nStrip));

	m_wf = wf | (m_wf & kfAniFreeAnimationData);
	m_cCountdown = m_panid->GetStripDelay(m_nStrip) + m_panid->GetFrameDelay(m_nStrip, m_nFrame);

	// Don't actually advance the animation the first time called.
	// This may seem strange but it ensures that the first frame will
	// be displayed for at least one Update before moving on.

	if (m_wf & kfAniIgnoreFirstAdvance) {
		if (m_cCountdown < 255)
			m_cCountdown++;
	}

	// If this animation only has one frame, remove the loop bit.
	// This will take much less cpu since it won't be calling ::Advance() much. 

	if ((m_wf & kfAniLoop) && m_panid->GetFrameCount(m_nStrip) == 1)
		m_wf &= ~kfAniLoop;

	return (m_wf & kfAniDone) == 0;
}


// update m_nFrame as appropriate given delays that may be in the animation and
// deal with end conditions
bool Animation::Advance(int cAdvance)
{
	if (m_wf & kfAniDone)
		return false;

	if (m_cCountdown >= cAdvance) {
		m_cCountdown -= cAdvance;
		return true;
	}

	if (m_nFrame + 1 < m_panid->GetFrameCount(m_nStrip)) {
		m_nFrame++;
	} else {
		if (m_wf & kfAniLoop) {
			m_nFrame = 0;
		} else {
			m_wf |= kfAniDone;
			if (m_wf & kfAniResetWhenDone) {
				m_nFrame = 0;
				m_cCountdown = 255;
				return true;
			}
			return false;
		}
	}

	m_cCountdown = m_cDelay + m_panid->GetFrameDelay(m_nStrip, m_nFrame);
	return true;
}

long Animation::GetRemainingStripTime()
{
	// NOTE: doesn't take update interval into account. We can add it if needed.

	int c = m_cCountdown;
	int cfrm = m_panid->GetFrameCount(m_nStrip);
	for (int ifrm = m_nFrame + 1; ifrm < cfrm; ifrm++)
		c += 1 + m_cDelay + m_panid->GetFrameDelay(m_nStrip, ifrm);

	return c * kctUpdate;
}

#ifdef MP_DEBUG_SHAREDMEM
void Animation::MPValidate(Animation *paniRemote)
{
	MPValidateMember(Animation, m_nStrip, paniRemote);
	MPValidateMember(Animation, m_nFrame, paniRemote);
	MPValidateMember(Animation, m_wf, paniRemote);
	MPValidateMember(Animation, m_cCountdown, paniRemote);
	MPValidateMember(Animation, m_cDelay, paniRemote);
}
#endif

} // namespace wi
