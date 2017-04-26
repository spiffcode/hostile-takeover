#include "ht.h"

namespace wi {

//
// Cut Scene implementation
//

class CutSceneForm : public ShellForm
{
public:
	CutSceneForm() secCutScene;
	
	// Form overrides

	virtual bool DoModal(const char *pszScene = NULL) secCutScene;
	virtual void OnControlSelected(word idc) secCutScene;
//	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secCutScene;
	virtual bool OnPenEvent(Event *pevt) secCutScene;

	// Timer interface

	virtual void OnTimer(long tCurrent) secCutScene;

private:
	void More() secCutScene;
	void Layout(char *pszBitmap) secCutScene;
	void HideCutSceneBitmap() secCutScene;
	void ShowCutSceneBitmap() secCutScene;

	char *m_pszText;
};

void CutScene(const char *pszText, bool fPauseSimulation)
{
	// Show the cut scene

	CutSceneForm *pfrm = (CutSceneForm *)gpmfrmm->LoadForm(gpiniForms, kidfCutScene, new CutSceneForm());
	if (pfrm != NULL) {
		if (fPauseSimulation)
			gsim.Pause(true);

		pfrm->DoModal(pszText);

		if (fPauseSimulation)
			gsim.Pause(false);
		delete pfrm;
	}
}

CutSceneForm::CutSceneForm()
{
}

#if 0
void CutSceneForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	ShellForm::OnPaintBackground(pbm, pupd);

	EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
	Rect rc;
	pect->GetRect(&rc);
	Font *pfnt = gapfnt[kifntDefault];
	rc.left = 0;
	rc.right = m_rc.Width();
	rc.bottom = rc.top + rc.Height() - (rc.Height() % pfnt->GetHeight());

	FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
}
#endif

bool CutSceneForm::DoModal(const char *pszText)
{
	m_pszText = (char *)pszText;
	More();

	int idc;
	gtimm.AddTimer(this, kctEcomOutputInterval);
	ShellForm::DoModal(&idc, false, false);
	gtimm.RemoveTimer(this);
	return true;
}

void CutSceneForm::HideCutSceneBitmap()
{
	BitmapControl *pbmc = (BitmapControl *)GetControlPtr(kidcBitmap);
	pbmc->Show(false);
	gpmfrmm->DrawFrame(false);
}

void CutSceneForm::ShowCutSceneBitmap()
{
	BitmapControl *pbmc = (BitmapControl *)GetControlPtr(kidcBitmap);
	pbmc->Show(true);
}

void CutSceneForm::More()
{
	ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
	pbtn->Show(false);

	// Spit out as much text as will fit within the EcomControl until
	// we hit a <img> tag at which time we relayout the form based on
	// the new image size and go from there.

	char *pch = m_pszText;
	while (*pch != 0) {
		if (*pch == '<') {

			// If there's text before the <img> tag then display it
			
			if (pch != m_pszText)
				break;

			char szBitmap[kcbFilename];
			int cch;
			// UNDONE: no image
			int cArgs = IniScanf(pch, "<img %s>%+", szBitmap, &cch);
			if (cArgs != 0) {
				if (cArgs == 1) {
					pch += strlen(pch);
                } else {
					pch += cch;
					ShowCutSceneBitmap();
				}

				Layout(szBitmap);
				m_pszText = pch;
			} else {
				// there was not an image, is there a sound?
			int sfxSound;
			int cArgs = IniScanf(pch, "<snd %d>%+", &sfxSound, &cch);
				if (cArgs != 0) {
					if (cArgs == 1)
						pch += strlen(pch);
					else
						pch += cch;
					
					gsndm.PlaySfx(Sfx(ksfxHappyEnding + sfxSound));
					m_pszText = pch;
				}
			}		
		} else {
			pch++;
		}
	}

	EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
	int cch = (int)(pch - m_pszText);
	char *pszT = new char[cch + 1];
	Assert(pszT != NULL, "out of memory!");
	if (pszT != NULL) {
		strncpyz(pszT, m_pszText, cch + 1);

		// How much of the text will fit?

		Font *pfnt = gapfnt[kifntShadow];
		Rect rcT;
		pect->GetRect(&rcT);
		
		int cyFont = pfnt->GetHeight() - pfnt->GetLineOverlap();
		int cLines = rcT.Height() / cyFont;
		char *pchBreak = pszT;
		while (cLines-- > 0 && pchBreak != NULL)
			pfnt->CalcBreak(rcT.Width(), &pchBreak);

		// If it all fits, scan to the end.

		if (pchBreak == NULL) {
			pchBreak = pszT;
			while (*pchBreak != 0)
				pchBreak++;
		}
		*pchBreak = 0;

		pect->SetText(pszT);

		// Start from the break next time around

		m_pszText += pchBreak - pszT;
		delete[] pszT;
	}
	gpmfrmm->DrawFrame(false);
}

void CutSceneForm::Layout(char *pszBitmap)
{
	Rect rcBitmap;
	if (pszBitmap == NULL) {
		rcBitmap.SetEmpty();
	} else {

		// Set the bitmap (which resizes its control)

		BitmapControl *pbmc = (BitmapControl *)GetControlPtr(kidcBitmap);
		pbmc->SetBitmap(CreateTBitmap(pszBitmap));
		pbmc->GetRect(&rcBitmap);
	}

	ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
	Rect rcOK;
	pbtn->GetRect(&rcOK);

	// Resize the Ecom control to take the remaining space above the "More..." button

	EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
	Rect rcT;
	pect->GetRect(&rcT);
	rcT.top = rcBitmap.bottom + PcFromFc(1);
	rcT.bottom = rcOK.top - PcFromFc(1);
	pect->SetRect(&rcT);
}

void CutSceneForm::OnControlSelected(word idc)
{
	if (idc != kidcOk) {
		EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
		pect->ShowAll();
		return;
	}

	if (*m_pszText == 0) {
		EndForm();
	} else {
		More();
	}
}

void CutSceneForm::OnTimer(long tCurrent)
{
	if (m_wf & kfFrmDoModal) {

		// invalidate the space for the next character. if it's done, 
		// make sure the More/Close button shows

		EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
		if (pect->ShowMoreText()) {
			ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
			if (*m_pszText == 0)
				pbtn->SetText("OK");
			else
				pbtn->SetText("More...");

			pbtn->Show(true);
		}
	}
}

bool CutSceneForm::OnPenEvent(Event *pevt)
{
	if (pevt->eType == penDownEvent) {
		for (int n = m_cctl - 1; n >= 0; n--) {
			// Is it on this control?

			Control *pctl = m_apctl[n];
			if (pctl->OnHitTest(pevt) >= 0) {
				return Form::OnPenEvent(pevt);
            }
		}

		// Not on a control

		EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
		pect->ShowAll();
	}
	return Form::OnPenEvent(pevt);
}

} // namespace wi
