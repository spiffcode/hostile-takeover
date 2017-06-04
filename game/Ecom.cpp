#include "ht.h"

namespace wi {

// font colors Jana, Andy, Olstrom, Fox

#define kiaiclrJana 0
#define kiaiclrAndy 1
#define kiaiclrOlstrom 2
#define kiaiclrFox 3

#define kcchPerSec 40		// TUNE: how many characters per second output during typed dialog
#define kctSpeechDelay 50	// TUNE: hsecs before a new speech

#define kfEcomAutoTakedown 1
#define kfEcomMore 2

class EcomForm : public Form, public Timer
{
public:
	EcomForm() secEcom;
	virtual ~EcomForm() secEcom;
	void SetAutoTakedown() {m_wfEcom |= kfEcomAutoTakedown;}
	bool DoModal(char *pszMessage, int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow, Sfx sfxHide = ksfxGuiFormHide) secEcom;

	// Form overrides

    virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf) secEcom;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secEcom;
	virtual void OnControlSelected(word idc) secEcom;
	virtual bool OnPenEvent(Event *pevt) secEcom;

	// Timer Overrides

	virtual void OnTimer(long tCurrent) secEcom;

private:
	void More() secEcom;

	word m_wfEcom;
	char *m_pszText;
	char *m_pszNext;
    TBitmap *m_ptbm;
};

//------------------------------------------------------------------------------

static char *s_aszEmailAddrs[] = {
	"Nobody", "Andy@@ACME", "Jana@@ACME", "Olstrom@@ACME", "Fox@@OMNI", "ACME Security", "OMNI Security", "Anonymous", ""
};
static char *s_aszNames[] = {\
"", "Andy: ", "Jana: ", "Olstrom: ", "Fox: ", "ACME Security: ", "OMNI Security: ", "Anonymous: ", ""
};
static char *s_aszPortraits[] = {
	NULL, "andyportrait.png", "jana.png", "olstrom.png", "fox.png", NULL, NULL, NULL, NULL
};

void Ecom(int nCharFrom, int nCharTo, char *pszMessage, int nBackground, bool fMore)
{
	// Don't show ecoms in recording stress mode

#ifdef STRESS
	if (gfStress)
		return;
#endif

	EcomForm *pfrm = (EcomForm *)gpmfrmm->LoadForm(gpiniForms, nBackground != knSmallLargeTypeLarge ? kidfEcomSmall : kidfEcomLarge, new EcomForm());
	if (pfrm == NULL)
		return;

#if 0
	if (nBackground != knSmallLargeTypeLarge && !fMore)
		pfrm->SetAutoTakedown();
#else
	// Don't special case type large. Some resolutions the large ecom doesn't
	// fill the whole screen, and autotakedown is desireable.
	// Don't special case fMore since that is now automatically handled.

	pfrm->SetAutoTakedown();
#endif

	Size sizPlayfield;
	ggame.GetPlayfieldSize(&sizPlayfield);

	if (nBackground == knSmallLargeTypeSmallTop) {
		Rect rc;
		pfrm->GetRect(&rc);
		Font *pfnt = gapfnt[kifntDefault];
		rc.Offset(0, -rc.top);
		pfrm->SetRect(&rc);
	} else if (nBackground == knSmallLargeTypeSmallBottom) {
		
		// position on bottom of screen. Different on 240 vs 160 screens
		Rect rc;
		pfrm->GetRect(&rc);
		rc.Offset(0, sizPlayfield.cy - rc.Height() - rc.top);
		pfrm->SetRect(&rc);
	}

	// For all ecoms, horizontally center. Usually fills the screen horizontally unless
	// we're in a weird data / screen mode combo.
	// TUNE: May want to vertically adjust as well but I'm not standing on my head (at the moment)

	Rect rcForm;
	pfrm->GetRect(&rcForm);
	rcForm.Offset((sizPlayfield.cx - rcForm.Width()) / 2, 0);
	if ((rcForm.left & 1) != 0)
		rcForm.Offset(-1, 0);
	pfrm->SetRect(&rcForm);

	// Initialize controls

	LabelControl *plbl = (LabelControl *)pfrm->GetControlPtr(kidcFrom);
	plbl->SetText(s_aszEmailAddrs[nCharFrom]);
	plbl = (LabelControl *)pfrm->GetControlPtr(kidcTo);
	plbl->SetText(s_aszEmailAddrs[nCharTo]);

	// The label self-adjusts its height

	if (nBackground == knSmallLargeTypeLarge) {
		BitmapControl *pbmc = (BitmapControl *)pfrm->GetControlPtr(kidcFromBitmap);
		char *pszBitmap = s_aszPortraits[nCharFrom];
		if (pszBitmap != NULL) {
			pbmc->SetBitmap(CreateTBitmap(pszBitmap));
			pbmc->Show(true);
		} else {
			pbmc->Show(false);
		}

		pbmc = (BitmapControl *)pfrm->GetControlPtr(kidcToBitmap);
		pszBitmap = s_aszPortraits[nCharTo];
		if (pszBitmap != NULL) {
			pbmc->SetBitmap(CreateTBitmap(pszBitmap));
			pbmc->Show(true);
		} else {
			pbmc->Show(false);
		}
	}

	pfrm->DoModal(pszMessage);
	delete pfrm;
}

//===========================================================================
// EcomForm implementation

// This is just here so we can be explicit about what section it ends up in
EcomForm::EcomForm()
{
	m_wfEcom = 0;
	m_pszText = NULL;
    m_ptbm = NULL;
}

EcomForm::~EcomForm()
{
	if (m_pszText != NULL)
		delete[] m_pszText;
    delete m_ptbm;
}

bool EcomForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
    m_ptbm = CreateTBitmap(idf == kidfEcomLarge ? (char *)"ecomlargebkgd.png" : (char *)"ecomsmallbkgd.png");
    return Form::Init(pfrmm, pini, idf);
}

bool EcomForm::DoModal(char *pszMessage, int *pnResult, Sfx sfxShow, Sfx sfxHide)
{
	// Expand the text first thing so subsequent operations like calcing the number
	// of lines that will fit on the ecom are based on the expanded text.
	// Use the tail end of the scratch buffer because ExpandVars (potentially) calls
	// StringTable::GetString which reads from the database, decompressing to
	// the front of the scratch buffer as part of the process.

	char *pszT = (char *)gpbScratch + (gcbScratch / 2);
	ExpandVars(pszMessage, pszT, gcbScratch / 2);
	m_pszText = new char[strlen(pszT) + 1];
	if (m_pszText == NULL) {
		pszT = "ECom text too long! Out of memory.";
		m_pszText = new char[strlen(pszT) + 1];
		if (m_pszText == NULL)
			return false;
	}
	strcpy(m_pszText, pszT);
	m_pszNext = m_pszText;

	More();

	gtimm.AddTimer(this, kctEcomOutputInterval);
	bool f = Form::DoModal(pnResult, sfxShow, sfxHide);
	gtimm.RemoveTimer(this);
	return f;
}

void EcomForm::More()
{
	ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
	pbtn->Show(false);

	EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
	char *pszT = new char[strlen(m_pszNext) + 1];
	strcpy(pszT, m_pszNext);

	// How much of the text will fit?

	Font *pfnt = gapfnt[kifntDefault];
	Rect rcT;
	pect->GetRect(&rcT);
	
	int cLines = rcT.Height() / pfnt->GetHeight();
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

	m_pszNext += pchBreak - pszT;
	delete[] pszT;

	bool fMore = (*m_pszNext != 0);
	if (fMore) {
		m_wfEcom |= kfEcomMore;
	} else {
		m_wfEcom &= ~kfEcomMore;
	}

	pbtn->SetText((char *)(fMore ? "More..." : "OK"));
}

bool EcomForm::OnPenEvent(Event *pevt)
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

void EcomForm::OnControlSelected(word idc)
{
	switch (idc) {
	case kidcCancel:
		EndForm(kidcCancel);
		return;

	case kidcMessage:
		{
			EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
			pect->ShowAll();
		}
		break;

	case kidcOk:
		if (*m_pszNext == 0)
			EndForm(kidcOk);
		else
			More();
		return;
	}
}

void EcomForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	BltHelper(pbm, m_ptbm, pupd, m_rc.left, m_rc.top);
}

void EcomForm::OnTimer(long tCurrent)
{
	if (m_wf & kfFrmDoModal) {

		// invalidate the space for the next character. if it's done, 
		// make sure the More/Close button shows

		EcomTextControl *pect = (EcomTextControl *)GetControlPtr(kidcMessage);
		if (pect->ShowMoreText()) {
			ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
			pbtn->Show(true);
			if ((m_wfEcom & (kfEcomMore | kfEcomAutoTakedown)) == kfEcomAutoTakedown)
				m_wf |= kfFrmAutoTakedown;
		}
	}
}

// EcomTextControl

EcomTextControl::EcomTextControl()
{
	m_cchCur = 0;
	m_ctPrevTime = HostGetTickCount();
}

EcomTextControl::~EcomTextControl()
{
}

bool EcomTextControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) "label" nfnt

	char szLabel[256];
	char szFlag1[32];
	szLabel[sizeof(szLabel) - 1] = 0;
	szFlag1[sizeof(szFlag1) - 1] = 0;

	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) \"%s\" %d %s",
			szLabel, &m_nfnt, szFlag1);
	Assert(szLabel[sizeof(szLabel) - 1] == 0);
	Assert(szFlag1[sizeof(szFlag1) - 1] == 0);
	
	if (cArgs < 2 || cArgs > 3)
		return false;

	m_szLabel = (char *)gmmgr.AllocPtr(strlen(szLabel) + 1);
	gmmgr.WritePtr(m_szLabel, 0, szLabel, strlen(szLabel) + 1);

	if (cArgs == 3) {
		if (strcmp(szFlag1, "center") == 0)
			m_wf |= kfLblCenterText;
		else if (strcmp(szFlag1, "right") == 0)
			m_wf |= kfLblRightText;
	}

	// always multiline

	m_wf |= kfLblMultiLine;

	m_cchCur = 0;
	m_aclrEcom[kiaiclrJana] = GetColor(kiclrJana);
	m_aclrEcom[kiaiclrAndy] = GetColor(kiclrAndy);
	m_aclrEcom[kiaiclrOlstrom] = GetColor(kiclrOlstrom);
	m_aclrEcom[kiaiclrFox] = GetColor(kiclrFox);

	return true;
}

bool EcomTextControl::ShowMoreText()
{
	// for now invalidate the whole control. Can we do better?

	// decide how many characters to output based on how much time has
	// gone by (if handheld was slow to paint it can be longer than the timer interval)
	// and paint what we need to to meet our chars per sec. At speech starts
	// we'll delay a specified number of hsecs
	// time is in hundredths of a sec

	if (m_szLabel[m_cchCur] == 0)
		return true;

	long ctCurTime = HostGetTickCount();		
	long ctDeltaTime = ctCurTime - m_ctPrevTime;

	// pause before a new character speaks. Make sure we don't count
	// our pause time as part of our character output time when pause ends.

	if (m_szLabel[m_cchCur] == '@') {
		if (ctDeltaTime < kctSpeechDelay) 
			return false;
		else 
			ctDeltaTime -= kctSpeechDelay;
	}

	// output the needed characters to get our output rate

	int cch = (int)((kcchPerSec * ctDeltaTime)/100);
	if (cch <= 0)
		return false;
	m_ctPrevTime = ctCurTime;

	// make sure we advance at least one character so we don't get stuck on
	// speech starts.

	do {
		m_cchCur++;
		cch--;
	} while ( (m_szLabel[m_cchCur] != 0) && (m_szLabel[m_cchCur] != '@') && cch > 0); 
	Invalidate();

	return m_szLabel[m_cchCur] == 0;
}

void EcomTextControl::ShowAll()
{
	// for now invalidate the whole control. Can we do better?

	m_cchCur = (int)strlen(m_szLabel);
	Invalidate();
}

void EcomTextControl::OnPaint(DibBitmap *pbm)
{
	if (m_szLabel == 0)
		return;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	Font *pfnt = gapfnt[m_nfnt];
	DrawText(pbm, pfnt, m_szLabel, m_rc.left + rcForm.left, m_rc.top + rcForm.top, m_rc.Width(), m_cchCur);
}

int EcomTextControl::OnHitTest(Event *pevt)
{
	// Label Control stubs this out since they're not usually selectable
	
	return Control::OnHitTest(pevt);
}

void EcomTextControl::CalcRect()
{
	// override the label version of this. Our rect stays whatever it is in forms.pp.ini
}

void EcomTextControl::SetText(char *psz)
{
	LabelControl::SetText(psz);
	m_cchCur = 0;
	m_ctPrevTime = HostGetTickCount();
}

//----------------------------------------------------------------

void EcomTextControl::DrawText(DibBitmap *pbm, Font *pfnt, char *psz, int x, int y, int cx, int cchMax)
{
	// this draws the string up to the point of cchMax. The string is broken up into character speeches 
	// seperated by an '@'. Scan forward to the next '@', set the color appropriately,  then 
	// draw that speech with the multiline drawing, repeat until we reach cchMax or null 
	// null is possible because we don't count the @X that signify color changes, but it does.

	char *pszNextSpeech = psz;
	int cchSpeech;
	Color clr = GetColor(kiclrWhite); // default color
	int xStart = x;
	int cxStart = cx;

		while ((*pszNextSpeech != 0) && (cchMax > 0)) {

			if (*pszNextSpeech != '@') {

				// FYI this data file's speeches are formatted wrong. 
				// the should have @[A | J | O | F] at the start of the text line.
				//feel free to continue

				//Assert(false);	
				cchSpeech = 0;
			}else {

			Assert(*pszNextSpeech == '@');
			pszNextSpeech++;
			int iszNames = 0;
			cchSpeech = 0;

			// set the color for a new speech
			switch (*pszNextSpeech) {
			case 'A':
				clr = m_aclrEcom[kiaiclrAndy];
				iszNames = 1;
				break;
			case 'J':
				clr = m_aclrEcom[kiaiclrJana];
				iszNames = 2;
				break;
			case 'O':
				clr = m_aclrEcom[kiaiclrOlstrom];
				iszNames = 3;
				break;
			case 'F':
				clr = m_aclrEcom[kiaiclrFox];
				iszNames = 4;
				break;
			default:
				Assert(false);
			}
			pszNextSpeech++;

			// we don't count the @format characters in the length - so adjust cchMax.
			cchMax -= 2;

			// output the character's Name:
			// seems wonky to use both dwWhite and dwiscColor but palm compiler
			// messes it up if I don't use the intermediate variable

			pfnt->DrawText(pbm, s_aszNames[iszNames], x, y, -1, &clr);
			int cxName = pfnt->GetTextExtent(s_aszNames[iszNames]);
			x += cxName;
			cx -= cxName;

			}//endif

		char *pszNextLine = pszNextSpeech;

		// point at the next speech and count length of this one

		while ((*pszNextSpeech != '@') && (*pszNextSpeech != 0)){
			cchSpeech++;
			pszNextSpeech++;
		}

		if (cchSpeech > cchMax)
			cchSpeech = cchMax;
		cchMax -= cchSpeech;

		// output this speech in a multiline way

		while (cchSpeech > 0 && pszNextLine != NULL) {
			Assert(pszNextLine != 0);
			char *pszStart = pszNextLine;
			int cch = pfnt->CalcBreak(cx, &pszNextLine);
			if (cch >= cchSpeech) 
				cch = cchSpeech;
			cchSpeech -= cch;
			int iret = pfnt->DrawText(pbm, pszStart, x, y, cch, &clr);

			// cch does not include the whitespace char being used
			// to break the line!

			cchSpeech--;
			y += pfnt->GetHeight() - pfnt->GetLineOverlap();
			x = xStart;
			cx = cxStart;
		}
	}
}

} // namespace wi
