#include "ht.h"

namespace wi {

#define kcchFont 256

Font *LoadFont(char *pszFont)
{
	Font *pfnt = new Font;
	Assert(pfnt != NULL, "out of memory!");
	if (pfnt == NULL)
		return NULL;
	if (!pfnt->Load(pszFont)) {
		delete pfnt;
		return NULL;
	}
	return pfnt;
}

Font::Font()
{
	m_pfnth = NULL;
	m_mpchpbCodeEven = NULL;
	m_mpchpbCodeOdd = NULL;
	m_nGlyphOverlap = 0;
	m_nLineOverlap = 0;
    m_cxEllipsis = 0;
}

Font::~Font()
{
	if (m_pfnth != NULL)
		gpakr.UnmapFile(&m_fmap);

	for (int ch = 0; ch < kcchFont; ch++) {
		if (m_mpchpbCodeEven != NULL) {
			if (m_mpchpbCodeEven[ch] != NULL)
				gmmgr.FreePtr(m_mpchpbCodeEven[ch]);
		}
		if (m_mpchpbCodeOdd != NULL) {
			if (m_mpchpbCodeOdd[ch] != NULL)
				gmmgr.FreePtr(m_mpchpbCodeOdd[ch]);
		}
	}
	if (m_mpchpbCodeEven != NULL)
		gmmgr.FreePtr(m_mpchpbCodeEven);
	if (m_mpchpbCodeOdd != NULL)
		gmmgr.FreePtr(m_mpchpbCodeOdd);
}

bool Font::Load(char *pszFont)
{
	m_pfnth = (FontHeader *)gpakr.MapFile(pszFont, &m_fmap);
	if (m_pfnth == NULL)
		return false;

	byte *apbT[kcchFont];
	memset(apbT, 0, sizeof(apbT));

	m_mpchpbCodeEven = (byte **)gmmgr.AllocPtr(sizeof(byte *) * kcchFont);
	if (m_mpchpbCodeEven == NULL)
		return false;
	gmmgr.WritePtr(m_mpchpbCodeEven, 0, apbT, sizeof(apbT));

	m_mpchpbCodeOdd = (byte **)gmmgr.AllocPtr(sizeof(byte *) * kcchFont);
	if (m_mpchpbCodeOdd == NULL)
		return false;
	gmmgr.WritePtr(m_mpchpbCodeOdd, 0, apbT, sizeof(apbT));

    m_cxEllipsis = GetTextExtent("...");

	return true;
}

int Font::GetTextExtent(const char *psz)
{
	int cx = 0;
	while (*psz != 0) {
        byte ch = *psz++;
		cx += m_pfnth->acxChar[ch] - m_nGlyphOverlap;
    }

	// Shadow allows 1 pixel overlap but the last char doesn't overlap

	return cx + m_nGlyphOverlap;
}

int Font::GetTextExtent(const char *psz, int cch)
{
	int cx = 0;
	while (cch-- > 0) {
        byte ch = *psz++;
		cx += m_pfnth->acxChar[ch] - m_nGlyphOverlap;
    }

	// Shadow allows 1 pixel overlap but the last char doesn't overlap

	return cx + m_nGlyphOverlap;
}

int Font::CalcMultilineHeight(char *psz, int cxMultiline)
{
	int cy = 0;
	char *pszNext = psz;
	while (pszNext != NULL) {
		char *pszStart = pszNext;
		CalcBreak(cxMultiline, &pszNext);
		cy += BigWord(m_pfnth->cy) - m_nLineOverlap;
	}

	return cy;
}

void Font::DrawText(DibBitmap *pbm, char *psz, int x, int y, int cx, int cy,
        bool fEllipsis)
{
	int cyFont = BigWord(m_pfnth->cy) - m_nLineOverlap;
	int cyT = cyFont;
	char *pszNext = psz;
	while (pszNext != NULL) {
		if (cy != -1 && cyT > cy)
			return;
		char *pszStart = pszNext;
		int cch = CalcBreak(cx, &pszNext);
        if (fEllipsis && pszNext != NULL && cy != -1 && cyT + cyFont > cy) {
            DrawTextWithEllipsis(pbm, pszStart, cch, x, y, cx, true);
        } else if (fEllipsis && pszNext == NULL) {
            DrawTextWithEllipsis(pbm, pszStart, cch, x, y, cx, false);
        } else {
            DrawText(pbm, pszStart, x, y, cch);
        }
		y += cyFont;
		cyT += cyFont;
	}
}

void Font::DrawTextWithEllipsis(DibBitmap *pbm, char *psz, int cch, int x,
        int y, int cx, bool fForce)
{
    if (!fForce) {
        // If not being forced and text fits, draw it without ellipsis
        if (GetTextExtent(psz) < cx) {
            DrawText(pbm, psz, x, y, strlen(psz));
            return;
        }
    }

    // Do a binary search to find out where the ellipsis is needed

    char szT[256];
    int imin = 0;

    // Convert to index and ensure room for ellipsis
    int imax = _min(cch - 1, (int)sizeof(szT) - 3 - 1);
    if (imax < 0) {
        imax = 0;
    }
    int icur = imax;
    int ifit = imax;
    while (true) {
        int icurT = imin + (imax - imin) / 2;
        if (icurT == icur) {
            break;
        }
        icur = icurT;
        int cxT = GetTextExtent(psz, icur + 1);
        if (cxT >= cx - m_cxEllipsis) {
            imax = icur - 1;
        } else {
            ifit = icur;
            imin = icur + 1;
        }
    }
    strncpyz(szT, psz, ifit + 2); // convert to count, add one for 0
    strcat(szT, "..."); // this fits without checks
    DrawText(pbm, szT, x, y, strlen(szT));
}

#define IsBreakingChar(ch) ((ch) == ' ' || (ch) == '\t')

char *Font::FindNextNonBreakingChar(char *psz)
{
	while (*psz != 0) {
		if (!IsBreakingChar(*psz))
			return psz;
		psz++;
	}
	return NULL;
}

int Font::CalcBreak(int cx, char **ppsz, bool fChop)
{
	char *pchBreakAfter = NULL;
	int cchBreak;
	int cxT = 0;
	char *pchT = *ppsz;
	int cch = 0;
	bool fFoundBreak = false;

	while (*pchT != 0) {
		char ch = *pchT;

		// Handle any combo of \n, \r, \n\r, or \r\n

		switch (ch) {
		case '\n':
			pchT++;
			if (*pchT == '\r')
				pchT++;
			*ppsz = FindNextNonBreakingChar(pchT);
			return cch;

		case '\r':
			pchT++;
			if (*pchT == '\n')
				pchT++;
			*ppsz = FindNextNonBreakingChar(pchT);
			return cch;
		}

		// Otherwise if breaking char, remember break point

		if (IsBreakingChar(*pchT)) {
			fFoundBreak = true;
			pchBreakAfter = FindNextNonBreakingChar(pchT);
			cchBreak = cch;
		}

		// At right edge yet?

		int cxChar = m_pfnth->acxChar[(byte)*pchT] - m_nGlyphOverlap;
		if (cxT + cxChar > cx) {
			// If last break exists, use it. Return pointer skips past break char

			if (fFoundBreak) {
				*ppsz = pchBreakAfter;
				return cchBreak;
			}

			// If no last break, sometimes callers want word chopping, sometimes not

			if (!fChop)
				return 0;

			// No last break; Scan forward for a breaking char so we know where to start
			// the next line

			while (*pchT != 0) {
				// Start the next past the break; only draw what fits in cx for this line

				if (IsBreakingChar(*pchT)) {
					*ppsz = FindNextNonBreakingChar(pchT);
					return cch;
				}

				// If carriage return, start at next char

				if (ch == '\n') {
					*ppsz = pchT + 1;
					return cch;
				}

				pchT++;
			}

			// At end of line with no break. Return length that fits in cx, and NULL to indicate done.

			if (*pchT == 0) {
				*ppsz = NULL;
				return cch;
			}
		}

		// Add char; next char

		cxT += cxChar;
		cch++;
		pchT++;
	}

	// Done without hitting edge. Return length and NULL to indicate done

	*ppsz = NULL;
	return cch;
}

int Font::DrawText(DibBitmap *pbm, char *psz, int x, int y, int cch, dword *mpscaiclr)
{
#ifdef DEBUG
	for (int ichT = 0; ichT < cch; ichT++)
		Assert((word)psz[ichT] < (word)kcchFont);
#endif

	if (cch == -1)
		cch = strlen(psz);

	// Clip entire line of text first.

	Size siz;
	pbm->GetSize(&siz);

	// Top clip

	if (y < 0)
		return 0;
	if (y + BigWord(m_pfnth->cy) > siz.cy)
		return 0;

	// Don't include x clipped chars
	// Left clip

	int ich = 0;
	int xT = x;
	char *pszT;
	for (pszT = psz; pszT - psz < cch; pszT++) {
		if (xT < 0) {
			ich++;
			xT += m_pfnth->acxChar[(byte)*pszT] - m_nGlyphOverlap;
			continue;
		}
		break;
	}
	int xDst = xT;

	// Right clip

	int cchT = 0;
	for (; pszT - psz < cch; pszT++) {
		int cx = m_pfnth->acxChar[(byte)*pszT] - m_nGlyphOverlap;
		if (xT + cx <= siz.cx) {
			cchT++;
			xT += cx;
			continue;
		}
		break;
	}
	if (cchT == 0)
		return 0;

	cch = cchT;

	// Draw

	xT = xDst;
	byte *pbDst = pbm->GetBits() + (long)y * siz.cx + xT;
	char *pszMax = &psz[ich + cch];

	for (pszT = &psz[ich]; pszT < pszMax; pszT++) {
		char ch = *pszT;
		int cxChar = m_pfnth->acxChar[(byte)ch];
		if (cxChar == 0)
			continue;

		byte *pbDraw;
		if (xT & 1) {
			pbDraw = m_mpchpbCodeOdd[(byte)ch];
		} else {
			pbDraw = m_mpchpbCodeEven[(byte)ch];
		}

		if (pbDraw == NULL) {
			ScanData *psd = (ScanData *)(((byte *)m_pfnth) +
                    BigWord(m_pfnth->mpchibsd[(byte)*pszT]));
			word cb = Compile8Thunk(gpbScratch, psd, xT & 1);
			byte *pbT = (byte *)gmmgr.AllocPtr(cb);
			if (pbT != NULL) {
				gmmgr.WritePtr(pbT, 0, gpbScratch, cb);
				if (xT & 1) {
					gmmgr.WritePtr(m_mpchpbCodeOdd, (byte)ch * sizeof(byte *),
                            &pbT, sizeof(byte *));
				} else {
					gmmgr.WritePtr(m_mpchpbCodeEven, (byte)ch * sizeof(byte *),
                            &pbT, sizeof(byte *));
				}
			}
			pbDraw = gpbScratch;
		}

		DrawDispatchThunk(pbDraw, NULL, pbDst, siz.cx - cxChar, mpscaiclr,
                gmpiclriclrShadow);
		pbDst += cxChar - m_nGlyphOverlap;
		xT += cxChar - m_nGlyphOverlap;
	}

	return xT - xDst;
}

} // namespace wi
