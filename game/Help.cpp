#include "ht.h"

namespace wi {

// Supported tags:
// <H1> -- heading on a new line, large font
// <H2> -- heading on a new line, normal font
// <BR> -- line break
// <HR> -- horizontal rule
// <A HREF="#link_target">link text</A> -- a hyperlink
// <A NAME="link_target"> -- a hyperlink target
// <IMG SRC="filename.amx"> -- uses animation strip named "help"


//
// Help Control
//

bool PaintChunk(int x, int y, int cx, int cy, int curIndex, int curY, int nyBottom, Chunk *pChunk, void *ptr) secHelpControl;
bool HitTestChunk(int x, int y, int cx, int cy, int curIndex, int curY, int nyBottom, Chunk *pChunk, void *ptr) secHelpControl;
bool FindPositionCallback(int x, int y, int cx, int cy, int curIndex, int curY, int nyBottom, Chunk *pChunk, void *ptr) secHelpControl;

HelpControl::HelpControl()
{
    m_fTimerAdded = false;
    m_fDrag = false;
	m_fLargeFont = true;
	m_nchCurrent = 0;
	m_pfil = NULL;
	memset(m_nchBack, 0, sizeof(m_nchBack));
    memset(&m_hittest, 0, sizeof(m_hittest));
}

HelpControl::~HelpControl()
{
	if (m_pfil != NULL) {
		gpakr.fclose(m_pfil);
    }
    if (m_fTimerAdded) {
        gtimm.RemoveTimer(this);
    }
}

bool HelpControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	m_cyPageAmount = (int)((long)m_rc.Height() * 85 / 100);

#if defined(IPHONE) || defined(SDL)
    m_wf |= kfHelpScrollPosition;
#endif

	return true;
}

bool HelpControl::SetFile(const char *pszFile)
{
	m_pfil = gpakr.fopen((char *)pszFile, "rb");
    if (m_pfil != NULL) {
        gpakr.fseek(m_pfil, 0, SEEK_END);
        m_cb = gpakr.ftell(m_pfil);
        gpakr.fseek(m_pfil, 0, SEEK_SET);
    }
	return m_pfil != NULL;
}

bool HelpControl::FollowLink(const char *str, int cch)
{
	m_fLargeFont = false;

	// Get file length

	gpakr.fseek(m_pfil, 0, SEEK_END);
	dword cbFile = gpakr.ftell(m_pfil);

	// Calc the tag length we're searching for

	char szTag[128];
	strcpy(szTag, "<A NAME=\"");
	strncpyz(&szTag[9], (char *)str, 1 + (cch != -1 ? cch : (int)strlen(str))); // + 1 for zero terminator
	int cchTag = (int)strlen(szTag);

	// Loop until we've found the tag or reached the end

	char szBuffer[512];
	dword nchFile = 0;
	while (nchFile < cbFile) {
		// Read the next chunk in, zero terminate it

		gpakr.fseek(m_pfil, nchFile, SEEK_SET);
		int cchRead = gpakr.fread(szBuffer, 1, sizeof(szBuffer) - 1, m_pfil);

		// We don't want a tag to get cut at the end of szBuffer so
		// terminate appropriately.

		szBuffer[cchRead] = 0;
		szBuffer[sizeof(szBuffer) - 1 - cchTag - 1] = 0;

		// Search our buffer for the link. 

		char *pszFind = strstr(szBuffer, szTag);
		if (pszFind != NULL) {
			// Assume what the link points to is always an <H?> tag
			// i.e., a large font

			m_fLargeFont = true;

			for(int i = 8; i >= 0; i--)
				m_nchBack[i + 1] = m_nchBack[i];
			m_nchBack[0] = m_nchCurrent;

			m_nchCurrent = nchFile + (int)(pszFind - szBuffer);
			Invalidate();
			return true;
		}
		// max link length is 30 + 9 for <A HREF="
		
		if (strlen(szBuffer) == 39)
			break;

		nchFile += strlen(szBuffer) - 39;
	}

	return false;
}

struct PaintInfo {
    int *pnchEnd;
    HitTest *phittest;
};

void HelpControl::OnPaint(DibBitmap *pbm)
{
	if (m_pfil == NULL) {
        return;
    }

    int nchEnd;
    PaintInfo info;
    info.pnchEnd = &nchEnd;
    info.phittest = &m_hittest;
    Layout(m_nchCurrent, m_fLargeFont, pbm, PaintChunk, &info);

    if (m_wf & kfHelpScrollPosition) {
        Rect rcT;
        Rect rcScrollPos;
        GetSubRects(&rcT, &rcScrollPos);

        pbm->Shadow(rcScrollPos.left, rcScrollPos.top, rcScrollPos.Width(),
                rcScrollPos.Height());
        pbm->Shadow(rcScrollPos.left, rcScrollPos.top, rcScrollPos.Width(),
                rcScrollPos.Height());
        int y1 = rcScrollPos.Height() * m_nchCurrent / m_cb;
        int y2 = rcScrollPos.Height() * nchEnd / m_cb;
        if (y2 - y1 < gcxyBorder * 2) {
            y2 = y1 + gcxyBorder * 2;
            if (y2 > rcScrollPos.bottom) {
                int dy = rcScrollPos.bottom - y2;
                y1 += dy;
                y2 += dy;
            }
        }
        pbm->Fill(rcScrollPos.left, rcScrollPos.top + y1,
                rcScrollPos.Width(), y2 - y1,
                GetColor(kiclrMediumGray));
    }
}

void HelpControl::OnPenEvent(Event *pevt)
{
    switch (pevt->eType) {
    case penDownEvent:
        m_fDrag = false;
        m_yDrag = pevt->y;
        memset(&m_hittest, 0, sizeof(m_hittest));
        m_hittest.x = pevt->x;
        m_hittest.y = pevt->y;
        m_hittest.dBest = 10000;
        Layout(m_nchCurrent, m_fLargeFont, NULL, HitTestChunk, &m_hittest);
        if (pevt->ff & kfEvtFinger) {
            // Need a constant here
            if (m_hittest.dBest <= 20) {
                m_hittest.fHit = true;
            }
        } else {
            if (m_hittest.dBest == 0) {
                m_hittest.fHit = true;
            }
        }
        Invalidate();
        break;

    case penMoveEvent:
        if (!m_fDrag) {
            if (abs(m_yDrag - pevt->y) >=
                    gapfnt[kifntDefault]->GetHeight() / 2) {
                m_fDrag = true;
                m_hittest.fHit = false;
            }
        }
        break;

    case penUpEvent:
        if (m_fDrag) {
            if (m_flics.Init(1, 1.0f, 0.12f, 33, false)) {
                m_yDragUp = pevt->y;
                gtimm.AddTimer(this, 10);
                m_fTimerAdded = true;
            }
            m_fDrag = false;
            return;
        } else {
            if (m_hittest.fHit) {
                m_hittest.fHit = false;
                FollowLink(m_hittest.szText, m_hittest.cch);
            }
        }
        break;

    default:
        m_fDrag = false;
        break;
	}

    if (!m_fDrag) {
        m_flics.Clear();
        return;
    }

    DragScroll(pevt->y);
}

void HelpControl::DragScroll(int y)
{
    int dy = m_yDrag - y;
    if (dy == 0) {
        return;
    }

    // Make it an accelerant since the positioning isn't 1:1 anyway, and
    // help is really long

    //dy *= 5;

    int nchNew;
    if (dy < 0) {
        nchNew = FindPrevPosition(m_nchCurrent, -dy, &m_fLargeFont);
    } else if (dy > 0) {
        nchNew = FindNextPosition(m_nchCurrent, dy, &m_fLargeFont, false, true);
    }

    if (nchNew != m_nchCurrent) {
        m_nchCurrent = nchNew;
        m_yDrag = y;
        Invalidate();
    }
}

void HelpControl::OnTimer(long tCurrent)
{
    if (!m_flics.HasMagnitude()) {
        gtimm.RemoveTimer(this);
        m_fTimerAdded = false;
        return;
    }

    // GetPosition returns a delta.
    Point pt;
    m_flics.GetPosition(&pt);

    DragScroll(pt.y + m_yDragUp);
}

void HelpControl::GetSubRects(Rect *prcInterior, Rect *prcScrollPos)
{
    if (m_wf & kfHelpScrollPosition) {
        if (prcScrollPos != NULL) {
            *prcScrollPos = m_rc;
            prcScrollPos->left = prcScrollPos->right - gcxyBorder * 2;
        }
        *prcInterior = m_rc;
        prcInterior->right -= gcxyBorder * 3;
    } else {
        *prcInterior = m_rc;
    }
}

bool HelpControl::Layout(dword nchStart, bool fLargeFont, DibBitmap *pbm, ChunkProc pfn, void *pv)
{
	bool fDone = false;
	dword nchBuffer = nchStart;
	int	cLines = 0;
	int cyCurrent = 0;

    Rect rcT;
    GetSubRects(&rcT);
	int xCurrent = m_rc.left;
	int yCurrent = m_rc.top;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	xCurrent += rcForm.left;
	yCurrent += rcForm.top;
	int xStart = xCurrent;
	int yTop = rcForm.top + yCurrent;
	int yBottom = rcForm.top + rcT.top + rcT.Height();

    // Layout from nchStart until a callback returns true indicating that we've
    // reached our finish condition: - HitTestChunk and PaintChunk return when
    // we've traversed rcT.bottom - rcT.top - FindPositionCallback will return
    // when the index has reached m_nchCurrent index

	gpakr.fseek(m_pfil, 0, SEEK_END);
	dword cbFile = gpakr.ftell(m_pfil);
	gpakr.fseek(m_pfil, nchBuffer, SEEK_SET);
   	while (nchBuffer < cbFile) {
		cyCurrent = yCurrent - yTop;	

		gpakr.fseek(m_pfil, nchBuffer, SEEK_SET);
		int cchRead = gpakr.fread(m_szText, 1, sizeof(m_szText) - 1, m_pfil);
		m_szText[cchRead] = 0;

		// If we haven't hit a tag then it's normal text
		
		if (m_szText[0] !='<') {
			// Ignore CR / LF at the beginning of the line

			if (m_szText[0] == '\n' || m_szText[0] == '\r') {
				nchBuffer++;
				if (m_szText[1] == '\n' || m_szText[1] == '\r')
					nchBuffer++;
				continue;
			}

			// Raw text to layout. Find out where this text breaks

			char *pszBreakNext = m_szText;
			int cchLine = gapfnt[fLargeFont ? kifntTitle : kifntDefault]->CalcBreak(rcT.right - xCurrent, &pszBreakNext);

			// Extra breaking rules: <
			// Remember if this occurs since we won't progress to the next line

			bool fNextLine = true;
			for (int i = 0; i < cchLine; i++) {
				if (m_szText[i] == '<') {
					pszBreakNext = &m_szText[i];
					cchLine = i;
					fNextLine = false;
					break;
				}
			}

			// Set up chunk for callback

			Chunk chnk;
			chnk.fLargeFont = fLargeFont;
			chnk.cch = cchLine;
			chnk.pbm = pbm;
			chnk.psz = m_szText;	
			chnk.nType = fLargeFont ? knChunkLargeText : knChunkRawText;
			int cx = gapfnt[fLargeFont ? kifntTitle : kifntDefault]->GetTextExtent(chnk.psz, chnk.cch);
			int cy = LineHeight(fLargeFont);

			// Call back to handle this chunk

			if (pfn(xCurrent, yCurrent, cx, cy, nchBuffer, cyCurrent, yBottom, &chnk, pv) )
				return true;

			// Advance; progress to the next line if necessary

			xCurrent += cx;
			if (fNextLine) {
				xCurrent = rcT.left;
				yCurrent += LineHeight(fLargeFont);
				cLines++;
			}

			// Advance to the next text after the break

			nchBuffer += pszBreakNext - m_szText;

			continue;
		}
		
		// H1 or H2?
		// H1 means heading on a new line, large font
		// H2 means heading on a new line, normal font

		if (strncmp(m_szText, "<H1>", 4) == 0 || strncmp(m_szText, "<H2>", 4) == 0) {
			// Call back to know if we're stopping since we need to stop before the <H?>.

			if (pfn(xCurrent, yCurrent, 0, gapfnt[kifntDefault]->GetHeight(), nchBuffer, cyCurrent, yBottom, NULL, pv))
				return true;

			// Format to the next line if not already there

			if (xCurrent != rcT.left) {
				cLines++;
				yCurrent += gapfnt[kifntDefault]->GetHeight();
				xCurrent = rcT.left;
			}
			fLargeFont = (m_szText[2] == '1') ? true : false;
			nchBuffer += 4;
			continue;
		}

		// BR means line break

		if (strncmp(m_szText, "<BR>", 4) == 0) {
			// Call back to know if layout is ending
			
			if (pfn(xCurrent, yCurrent, 0, gapfnt[kifntDefault]->GetHeight(), nchBuffer, cyCurrent, yBottom, NULL, pv))
				return true;

			// Format to the next line

			cLines++;
			yCurrent += gapfnt[kifntDefault]->GetHeight();
			xCurrent = rcT.left;
			nchBuffer += 4;
			continue;
		}

		// HR means horizontal rule
		// a HR always has it's own line, which occupies gapfnt[kifntDefault]->GetHeight() of vertical space

		if (strncmp(m_szText, "<HR>", 4) == 0) {
			Chunk chnk;
			chnk.fLargeFont = fLargeFont;
			chnk.nType = knChunkHRTag;
			chnk.pbm = pbm;
			if(xCurrent != rcT.left) {
				cLines++;
				xCurrent = rcT.left;
				yCurrent += gapfnt[kifntDefault]->GetHeight();
				cyCurrent = yCurrent - yTop;
			}	
			
			// Call back to know if layout is ending
			
			if (pfn(xCurrent, yCurrent, rcT.Width(), knHelpControlBRHeight, nchBuffer, cyCurrent, yBottom, &chnk, pv))
				return true;

			// Format to the next line

			cLines++;
			yCurrent += knHelpControlBRHeight;
			xCurrent = rcT.left;
			nchBuffer += 4;
			continue;
		}

		// <A HREF="#link">link text</A>

		if (strncmp(m_szText, "<A HREF", 7) == 0) {	// link or target
				
			// Otherwise we have a link. Find the link text

			char *pchLinkStart = strchr(&m_szText[10], '>') + 1;
			char *pchLinkEnd = strchr(&m_szText[10], '<');
			Assert(pchLinkStart != NULL);
			Assert(pchLinkEnd != NULL);
			Assert(pchLinkEnd - pchLinkStart < 100);

			// Word wrap the link text

			char *pchT = pchLinkStart;
			while (pchT < pchLinkEnd) {
				char *pchBreakNext = pchT;
				bool fNewLine = true;
				int cchChunk = gapfnt[kifntDefault]->CalcBreak(rcT.right - xCurrent, &pchBreakNext, xCurrent == xStart);
				if (cchChunk > pchLinkEnd - pchT) {
					cchChunk = (int)(pchLinkEnd - pchT);
					pchBreakNext = pchLinkEnd;
					fNewLine = false;
				}	

				// Handle this chunk

				Chunk chnk;
				chnk.fLargeFont = fLargeFont;
				chnk.psz = pchT;
				chnk.nType = knChunkLinkText;
				chnk.pbm = pbm;
				chnk.cch = cchChunk;
				strncpy(chnk.szText, &m_szText[10],pchLinkStart - &m_szText[10] - 2);
				chnk.szText[pchLinkStart - &m_szText[10] - 2] = 0;

				int cx = gapfnt[kifntDefault]->GetTextExtent(chnk.psz, chnk.cch);
				int cy = gapfnt[kifntDefault]->GetHeight();
				if (pfn(xCurrent, yCurrent, cx, cy, nchBuffer, cyCurrent, yBottom, &chnk, pv))
					return true;

				// Adjust x, y position

				xCurrent += cx;
				if (fNewLine) {
					cLines++;
					xCurrent = rcT.left;
					yCurrent += cy;
				}

				// Go to next text chunk

				pchT = pchBreakNext;
			}

			// Advance past the </A>

			nchBuffer += pchLinkEnd - m_szText + 4;
			continue;
		}

		// <A NAME="link_target">

		if (strncmp(m_szText, "<A NAME=\"", 9) == 0) {
			// If it's a link target, just skip over it since nothing is displayed

			char *pchT = strchr(m_szText, '>');
			if (pchT != NULL)
				nchBuffer += (pchT - m_szText) + 1;
			continue;
		}

		// Image?
		// <IMG SRC="filename.amx">

		if (strncmp(m_szText, "<IMG SRC=\"", 10) == 0) {
			// Find the filename delimiters (quotes)

			char *pchFilenameStart = &m_szText[10];
			char *pchFilenameEnd = strchr(pchFilenameStart, '\"');
			Assert(pchFilenameEnd != NULL);
			Assert(pchFilenameEnd - pchFilenameStart < 32);
			*pchFilenameEnd = 0;

			// Load the file, get bounds

			AnimationData* panid = LoadAnimationData(pchFilenameStart);
			Assert(panid != NULL);
			if (panid == NULL) {
				nchBuffer += pchFilenameEnd - m_szText + 2;
				continue;
			}
			Rect rc;
			panid->GetBounds(panid->GetStripIndex("help"), 0, &rc);

			// Images always start on a new line

			if (xCurrent != rcT.left) {
				yCurrent += LineHeight(fLargeFont);
				cLines++;
				xCurrent = rcT.left;
			}

			// Handle this chunk

			Chunk chnk;
			chnk.nType = knChunkAniData;
			chnk.pv = panid;
			chnk.fLargeFont = fLargeFont;
			chnk.pbm = pbm;
			if (pfn(xCurrent, yCurrent, rc.Width(), rc.Height(), nchBuffer, cyCurrent, yBottom, &chnk, pv)) {
				delete panid;
				return true;
			}

			// Always go to the next line

			yCurrent += rc.Height();
			nchBuffer += pchFilenameEnd - m_szText + 2;

			delete panid;
		} 

		// skip over any </ tags that indicate ending use of a font 

		if (strncmp(m_szText, "</", 2) == 0) {
			int i;
			for (i = 1; m_szText[i] != '>'; i++);

			nchBuffer += i + 1;
		}
	}
 	return true;
}

int HelpControl::LineHeight(bool fLargeFont)
{
	return gapfnt[fLargeFont ? kifntTitle : kifntDefault]->GetHeight();
}

void HelpControl::DoNextPage()
{
	//take care of updating the back array
	for(int i = 8; i >= 0; i--)
		m_nchBack[i + 1] = m_nchBack[i];
	m_nchBack[0] = m_nchCurrent;

	m_nchCurrent = FindNextPosition(m_nchCurrent, m_cyPageAmount, &m_fLargeFont, false, false);
	Invalidate();
}

void HelpControl::DoPrevPage()
{
	//take care of updating the back array
	for(int i = 8; i >= 0; i--)
		m_nchBack[i + 1] = m_nchBack[i];
	m_nchBack[0] = m_nchCurrent;

	m_nchCurrent = FindPrevPosition(m_nchCurrent, m_cyPageAmount, &m_fLargeFont);
	Invalidate();
}

int HelpControl::FindPrevPosition(int nchFrom, int cyAmount, bool *pfLargeFont)
{
	// If no room to scroll back, we're done

	if (nchFrom == 0)
		return 0;

	// check if there is an <HR> either directly above the current location
	// or there is a link target above the location with an <HR> above that

	char sz[64];
	gpakr.fseek(m_pfil, nchFrom - (sizeof(sz) - 1), SEEK_SET);
	int cchRead = gpakr.fread(sz, 1, sizeof(sz) - 1, m_pfil);
	sz[cchRead] = 0;

    // if there is a tag directly above us figure out if it is a HR or a link
    // and move nchFrom to directly before those tags. Since we're finding the
    // previous heading if the HR would have fit on the page anyways it will be
    // displayed, but if it wouldn't have fit then we don't want to display it.
	
    if (sz[cchRead - 1] == '>' || sz[cchRead - 3] == '>') { int i = cchRead -
1; while(sz[i] != '<') i--;
        // we found the link for the heading and need to check for an HR before
        // it
        if(sz[i + 1] == 'A') for(i--; sz[i] != '<'; i--);

		// we want to ignore the HR at the bottom of the screen in case the 
		// section above takes up the entire screen.
	
		if(strncmp(&sz[i], "<HR>", 4) == 0)
			nchFrom -= cchRead - i;
	}

    // Search back for a header since we know that'll format starting on a left
    // edge.  Then layout forward until we've formatted more space than the
    // scroll back amount.  Then layout forward the difference between the
    // heading we found and the desired scroll back amount.

	int nchSeek = nchFrom;
	FindPositionHelper fph;
	fph.nIndex = 0;
	fph.nDistY = 0;
	while (nchSeek != 0) {
		// Find the position to read forward from

		char szT[256];
		szT[255] = 0;
		nchSeek -= sizeof(szT) - 1;
		if (nchSeek < 0)
			nchSeek = 0;

		// Read in this piece

		gpakr.fseek(m_pfil, nchSeek, SEEK_SET);
		int cchRead = gpakr.fread(szT, 1, sizeof(szT) - 1, m_pfil);
		szT[cchRead] = 0;

		// Find prev heading <h1> in buffer that when formatted forward from 
		// gives us a vertical size greater than the desired page amount

		bool fDone = false;
		for (int i = cchRead - 1; i >= 0; i--) {
			if (strncmp(&szT[i], "<H1>", 4) == 0) {
				fph.cySpan = 0;
				fph.nCondition = knFindPosRunToIndex;
				fph.nIndex = nchFrom;

				Layout(nchSeek + i, false, NULL, FindPositionCallback, &fph);
				
				fph.nIndex = nchSeek + i;
// CRM temp: alternate means of paging starts here
				if (fph.cySpan <= m_rc.Height())
					return fph.nIndex;
				else
					return FindNextPosition(fph.nIndex, fph.cySpan - cyAmount, pfLargeFont, true, false);
			}
		}
	}

	return 0;
// temp: ends here
/* CRM Removed to try alternate means of paging
				if (fph.cySpan >= cyAmount) {
					*pfLargeFont = fph.fLargeFont;
					fDone = true;
					break;
				}

			}
		}
		if (fDone)
			break;
	}
	if (fph.cySpan - cyAmount > 0)
		return FindNextPosition(fph.nIndex, fph.cySpan - cyAmount, pfLargeFont, true, false);

	*pfLargeFont = fph.fLargeFont;
	return fph.nIndex;
*/
	return 0;
}

// fCondition is used to indicate whether we are to travel at most cyAmount or at least
// cyAmount

int HelpControl::FindNextPosition(int nchFrom, int cyAmount, bool *pfLargeFont, bool fCondition, bool fSmooth)
{
	FindPositionHelper fph;
    memset(&fph, 0, sizeof(fph));
	fph.nIndex = nchFrom;
	fph.nDistY = cyAmount;
	fph.cySpan = cyAmount;
	fph.cyControl = m_rc.Height();
#if defined(IPHONE) || defined(SDL)
    fph.nCondition = knFindPosFingerScroll;
#else
	fph.nCondition = fCondition ? knFindPosAtLeastY : knFindPosAtMostY;
#endif
	fph.nchLastHR = -1;
	fph.nchFirstHR = -1;
	fph.nchSpanMet = -1;

	// advance fph.nDistY

	Layout(nchFrom, *pfLargeFont, NULL, FindPositionCallback, &fph);

    // if we found a HR that isn't directly above the screen we are paging up
    // from account for both "\cr\lf<HR>" and "<HR>\cr\lf

	if (!fSmooth && fph.nchLastHR != -1 && fph.nchFirstHR + 4 != m_nchCurrent 
			&& fph.nchFirstHR + 6 != m_nchCurrent) {

        // decide which HR rule to use. If we are paging up then we use the
        // first and if we are paging down we use the last

/* CRM temp: trying the heading at a time thing
		int nchIndexOfHR = fph.nchLastHR;
*/
// trying out the one heading at a time method
		int nchIndexOfHR = fph.nchFirstHR;
// end
		bool fLargeFontT = fph.fLargeFontLastHR;
		if (fph.nCondition == knFindPosAtLeastY) {
			nchIndexOfHR = fph.nchFirstHR;
			fLargeFontT = fph.fLargeFontFirstHR;
		}

		// find <HR>

		char szBuffer[20];
		gpakr.fseek(m_pfil, nchIndexOfHR, SEEK_SET);
		int cchRead = gpakr.fread(szBuffer, 1, sizeof(szBuffer) - 1, m_pfil);
		szBuffer[cchRead] = 0;
		char *pszFind = strstr(szBuffer, "<HR>");
		
		*pfLargeFont = fLargeFontT;
		return nchIndexOfHR + (int)(pszFind + 4 - szBuffer);
	} else {
		// otherwise we should use the position at which the span was met

		if (fph.nchSpanMet != -1) {
			*pfLargeFont = fph.fLargeFontSpanMet;
			return fph.nchSpanMet;
		} else {
			return nchFrom;
		}
	}

	// Should get to here because the span should always be met.

	Assert(false);
	return nchFrom;
}

void HelpControl::DoIndex()
{
	m_nchCurrent = 0;
	m_fLargeFont = false;
	Invalidate();
}

void HelpControl::DoBack()
{
	m_nchCurrent = m_nchBack[0];
	m_fLargeFont = false;

	for(int i = 0; i < 9; i++)
		m_nchBack[i] = m_nchBack[i + 1];
	Invalidate();
}

bool PaintChunk(int x, int y, int cx, int cy, int nchBuffer, int cyTotal, int nyBottom, Chunk *pChunk, void *pv = NULL)
{
    PaintInfo *pinfo = (PaintInfo *)pv;
    int *pnchEnd = pinfo->pnchEnd;
    if (y < nyBottom) {
        *pnchEnd = nchBuffer;
    }

	if (y  + cy >= nyBottom)
		return true;

	if (pChunk != NULL) {
		switch (pChunk->nType) {
		case knChunkRawText:
			gapfnt[kifntDefault]->DrawText(pChunk->pbm, pChunk->psz, x, y, pChunk->cch);
			break;
		
		case knChunkLargeText:
			gapfnt[kifntTitle]->DrawText(pChunk->pbm, pChunk->psz, x, y, pChunk->cch);
			break;

		case knChunkBitmap:
			( (TBitmap *)pChunk->pv)->BltTo(pChunk->pbm, x, y);
			break;
		
		case knChunkAniData:
			{
				int index = ( (AnimationData *)pChunk->pv)->GetStripIndex("help");
				
				Rect rc;
				( (AnimationData *)pChunk->pv)->GetBounds(index, 0, &rc);

				if (rc.Height() + y > nyBottom)
					return true;
				( (AnimationData *)pChunk->pv)->DrawFrame(index, 0, pChunk->pbm, x - rc.left, y - rc.top, kside1);
			}
			break;

		case knChunkLinkText:
			{
                // +10 is a hack since nchBuffer points to the anchor tag
                if (pinfo->phittest->fHit &&
                        (pinfo->phittest->nchBuffer == nchBuffer ||
                        (pinfo->phittest->nchBuffer < nchBuffer &&
                        pinfo->phittest->nchBuffer + 10 >= nchBuffer))) {
                    int cx = gapfnt[kifntDefault]->GetTextExtent(pChunk->psz,
                            pChunk->cch);
                    int cy = gapfnt[kifntDefault]->GetHeight();
                    pChunk->pbm->Fill(x, y - 1, cx, cy + 2,
                            GetColor(kiclrButtonFillHighlight));
                }

				gapfnt[kifntDefault]->DrawText(pChunk->pbm, pChunk->psz, x, y, pChunk->cch);
				
				// BUG: fix for descender height
				// HACK: this will not work properly if the help control doesn't span below half the
				// screen.
				if (pChunk->cch != 0) {
					int descenderAdjust = (nyBottom <= 160? 1: 2);
					y += gapfnt[kifntDefault]->GetHeight() - descenderAdjust;
					pChunk->pbm->DrawLine(x, y, x + cx, y, GetColor(kiclrJana));
				}
			}
			break;

		case knChunkHRTag:	
			{
				// draw three rectangles
				// draw center rectangle

				int cxDot = PcFromFc(2);
				pChunk->pbm->Fill(x + cx * 3 / 8 - cxDot / 2, y + cy / 2 - cxDot / 2, cxDot, cxDot, GetColor(kiclrWhite));

				// draw left rectangle

				pChunk->pbm->Fill(x + cx * 4 / 8 - cxDot / 2, y + cy / 2 - cxDot / 2, cxDot, cxDot, GetColor(kiclrWhite));

				// draw right rectangle

				pChunk->pbm->Fill(x + cx * 5 / 8 - cxDot / 2, y + cy / 2 - cxDot / 2, cxDot, cxDot, GetColor(kiclrWhite));
			}
			break;
		}
		
	}
	return false;
}

bool HitTestChunk(int x, int y, int cx, int cy, int nchBuffer, int cyTotal, int nyBottom, Chunk *pChunk, void *pv) 
{
	if (y + cy >= nyBottom)
		return true;

    Rect rc;
	if (pChunk != NULL) {
		switch (pChunk->nType) {
		case knChunkLinkText:	// we only care to hittest text
			HitTest *pHitTest = (HitTest *)pv;
            rc.Set(x, y, x + cx, y + cy);
            int d = rc.GetDistance(pHitTest->x, pHitTest->y);
            if (d < pHitTest->dBest) {
				pHitTest->dBest = d;
                pHitTest->nchBuffer = nchBuffer;
				strcpy(pHitTest->szText, pChunk->szText);
				pHitTest->cch = (int)strlen(pHitTest->szText);
			}
			break;
		}
	}
	return false;
}

bool FindPositionCallback(int x, int y, int cx, int cy, int nchBuffer, int cyTotal, int nyBottom, Chunk *pChunk, void *pv)
{
	FindPositionHelper *pFPH = (FindPositionHelper*)pv;

#if 0
	if (pChunk != NULL && (pChunk->nType == knChunkRawText ||
            pChunk->nType == knChunkLargeText)) {
		pFPH->fLargeFont = (pChunk->nType == knChunkRawText) ? false : true;
    }
#else
    if (pChunk != NULL) {
        if (pChunk->nType == knChunkLargeText) {
            pFPH->fLargeFont = true;
        } else {
            pFPH->fLargeFont = false;
        }
    }
#endif

    // we're counting the vertical (Y) distance traveled from a given point to
    // pFPH->nIndex

	if (pFPH->nCondition == knFindPosRunToIndex) {
		if(nchBuffer >= pFPH->nIndex) {
			pFPH->cySpan = cyTotal;
			return true;
		}
		return false;
	}

    // keep track of the indexes where we found the first and last HR tags as
    // well as what the font was at that HR tag. We do this check after the
    // PosRunToIndex check because we may need to break prior to this point.

	if (pChunk != NULL && pChunk->nType == knChunkHRTag && cyTotal < pFPH->cyControl) {
		if (pFPH->nchFirstHR == -1) {
			pFPH->nchFirstHR = nchBuffer;
			pFPH->fLargeFontFirstHR = pChunk->fLargeFont;
		}
		pFPH->nchLastHR = nchBuffer;
		pFPH->fLargeFontLastHR = pChunk->fLargeFont;
	}

    // in these cases we travel until we've traveled the height of the control
    // so that we can be sure to find any HR tags we should break at within the
    // page.

	switch (pFPH->nCondition) {
	case knFindPosAtMostY:
        // Used when paging forward. Remember index at cySpan (7/8ths
        // currently).

		if (cy + cyTotal >= pFPH->cySpan && pFPH->nchSpanMet == -1) {
			pFPH->nchSpanMet = nchBuffer;
			pFPH->fLargeFontSpanMet = pFPH->fLargeFont;
		}

		// check to see if we've traveled the height of the control

		if (cyTotal > pFPH->cyControl) {
			pFPH->nIndex = nchBuffer;
			return true;
		}
		break;

	case knFindPosAtLeastY:
		// Used when implementing page back by doing a forward Layout by
		// a specific delta. 

		if (cyTotal >= pFPH->cySpan && pFPH->nchSpanMet == -1) {
			pFPH->nchSpanMet = nchBuffer;
			pFPH->fLargeFontSpanMet = pFPH->fLargeFont;
		}

		// check to see if we've traveled the height of the control

		if (cyTotal > pFPH->cyControl && pFPH->nchSpanMet != -1)
			return true;
		break;

    case knFindPosFingerScroll:
        pFPH->nchSpanMet = nchBuffer;
        pFPH->fLargeFontSpanMet = pFPH->fLargeFont;
		return cyTotal >= pFPH->cySpan;
	}
	return false;
}

//
// Help implementation
//

class HelpForm : public ShellForm
{
public:
	HelpForm() secHelpForm;
	virtual bool DoModal(const char *pszLink, const char *pszFile) secHelpForm;
	virtual void OnControlSelected(word idc) secHelpForm;
	virtual bool EventProc(Event *pevt) secHelpForm;
};

void Help(const char *pszAnchor, bool fPauseSimulation, const char *pszFile)
{
    word idf;
    Size siz;
    gpmfrmm->GetDib()->GetSize(&siz);
    if (siz.cx >= 480) {
        idf = kidfHelpWide;
    } else {
        idf = kidfHelp;
    }

	HelpForm *pfrm = (HelpForm *)gpmfrmm->LoadForm(gpiniForms, idf, new HelpForm());
	if (pfrm != NULL) {
		if (fPauseSimulation)
			gsim.Pause(true);
		pfrm->DoModal(pszAnchor, pszFile);
		if (fPauseSimulation)
			gsim.Pause(false);
		delete pfrm;
	}
}

HelpForm::HelpForm()
{
}

bool HelpForm::DoModal(const char *pszLink, const char *pszFile)
{
	HelpControl *pctl = (HelpControl *)GetControlPtr(kidcHelp);
	pctl->SetFile(pszFile == NULL ? "help.txt" : pszFile);
	if (pszLink != NULL)
		pctl->FollowLink(pszLink);

    Control *pctlT = GetControlPtr(kidcNextPage);
    pctlT->Show(true);
    pctlT = GetControlPtr(kidcPrevPage);
    pctlT->Show(true);
    pctlT = GetControlPtr(kidcBack);
    pctlT->Show(true);

	int idc;
	ShellForm::DoModal(&idc, false);
	if (idc == kidcCancel)
		return false;
	return true;
}

void HelpForm::OnControlSelected(word idc)
{
	HelpControl *pctl = (HelpControl *)GetControlPtr(kidcHelp);

	switch (idc) {
	case kidcPrevPage:
		pctl->DoPrevPage();
		break;

	case kidcNextPage:
		pctl->DoNextPage();
		break;
	
	case kidcBack:
		pctl->DoBack();
		break;

	case kidcIndex:
		pctl->DoIndex();
		break;

    case kidcOk:
		EndForm(idc);
        break;
	}
}

bool HelpForm::EventProc(Event *pevt)
{
	switch (pevt->eType) {
	case keyDownEvent:
		switch (pevt->chr) {
		case chrUp:
			OnControlSelected(kidcPrevPage);
			return true;
		
		case chrDown:
			OnControlSelected(kidcNextPage);
			return true;

        case vchrBack:
            OnControlSelected(kidcOk);
            return true;
		}
	default:
		return Form::EventProc(pevt);
	}
	return false;
}

} // namespace wi
