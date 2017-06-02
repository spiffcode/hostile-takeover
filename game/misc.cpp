#include "ht.h"
#include "wistrings.h"

namespace wi {

// GCCBUG: We must not have this inline because when Gcc inlines the call to
// the base classes' constructor (Timer::Timer) it gets the reference wrong.

TimeoutTimer::TimeoutTimer() 
{
	m_ptimo = NULL;
}

//
// CommandQueue implementation
//

CommandQueue::CommandQueue()
{
	m_amsg = NULL;
	m_cmsg = 0;
}

CommandQueue::~CommandQueue()
{
	Assert(m_amsg == NULL);
}

bool CommandQueue::Init(int cmsgMax)
{
	m_cmsg = 0;
	m_cmsgMax = cmsgMax;
	Assert(m_amsg == NULL);
	m_amsg = (Message *)gmmgr.AllocPtr(sizeof(Message) * m_cmsgMax);
	return m_amsg != NULL;
}

void CommandQueue::Exit()
{
	if (m_amsg != NULL) {
		gmmgr.FreePtr(m_amsg);
		m_amsg = NULL;
	}
}

#define knVerCommandQueueState 1
bool CommandQueue::LoadState(Stream *pstm)
{
	// Version check

	byte nVer = pstm->ReadByte();
	if (nVer != knVerCommandQueueState)
		return false;

	// Read msg count

	int cmsgs = pstm->ReadWord();

	// Read in messages

	while (cmsgs-- != 0) {
		Message msg;
		if (pstm->Read(&msg, sizeof(msg)) == 0)
			return false;
		Enqueue(&msg);
	}

	return pstm->IsSuccess();
}

bool CommandQueue::SaveState(Stream *pstm)
{
	// Save version

	pstm->WriteByte(knVerCommandQueueState);

	// Save count of messages

	pstm->WriteWord(m_cmsg);

	// Save messages

	for (int nmsg = 0; nmsg < m_cmsg; nmsg++)
		pstm->Write(&m_amsg[nmsg], sizeof(m_amsg[0]));

	return pstm->IsSuccess();
}

void CommandQueue::Enqueue(MessageId mid, StateMachineId smidReceiver)
{
	Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.mid = mid;
	msg.smidReceiver = smidReceiver;
	SetEntry(&msg);
}

void CommandQueue::Enqueue(Message *pmsg)
{
	pmsg->tDelivery = 0;
	SetEntry(pmsg);
}

void CommandQueue::SetEntry(Message *pmsg)
{
	if (m_cmsg < m_cmsgMax) {
		gmmgr.WritePtr(m_amsg, m_cmsg * sizeof(Message), pmsg, sizeof(Message));
		m_cmsg++;
	}
}

bool IsTileFree(TCoord tx, TCoord ty, byte bf, Gob **ppgob)
{
	// Gob to return if there is one

	if (ppgob != NULL)
		*ppgob = NULL;

	// TCoord needs to be on the map

	Size sizMap;   
    gsim.GetLevel()->GetTileMap()->GetTCoordMapSize(&sizMap);   
	if (tx < 0 || tx >= sizMap.cx || ty < 0 || ty >= sizMap.cy)
		return false;

	// Needs to be free of gobs

	if (bf & (kbfMobileUnit | kbfStructure)) {
		for (Gid gid = ggobm.GetFirstGid(tx, ty); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
			Gob *pgob = ggobm.GetGob(gid);
			if (pgob != NULL) {
				if (ppgob != NULL)
					*ppgob = pgob;
				return false;
			}
		}
	}

	// No gobs can be transitioning into this tile

	if (bf & kbfMobileUnit) {
		Gob *pgob = MobileUnitGob::AnyTransitionsIntoTile(tx, ty, NULL);
		if (pgob != NULL) {
			if (ppgob != NULL)
				*ppgob = pgob;
			return false;
		}
	}

	// Tile can't be occupied

	if (gsim.GetLevel()->GetTerrainMap()->IsBlocked(tx, ty, bf))
		return false;

	// Looks free!

	return true;
}

void BringInBounds(WCoord *pwx, WCoord *pwy)
{
	Size sizMap;   
    gsim.GetLevel()->GetTileMap()->GetTCoordMapSize(&sizMap);
	WCoord cwxMap = WcFromTc(sizMap.cx);
	WCoord cwyMap = WcFromTc(sizMap.cy);
	if (*pwx < 0)
		*pwx = 0;
	else if (*pwx >= cwxMap)
		*pwx = cwxMap - 1;
	if (*pwy < 0)
		*pwy = 0;
	else if (*pwy >= cwyMap)
		*pwy = cwyMap - 1;
}

//
// Rect implementation
//

bool Rect::Intersect(Rect *prcSrc1, Rect *prcSrc2)
{
    left  = _max(prcSrc1->left, prcSrc2->left);
    right = _min(prcSrc1->right, prcSrc2->right);

    // Check for empty rect

    if (left < right) {

        top = _max(prcSrc1->top, prcSrc2->top);
        bottom = _min(prcSrc1->bottom, prcSrc2->bottom);

        // Check for empty rect

        if (top < bottom)
			return true;
    }

	SetEmpty();
    return false;
}

void Rect::Set(Point pt1, Point pt2) 
{
	if (pt1.x < pt2.x) {
		left = pt1.x;
		right = pt2.x;
	} else {
		left = pt2.x;
		right = pt1.x;
	}
	if (pt1.y < pt2.y) {
		top = pt1.y;
		bottom = pt2.y;
	} else {
		top = pt2.y;
		bottom = pt1.y;
	}
}

bool Rect::Subtract(Rect *prcSrc1, Rect *prcSrc2)
{
	// prcSrc1 - prcSrc2

	int xLeft = prcSrc1->left;
	int yTop = prcSrc1->top;
	int xRight = prcSrc1->right;
	int yBottom = prcSrc1->bottom;

	// Vert edges
	
	if (prcSrc2->top <= prcSrc1->top && prcSrc2->bottom >= prcSrc1->bottom) {
		// left edge

		if (prcSrc2->left <= prcSrc1->left && prcSrc2->right > prcSrc1->left)
			xLeft = prcSrc2->right;

		// right edge

		if (prcSrc2->left < prcSrc1->right && prcSrc2->right >= prcSrc1->right)
			xRight = prcSrc2->left;
	}

	// Horz edges

	if (prcSrc2->left <= prcSrc1->left && prcSrc2->right >= prcSrc1->right) {
		// top edge

		if (prcSrc2->top <= prcSrc1->top && prcSrc2->bottom > prcSrc1->top)
			yTop = prcSrc2->bottom;

		// bottom edge

		if (prcSrc2->top < prcSrc1->bottom && prcSrc2->bottom >= prcSrc1->bottom)
			yBottom = prcSrc2->top;
	}

	Set(xLeft, yTop, xRight, yBottom);
	return !IsEmpty();
}

void Rect::Add(Rect *prcSrc1, Rect *prcSrc2)
{
	if (prcSrc1->IsEmpty()) {
		*this = *prcSrc2;
		return;
	}
	if (prcSrc2->IsEmpty()) {
		*this = *prcSrc1;
		return;
	}

	// prcSrc1 + prcSrc2 (not Union)

	int xLeft = prcSrc1->left;
	int yTop = prcSrc1->top;
	int xRight = prcSrc1->right;
	int yBottom = prcSrc1->bottom;

	// Vert edges
	
	if (prcSrc2->top <= prcSrc1->top && prcSrc2->bottom >= prcSrc1->bottom) {
		// left edge

		if (prcSrc2->left < prcSrc1->left && prcSrc2->right >= prcSrc1->left)
			xLeft = prcSrc2->left;

		// right edge

		if (prcSrc2->left <= prcSrc1->right && prcSrc2->right > prcSrc1->right)
			xRight = prcSrc2->right;
	}

	// Horz edges

	if (prcSrc2->left <= prcSrc1->left && prcSrc2->right >= prcSrc1->right) {
		// top edge

		if (prcSrc2->top < prcSrc1->top && prcSrc2->bottom >= prcSrc1->top)
			yTop = prcSrc2->top;

		// bottom edge

		if (prcSrc2->top <= prcSrc1->bottom && prcSrc2->bottom > prcSrc1->bottom)
			yBottom = prcSrc2->bottom;
	}

	Set(xLeft, yTop, xRight, yBottom);
}

void Rect::FromWorldRect(WRect *pwrc)
{
	left = PcFromWc(pwrc->left);
	right = PcFromWc(pwrc->right);
	top = PcFromWc(pwrc->top);
	bottom = PcFromWc(pwrc->bottom);
}

void Rect::FromTileRect(TRect *ptrc)
{
	left = PcFromTc(ptrc->left);
	right = PcFromTc(ptrc->right);
	top = PcFromTc(ptrc->top);
	bottom = PcFromTc(ptrc->bottom);
}

void Rect::Union(Rect *prc)
{
	if (prc->left >= prc->right || prc->top >= prc->bottom)
		return;
	if (prc->left < left)
		left = prc->left;
	if (prc->top < top)
		top = prc->top;
	if (prc->right > right)
		right = prc->right;
	if (prc->bottom > bottom)
		bottom = prc->bottom;
}

bool Rect::Equal(Rect *prc)
{
	if (left == prc->left && top == prc->top && right == prc->right && bottom == prc->bottom)
		return true;
	return false;
}

void Rect::GetCenter(Point *ppt)
{
	ppt->x = left + (right - left) / 2;
	ppt->y = top + (bottom - top) / 2;
}

int Rect::GetDistance(int x, int y)
{
    // Use isqrt. If too expensive, could change this to
    // GetSquaredDistance and change callers.

    if (y >= bottom) {
        if (x >= right) {
            return isqrt(x - right, y - bottom);
        }
        if (x >= left) {
            return y - bottom;
        }
        return isqrt(left - x, y - bottom);
    } else if (y >= top) {
        if (x >= right) {
            return x - right;
        }
        if (x >= left) {
            return 0;
        }
        return left - x;
    } else {
        if (x >= right) {
            return isqrt(x - right, top - y);
        }
        if (x >= left) {
            return top - y;
        }
        return isqrt(left - x, top - y);
    }
}
    
//
// WRect implementation
//

void WRect::Set(WPoint wpt1, WPoint wpt2)
{
	if (wpt1.wx < wpt2.wx) {
		left = wpt1.wx;
		right = wpt2.wx;
	} else {
		left = wpt2.wx;
		right = wpt1.wx;
	}
	if (wpt1.wy < wpt2.wy) {
		top = wpt1.wy;
		bottom = wpt2.wy;
	} else {
		top = wpt2.wy;
		bottom = wpt1.wy;
	}
}

#if 0 // UNUSED
void WRect::FromPixelRect(Rect *prc)
{
	left = WcFromPc(prc->left);
	right = WcFromPc(prc->right);
	top = WcFromPc(prc->top);
	bottom = WcFromPc(prc->bottom);
}
#endif

void WRect::FromTileRect(TRect *ptrc)
{
	left = WcFromTc(ptrc->left);
	right = WcFromTc(ptrc->right);
	top = WcFromTc(ptrc->top);
	bottom = WcFromTc(ptrc->bottom);
}

// fgets

char *_fgets(char *psz, int cch, File *pfil)
{
	// Read into the buffer

	dword cbRead = gpakr.fread(psz, 1, cch, pfil);
	if (cbRead == 0)
		return NULL;
	psz[cbRead] = 0;

	// Look for a newline, and end there

	int cchOld = (int)strlen(psz);
	int nchNew;
	for (nchNew = 0; nchNew < cchOld; nchNew++) {
		if (psz[nchNew] == '\n') {
			nchNew++;
			psz[nchNew] = 0;
			break;
		}
	}

	// Set the file pointer backwards

	int cchNew = nchNew;
	gpakr.fseek(pfil, cchNew - cchOld, SEEK_CUR);
	return psz;
}

// String uppercasing function because PalmOS doesn't have one

void HtStrupr(char *psz)
{
	while (true) {
		char ch = *psz;
		if (ch == 0)
			break;
		if (ch >= 'a' && ch <= 'z')
			*psz = ch + 'A' - 'a';
		psz++;
	}
}

// Bring up an Hostile Takeover-style message box with a title and a formatted message

bool HtMessageBox(word wf, const char *pszTitle, const char *pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	char szBody[512];
	vsprintf(szBody, pszFormat, va);
	va_end(va);

	return HtMessageBox(kidfMessageBox, wf, pszTitle, szBody);
}

bool HtMessageBox(word idf, word wf, const char *pszTitle, const char *pszBody)
{
    DialogForm *pfrm = CreateHtMessageBox(idf, wf, pszTitle, pszBody);

	if (!(wf & kfMbKeepTimersEnabled)) {
		gtimm.Enable(false);
    }

	// Bring up form

	bool f = pfrm->DoModal();
	delete pfrm;

	if (!(wf & kfMbKeepTimersEnabled)) {
		gtimm.Enable(true);
    }

    return f;
}

DialogForm *CreateHtMessageBox(word idf, word wf, const char *pszTitle,
        const char *pszBody) {
	DialogForm *pfrm = (DialogForm *)gpmfrmm->LoadForm(gpiniForms, idf, new DialogForm());
	Assert(pfrm != NULL, "Unable to load HtMessageBox form");

	if (wf & kfMbWhiteBorder)
		pfrm->SetBorderColorIndex(kiclrWhite);

	pfrm->SetTitleColor(GetColor(kiclrRed));
	pfrm->SetBackgroundColorIndex(kiclrShadow2x);

	LabelControl *plbl = (LabelControl *)pfrm->GetControlPtr(kidcTitle);
	plbl->SetText(pszTitle);

	// The label self-adjusts its height. Use that to format the messagebox as well

	plbl = (LabelControl *)pfrm->GetControlPtr(kidcMessage);
	Rect rcOld;
	plbl->GetRect(&rcOld);
	plbl->SetText(pszBody);
	Rect rcNew;
	plbl->GetRect(&rcNew);

	// Move the buttons

	ButtonControl *pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcOk);
	Rect rcButton;
	pbtn->GetRect(&rcButton);
	rcButton.Offset(0, rcNew.Height() - rcOld.Height());
	pbtn->SetRect(&rcButton);

	pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcCancel);
	if (pbtn != NULL) {
		pbtn->GetRect(&rcButton);
		rcButton.Offset(0, rcNew.Height() - rcOld.Height());
		pbtn->SetRect(&rcButton);
	}

	// Resize the form

	Rect rcForm;
	pfrm->GetRect(&rcForm);
	rcForm.bottom += rcNew.Height() - rcOld.Height();

	// Now center the form again

	DibBitmap *pbm = pfrm->GetFormMgr()->GetDib();
	Size siz;
	pbm->GetSize(&siz);
	int yNew = (siz.cy - rcForm.Height()) / 2;
	rcForm.Offset(0, yNew - rcForm.top);
	pfrm->SetRect(&rcForm);

	// Clear if asked

	if (wf & kfMbClearDib) {
		pfrm->SetClearDibFlag();
    }

    return pfrm;
}

#ifdef STATUS_LINE
void Status(const char *psz)
{
	// Display::DrawText draws directly to the screen, no buffering

	if (gpdisp != NULL) {
		gpdisp->DrawText(psz, 0, -1, kfDtClearLine);
		Trace((char *)psz);
	}
}
#endif

// Returns a pseudo-random number 0 through 32767.
// This random number generator must be kept in sync across all
// machines in a multiplayer game. Use it in response to game
// events that are seen by all players.

static long glSeed = 1L;

#ifdef DEBUG
int _GetRandom(char *pszFile, int nLine)
#else
int _GetRandom()
#endif
{
#ifdef DEBUG
//	MpTrace("- GetRandom() [%s, %d]", pszFile, nLine);
#endif
    return ((glSeed = glSeed * 214013L + 2531011L) >> 16) & 0x7fff;
}

void SetRandomSeed(unsigned long nSeed)
{
	glSeed = (long)nSeed;
}

unsigned long GetRandomSeed()
{
	return glSeed;
}

// This random number generator does not need to be in sync across
// all machines in a multiplayer game. Use it when you want a random
// number that is only relevent to the local player (e.g., picking
// a random voice response for a unit).

static long glAsyncSeed = 1L;

int GetAsyncRandom()
{
    return ((glAsyncSeed = glAsyncSeed * 214013L + 2531011L) >> 16) & 0x7fff;
}

Direction TurnToward(Direction dirTo, Direction dirFrom)
{
	int d = dirTo - dirFrom;
	if (d == 0)
		return dirTo;

	if (d < -4)
		d = 1;
	else if (d > 4)
		d = -1;
	if (d < 0)
		dirFrom--;
	else
		dirFrom++;
	return ((unsigned int)dirFrom) % 8;
}

Direction16 TurnToward16(Direction16 dirTo, Direction16 dirFrom)
{
	int d = dirTo - dirFrom;
	if (d == 0)
		return dirTo;

	if (d < -8)
		d = 1;
	else if (d > 8)
		d = -1;
	if (d < 0)
		dirFrom--;
	else
		dirFrom++;
	return ((unsigned int)dirFrom) % 16;
}

int isqrt(int val1, int val2)
{
    return (int)isqrt((dword)val1 * (dword)val1 + (dword)val2 * (dword)val2);
}

unsigned int isqrt(unsigned long val) 
{
  unsigned long temp, g=0;

  if (val >= 0x40000000) {
    g = 0x8000; 
    val -= 0x40000000;
  }

#define INNER_MBGSQRT(s)                      \
  temp = (g << (s)) + (1L << ((s) * 2L - 2L));   \
  if (val >= temp) {                          \
    g += 1L << ((s)-1L);                        \
    val -= temp;                              \
  }

  INNER_MBGSQRT (15)
  INNER_MBGSQRT (14)
  INNER_MBGSQRT (13)
  INNER_MBGSQRT (12)
  INNER_MBGSQRT (11)
  INNER_MBGSQRT (10)
  INNER_MBGSQRT ( 9)
  INNER_MBGSQRT ( 8)
  INNER_MBGSQRT ( 7)
  INNER_MBGSQRT ( 6)
  INNER_MBGSQRT ( 5)
  INNER_MBGSQRT ( 4)
  INNER_MBGSQRT ( 3)
  INNER_MBGSQRT ( 2)

#undef INNER_MBGSQRT

  temp = g+g+1;
  if (val >= temp) g++;
  return (unsigned int)g;
}

// LineIterator implementation
// OPT: expensive! Lots of multiplies, divides and that fixed-point square root

void WLineIterator::Init(WCoord wx1, WCoord wy1, WCoord wx2, WCoord wy2, int nIncr)
{
	Assert(nIncr != 0);

	m_wx = wx1;
	m_wy = wy1;

	// Calc the number of nSteps needed to step the line:
	// cSteps = sqrt(dx^2 + dy^2) / nStep

	WCoord wdx = wx2 - wx1;
	WCoord wdy = wy2 - wy1;
	int nLen = isqrt(((long)wdx * (long)wdx) + ((long)wdy * (long)wdy));

	// Handle case where destination is less than one step away from the start.

	if (nLen < nIncr) {
		m_cStepsRemaining = 0;
		return;
	}

	int cSteps = nLen / nIncr;
	Assert(cSteps != 0);

	m_wdx = wdx / cSteps;
	m_wdy = wdy / cSteps;
	m_cStepsRemaining = cSteps;

#if 0	// doing without for now
	// Some fraction of a step is lost when we convert cSteps to an integer.
	// Add this fractional step to m_fx, m_fy to start things off so the
	// final step will leave us at the desired destination.

	fix fxFrac = fracfx(cfxSteps);
	m_fx = addfx(m_fx, (fix)mulfx(m_fdx, fxFrac));
	m_fy = addfx(m_fy, (fix)mulfx(m_fdy, fxFrac));
#endif
}

//
// Color helpers
//

const short SCALEFACTOR = 128;
const word SCALEMAX = 256 * (word)SCALEFACTOR;

// Takes byte-sized RGB values in the range from 0-255 and returns
// word-sized HSL values in the range from 0-32768 (H, S, L).
// H is special and ranges from 0 to SCALEFACTOR (128) * 6

void RgbToHsl(byte bR, byte bG, byte bB, word *pnH, word *pnS, word *pnL)
{
	word nR = bR * SCALEFACTOR;
	word nG = bG * SCALEFACTOR;
	word nB = bB * SCALEFACTOR;

	word nMax = _max(nR, nG);
	nMax = _max(nMax, nB);

	word nMin = _min(nR, nG);
	nMin = _min(nMin, nB);

	*pnH = 0;
	*pnS = 0;
	*pnL = (word)((nMin + nMax) / 2);
	if (*pnL == 0) 
		return;

	word nDelta = nMax - nMin;
	*pnS = nDelta;
	if (nDelta == 0)
		return;

	word n;
	if (*pnL < SCALEMAX / 2)
		n = nMax + nMin;
	else
		n = (word)(((long)SCALEMAX * 2) - nMax - nMin);
	*pnS = (word)((nDelta * (long)SCALEMAX) / n);

	word nDeltaDiv = nDelta / SCALEFACTOR;

	word r2 = (nMax - nR) / nDeltaDiv;
	word g2 = (nMax - nG) / nDeltaDiv;
	word b2 = (nMax - nB) / nDeltaDiv;

	if (nR == nMax) {
		if (nG == nMin) {
			if (b2 == SCALEFACTOR)
				*pnH = 0;
			else
				*pnH = (5 * SCALEFACTOR) + b2;
		} else {
			*pnH = (1 * SCALEFACTOR) - g2;
		}
	} else if (nG == nMax) {
		*pnH = (nB == nMin ? (1 * SCALEFACTOR) + r2 : (3 * SCALEFACTOR) - b2);
	} else {
		*pnH = (nR == nMin ? (3 * SCALEFACTOR) + g2 : (5 * SCALEFACTOR) - r2);
	}
}

void HslToRgb(word nH, word nS, word nL, byte *pbR, byte *pbG, byte *pbB)
{
	word v;

	if (nL <= SCALEMAX / 2) {
		Assert((nL * (SCALEMAX + (long)nS)) / SCALEMAX < 65536);
		v = (word)((nL * (SCALEMAX + (long)nS)) / SCALEMAX);
	} else {
		Assert((nL + nS - ((nL * (long)nS) / SCALEMAX)) < 65536);
		v = (word)(nL + nS - ((nL * (long)nS) / SCALEMAX));
	}

	if (v == 0) {
		*pbR = *pbG = *pbB = 0;
		return;
	}

	word m = nL + nL - v;
	word sv = (word)(((v - m) * (long)SCALEMAX) / v);

	word sextant = nH / SCALEFACTOR;	
	word fract = nH & (SCALEFACTOR - 1);
	word vsf = (word)((((v * (long)sv) / SCALEMAX) * fract) / SCALEFACTOR);
	word mid1 = m + vsf;
	word mid2 = v - vsf;

	// UNDONE: remove
	if (v == SCALEMAX)
		v = SCALEMAX - 1;
	if (m == SCALEMAX)
		m = SCALEMAX - 1;
	if (mid1 == SCALEMAX)
		mid1 = SCALEMAX - 1;
	if (mid2 == SCALEMAX)
		mid2 = SCALEMAX - 1;

	Assert(v / SCALEFACTOR < 256);
	Assert(m / SCALEFACTOR < 256);
	Assert(mid1 / SCALEFACTOR < 256);
	Assert(mid2 / SCALEFACTOR < 256);

	switch (sextant) {
	case 0: *pbR = v / SCALEFACTOR; *pbG = mid1 / SCALEFACTOR; *pbB = m / SCALEFACTOR; break;
	case 1: *pbR = mid2 / SCALEFACTOR; *pbG = v / SCALEFACTOR; *pbB = m / SCALEFACTOR; break;
	case 2: *pbR = m / SCALEFACTOR; *pbG = v / SCALEFACTOR; *pbB = mid1 / SCALEFACTOR; break;
	case 3: *pbR = m / SCALEFACTOR; *pbG = mid2 / SCALEFACTOR; *pbB = v / SCALEFACTOR; break;
	case 4: *pbR = mid1 / SCALEFACTOR; *pbG = m / SCALEFACTOR; *pbB = v / SCALEFACTOR; break;
	case 5: *pbR = v / SCALEFACTOR; *pbG = m / SCALEFACTOR; *pbB = mid2 / SCALEFACTOR; break;
	}
}

int CompareDates(Date *pdate1, Date *pdate2)
{
	// Check years

	if (pdate1->nYear < pdate2->nYear)
		return -1;
	if (pdate1->nYear > pdate2->nYear)
		return 1;

	// Years are equal. Check months.

	if (pdate1->nMonth < pdate2->nMonth)
		return -1;
	if (pdate1->nMonth > pdate2->nMonth)
		return 1;

	// Months are equal. Check days

	if (pdate1->nDay < pdate2->nDay)
		return -1;
	if (pdate1->nDay > pdate2->nDay)
		return 1;

	// All equal

	return 0;
}

// CheckBetaTimeout
// false means beta has timed out
// true means beta has not timed out

#ifdef BETA_TIMEOUT
#define knMonthBetaStart 10
#define knDayBetaStart 1
#define knYearBetaStart 2009

#define knMonthBetaEnd 12
#define knDayBetaEnd 1
#define knYearBetaEnd 2009

bool CheckBetaTimeout()
{
	// Get current date

	Date date;
	HostGetCurrentDate(&date);

	// Get prefs. If we have prefs, get the last date run from there

	Date dateLast;
	dateLast.nYear = gprefsInit.nYearLastRun;
	dateLast.nMonth = gprefsInit.nMonthLastRun;
	dateLast.nDay = gprefsInit.nDayLastRun;

	// See if the date has been set backwards to trick us :)

	if (CompareDates(&date, &dateLast) < 0) {
TimeOut:
		HtMessageBox(kfMbWhiteBorder | kfMbClearDib, "Test Release", "Thank you for playing Hostile Takeover. Unfortunately this test release has expired! Please contact us at:\n\nhttp://www.spiffcode.com");
		return false;
	}

	// See if the current date is ahead of the demo start date

	Date dateStart;
	dateStart.nYear = knYearBetaStart;
	dateStart.nMonth = knMonthBetaStart;
	dateStart.nDay = knDayBetaStart;
	if (CompareDates(&date, &dateStart) < 0)
		goto TimeOut;

	// See if the current date is before the demo end date

	Date dateEnd;
	dateEnd.nYear = knYearBetaEnd;
	dateEnd.nMonth = knMonthBetaEnd;
	dateEnd.nDay = knDayBetaEnd;
	if (CompareDates(&date, &dateEnd) > 0)
		goto TimeOut;

	// All looks ok. Save the prefs which'll save the current date

	ggame.SavePreferences();

	return true;
}
#endif

// Hardwired sfx categories

// Infantry destroyed

Sfx gasfxInfantryDestroyed[] = {
	ksfxInfantryDestroyed0,
	ksfxInfantryDestroyed1,
	ksfxInfantryDestroyed2,
	ksfxInfantryDestroyed3,
	ksfxInfantryDestroyed4,
};

// Vehicle destroyed

Sfx gasfxVehicleDestroyed[] = {
	ksfxVehicleDestroyed,
};

// Male01

Sfx gasfxMale01Select[] = {
	ksfxMale01Select0,
	ksfxMale01Select1,
	ksfxMale01Select2,
	ksfxMale01Select3,
};

Sfx gasfxMale01Move[] = {
	ksfxMale01Move0,
	ksfxMale01Move1,
	ksfxMale01Move2,
	ksfxMale01Move3,
};

Sfx gasfxMale01Attack[] = {
	ksfxMale01Attack0,
	ksfxMale01Attack1,
	ksfxMale01Attack2,
	ksfxMale01Attack3,
};

// Male03

Sfx gasfxMale03Select[] = {
	ksfxMale03Select0,
	ksfxMale03Select1,
	ksfxMale03Select2,
	ksfxMale03Select3,
};

Sfx gasfxMale03Move[] = {
	ksfxMale03Move0,
	ksfxMale03Move1,
	ksfxMale03Move2,
	ksfxMale03Move3,
};

Sfx gasfxMale03Attack[] = {
	ksfxMale03Attack0,
	ksfxMale03Attack1,
	ksfxMale03Attack2,
	ksfxMale03Attack3,
};

// Male06

Sfx gasfxMale06Select[] = {
	ksfxMale06Select0,
	ksfxMale06Select1,
	ksfxMale06Select2,
	ksfxMale06Select3,
};

Sfx gasfxMale06Move[] = {
	ksfxMale06Move0,
	ksfxMale06Move1,
	ksfxMale06Move2,
	ksfxMale06Move3,
};

Sfx gasfxMale06Attack[] = {
	ksfxMale06Attack0,
	ksfxMale06Attack1,
	ksfxMale06Attack2,
	ksfxMale06Attack3,
};

// Major01

Sfx gasfxMajor01Select[] = {
	ksfxMajor01Select0,
	ksfxMajor01Select1,
	ksfxMajor01Select2,
	ksfxMajor01Select3,
};

Sfx gasfxMajor01Move[] = {
	ksfxMajor01Move0,
	ksfxMajor01Move1,
	ksfxMajor01Move2,
	ksfxMajor01Move3,
};

Sfx gasfxMajor01Attack[] = {
	ksfxMajor01Attack0,
	ksfxMajor01Attack1,
	ksfxMajor01Attack2,
	ksfxMajor01Attack3,
};

// Major02

Sfx gasfxMajor02Select[] = {
	ksfxMajor02Select0,
	ksfxMajor02Select1,
	ksfxMajor02Select2,
	ksfxMajor02Select3,
};

Sfx gasfxMajor02Move[] = {
	ksfxMajor02Move0,
	ksfxMajor02Move1,
	ksfxMajor02Move2,
	ksfxMajor02Move3,
};

Sfx gasfxMajor02Attack[] = {
	ksfxMajor02Attack0,
	ksfxMajor02Attack1,
	ksfxMajor02Attack2,
	ksfxMajor02Attack3,
};

// Andy

Sfx gasfxAndySelect[] = {
	ksfxAndySelect,
};

Sfx gasfxAndyMove[] = {
	ksfxAndyMove,
};

Sfx gasfxAndyAttack[] = {
	ksfxAndyAttack,
};

Sfx gasfxAndyDestroyed[] = {
	ksfxAndyDestroyed,
};

// Fox

Sfx gasfxFoxDestroyed[] = {
	ksfxFoxDestroyed,
};

struct SfxCategoryEntry // sfxce
{
	Sfx *asfx;
	int csfx;
};

SfxCategoryEntry gasfxce[] = {
	{ gasfxInfantryDestroyed, ARRAYSIZE(gasfxInfantryDestroyed) },
	{ gasfxVehicleDestroyed, ARRAYSIZE(gasfxVehicleDestroyed) },
	{ gasfxMale01Select, ARRAYSIZE(gasfxMale01Select) },
	{ gasfxMale01Move, ARRAYSIZE(gasfxMale01Move) },
	{ gasfxMale01Attack, ARRAYSIZE(gasfxMale01Attack) },
	{ gasfxMale03Select, ARRAYSIZE(gasfxMale03Select) },
	{ gasfxMale03Move, ARRAYSIZE(gasfxMale03Move) },
	{ gasfxMale03Attack, ARRAYSIZE(gasfxMale03Attack) },
	{ gasfxMale06Select, ARRAYSIZE(gasfxMale06Select) },
	{ gasfxMale06Move, ARRAYSIZE(gasfxMale06Move) },
	{ gasfxMale06Attack, ARRAYSIZE(gasfxMale06Attack) },
	{ gasfxMajor01Select, ARRAYSIZE(gasfxMajor01Select) },
	{ gasfxMajor01Move, ARRAYSIZE(gasfxMajor01Move) },
	{ gasfxMajor01Attack, ARRAYSIZE(gasfxMajor01Attack) },
	{ gasfxMajor02Select, ARRAYSIZE(gasfxMajor02Select) },
	{ gasfxMajor02Move, ARRAYSIZE(gasfxMajor02Move) },
	{ gasfxMajor02Attack, ARRAYSIZE(gasfxMajor02Attack) },
	{ gasfxAndySelect, ARRAYSIZE(gasfxAndySelect) },
	{ gasfxAndyMove, ARRAYSIZE(gasfxAndyMove) },
	{ gasfxAndyAttack, ARRAYSIZE(gasfxAndyAttack) },
	{ gasfxAndyDestroyed, ARRAYSIZE(gasfxAndyDestroyed) },
	{ gasfxFoxDestroyed, ARRAYSIZE(gasfxFoxDestroyed) },
};

Sfx SfxFromCategory(SfxCategory sfxc)
{
	if (sfxc < 0 || sfxc >= ARRAYSIZE(gasfxce))
		return (Sfx)-1;
	SfxCategoryEntry *psfxce = &gasfxce[sfxc];

	// NOTE: We use GetAsyncRandom() to avoid disturbing the in-sync random 
	// number generator when we're performing this operation which is meant 
	// only for the local player.

	return psfxce->asfx[GetAsyncRandom() % psfxce->csfx];
}

// Stream helpers

void Stream::ReadString(char *psz, int cb)
{
	// Fill the passed buffer

	*psz = 0;
	while (cb-- != 0) {
		char ch = (char)ReadByte();
		*psz++ = ch;
		if (ch == 0)
			return;
	}

	// More string in the stream than size of psz. Cap off psz, read till 0

	psz[cb - 1] = 0;
	while (ReadByte() != 0)
		;
}

void Stream::WriteString(char *psz)
{
	Write(psz, (int)(strlen(psz) + 1));
}

void Stream::WriteBytesRLE(byte *pb, int cb)
{
	byte *pbMax = &pb[cb];
	for (byte *pbChunk = pb; pbChunk < pbMax; ) {
		// Literal runs are separated by repeats of at least 3 or more.

		byte *pbRepeat = FindRLERepeat(pbChunk, pbMax, 3);

		// If there is a literal to write, do it

		int cbLiteral = 0;
		if (pbRepeat == NULL) {
			cbLiteral = (int)(pbMax - pbChunk);
		} else if (pbRepeat > pbChunk) {
			cbLiteral = (int)(pbRepeat - pbChunk);
		}
		if (cbLiteral != 0) {
			WriteRLEChunk(pbChunk, cbLiteral, false);
			pbChunk += cbLiteral;
			continue;
		}

		// There should be a repeat to write

		Assert(pbRepeat != NULL && pbRepeat == pbChunk);
		byte *pbT;
		for (pbT = pbRepeat + 1; pbT < pbMax; pbT++) {
			if (*pbT != *pbRepeat)
				break;
		}
		int cbRepeat = (int)(pbT - pbRepeat);
		Assert(cbRepeat >= 3);
		WriteRLEChunk(pbChunk, cbRepeat, true);
		pbChunk += cbRepeat;
	}
}

byte *Stream::FindRLERepeat(byte *pbStart, byte *pbMax, int cbMin)
{
	for (; pbStart < pbMax; pbStart++) {
		byte bFind = *pbStart;

		int cRepeat = 1;
		for (byte *pbT = pbStart + 1; pbT < pbMax; pbT++) {
			if (*pbT != bFind)
				break;
			cRepeat++;
			if (cRepeat >= cbMin)
				return pbStart;
		}
	}
	return 0;
}

void Stream::WriteRLEChunk(byte *pb, int cb, bool fRepeat)
{
	if (!fRepeat) {
		// Literal

		while (cb != 0) {
			int cbWrite = cb < 128 ? cb : 128;
			WriteByte(128 | (cbWrite - 1));
			Write(pb, cbWrite);
			pb += cbWrite;
			cb -= cbWrite;
			if (!IsSuccess())
				break;
		}
	} else {
		// Repeat

		while (cb != 0) {
			int cbWrite = cb < 128 ? cb : 128;
			WriteByte(0 | (cbWrite - 1));
			WriteByte(*pb);
			cb -= cbWrite;
			if (!IsSuccess())
				break;
		}
	}
}

void Stream::ReadBytesRLE(byte *pb, int cb)
{
	while (cb != 0) {
		byte bT = ReadByte();
		int cbRun = (bT & ~128) + 1;
		if (bT & 128) {
			Read(pb, cbRun);
		} else {
			byte bRepeat = ReadByte();
			memset(pb, bRepeat, cbRun);
		}
		pb += cbRun;
		cb -= cbRun;
		if (!IsSuccess())
			break;
	}
}

//
// Return the appropriate color for the current device
//

# if 0
Color gaclr4bpp[] = {
	0x000f, // black
	0x0000, // white
	0x0007, // red
	0x0000, // green
	0x0003, // yellow
	0x0008, // side 1 color
	0x000d, // side 2 color
	0x0004, // side 3 color
	0x000f, // side 4 color
	0x0008, // button fill
	0x000a, // button border
	0x000f, // menu background
	0x000f, // form background
	0x000d, // mini map border area
	0x000b, // Galaxite
	0x0007, // button fill highlight
	0x0008, // medium gray
	0x0004, // blue side 0
	0x0006, // blue side 1
	0x0008, // blue side 2
	0x000a, // blue side 3
	0x000c, // blue side 4
	0x000b, // red side 0
	0x000c, // red side 1
	0x000d, // red side 2
	0x000e, // red side 3
	0x000f, // red side 4
	0x0001, // yellow side 0
	0x0003, // yellow side 1
	0x0004, // yellow side 2
	0x0005, // yellow side 3
	0x0005, // yellow side 4
	0x0000, // cyan side 0
	0x000c, // cyan side 1
	0x0002, // cyan side 2
	0x000e, // cyan side 3
	0x0004, // cyan side 4
	0x000e, // list background
	0x0003, // list border
	0x0000, // Jana Font
	0x0000, // Andy Font
	0x0000, // Olstrom Font
	0x0000, // Fox Font
	0x000f, // neutral side 0
	0x000f, // neutral side 1
	0x000f, // neutral side 2
	0x000f, // neutral side 3
	0x000f, // neutral side 4
};
#endif

#if 0
Color gaclr8bpp[] = {
	0x0000, // black
	0x0001, // white
	0x0002, // red
	0x0003, // green
	0x0004, // yellow
	0x0010, // side 1 color
	0x0015, // side 2 color
	0x001a, // side 3 color
	0x000b, // side 4 color
	0x0005, // button fill
	0x0011, // button border
	0x0000, // menu background
	0x0000, // form background
	0x000f, // mini map border area
	0x001f, // Galaxite minimap color
	0x000c, // button fill highlight
	35,		// medium gray
	0x0010, // blue side 0
	0x0011, // blue side 1
	0x0012, // blue side 2
	0x0013, // blue side 3
	0x0014, // blue side 4
	0x0015, // red side 0
	0x0016, // red side 1
	0x0017, // red side 2
	0x0018, // red side 3
	0x0019, // red side 4
	0x001a, // yellow side 0
	0x001b, // yellow side 1
	0x001c, // yellow side 2
	0x001d, // yellow side 3
	0x001e, // yellow side 4
	0x000b, // cyan side 0
	0x000c, // cyan side 1
	0x000d, // cyan side 2
	0x000e, // cyan side 3
	0x000f, // cyan side 4
	0x000a, // list background
	0x000c, // list border
	0x0006, // Jana font
	0x000b, // Andy font
	0x001a, // Olstrom font
	0x0015, // Fox font
	32,		// neutral side 0
	33,		// neutral side 1
	36,		// neutral side 2
	40,		// neutral side 3
	45,		// neutral side 4
	52,		// Galaxite fullness indicator/neon pink
};
#endif

Color gaclr24bpp[] = {
    { 0,   0,   0   }, // black
    { 252, 252, 252 }, // white
    { 252, 0,   0   }, // red
    { 0,   252, 0   }, // green
    { 252, 252, 0   }, // yellow
    { 0,   116, 232 }, // side 1 color
    { 232, 32,  0   }, // side 2 color
    { 232, 228, 0   }, // side 3 color
    { 104, 252, 252 }, // side 4 color
    { 0,   192, 196 }, // button fill
    { 0,   96,  196 }, // button border
    { 0,   0,   0   }, // menu background
    { 0,   0,   0   }, // form background
    { 28,  56,  59  }, // mini map border area
    { 155, 88,  140 }, // Galaxite minimap color
    { 84,  160, 172 }, // button fill highlight
    { 132, 128, 128 }, // medium gray
    { 0,   116, 232 }, // blue side 0
    { 0,   96,  196 }, // blue side 1
    { 0,   64,  120 }, // blue side 2
    { 0,   48,  92  }, // blue side 3
    { 0,   32,  64  }, // blue side 4
    { 232, 32,  0   }, // red side 0
    { 196, 29,  0   }, // red side 1
    { 120, 8,   0   }, // red side 2
    { 92,  8,   0   }, // red side 3
    { 64,  8,   0   }, // red side 4
    { 232, 228, 0   }, // yellow side 0
    { 196, 192, 0   }, // yellow side 1
    { 120, 116, 0   }, // yellow side 2
    { 92,  88,  0   }, // yellow side 3
    { 64,  60,  0   }, // yellow side 4
    { 104, 252, 252 }, // cyan side 0
    { 84,  160, 172 }, // cyan side 1
    { 56,  120, 131 }, // cyan side 2
    { 40,  80,  88  }, // cyan side 3
    { 28,  56,  59  }, // cyan side 4
    { 16,  40,  43  }, // list background
    { 84,  160, 172 }, // list border
    { 184, 248, 248 }, // Jana font
    { 104, 252, 252 }, // Andy font
    { 232, 228, 0   }, // Olstrom font
    { 232, 32,  0   }, // Fox font
    { 216, 216, 216 }, // neutral side 0
    { 168, 168, 168 }, // neutral side 1
    { 120, 120, 120 }, // neutral side 2
    { 92,  92,  92  }, // neutral side 3
    { 64,  64,  64  }, // neutral side 4
    { 252, 0,   252 }  // Galaxite fullness indicator/neon pink
};

int gaiclrSide[kcSides] = { kiclrSideNeutral, kiclrSide1, kiclrSide2, kiclrSide3, kiclrSide4 };

void DrawBorder(DibBitmap *pbm, Rect *prc, int nThickness, Color clr, UpdateMap *pupd)
{
	if (pupd == NULL) {
		pbm->Fill(prc->left, prc->top, prc->Width(), nThickness, clr);
		pbm->Fill(prc->left, prc->top, nThickness, prc->Height(), clr);
		pbm->Fill(prc->right - nThickness, prc->top, nThickness, prc->Height(), clr);
		pbm->Fill(prc->left, prc->bottom - nThickness, prc->Width(), nThickness, clr);
	} else {
		Rect rcT;
		rcT.Set(prc->left, prc->top, prc->right, prc->top + nThickness);
		FillHelper(pbm, pupd, &rcT, clr);
		rcT.Set(prc->left, prc->top, prc->left + nThickness, prc->bottom);
		FillHelper(pbm, pupd, &rcT, clr);
		rcT.Set(prc->right - nThickness, prc->top, prc->right, prc->bottom);
		FillHelper(pbm, pupd, &rcT, clr);
		rcT.Set(prc->left, prc->bottom - nThickness, prc->right, prc->bottom);
		FillHelper(pbm, pupd, &rcT, clr);
	}
}

#if 0
void DrawBitmapBorder(DibBitmap *pbm, UpdateMap *pupd, Rect *prc, AnimationData *panid, int ifrm, Side side)
{
	// Get piece bounds

	Rect rcTL;
	panid->GetBounds(0, ifrm, &rcTL);
	Rect rcT;
	panid->GetBounds(1, ifrm, &rcT);
	Rect rcTR;
	panid->GetBounds(2, ifrm, &rcTR);
	Rect rcR;
	panid->GetBounds(3, ifrm, &rcR);
	Rect rcBR;
	panid->GetBounds(4, ifrm, &rcBR);
	Rect rcB;
	panid->GetBounds(5, ifrm, &rcB);
	Rect rcBL;
	panid->GetBounds(6, ifrm, &rcBL);
	Rect rcL;
	panid->GetBounds(7, ifrm, &rcL);

	// Draw repeating top edge

	Rect rcBounds;
	int xL = prc->left + rcTL.Width();
	int xR = prc->right - rcTR.Width();
	int cxPiece = rcT.Width();
	for (; xL < xR; xL += cxPiece) {
		rcBounds = rcT;
		rcBounds.Offset(xL, prc->top);
		if (pupd->IsRectInvalid(&rcBounds))
			panid->DrawFrame(1, ifrm, pbm, xL, prc->top, side);
	}

	// Draw repeating bottom edge

	xL = prc->left + rcBL.Width();
	xR = prc->right - rcBR.Width();
	cxPiece = rcB.Width();
	for (; xL < xR; xL += cxPiece) {
		rcBounds = rcB;
		rcBounds.Offset(xL, prc->bottom);
		if (pupd->IsRectInvalid(&rcBounds))
			panid->DrawFrame(5, ifrm, pbm, xL, prc->bottom, side);
	}

	// Draw repeating left edge

	int yT = prc->top + rcTL.Height();
	int yB = prc->bottom - rcBL.Height();
	int cyPiece = rcL.Height();
	for (; yT < yB; yT += cyPiece) {
		rcBounds = rcL;
		rcBounds.Offset(prc->left, yT);
		if (pupd->IsRectInvalid(&rcBounds))
			panid->DrawFrame(7, ifrm, pbm, prc->left, yT, side);
	}

	// Draw repeating right edge

	yT = prc->top + rcTR.Height();
	yB = prc->bottom - rcBR.Height();
	cyPiece = rcR.Height();
	for (; yT < yB; yT += cyPiece) {
		rcBounds = rcR;
		rcBounds.Offset(prc->right, yT);
		if (pupd->IsRectInvalid(&rcBounds))
			panid->DrawFrame(3, ifrm, pbm, prc->right, yT, side);
	}

	// Draw the corners

	rcBounds = rcTL;
	rcBounds.Offset(prc->left, prc->top);
	if (pupd->IsRectInvalid(&rcBounds))
		panid->DrawFrame(0, ifrm, pbm, prc->left, prc->top, side);

	rcBounds = rcTR;
	rcBounds.Offset(prc->right, prc->top);
	if (pupd->IsRectInvalid(&rcBounds))
		panid->DrawFrame(2, ifrm, pbm, prc->right, prc->top, side);

	rcBounds = rcBR;
	rcBounds.Offset(prc->right, prc->bottom);
	if (pupd->IsRectInvalid(&rcBounds))
		panid->DrawFrame(4, ifrm, pbm, prc->right, prc->bottom, side);

	rcBounds = rcBL;
	rcBounds.Offset(prc->left, prc->bottom);
	if (pupd->IsRectInvalid(&rcBounds))
		panid->DrawFrame(6, ifrm, pbm, prc->left, prc->bottom, side);
}
#endif

//
// Basic string dictionary class. Limited to short keys and values.
//

Dictionary::Dictionary()
{
	m_cde = 0;
	m_pdeHead = NULL;
}

Dictionary::~Dictionary()
{
	Clear();
}

// Clone the passed-in dictionary over self

bool Dictionary::Init(Dictionary *pdict)
{
	Clear();

	DictionaryEntry *pdeSrc = pdict->m_pdeHead;
	DictionaryEntry **ppdeDst = &m_pdeHead;
	while (pdeSrc != NULL) {
		DictionaryEntry *pdeDst = new DictionaryEntry;
		Assert(pdeDst != NULL, "out of memory!");
		if (pdeDst == NULL) {
			Clear();
			return false;
		}
		pdeDst->pdeNext = NULL;
		strcpy(pdeDst->szName, pdeSrc->szName);
		strcpy(pdeDst->szValue, pdeSrc->szValue);
		*ppdeDst = pdeDst;
		ppdeDst = &pdeDst->pdeNext;
		pdeSrc = pdeSrc->pdeNext;
		m_cde++;
	}

	return true;
}

void Dictionary::Clear()
{
	DictionaryEntry *pde = m_pdeHead;
	while (pde != NULL) {
		DictionaryEntry *pdeNext = pde->pdeNext;
		delete pde;
		pde = pdeNext;
	}
	m_cde = 0;
	m_pdeHead = NULL;
}

const char *Dictionary::Get(const char *pszName)
{
	DictionaryEntry **ppde = Find(pszName);
	if (*ppde == NULL)
		return NULL;

	// Relink at head of the list (if not already there)

	DictionaryEntry *pde = *ppde;
	if (m_pdeHead != pde) {
		*ppde = pde->pdeNext;
		pde->pdeNext = m_pdeHead;
		m_pdeHead = pde;
	}
	return pde->szValue;
}

bool Dictionary::Set(const char *pszName, const char *pszValue)
{
	// Is it already in the dictionary?

	DictionaryEntry **ppde = Find(pszName);

	// Create new entry if needed

	DictionaryEntry *pde;
	if (*ppde == NULL) {
		pde = new DictionaryEntry;
		Assert(pde != NULL, "out of memory!");
		if (pde == NULL)
			return false;
		m_cde++;
	} else {
		pde = *ppde;
		*ppde = pde->pdeNext;
	}

	// Set its name/value and link it to the head of the list

	pde->pdeNext = m_pdeHead;
	m_pdeHead = pde;
	strcpy(pde->szName, pszName);
	strcpy(pde->szValue, pszValue);

	return true;
}

bool Dictionary::Remove(const char *pszName)
{
	DictionaryEntry **ppde = Find(pszName);
	if (*ppde == NULL)
		return false;

	m_cde--;
	DictionaryEntry *pde = *ppde;
	*ppde = pde->pdeNext;
	delete pde;
	return true;
}

#define knVerDictionaryState 0
bool Dictionary::LoadState(Stream *pstm)
{
	Clear();

	// Do version handling

	byte nVer = pstm->ReadByte();
	if (nVer != knVerDictionaryState)
		return false;

	DictionaryEntry **ppde = &m_pdeHead;
	m_cde = pstm->ReadByte();
	for (int i = 0; i < m_cde; i++) {
		DictionaryEntry *pde = new DictionaryEntry;
		Assert(pde != NULL, "out of memory!");
		if (pde == NULL)
			return false;
		pstm->ReadString(pde->szName, sizeof(pde->szName));
		pstm->ReadString(pde->szValue, sizeof(pde->szValue));

		pde->pdeNext = NULL;
		*ppde = pde;
		ppde = &pde->pdeNext;
	}

	return pstm->IsSuccess();
}

bool Dictionary::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerDictionaryState);
	pstm->WriteByte(m_cde);

	DictionaryEntry *pde = m_pdeHead;
	while (pde != NULL) {
		pstm->WriteString(pde->szName);
		pstm->WriteString(pde->szValue);
		pde = pde->pdeNext;
	}

	return pstm->IsSuccess();
}

DictionaryEntry **Dictionary::Find(const char *pszName)
{
	DictionaryEntry **ppde = &m_pdeHead;
	while (*ppde != NULL) {
		// OPT: upcase entries as they're added to the dictionary
		if (stricmp((*ppde)->szName, pszName) == 0)
			break;
		ppde = &(*ppde)->pdeNext;
	}

	return ppde;
}

// Takes a string with embedded variable references in the form of "{pvar}"
// and expands them to the appropriate values.

void ExpandVars(char *pszSrc, char *pszBuff, int cbBuff)
{
	while (*pszSrc != 0 && cbBuff > 1) {
		char ch = *pszSrc++;

		// Variable?

		if (ch == '{') {

			// yes

			char szT[kcbPvarNameMax];
			char *pszT = szT;
			*pszT = 0;

			int cbT = sizeof(szT);
			char *pszBeforeVar = pszSrc;
			while (*pszSrc != 0 && *pszSrc != '}' && cbT-- > 0)
                *pszT++ = *pszSrc++;

			if (*pszSrc != 0) {
				*pszT = 0;
				ggame.GetVar(szT, pszBuff, cbBuff);

				int cb = (int)strlen(pszBuff);
				cbBuff -= cb;
				pszBuff += cb;
				pszSrc++;
			} else {

				// Variable not terminated properly

				pszSrc = pszBeforeVar;
				*pszBuff++ = ch;
				cbBuff--;
			}
		} else {

			// No, just copy it

			*pszBuff++ = ch;
			cbBuff--;
		}
	}

	*pszBuff = 0;
}

void GetRankTitle(char *psz, int cb)
{
	// Note: rank "Challenger" is hardcoded in the level files as -1

	int nRank = 0;
	char szT[20];
	if (ggame.GetVar("rank", szT, sizeof(szT)))
		nRank = atoi(szT);
	if (!gpstrtbl->GetString(kidsRank0 + nRank, psz, cb))
		*psz = 0;
}

// Helpers - no need to call runtime for these
// These are here because Metrowerks doesn't have ARM runtime support for them, and we
// don't want to create thunks

int strnicmp(const char *psz1, const char *psz2, int cch)
{
	while (cch-- != 0) {
		byte b1 = *psz1++;
		if (b1 >= 'A' && b1 <= 'Z')
			b1 += 'a' - 'A';
		byte b2 = *psz2++;
		if (b2 >= 'A' && b2 <= 'Z')
			b2 += 'a' - 'A';		
		if (b1 != b2)
			return b1 - b2;
		if (b1 == 0)
			return 0;
	}
	return 0;
}

static void xtoa (
        unsigned long val,
        char *buf,
        unsigned radix,
        int is_neg
        )
{
        char *p;                /* pointer to traverse string */
        char *firstdig;         /* pointer to first digit */
        char temp;              /* temp char */
        unsigned digval;        /* value of digit */

        p = buf;

        if (is_neg) {
            /* negative, so output '-' and negate */
            *p++ = '-';
            val = (unsigned long)(-(long)val);
        }

        firstdig = p;           /* save pointer to first digit */

        do {
            digval = (unsigned) (val % radix);
            val /= radix;       /* get next digit */

            /* convert to ascii and store */
            if (digval > 9)
                *p++ = (char) (digval - 10 + 'a');  /* a letter */
            else
                *p++ = (char) (digval + '0');       /* a digit */
        } while (val > 0);

        /* We now have the digit of the number in the buffer, but in reverse
           order.  Thus we reverse them now. */

        *p-- = '\0';            /* terminate string; p points to last digit */

        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;   /* swap *p and *firstdig */
            --p;
            ++firstdig;         /* advance to next two digits */
        } while (firstdig < p); /* repeat until halfway */
}

/* Actual functions just call conversion helper with neg flag set correctly,
   and return pointer to buffer. */

char *itoa (
        int val,
        char *buf,
        int radix
        )
{
        if (radix == 10 && val < 0)
            xtoa((unsigned long)val, buf, radix, 1);
        else
            xtoa((unsigned long)(unsigned int)val, buf, radix, 0);
        return buf;
}

} // namespace wi
