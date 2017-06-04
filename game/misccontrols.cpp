#include "ht.h"

namespace wi {

// Button control

TBitmap *ButtonControl::s_ptbmLeftUp;
TBitmap *ButtonControl::s_ptbmMidUp;
TBitmap *ButtonControl::s_ptbmRightUp;
TBitmap *ButtonControl::s_ptbmLeftDown;
TBitmap *ButtonControl::s_ptbmMidDown;
TBitmap *ButtonControl::s_ptbmRightDown;

bool ButtonControl::InitClass()
{
	s_ptbmLeftUp = CreateTBitmap("buttonleftup.png");
	s_ptbmMidUp = CreateTBitmap("buttonmidup.png");
	s_ptbmRightUp = CreateTBitmap("buttonrightup.png");
	s_ptbmLeftDown = CreateTBitmap("buttonleftdown.png");
	s_ptbmMidDown = CreateTBitmap("buttonmiddown.png");
	s_ptbmRightDown = CreateTBitmap("buttonrightdown.png");
	return true;
}

void ButtonControl::ExitClass()
{
}

ButtonControl::ButtonControl()
{
	m_nfnt = 0;
	m_ptbmUp = NULL;
	m_ptbmDown = NULL;
	m_szLabel = NULL;
}

ButtonControl::~ButtonControl()
{
	if (m_szLabel != NULL)
		gmmgr.FreePtr(m_szLabel);
}

bool ButtonControl::Init(Form *pfrm, word idc, int x, int y, int cx, int cy, 
		char *pszLabel, int nfnt, char *szFnUp, char *szFnDown, char *szFnDisabled)
{
	if (!Control::Init(pfrm, idc, x, y, cx, cy))
		return false;

	return Init(pszLabel, nfnt, szFnUp, szFnDown, szFnDisabled, false);
}

bool ButtonControl::Init(char *pszLabel, int nfnt, char *szFnUp, char *szFnDown, char *szFnDisabled, bool fCenter)
{
	m_nfnt = nfnt;
	m_szLabel = (char *)gmmgr.AllocPtr(strlen(pszLabel) + 1);
	gmmgr.WritePtr(m_szLabel, 0, pszLabel, strlen(pszLabel) + 1);

	if (szFnUp != NULL)
		m_ptbmUp = CreateTBitmap(szFnUp);
	if (szFnDown != NULL)
		m_ptbmDown = CreateTBitmap(szFnDown);
	if (szFnDisabled != NULL)
		m_ptbmDisabled = CreateTBitmap(szFnDisabled);

	if (m_ptbmUp != NULL && m_ptbmDown != NULL) {
		Size siz1 = { 0, 0 };
		m_ptbmUp->GetSize(&siz1);
		Size siz2 = { 0, 0 };
		m_ptbmDown->GetSize(&siz2);
		int cx = _max(siz1.cx, siz2.cx);
		int cy = _max(siz1.cy, siz2.cy);
		if (cx != 0 && cy != 0) {
			m_rc.right = m_rc.left + _max(siz1.cx, siz2.cx);
			m_rc.bottom = m_rc.top + _max(siz1.cy, siz2.cy);
		}
	} else {
#if 0 // old-style filled-rect buttons
		Font *pfnt = gapfnt[m_nfnt];
		int cx = pfnt->GetTextExtent(m_szLabel);
		int cy = pfnt->GetHeight();
		if (m_rc.right == m_rc.left)
				m_rc.right = m_rc.left + cx + PcFromFc(3 + 3);
		if (m_rc.bottom == m_rc.top)
			m_rc.bottom = m_rc.top + cy + PcFromFc(1 + 2);
#else
		Size sizMid;
		s_ptbmMidUp->GetSize(&sizMid);

		// Always set the height correctly so that updatemap is correct
		// and TBitmap::Fill() doesn't need to clip.

		m_rc.bottom = m_rc.top + sizMid.cy;

		// Default width

		if (m_rc.right == m_rc.left)
			m_rc.right = m_rc.left + (sizMid.cx * 3);
#endif
	}

	if (fCenter)
		m_rc.Offset(-m_rc.Width() / 2, 0);

	return true;
}

bool ButtonControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) "label" nfnt center buttonup.png buttondown.png 

	char szFnUp[kcbFilename];
	char szFnDown[kcbFilename];
	char szFnDisabled[kcbFilename];
	
	char szT[32];
	char szLabel[64];
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) \"%s\" %d %s %s %s %s",
			szLabel, &m_nfnt, szT, szFnUp, szFnDown, szFnDisabled);
	switch (cArgs) {
	case 2:
		return Init(szLabel, m_nfnt, NULL, NULL, NULL, false);
	case 3:
		return Init(szLabel, m_nfnt, NULL, NULL, NULL, strcmp(szT, "center") == 0);
	case 5:
		return Init(szLabel, m_nfnt, szFnUp, szFnDown, NULL, strcmp(szT, "center") == 0);
	case 6:
		return Init(szLabel, m_nfnt, szFnUp, szFnDown, szFnDisabled, strcmp(szT, "center") == 0);
	}

	return false;
}

void ButtonControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	bool fSelected = m_pfrm->IsControlInside(this);
	if (m_ptbmDown != NULL && m_ptbmUp != NULL) {
		// Draw up / down image (if present)

		TBitmap *ptbm;
		if (m_wf & kfCtlDisabled)
			ptbm = m_ptbmDisabled;
		else
			ptbm = fSelected ? m_ptbmDown : m_ptbmUp;

		ptbm->BltTo(pbm, m_rc.left + rcForm.left, m_rc.top + rcForm.top, m_wf & kfCtlUseSide1Colors ? kside1 : ksideNeutral);

		// Center the text (if present)

		if (m_szLabel[0] != 0) {
			Font *pfnt = gapfnt[m_nfnt];
			int cx = pfnt->GetTextExtent(m_szLabel);
			int cy = pfnt->GetHeight();
			int x = m_rc.left + (m_rc.Width() - cx + 1) / 2;
			int y = m_rc.top + (m_rc.Height() - cy + gcxyBorder) / 2;
			if (fSelected) {
				x++;
				y++;
			}
			gapfnt[m_nfnt]->DrawText(pbm, m_szLabel, x + rcForm.left, y + rcForm.top);
		}
	} else {
#if 0 // old-style filled-rect buttons
		Rect rcT = m_rc;
		rcT.Offset(rcForm.left, rcForm.top);

		int cxyBorder2x = gcxyBorder * 2;
		pbm->Fill(rcT.left + gcxyBorder, rcT.top + gcxyBorder, rcT.Width() - cxyBorder2x, rcT.Height() - cxyBorder2x, GetColor(fSelected ? kiclrButtonFillHighlight : kiclrButtonFill));

		int iclr = GetColor(fSelected ? kiclrWhite : kiclrButtonBorder);
		pbm->Fill(rcT.left + gcxyBorder, rcT.top, rcT.Width() - cxyBorder2x, gcxyBorder, iclr);
		pbm->Fill(rcT.left, rcT.top + gcxyBorder, gcxyBorder, rcT.Height() - cxyBorder2x, iclr);
		pbm->Fill(rcT.right - gcxyBorder, rcT.top + gcxyBorder, gcxyBorder, rcT.Height() - cxyBorder2x, iclr);
		pbm->Fill(rcT.left + gcxyBorder, rcT.bottom - gcxyBorder, rcT.Width() - cxyBorder2x, gcxyBorder, iclr);

		if (m_szLabel[0] != 0) {
			Font *pfnt = gapfnt[m_nfnt];
			int cx = pfnt->GetTextExtent(m_szLabel);
			int cy = pfnt->GetHeight();
			int x = m_rc.left + (m_rc.Width() - cx + 1) / 2;
			int y = m_rc.top + (m_rc.Height() - cy + gcxyBorder) / 2;
			gapfnt[m_nfnt]->DrawText(pbm, m_szLabel, x + rcForm.left, y + rcForm.top);
		}
#else
		Rect rcT = m_rc;
		rcT.Offset(rcForm.left, rcForm.top);

		TBitmap *ptbmLeft, *ptbmMid, *ptbmRight;
		if (!fSelected) {
			ptbmLeft = s_ptbmLeftUp;
			ptbmMid = s_ptbmMidUp;
			ptbmRight = s_ptbmRightUp;
		} else {
			ptbmLeft = s_ptbmLeftDown;
			ptbmMid = s_ptbmMidDown;
			ptbmRight = s_ptbmRightDown;
		}

		Size sizLeft, sizMid, sizRight;
		ptbmLeft->GetSize(&sizLeft);
		ptbmMid->GetSize(&sizMid);
		ptbmRight->GetSize(&sizRight);

		int x = rcT.left;
		int xRight = rcT.right - sizRight.cx;

		ptbmLeft->BltTo(pbm, x, rcT.top);
		x += sizLeft.cx;
#if 0
		while (x < xRight) {
			ptbmMid->BltTo(pbm, x, rcT.top);
			x += sizMid.cx;
		}
#else
		ptbmMid->FillTo(pbm, x, rcT.top, xRight - x, rcT.Height());
		x = xRight;
#endif

		ptbmRight->BltTo(pbm, xRight, rcT.top);

		if (m_szLabel[0] != 0) {
			Font *pfnt = gapfnt[m_nfnt];
			int cx = pfnt->GetTextExtent(m_szLabel);
			int cy = pfnt->GetHeight();
			int x = m_rc.left + (m_rc.Width() - cx + 1) / 2;
			int y = m_rc.top + ((sizMid.cy - gapfnt[m_nfnt]->GetHeight() + 1) / 2);
			gapfnt[m_nfnt]->DrawText(pbm, m_szLabel, x + rcForm.left, y + rcForm.top);
		}
#endif
	}
}

void ButtonControl::OnSelect(int nSelect)
{
	if (m_wf & kfCtlDisabled)
		return;

	switch (nSelect) {
	case knSelMoveInside:
	case knSelDownInside:
		gsndm.PlaySfx(ksfxGuiButtonTap);
		Invalidate();
		break;

	case knSelUpInside:
	case knSelMoveOutside:
		Invalidate();
		break;
	}

	Control::OnSelect(nSelect);
}

void ButtonControl::SetText(char *psz)
{
	if (strcmp(m_szLabel, psz) == 0)
		return;

	if (m_szLabel != NULL)
		gmmgr.FreePtr(m_szLabel);
	m_szLabel = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_szLabel, 0, psz, strlen(psz) + 1);

	Invalidate();
}

#if 0 // not used now
// Preset button control

bool PresetButtonControl::Init(char *pszLabel, int nfnt, char *szFnUp, char *szFnDown, bool fCenter)
{
	if (szFnUp != NULL)
		m_ptbmUp = CreateTBitmap(szFnUp);
	if (szFnDown != NULL)
		m_ptbmDown = CreateTBitmap(szFnDown);

	if (m_ptbmUp != NULL) {
		Size siz1 = { 0, 0 };
		m_ptbmUp->GetSize(&siz1);
		if (siz1.cx != 0 && siz1.cy != 0) {
			m_rc.right = m_rc.left + siz1.cx + 2;
			m_rc.bottom = m_rc.top + siz1.cy + 2;
		}
	}
	if (fCenter)
		m_rc.Offset(-m_rc.Width() / 2, 0);

	return true;
}

void PresetButtonControl::OnPaint(DibBitmap *pbm)
{
	bool fSet = (m_wf & kfCtlSet) != 0;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	bool fSelected = m_pfrm->IsControlInside(this);
	Rect rcT = m_rc;
	rcT.Offset(rcForm.left, rcForm.top);

	if (fSelected) {
		pbm->Fill(rcT.left + 1, rcT.top + 1, rcT.Width() - 2, rcT.Height() - 2, GetColor(kiclrButtonFillHighlight));
	}

	TBitmap *ptbm;
	if (fSet)
		ptbm = m_ptbmDown;
	else
		ptbm = m_ptbmUp;

	if (ptbm != NULL)
		ptbm->BltTo(pbm, m_rc.left + rcForm.left + 1, m_rc.top + rcForm.top + 1);
}
#endif

// CheckBox Control

TBitmap *CheckBoxControl::s_ptbmOnUp;
TBitmap *CheckBoxControl::s_ptbmOnDown;
TBitmap *CheckBoxControl::s_ptbmOffUp;
TBitmap *CheckBoxControl::s_ptbmOffDown;

CheckBoxControl::CheckBoxControl()
{
	m_fChecked = false;
	m_ifnt = 0;
	m_szLabel[0] = 0;
}

CheckBoxControl::~CheckBoxControl()
{
}

bool CheckBoxControl::Init(Form *pfrm, word idc, int x, int y, char *pszLabel, int ifnt, bool fChecked)
{
	if (!Control::Init(pfrm, idc, x, y, 0, 0))
		return false;

	return Init(pszLabel, ifnt, fChecked);
}

bool CheckBoxControl::InitClass()
{
	s_ptbmOnUp = CreateTBitmap("checkboxonup.png");
	s_ptbmOnDown = CreateTBitmap("checkboxondown.png");
	s_ptbmOffUp = CreateTBitmap("checkboxoffup.png");
	s_ptbmOffDown = CreateTBitmap("checkboxoffdown.png");

	return true;
}

void CheckBoxControl::ExitClass()
{
}

bool CheckBoxControl::Init(char *pszLabel, int ifnt, bool fChecked)
{
	m_fChecked = fChecked;
	m_ifnt = ifnt;
	SetText(pszLabel);
	return true;
}

bool CheckBoxControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) "label" ifnt [checked]

	char szChecked[64];
	szChecked[0] = 0;
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) \"%s\" %d %s",
			m_szLabel, &m_ifnt, szChecked);
	if (cArgs == 2)
		return Init(m_szLabel, m_ifnt, false);
	if (cArgs != 3)
		return false;

	return Init(m_szLabel, m_ifnt, strcmp(szChecked, "checked") == 0);
}

void CheckBoxControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	// Draw up / down image (if present)

	TBitmap *ptbm;
	bool fPenDownInside = m_pfrm->IsControlInside(this);
	if (fPenDownInside) {
		ptbm = m_fChecked ? s_ptbmOnDown : s_ptbmOffDown;
	} else {
		ptbm = m_fChecked ? s_ptbmOnUp : s_ptbmOffUp;
	}

	// Center the text horizontally and draw to the right of the checkbox

	Font *pfnt = gapfnt[m_ifnt];
	int cy = pfnt->GetHeight();
	Size siz;
	ptbm->GetSize(&siz);
	ptbm->BltTo(pbm, m_rc.left + rcForm.left, m_rc.top + (cy - siz.cy) / 2 + rcForm.top - 1);
	gapfnt[m_ifnt]->DrawText(pbm, m_szLabel, m_rc.left + rcForm.left + siz.cx, m_rc.top + rcForm.top);
}

void CheckBoxControl::OnSelect(int nSelect)
{
	switch (nSelect) {
	case knSelMoveInside:
	case knSelDownInside:
		gsndm.PlaySfx(ksfxGuiCheckBoxTap);
		Invalidate();
		break;

	case knSelUpInside:
		m_fChecked ^= 1;
		Invalidate();
		break;

	case knSelMoveOutside:
		Invalidate();
		break;
	}

	Control::OnSelect(nSelect);
}

void CheckBoxControl::SetText(char *psz)
{
	strncpyz(m_szLabel, psz, sizeof(m_szLabel));
	Size siz;
	s_ptbmOnUp->GetSize(&siz);
	Font *pfnt = gapfnt[m_ifnt];
	int cx = pfnt->GetTextExtent(m_szLabel);
	m_rc.right = m_rc.left + siz.cx + pfnt->GetTextExtent(m_szLabel);
	m_rc.bottom = m_rc.top + pfnt->GetHeight();
	Invalidate();
}

bool CheckBoxControl::IsChecked()
{
	return m_fChecked;
}

void CheckBoxControl::SetChecked(bool fChecked)
{
	m_fChecked = fChecked;
	Invalidate();
}

// Label Control

LabelControl::LabelControl()
{
	m_nfnt = 0;
	m_szLabel = NULL;
}

LabelControl::~LabelControl()
{
	if (m_szLabel != NULL)
		gmmgr.FreePtr(m_szLabel);
}

bool LabelControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) "label" nfnt

	char szLabel[256];
	char szFlag1[32], szFlag2[32];
	szFlag1[0] = 0;
	szFlag2[0] = 0;
	int nfnt;
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) \"%s\" %d %s %s",
			szLabel, &nfnt, szFlag1, szFlag2);
	if (cArgs < 2 || cArgs > 4)
		return false;

	return Init(nfnt, szLabel, szFlag1, szFlag2);
}

bool LabelControl::Init(int nfnt, char *pszLabel, char *pszFlag1, char *pszFlag2)
{
	m_nfnt = nfnt;
	char *pszT = (char *)gpbScratch + (gcbScratch / 2);
	ExpandVars(pszLabel, pszT, kcbLabelTextMax);
	m_szLabel = (char *)gmmgr.AllocPtr(strlen(pszT) + 1);
	gmmgr.WritePtr(m_szLabel, 0, pszT, strlen(pszT) + 1);

	if (strcmp(pszFlag1, "center") == 0)
		m_wf |= kfLblCenterText;
	else if (strcmp(pszFlag1, "multiline") == 0)
		m_wf |= kfLblMultiLine;
	else if (strcmp(pszFlag1, "right") == 0)
		m_wf |= kfLblRightText;

	if (strcmp(pszFlag2, "center") == 0)
		m_wf |= kfLblCenterText;
	else if (strcmp(pszFlag2, "multiline") == 0)
		m_wf |= kfLblMultiLine;
	else if (strcmp(pszFlag2, "right") == 0)
		m_wf |= kfLblRightText;
    else if (strcmp(pszFlag2, "clipvert") == 0)
        m_wf |= kfLblClipVertical;

	CalcRect();
	return true;
}

void LabelControl::OnPaint(DibBitmap *pbm)
{
	if (m_szLabel == NULL)
		return;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	Font *pfnt = gapfnt[m_nfnt];
	if (m_wf & kfLblMultiLine) {
        Font *pfnt = gapfnt[m_nfnt];
        int cyClip = -1;
        if (m_wf & kfLblClipVertical) {
            cyClip = m_rc.Height();
        }
        pfnt->DrawText(pbm, m_szLabel, m_rc.left + rcForm.left,
                m_rc.top + rcForm.top, m_rc.Width(), cyClip,
                (m_wf & kfLblEllipsis) != 0);
	} else {
		DrawFancyText(pbm, pfnt, m_szLabel, m_rc.left + rcForm.left,
                m_rc.top + rcForm.top);
	}
}

int LabelControl::OnHitTest(Event *pevt)
{
    if (m_wf & kfLblHitTest) {
        return Control::OnHitTest(pevt);
    }

	// Assuming that passing Label selected events through to Forms does
	// more harm than good (e.g., Forms have to case out labels) we'll
	// just nip this in the bud.

	return -1;
}

void LabelControl::SetText(const char *psz)
{
	if (psz == NULL) {
		if (m_szLabel != NULL) {
			gmmgr.FreePtr(m_szLabel);
			m_szLabel = NULL;
		}
	} else {
		if (strcmp(psz, m_szLabel) == 0)
			return;
		if (m_szLabel != NULL)
			gmmgr.FreePtr(m_szLabel);

        // Use the tail end of the scratch buffer because ExpandVars
        // (potentially) calls StringTable::GetString which reads from the
        // database, decompressing to the front of the scratch buffer as part
        // of the process.

		char *pszT = (char *)gpbScratch + (gcbScratch / 2);
		ExpandVars((char *)psz, pszT, kcbLabelTextMax);

		m_szLabel = (char *)gmmgr.AllocPtr(strlen(pszT) + 1);
		gmmgr.WritePtr(m_szLabel, 0, pszT, strlen(pszT) + 1);
	}
	CalcRect();
}

void LabelControl::CalcRect()
{
	Rect rcNew = m_rc;
	int cx, cy;

	if (m_szLabel == NULL) {
		cx = cy = 0;
	} else {
		Font *pfnt = gapfnt[m_nfnt];
		if (m_wf & kfLblMultiLine) {
			cx = m_rc.Width();
            if (m_wf & kfLblClipVertical) {
                cy = m_rc.Height();
            } else {
                cy = pfnt->CalcMultilineHeight(m_szLabel, m_rc.Width());
            }
		} else {
			cx = GetFancyTextExtent(pfnt, m_szLabel);
			cy = pfnt->GetHeight();
		}
	}

	if (m_wf & kfLblCenterText) {
		rcNew.left = (m_rc.left + m_rc.Width() / 2) - cx / 2;
	} else if (m_wf & kfLblRightText) {
		rcNew.left = m_rc.right - cx;
	} else {
		rcNew.left = m_rc.left;
	}
	rcNew.right = rcNew.left + cx;
	rcNew.bottom = rcNew.top + cy;
	SetRect(&rcNew, false);
}

// FancyText supports special character sequences to control the formatting 
// of the text as it is displayed. Callers should also note that the default
// font has a number of special symbol characters in it.
// 
// Special characters
// ------------------
// @ - Galaxite icon
// \ - Power (Reactor) icon
//
// Control sequences
// -----------------
// @S. - subsequent characters' side colors are mapped to the local player's side
// @@ - escaped '@'. Use to output an '@'

int FancyTextCore(DibBitmap *pbm, Font *pfntDefault, char *psz, int x, int y, int cch, bool fGetTextExtent) secControl;

int DrawFancyText(DibBitmap *pbm, Font *pfntDefault, char *psz, int x, int y, int cch)
{
	return FancyTextCore(pbm, pfntDefault, psz, x, y, cch, false);
}

int GetFancyTextExtent(Font *pfntDefault, char *psz, int cch)
{
	return FancyTextCore(NULL, pfntDefault, psz, 0, 0, cch, true);
}

int FancyTextCore(DibBitmap *pbm, Font *pfntDefault, char *psz, int x, int y, int cch, bool fGetExtent)
{
	Color *pclr = NULL;
	Font *pfnt = pfntDefault;

	if (cch == 0)
		cch = (int)strlen(psz);

	int cx = 0;

	int cchT = cch;
	char *pchT = psz;
	while (cchT > 0) {
		char ch = *pchT++;
		cchT--;

		if (ch == '@') {
			if (pchT != psz + 1) {
				if (fGetExtent) {
					cx += pfnt->GetTextExtent(psz, (int)(pchT - psz - 1));
				} else {
					cx += pfnt->DrawText(pbm, psz, x, y, (int)(pchT - psz - 1), pclr);
					x += cx;
				}
				psz = pchT;
			}

			switch (*pchT) {
			// "@@" outputs a single @
			case '@':
				cchT--;
				psz = pchT++;
				continue;

			// TODO: "@S." turns on mapping to local player's side color
			case 'S':
				cchT--;
				pchT++;
				break;
			}

			Assert(*pchT == '.');

			// Skip past '.'
			cchT--;
			pchT++;
			psz = pchT;
		}

	}

	if (pchT != psz) {
		if (fGetExtent)
			cx += pfnt->GetTextExtent(psz, (int)(pchT - psz));
		else
			cx += pfnt->DrawText(pbm, psz, x, y, (int)(pchT - psz), pclr);
	}

	return cx;
}

// Bitmap Control

BitmapControl::BitmapControl()
{
	m_ptbm = NULL;
}

BitmapControl::~BitmapControl()
{
	delete m_ptbm;
}

bool BitmapControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) bitmap.png

	char szBitmap[kcbFilename];
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) %s", szBitmap);
	if (cArgs == 0) {
		m_rc.right = m_rc.left;
		m_rc.bottom = m_rc.top;
		return true;
	}

	if (cArgs != 1)
		return false;

    m_ptbm = CreateTBitmap(szBitmap);
    if (!m_ptbm)
		return false;

	Size siz = { 0, 0 };
	m_ptbm->GetSize(&siz);
	m_rc.right = m_rc.left + siz.cx;
	m_rc.bottom = m_rc.top + siz.cy;
	return true;
}

void BitmapControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	// Draw image

	if (m_ptbm != NULL)
		
		// HACK: this "& ~1" is to force word alignment on CE as required by RawBitmap::BltTo
		// Character portrats have some side color in them that must be mapped to blue

		m_ptbm->BltTo(pbm, (m_rc.left + rcForm.left) & ~1, m_rc.top + rcForm.top, kside1);
}

void BitmapControl::SetBitmap(TBitmap *ptbm)
{
	if (m_ptbm != NULL)
		delete m_ptbm;

	m_ptbm = ptbm;

	if (m_ptbm != NULL) {
		Size siz;
		m_ptbm->GetSize(&siz);
		if (m_rc.Width() < siz.cx)
			m_rc.right = m_rc.left + siz.cx;
		if (m_rc.Height() < siz.cy)
			m_rc.bottom = m_rc.top + siz.cy;
	}

	Invalidate();
}

// Edit Control

EditControl::EditControl()
{
	m_nfnt = 0;
	m_szText[0] = 0;
}

bool EditControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) "Text" nfnt

	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) \"%s\" %d",
			m_szText, &m_nfnt);
	if (cArgs != 2)
		return false;

	if (m_rc.Height() == 0)
		m_rc.bottom = m_rc.top + gapfnt[m_nfnt]->GetHeight();

	return true;
}

void EditControl::OnPaint(DibBitmap *pbm)
{
	Font *pfnt = gapfnt[m_nfnt];
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	int x = m_rc.left + rcForm.left;
	int y = m_rc.top + rcForm.top;
	pbm->Fill(x, y + m_rc.Height() - 1, m_rc.Width(), 1, GetColor(kiclrButtonBorder));
	pfnt->DrawText(pbm, m_szText, x + 1, y);
}

void EditControl::SetText(const char *psz)
{
	strncpyz(m_szText, psz, sizeof(m_szText));
	Invalidate();
}

void EditControl::GetText(char *psz, int cb)
{
	strncpyz(psz, m_szText, cb);
}

//
// List Control
//

#define kcxyListBorder gcxyBorder
#define kcxListIntMargin PcFromFc(2)
#define kcyListLineSpace gcxyBorder

TBitmap *ListControl::s_ptbmScrollUpUp;
TBitmap *ListControl::s_ptbmScrollUpDown;
TBitmap *ListControl::s_ptbmScrollDownUp;
TBitmap *ListControl::s_ptbmScrollDownDown;

bool ListControl::InitClass()
{
	s_ptbmScrollUpUp = CreateTBitmap("scrollupup.png");
	s_ptbmScrollUpDown = CreateTBitmap("scrollupdown.png");
	s_ptbmScrollDownUp = CreateTBitmap("scrolldownup.png");
	s_ptbmScrollDownDown = CreateTBitmap("scrolldowndown.png");
	return true;
}

void ListControl::ExitClass()
{
}

ListControl::ListControl()
{
	m_nfnt = 0;
	m_pliFirst = NULL;
	m_pliLast = NULL;
	m_cyItem = 0;
	m_iliTop = 0;
	m_cli = 0;
    memset(m_awfTab, 0, sizeof(m_awfTab));
    memset(m_axTab, 0xff, sizeof(m_axTab));
    m_axTab[0] = 0;
    m_fDrag = false;
    m_fTimerAdded = false;
    m_iclrScrollPos = kiclrMediumGray;
    m_fPenDown = false;
}

ListControl::~ListControl()
{
	// Do this to free any allocated ListItem structures

	Clear();

#if 0 // Don't delete these because the form will do it
	if (m_pbtnUp != NULL)
		delete m_pbtnUp;
	if (m_pbtnDown != NULL)
		delete m_pbtnDown;
#endif

    if (m_fTimerAdded) {
        gtimm.RemoveTimer(this);
    }
}

bool ListControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) "Text" nfnt

	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) %d",
			&m_nfnt);
	if (cArgs != 1)
		return false;

	Font *pfnt = gapfnt[m_nfnt];
	m_cyItem = pfnt->GetHeight() + kcyListLineSpace - pfnt->GetLineOverlap();
    m_cxEllipsis = pfnt->GetTextExtent("...");

#if defined(IPHONE) || defined(__IPHONEOS__) || defined(__ANDROID__)
    m_wf |= kfLstcScrollPosition;
#endif

	return true;
}

void ListControl::GetSubRects(Rect *prcInterior, Rect *prcUpArrow,
        Rect *prcDownArrow, Rect *prcScrollPosition)
{
	Rect rc = m_rc;
	if (m_wf & kfLstcBorder) {
		rc.Inflate(-gcxyBorder * 2, -gcxyBorder * 2);
    }

    if (prcScrollPosition != NULL) {
        prcScrollPosition->SetEmpty();
    }
    if (prcUpArrow != NULL) {
        prcUpArrow->SetEmpty();
    }
    if (prcDownArrow != NULL) {
        prcDownArrow->SetEmpty();
    }

	// If all the elements can be displayed without scrolling than the interior
	// area can fill the whole ListControl

    Size sizArrow;
    s_ptbmScrollUpUp->GetSize(&sizArrow);
	if (rc.Height() / m_cyItem >= m_cli) {
        if (m_wf & kfLstcKeepInteriorPositioning) {
            *prcInterior = rc;
            prcInterior->Inflate(0, -sizArrow.cy);
        } else {
            *prcInterior = rc;
        }
		return;
	}

    if (m_wf & kfLstcScrollPosition) {
        // Maintain interior positioning (with arrows) so that form
        // layout doesn't change if this bit is on.

        prcInterior->Set(rc.left, rc.top + sizArrow.cy,
                rc.right - gcxyBorder * 3, rc.bottom - sizArrow.cy);

        if (prcScrollPosition != NULL) {
            prcScrollPosition->left = rc.right - gcxyBorder * 2;
            prcScrollPosition->top = prcInterior->top;
            prcScrollPosition->right = rc.right;
            prcScrollPosition->bottom = prcInterior->bottom -
                    prcInterior->Height() % m_cyItem;
        }
    } else {
        int xArrow = rc.left + ((rc.Width() - sizArrow.cx) / 2);

        if (prcUpArrow != NULL) {
            prcUpArrow->Set(xArrow - sizArrow.cx, rc.top,
                    xArrow + sizArrow.cx * 2, rc.top + sizArrow.cy);
        }
        if (prcDownArrow != NULL) {
            prcDownArrow->Set(xArrow - sizArrow.cx, rc.bottom - sizArrow.cy,
                    xArrow + sizArrow.cx * 2, rc.bottom);
        }
        if (prcInterior != NULL) {
            prcInterior->Set(rc.left, rc.top + sizArrow.cy, rc.right,
                    rc.bottom - sizArrow.cy);
        }
    }
}

void ListControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	Rect rc;
    Rect rcScrollPos;
	GetSubRects(&rc, NULL, NULL, &rcScrollPos);
	rc.Offset(rcForm.left, rcForm.top);
	rcScrollPos.Offset(rcForm.left, rcForm.top);

	Size sizArrow;
	s_ptbmScrollUpUp->GetSize(&sizArrow);

	int cliDraw = GetVisibleItemCount();
	int cx = rc.Width();
	int x = rc.left;
	int y = rc.top;
	int xArrow = rc.left + ((cx - sizArrow.cx) / 2);

	// Walk through the list to the item that should be displayed first

	ListItem *pli;
	int ili = 0;
	for (pli = m_pliFirst; ili < m_iliTop; pli = pli->pliNext, ili++);

    int cliDrawn = 0;
	for (; pli != NULL && cliDraw > 0;
                pli = pli->pliNext, y += m_cyItem, cliDraw--) {
		DrawItem(pbm, pli, x, y, cx, m_cyItem);
        cliDrawn++;
    }

    bool fNeedsScrollUp = NeedsScrollUpArrow();
    bool fNeedsScrollDown = NeedsScrollDownArrow();

    if (m_wf & kfLstcScrollPosition) {
        if (fNeedsScrollUp || fNeedsScrollDown) {
            pbm->Shadow(rcScrollPos.left, rcScrollPos.top, rcScrollPos.Width(),
                    rcScrollPos.Height());
            pbm->Shadow(rcScrollPos.left, rcScrollPos.top, rcScrollPos.Width(),
                    rcScrollPos.Height());
            int y1 = rcScrollPos.Height() * m_iliTop / m_cli;
            int y2 = rcScrollPos.Height() * (m_iliTop + cliDrawn) / m_cli;
            pbm->Fill(rcScrollPos.left, rcScrollPos.top + y1,
                    rcScrollPos.Width(), y2 - y1,
#if 0
                    m_fPenDown ? GetColor(kiclrWhite) :
                    GetColor(m_iclrScrollPos));
#else
                    GetColor(m_iclrScrollPos));
#endif
        }
    } else {
        if (fNeedsScrollUp) {
            s_ptbmScrollUpUp->BltTo(pbm, xArrow, rc.top - sizArrow.cy);
        }
        if (fNeedsScrollDown) {
            s_ptbmScrollDownUp->BltTo(pbm, xArrow, rc.bottom);
        }
    }

	if (m_wf & kfLstcBorder) {
		Rect rcT;
		rcT = m_rc;
		rcT.Offset(rcForm.left, rcForm.top);
		DrawBorder(pbm, &rcT, 1, GetColor(kiclrListBorder));
	}
}

bool ListControl::NeedsScrollUpArrow()
{
	return m_iliTop != 0;
}

bool ListControl::NeedsScrollDownArrow()
{
	int ciliVisible = GetVisibleItemCount();
	return m_cli > m_iliTop + ciliVisible;
}

void ListControl::OnControlSelected(word idc)
{
	int iliVisible = GetVisibleItemCount();
	if (idc == kidcScrollUpButton) {
		m_iliTop -= iliVisible;
	} else {
		m_iliTop += iliVisible;
	}
	Invalidate();
}

void ListControl::OnControlNotify(word idc, int nNotify)
{
}

bool ListControl::OnControlHeld(word idc)
{
	return false;
}

void ListControl::SetTabStops(int x0, int x1, int x2, int x3)
{
    m_axTab[0] = x0;
    m_axTab[1] = x1;
    m_axTab[2] = x2;
    m_axTab[3] = x3;
}

void ListControl::SetTabFlags(word wf0, word wf1, word wf2, word wf3)
{
    m_awfTab[0] = wf0;
    m_awfTab[1] = wf1;
    m_awfTab[2] = wf2;
    m_awfTab[3] = wf3;
}

void ListControl::DrawItem(DibBitmap *pbm, ListItem *pli, int x, int y,
        int cx, int cy)
{
	if (pli->fSelected) {
		pbm->Fill(x, y, cx, cy, GetColor(kiclrButtonFillHighlight));
    }

    char *pszTabNext = pli->szText - 1;
    int iTab = 0;
    do {
        char *pszAfterTab = pszTabNext + 1;
        pszTabNext = strchr(pszAfterTab, '\t');

        // Figure out what to draw - up to the next tab

        char szT[80];
        char *pszDraw;
        if (pszTabNext == NULL) {
            pszDraw = pszAfterTab;
        } else {
            strncpyz(szT, pszAfterTab,
                    _min((int)sizeof(szT), (int)(pszTabNext - pszAfterTab + 1)));
            pszDraw = szT;
        }

        // Figure out the width of this tab column

        int cxT;
        if (iTab < ARRAYSIZE(m_axTab) - 1 && m_axTab[iTab + 1] >= 0) {
            cxT = m_axTab[iTab + 1] - m_axTab[iTab];
        } else {
            cxT = cx - m_axTab[iTab];
        }

        // Draw
        DrawText(pbm, pszDraw, x + m_axTab[iTab], y, cxT, cy,
                m_awfTab[iTab]);
        iTab++;
    } while (pszTabNext != NULL && iTab < ARRAYSIZE(m_axTab));
}

void ListControl::DrawText(DibBitmap *pbm, char *psz, int x, int y,
        int cx, int cy, word wf)
{
	Font *pfnt = gapfnt[m_nfnt];
    int dx = 0;
    if (wf & kfLstTabCenter) {
        int cxT = pfnt->GetTextExtent(psz);
        if (cxT > cx) {
            Rect rc;
            GetSubRects(&rc);
            int xT = x + (cx - cxT);
            if (xT <= rc.left) {
                dx = rc.left - x;
            } else {
                dx = xT - x;
                wf &= ~kfLstTabEllipsis;
            }
        } else {
            dx = (cx - cxT) / 2;
            wf &= ~kfLstTabEllipsis;
        }
    }
    if (wf & kfLstTabCenterOn) {
        int cxT = pfnt->GetTextExtent(psz);
        dx = -cxT / 2;

    }
    if (wf & kfLstTabRight) {
        int cxT = pfnt->GetTextExtent(psz);
        dx = -cxT;
    }
    x += dx;
    cx -= dx;

    // Make room before the next tab stop for ellipsis

    if (wf & kfLstTabEllipsis) {
        pfnt->DrawTextWithEllipsis(pbm, psz, (int)strlen(psz), x,
                y + kcyListLineSpace, cx);
    } else {
        pfnt->DrawText(pbm, psz, x, y + kcyListLineSpace, cx, cy, false);
    }
}

void ListControl::OnPenEvent(Event *pevt)
{
    switch (pevt->eType) {
    case penDownEvent:
        m_fDrag = false;
        m_yDrag = pevt->y;
        m_iliTopDrag = m_iliTop;
        m_fPenDown = true;
        Invalidate();
        break;

    case penMoveEvent:
        if (!m_fDrag) {
            if (abs(m_yDrag - pevt->y) >= m_cyItem / 2) {
                m_fDrag = true;
            }
        }
        break;

    case penUpEvent:
        m_fPenDown = false;
        Invalidate();
        if (m_fDrag) {
            if (m_flics.Init(1, 1.0f, 0.12f, 33, false)) {
                m_yDragUp = pevt->y;
                gtimm.AddTimer(this, 10);
                m_fTimerAdded = true;
            }
            m_fDrag = false;
            return;
        }
        break;

    default:
        m_fDrag = false;
        break;
    }

    if (!m_fDrag) {
        m_flics.Clear();
        OnPenEvent2(pevt);
        return;
    }

    DragScroll(pevt->y);
}

void ListControl::DragScroll(int y)
{
    int dy = m_yDrag - y;
    int ciScroll = (abs(dy) + m_cyItem / 2) / m_cyItem;
    if (ciScroll == 0) {
        return;
    }
    if (dy < 0) {
        ciScroll = -ciScroll;
    }

    int iliTopNew = m_iliTopDrag + ciScroll;
    if (iliTopNew < 0) {
        iliTopNew = 0;
    }

    int cVisible = GetVisibleItemCount();
    if (iliTopNew > m_cli - cVisible) {
        iliTopNew = m_cli - cVisible;
        if (iliTopNew < 0) {
            iliTopNew = 0;
        }
    }

    if (iliTopNew == m_iliTop) {
        return;
    }
    m_iliTop = iliTopNew;
    Invalidate();

    //Trace("DragScroll: m_yDrag=%d y=%d dy=%d", m_yDrag, y, dy);
}

void ListControl::OnTimer(long tCurrent)
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

void ListControl::OnPenEvent2(Event *pevt)
{
	// UNDONE: track pen inside/outside of arrows and arrow pressed state
    
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

    // Finger hittesting works as follows:
    // 1. Out of the desired hit test rects, find the "closest" one
    // 2. See if the event is within the "finger rect", an expansion of the
    //    regular rect.
    
	Rect rcInterior, rcUpArrow, rcDownArrow;
	GetSubRects(&rcInterior, &rcUpArrow, &rcDownArrow);
	rcInterior.Offset(rcForm.left, rcForm.top);
	rcUpArrow.Offset(rcForm.left, rcForm.top);
	rcDownArrow.Offset(rcForm.left, rcForm.top);

    // Find the closest selectable item
    
	int iliBottom = (m_iliTop + (rcInterior.Height() / m_cyItem)) - 1;
	if (iliBottom >= m_cli)
		iliBottom = m_cli - 1;

    int nDistClosest = 99999;
    int iClosest = -1;
    for (int ili = m_iliTop; ili <= iliBottom; ili++) {
        Rect rc;
        rc.left = rcInterior.left;
        rc.top = rcInterior.top + (ili - m_iliTop) * m_cyItem;
        rc.right = rcInterior.right;
        rc.bottom = rc.top + m_cyItem;
        int nDist = rc.GetDistance(pevt->x, pevt->y);
        if (nDist < nDistClosest) {
            iClosest = ili - m_iliTop;            
            nDistClosest = nDist;
        }
    }
    
    int nDistHit = 0;
    if (pevt->ff & kfEvtFinger) {
        nDistHit = 20; // Need a constant here
    }
   
    // Compare this best distance against the up and down arrows.    
    if (!(m_wf & kfLstcScrollPosition)) {
        int nDistUp = rcUpArrow.GetDistance(pevt->x, pevt->y);    
        int nDistDown = rcDownArrow.GetDistance(pevt->x, pevt->y);    
        if (nDistUp < nDistDown) {
            if (nDistUp < nDistClosest) {
                if (nDistUp <= nDistHit) {
                    if (pevt->eType == penUpEvent && NeedsScrollUpArrow()) {
                        gsndm.PlaySfx(ksfxGuiScrollingListSelectItem);
                        OnControlSelected(kidcScrollUpButton);
                    }
                }
                return;
            }
        } else {
            if (nDistDown < nDistClosest) {
                if (nDistDown <= nDistHit) {
                    if (pevt->eType == penUpEvent && NeedsScrollDownArrow()) {
                        gsndm.PlaySfx(ksfxGuiScrollingListSelectItem);
                        OnControlSelected(kidcScrollDownButton);
                    }
                }
                return;
            }
        }
    }

	if (pevt->eType != penDownEvent) {
		return;
    }

	// Pin selection to min/max entry

    Assert(iClosest >= 0 && iClosest <= iliBottom - m_iliTop);
    if (iClosest < 0)
        return;
    
    int iliSel;
    bool fOnItem;
    if (nDistClosest <= nDistHit) {
        iliSel = m_iliTop + iClosest;
        fOnItem = true;
    } else {
        iliSel = -1;
        fOnItem = false;
    }

	int ili = 0;
	bool fChange = false;
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext, ili++) {
		if (pli->fSelected != (ili == iliSel)) {
			pli->fSelected = (ili == iliSel);
			fChange = true;
		}
	}

	if (fChange) {
		m_pceh->OnControlNotify(m_idc, knNotifySelectionChange);
		gsndm.PlaySfx(ksfxGuiScrollingListSelectItem);
		Invalidate();
	} else {
		if (fOnItem && pevt->eType == penDownEvent)
			m_pceh->OnControlNotify(m_idc, knNotifySelectionTap);
	}
}

void ListControl::Clear()
{
	ListItem *pli = m_pliFirst;
	while (pli != NULL) {
		ListItem *pliDel = pli;
		pli = pli->pliNext;
		delete pliDel;
	}
	m_pliFirst = NULL;
	m_pliLast = NULL;
	m_cli = 0;
	m_iliTop = 0;

	Invalidate();
}

Font *ListControl::GetFont()
{
    return gapfnt[m_nfnt];
}

int ListControl::GetCount()
{
	return m_cli;
}

void ListControl::OnSelect(int nSelect)
{
	Control::OnSelect(nSelect);
}

int ListControl::GetVisibleItemCount()
{
	Rect rc;
	GetSubRects(&rc);
	return rc.Height() / m_cyItem;
}

bool ListControl::Add(ListItem *pli)
{
	pli->fSelected = false;
	pli->pliNext = NULL;

	if (m_pliLast != NULL)
		m_pliLast->pliNext = pli;
	else
		m_pliFirst = pli;
 	m_pliLast = pli;
	m_cli++;

	Invalidate();
	return true;
}

bool ListControl::Add(const char *psz, void *pvData)
{
	ListItem *pli = new ListItem;
	if (pli == NULL)
		return false;

	strncpyz(pli->szText, psz, sizeof(pli->szText));
	pli->pvData = pvData;

	return Add(pli);
}

void ListControl::Select(int iliSelect, bool fOnly, bool fMakeCenter)
{
	ListItem *pli = m_pliFirst;
	bool fChange = false;

	// Single selection only

	if (fOnly) {
		int ili = 0;
		for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext, ili++) {
			if (iliSelect != ili) {
				if (pli->fSelected)
					fChange = true;
				pli->fSelected = false;
			} else {
				if (!pli->fSelected)
					fChange = true;
				pli->fSelected = true;
			}
		}
	}

	int ili = 0;
	pli = m_pliFirst;
	for (; pli != NULL && ili < iliSelect; pli = pli->pliNext, ili++)
		;

	if (ili == iliSelect && pli != NULL) {
		pli->fSelected = true;

		int ciliVisible = GetVisibleItemCount();
		int iliTop;

        if (fMakeCenter) {
            // Almost center
            iliTop = iliSelect - ciliVisible * 2 / 5;
            if (iliTop < 0) {
                iliTop = 0;
            }
        } else {
            // Make sure the selected item is visible.
            int ciliVisible = GetVisibleItemCount();
            iliTop = iliSelect - (iliSelect % ciliVisible);
        }

        // Make sure a full list is shown if selected item is near the end.

        if (iliSelect > m_cli - ciliVisible) {
            iliTop = m_cli - ciliVisible;
            if (iliTop < 0) {
                iliTop = 0;
            }
        }

        // If the selected item can be visible and the list be at
        // the beginning, do that because it feels better.

        if (iliSelect < ciliVisible) {
            iliTop = 0;
        }
            
        m_iliTop = iliTop;
		OnSelect(knSelUpInside);
		m_pceh->OnControlNotify(m_idc, knNotifySelectionChange);
	}

	if (fChange)
		Invalidate();
}

#if 0
// Enumerate the list Items

ListItem *ListControl::EnumItemPtr(Enum *penm)
{
	if (penm->m_pvNext == (void *)kEnmFirst) 
		penm->m_pvNext = (void *)m_pliFirst;
	else if (penm->m_pvNext != NULL)
		penm->m_pvNext = (void *)((ListItem *)penm->m_pvNext)->pliNext;

	return (ListItem *)penm->m_pvNext;
}
#endif

int ListControl::GetSelectedItemIndex()
{
	int i = 0;
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext) {
		if (pli->fSelected)
			return i;
		else
			i++;
	}
	return -1;
}

// Return first selected item's data pointer

void *ListControl::GetSelectedItemData()
{
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext)
		if (pli->fSelected)
			return pli->pvData;

	return NULL;
}

// Return first selected item's ListItem pointer

ListItem *ListControl::GetSelectedItem()
{
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext)
		if (pli->fSelected)
			return pli;

	return NULL;
}

// set same item's data

void ListControl::SetSelectedItemData(void *pvData)
{
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext)
		if (pli->fSelected)
			pli->pvData = pvData;
}

// Return first selected item's text

bool ListControl::GetSelectedItemText(char *psz, int cb)
{
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext) {
		if (pli->fSelected) {
			strncpyz(psz, pli->szText, cb);
			return true;
		}
	}

	return false;
}

// Set first selected item's text

bool ListControl::SetSelectedItemText(const char *psz)
{
	for (ListItem *pli = m_pliFirst; pli != NULL; pli = pli->pliNext) {
		if (pli->fSelected) {
			strncpyz(pli->szText, psz, sizeof(pli->szText));
            Invalidate();
			return true;
		}
	}

	return false;
}

//
// SilkButtonControl
//

void SilkButtonControl::OnSelect(int nSelect)
{
	switch (nSelect) {
	case knSelMoveInside:
	case knSelDownInside:
		gsndm.PlaySfx(ksfxGuiButtonTap);
		break;
	}

	Control::OnSelect(nSelect);
}

//
// SliderControl
//

#define kcxyTrack gcxyBorder
#define kcxySlider PcFromFc(4)

SliderControl::SliderControl()
{
	m_nMin = m_nMax = m_nValue = 0;
}

void SliderControl::OnPaint(DibBitmap *pbm)
{
	int cx = m_rc.Width();
	int cy = m_rc.Height();

	Rect rc;
	m_pfrm->GetRect(&rc);

	rc.left += m_rc.left;
	rc.top += m_rc.top;
	rc.right = rc.left + cx;
	rc.bottom = rc.top + cy;

	// Draw track

	pbm->Fill(rc.left, rc.top + (cy / 2), cx, kcxyTrack, GetColor(kiclrButtonBorder));

	// Draw slider

	int x;
	if (m_nMax - m_nMin <= 0)
		x = 0;
	else
		x = (short)(((m_nValue - m_nMin) * (long)(cx - kcxySlider)) / (m_nMax - m_nMin));
	pbm->Fill(rc.left + x, rc.top, kcxySlider, cy, GetColor(kiclrButtonFill));
}

void SliderControl::OnSelect(int nSelect)
{
	if (nSelect == knSelDownInside)
		gsndm.PlaySfx(ksfxGuiButtonTap);
}

void SliderControl::OnPenEvent(Event *pevt)
{
	if (pevt->eType == penDownEvent || pevt->eType == penMoveEvent) {
		Rect rcForm;
		m_pfrm->GetRect(&rcForm);

		int x = pevt->x + (kcxySlider / 2) - (rcForm.left + m_rc.left);
		
		m_nValue = m_nMin + ((x  * (m_nMax - m_nMin)) / m_rc.Width());
		if (m_nValue < m_nMin)
			m_nValue = m_nMin;
		else if (m_nValue > m_nMax)
			m_nValue = m_nMax;

		Invalidate();

		m_pceh->OnControlSelected(m_idc);
	}
}

void SliderControl::SetRange(int nMin, int nMax)
{
	Assert(nMax >= nMin);

	m_nMin = nMin;
	m_nMax = nMax;
	if (m_nValue < m_nMin)
		m_nValue = m_nMin;
	if (m_nValue > m_nMax)
		m_nValue = m_nMax;

	Invalidate();
}

void SliderControl::SetValue(int n)
{
	if (m_nValue < m_nMin)
		m_nValue = m_nMin;
	else if (m_nValue > m_nMax)
		m_nValue = m_nMax;
	else
		m_nValue = n;

	Invalidate();
}

int SliderControl::GetValue()
{
	return m_nValue;
}

//
// Pip Meter control
//

TBitmap *PipMeterControl::s_ptbmPip;

bool PipMeterControl::InitClass()
{
	s_ptbmPip = CreateTBitmap("pip.png");
	return true;
}

void PipMeterControl::ExitClass()
{
}

PipMeterControl::PipMeterControl()
{
	m_ptbmPip = NULL;
	m_nValue = 0;
}

PipMeterControl::~PipMeterControl()
{
}

bool PipMeterControl::Init(Form *pfrm, word idc, int x, int y, int cx, int cy, 
		char *szPip)
{
	if (!Control::Init(pfrm, idc, x, y, cx, cy))
		return false;

	return Init(szPip);
}

bool PipMeterControl::Init(char *szPip)
{
	if (szPip != NULL)
		m_ptbmPip = CreateTBitmap(szPip);
	else
		m_ptbmPip = s_ptbmPip;

	return true;
}

bool PipMeterControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) [pip.png]

	char szPip[kcbFilename];
	
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) %s", szPip);
	return Init(cArgs == 1 ? szPip : NULL);
}

void PipMeterControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	Rect rcT = m_rc;
	rcT.Offset(rcForm.left, rcForm.top);

	Size sizPip;
	s_ptbmPip->GetSize(&sizPip);
	int cxPips = (rcT.Width() * m_nValue) / 100;
	int cPips = (cxPips + (sizPip.cx / 2)) / sizPip.cx;

	int x = rcT.left;
	for (int i = 0; i < cPips; i++) {
		m_ptbmPip->BltTo(pbm, x, rcT.top);
		x += sizPip.cx;
	}
}

void PipMeterControl::SetValue(int nValue)
{
	// Don't cause unnecessary invalidation

	if (nValue == m_nValue)
		return;

	// for the bar to fit in the rect, nValue must be less than or equal to 100

	Assert(nValue <= 100, "value out of PipMeter range"); 
	m_nValue = nValue;
	Invalidate();
}

//
// Damage Meter control
//

TBitmap *DamageMeterControl::s_ptbmInfantry;
TBitmap *DamageMeterControl::s_ptbmVehicle;
TBitmap *DamageMeterControl::s_ptbmStructure;

bool DamageMeterControl::InitClass()
{
	s_ptbmInfantry = CreateTBitmap("damage_infantry.png");
	s_ptbmVehicle = CreateTBitmap("damage_vehicle.png");
	s_ptbmStructure = CreateTBitmap("damage_structure.png");
	return true;
}

void DamageMeterControl::ExitClass()
{
}

DamageMeterControl::DamageMeterControl()
{
	m_puntc = NULL;
}

DamageMeterControl::~DamageMeterControl()
{
}

#define kfxDamagePerSecMax itofx(16)

void DrawPips(DibBitmap *pbm, TBitmap *ptbm, int *px, int y, int nDamage, int ctFiringRate) secControl;
void DrawPips(DibBitmap *pbm, TBitmap *ptbm, int *px, int y, int nDamage, int ctFiringRate)
{
	if (nDamage == 0)
		return;

	fix fxDamagePerSec = (fix)mulfx(itofx(nDamage), divfx(itofx(100), itofx(ctFiringRate)));
	int cPips = fxtoi(addfx((fix)divfx(8 * (long)fxDamagePerSec, kfxDamagePerSecMax), itofx(1) / itofx(2)));
	if (cPips > 8)
		cPips = 8;

	Size sizPip;
	ptbm->GetSize(&sizPip);

	for (int i = 0; i < cPips; i++) {
		ptbm->BltTo(pbm, *px, y);
		*px += sizPip.cx + 1;
	}
}

void DamageMeterControl::OnPaint(DibBitmap *pbm)
{
	if (m_puntc == NULL)
		return;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	Rect rcT = m_rc;
	rcT.Offset(rcForm.left, rcForm.top);

	int x = rcT.left;

	DrawPips(pbm, s_ptbmInfantry, &x, rcT.top, m_puntc->nInfantryDamage, m_puntc->ctFiringRate);
	DrawPips(pbm, s_ptbmVehicle, &x, rcT.top, m_puntc->nVehicleDamage, m_puntc->ctFiringRate);
	DrawPips(pbm, s_ptbmStructure, &x, rcT.top, m_puntc->nStructureDamage, m_puntc->ctFiringRate);
}

void DamageMeterControl::SetUnitConsts(UnitConsts *puntc)
{
	// Don't cause unnecessary invalidation

	if (m_puntc == puntc)
		return;
	m_puntc = puntc;
	Invalidate();
}

RadioButtonBarControl::RadioButtonBarControl()
{
    m_isel = 0;
    m_ifnt = kifntButton;
    m_cLabels = 0;
    memset(m_apszLabels, 0, sizeof(m_apszLabels));
}

RadioButtonBarControl::~RadioButtonBarControl()
{
    for (int i = 0; i < m_cLabels; i++) {
        delete[] m_apszLabels[i];
    }
}

bool RadioButtonBarControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind)) {
		return false;
    }

    // idc (x y cx cy) "label" ifnt [isel]

    char szLabel[256];
    int ifnt;
    int isel;
	int cArgs = pini->GetPropertyValue(pfind,
            "%*d (%*d %*d %*d %*d) \"%s\" %d %d",
            szLabel, &ifnt, &isel);
    if (cArgs == 2) {
        return Init(szLabel, ifnt, 0);
    }
    if (cArgs == 3) {
        return Init(szLabel, ifnt, isel);
    }

    return false;
}

bool RadioButtonBarControl::Init(const char *pszLabel, int ifnt, int isel)
{
    // Parse pszLabel into string pieces

    const char *pszNext = pszLabel;
    while (true) {
        const char *pszT = strchr(pszNext, '|');
        if (pszT == NULL) {
            AddLabel(pszNext, (int)strlen(pszNext));
            break;
        }
        AddLabel(pszNext, (int)(pszT - pszNext));
        pszNext = pszT + 1;
    }

    m_isel = 0;
    if (isel >= 0 && isel < m_cLabels) {
        m_isel = isel;
    }
    m_ifnt = ifnt;

    // Figure out positioning
    // If cx == 0, then center horizontally

    if (m_cLabels == 0) {
        return true;
    }
    Rect rcInner, rcOuter;
    GetCellRects(0, &rcInner, &rcOuter);
    for (int i = 1; i < m_cLabels; i++) {
        Rect rcInnerT, rcOuterT;
        GetCellRects(i, &rcInnerT, &rcOuterT);
        rcOuter.Union(&rcOuterT);
    }
    if (m_rc.Width() == 0) {
        Rect rcForm;
        m_pfrm->GetRect(&rcForm);
        m_rc.left = (rcForm.Width() - rcOuter.Width()) / 2;
        // m_rc.top stays the same
        m_rc.right = m_rc.left + rcOuter.Width();
        m_rc.bottom = m_rc.top + rcOuter.Height();
    }
    return true;
}

void RadioButtonBarControl::AddLabel(const char *psz, int cch)
{
    if (m_cLabels >= ARRAYSIZE(m_apszLabels)) {
        return;
    }
    char *pszT = new char[cch + 1];
    if (pszT == NULL) {
        return;
    }
    strncpyz(pszT, psz, cch + 1);
    m_apszLabels[m_cLabels] = pszT;
    m_cLabels++;
}

void RadioButtonBarControl::GetCellRects(int icell, Rect *prcInner,
        Rect *prcOuter)
{
    // First step through the cells up to icell, since a starting x has
    // to be calculated.

    int xStart = m_rc.left;
    for (int i = 0; i < icell; i++) {
        Size sizT;
        GetOuterCellSize(i, &sizT);
        xStart += sizT.cx - gcxyBorder;
    }

    Size sizT;
    GetOuterCellSize(icell, &sizT);
    prcOuter->Set(xStart, m_rc.top, xStart + sizT.cx, m_rc.top + sizT.cy);
    *prcInner = *prcOuter;
    prcInner->Inflate(-gcxyBorder, -gcxyBorder);
}

void RadioButtonBarControl::GetOuterCellSize(int icell, Size *psiz)
{
#define kcxLabelGap (gcxyBorder * 4)
#define kcyLabelGap (gcxyBorder * 1)

    // Pad around start and end, and border

    Font *pfnt = gapfnt[m_ifnt];
    psiz->cx = pfnt->GetTextExtent(m_apszLabels[icell]);
    psiz->cx += kcxLabelGap * 2 + gcxyBorder * 2;
    psiz->cy = pfnt->GetHeight();
    psiz->cy += kcyLabelGap * 2 + gcxyBorder * 2;
}

void RadioButtonBarControl::OnPaint(DibBitmap *pbm)
{
    Rect rcForm;
    m_pfrm->GetRect(&rcForm);
    Font *pfnt = gapfnt[m_ifnt];

    for (int icell = 0; icell < m_cLabels; icell++) {
        Rect rcInner, rcOuter;
        GetCellRects(icell, &rcInner, &rcOuter);
        int xText = rcForm.left + rcInner.left + kcxLabelGap;
        int yText = rcForm.top + rcInner.top + kcyLabelGap;

        if (icell < m_cLabels - 1) {
            int cyDescender = gcxyBorder * 2; // hack
            pbm->Fill(rcInner.right + rcForm.left, yText,
                gcxyBorder, pfnt->GetHeight() - cyDescender,
                GetColor(kiclrWhite));
        }

        if (icell == m_isel) {
            pbm->Fill(xText, yText + pfnt->GetHeight(),
                    pfnt->GetTextExtent(m_apszLabels[icell]), 1,
                    GetColor(kiclrWhite));
        }

        pfnt->DrawText(pbm, (char *)m_apszLabels[icell], xText, yText);
    }
}

void RadioButtonBarControl::OnPenEvent(Event *pevt)
{
    // Change selection on down

    if (pevt->eType != penDownEvent) {
        return;
    }

    // Figure out which cell the down was in

    int isel = -1;
    Rect rcForm;
    m_pfrm->GetRect(&rcForm);
    for (int i = 0; i < m_cLabels; i++) {
        Rect rcInner, rcOuter;
        GetCellRects(i, &rcInner, &rcOuter);
        int x = pevt->x;
        int y = rcForm.top + rcInner.top;
        if (rcOuter.PtIn(x, y)) {
            isel = i;
            break;
        }
    }

    // Off the ends?

    Rect rcInner, rcOuter;
    GetCellRects(0, &rcInner, &rcOuter);
    if (pevt->x < rcOuter.left + rcForm.left) {
        isel = 0;
    }
    if (m_cLabels > 0) {
        GetCellRects(m_cLabels - 1, &rcInner, &rcOuter);
        if (pevt->x >= rcOuter.right + rcForm.left) {
            isel = m_cLabels - 1;
        }
    }

    // Any change?

    if (isel == m_isel) {
        return;
    }

    // Notify of change, invalidate to redraw

    m_isel = isel;
    m_pceh->OnControlNotify(m_idc, knNotifySelectionChange);
    gsndm.PlaySfx(ksfxGuiButtonTap);
    Invalidate();
}

int RadioButtonBarControl::GetSelectionIndex()
{
    return m_isel;
}

void RadioButtonBarControl::SetSelectionIndex(int isel)
{
    if (isel < 0 || isel >= m_cLabels) {
        return;
    }
    if (isel == m_isel) {
        return;
    }
    m_isel = isel;
    Invalidate();
}

} // namespace wi
