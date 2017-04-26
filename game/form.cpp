#include "ht.h"

namespace wi {

//
// Form
//

Form::Form()
{
	m_iclrBack = kiclrFormBackground;
	m_pctlCapture = NULL;
	m_pfrmm = NULL;
	m_ptbm = NULL;
	m_wf = 0;
	m_idf = 0;
	m_idcDefault = 0;
	m_idcLast = 0;
	m_nResult = 0;
	m_cctl = 0;
	memset(m_apctl, 0, sizeof(m_apctl));
	m_rc.SetEmpty();
	m_pUserData = NULL;
}

Form::~Form()
{
	// Delete controls

	for (int n = 0; n < m_cctl; n++)
		delete m_apctl[n];

	// Delete TBitmap

	delete m_ptbm;

	// Remove this form
	
	if (m_pfrmm != NULL)
		m_pfrmm->RemoveForm(this);

	// Mark deleted for debugging purposes

	m_wf |= kfFrmDeleted;
}

bool Form::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	// Initialize form properties

	char szForm[10];
	itoa(idf, szForm, 10);
	if (!InitFromProperties(pfrmm, idf, pini, szForm))
		return false;

	// Step through and create controls

	char szProp[32];
	FindProp find;
	while (pini->FindNextProperty(&find, szForm, szProp, sizeof(szProp))) {
		// Instantiate and initialize control

		Control *pctl = NULL;
		if (strcmp(szProp, "BUTTON") == 0) {
			pctl = new ButtonControl;
		}
        else if (strcmp(szProp, "RADIOBUTTONBAR") == 0) {
            pctl = new RadioButtonBarControl;
        }
		else if (strcmp(szProp, "LABEL") == 0) {
			pctl = new LabelControl;
		}
		else if (strcmp(szProp, "ECOMTEXT") == 0) {
			pctl = new EcomTextControl;
		}
		else if (strcmp(szProp, "CHECKBOX") == 0) {
			pctl = new CheckBoxControl;
		}
		else if (strcmp(szProp, "BITMAP") == 0) {
			pctl = new BitmapControl;
		}
#if 0 // not used now
		else if (strcmp(szProp, "PRESETBUTTON") == 0) {
			pctl = new PresetButtonControl;
		}
#endif
		else if (strcmp(szProp, "EDIT") == 0) {
			pctl = new EditControl;
		}
		else if (strcmp(szProp, "LIST") == 0) {
			pctl = new ListControl;
		}
		else if (strcmp(szProp, "ANIMLIST") == 0) {
			pctl = new BuildListControl;
		}
		else if (strcmp(szProp, "GRAFFITISCROLL") == 0) {
			pctl = new GraffitiScrollControl;
		}
		else if (strcmp(szProp, "SILKBUTTON") == 0) {
			pctl = new SilkButtonControl;
		}
		else if (strcmp(szProp, "SLIDER") == 0) {
			pctl = new SliderControl;
		}
		else if (strcmp(szProp, "MINIMAP") == 0) {
			pctl = new MiniMapControl;
		}
		else if (strcmp(szProp, "PIPMETER") == 0) {
			pctl = new PipMeterControl;
		}
		else if (strcmp(szProp, "DAMAGEMETER") == 0) {
			pctl = new DamageMeterControl;
		}
		else if (strcmp(szProp, "HELP") == 0) {
			pctl = new HelpControl;
		}
		else if (strcmp(szProp, "ALERT") == 0) {
			pctl = new AlertControl;
		}
		else if (strcmp(szProp, "CREDITS") == 0) {
			pctl = new CreditsControl;
		}
		else if (strcmp(szProp, "POWER") == 0) {
			pctl = new PowerControl;
		}
		else {
			continue;
		}
		Assert(pctl != NULL, "out of memory!");
		if (pctl == NULL)
			return false;
		if (!pctl->Init(this, pini, &find))
			return false;

		// Add it to the list for this form

		Assert(m_cctl < kcControlsMax);
		m_apctl[m_cctl++] = pctl;
	}

	// Add the form to the form mgr

	m_pfrmm->AddForm(this);

	// Show the form. This is a asynchronous event, which'll give time
	// to the derived form to initialize.

	Show(true);
	return true;
}

bool Form::InitFromProperties(FormMgr *pfrmm, word idf, IniReader *pini, char *pszForm)
{
	// FORM=(x y cx cy) idcDefault

	int x, y, cx, cy;
	char szBitmap[kcbFilename];
	int idcDefault;
	char szArgs[3][32];
	int cArgs = pini->GetPropertyValue(pszForm, "FORM", "(%d %d %d %d) %d %s %s %s",
			&x, &y, &cx, &cy, &idcDefault, szArgs[0], szArgs[1], szArgs[2]);
	if (cArgs < 5)
		return false;
	m_idcDefault = (word)idcDefault;

	// Scale form coordinates depending on the resolution of the device.

	bool fCenter = false;
	bool fScale = true;
	bool fTopMost = false;

	int csz = _min((int)ARRAYSIZE(szArgs), cArgs - 5);
	for (int n = 0; n < csz; n++) {
		if (strcmp(szArgs[n], "noscale") == 0) {
			fScale = false;
			continue;
		}
		if (strcmp(szArgs[n], "center") == 0) {
			fCenter = true;
			continue;
		}
		if (strcmp(szArgs[n], "topmost") == 0) {
			fTopMost = true;
			continue;
		}
	}

	if (fScale) {
		m_wf |= kfFrmScaleCoords;
		x = PcFromFc(x);
		y = PcFromFc(y);
		cx = PcFromFc(cx);
		cy = PcFromFc(cy);
	}

	if (fCenter) {
		DibBitmap *pbm = pfrmm->GetDib();
		Size siz;
		pbm->GetSize(&siz);
		x = ((siz.cx - cx) / 2) & ~1;
		y = (siz.cy - cy) / 2;
	}

	if (fTopMost)
		m_wf |= kfFrmTopMost;

	cArgs = pini->GetPropertyValue(pszForm, "FORMBITMAP", "%s", szBitmap);
	if (cArgs == 0)
		szBitmap[0] = 0;
	cArgs = pini->GetPropertyValue(pszForm, "FORMBACKCOLOR", "%d", &m_iclrBack);
	if (cArgs == 0)
		m_iclrBack = -1;

	// Fix the size of the form to the bitmap width and height

	m_ptbm = NULL;
	m_rc.Set(x, y, x + cx, y + cy);
	if (szBitmap[0] != 0) {
		m_ptbm = CreateTBitmap(szBitmap);
		if (m_ptbm == NULL)
			return false;
		Size siz;
		m_ptbm->GetSize(&siz);
		m_rc.Set(x, y, x + siz.cx, y + siz.cy);
	}
	m_pfrmm = pfrmm;
	m_idf = idf;

	return true;
}

bool Form::AddControl(Control *pctl)
{
	// Add it to the list for this form

	// bottommost

	Assert(m_cctl < kcControlsMax);
	m_apctl[m_cctl++] = pctl;
	return true;
}

void Form::SetUserDataPtr(void *pUserData)
{
	m_pUserData = pUserData;
}

void *Form::GetUserDataPtr()
{
	return m_pUserData;
}

word Form::GetId()
{
	return m_idf;
}

word Form::GetFlags()
{
	return m_wf;
}

void Form::SetFlags(word wf)
{
	m_wf = wf;
}

void Form::Show(bool fShow)
{
	if (fShow) {
		if (m_wf & kfFrmVisible)
			return;
		m_wf |= kfFrmVisible;
		InvalidateRect(NULL);
	} else {
		if (!(m_wf & kfFrmVisible))
			return;
		InvalidateRect(NULL);
		m_wf &= ~kfFrmVisible;
	}
}

void Form::OnUpdateMapInvalidate(UpdateMap *pupd, Rect *prcOpaque)
{
	for (int n = 0; n < m_cctl; n++) {
		if (!(m_apctl[n]->m_wf & kfCtlVisible))
			continue;

		Rect rcT;
		rcT = m_apctl[n]->m_rc;
		rcT.Offset(m_rc.left, m_rc.top);

		// Only mark the control invalid

		if (!prcOpaque->RectIn(&rcT)) {
			if (pupd->IsRectInvalidAndTrackDamage(&rcT))
				m_apctl[n]->SetFlags(m_apctl[n]->GetFlags() | kfCtlRedraw);
		}
	}
}

void Form::FrameStart()
{
}

void Form::FrameComplete()
{
}
    
void Form::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	// Draw invalid parts of the background

	if (m_iclrBack == -1)
		return;
	FillHelper(pbm, pupd, &m_rc, GetColor(m_iclrBack));
}

void Form::OnPaint(DibBitmap *pbm)
{
	if (m_ptbm != NULL)
		m_ptbm->BltTo(pbm, m_rc.left, m_rc.top);
}

void Form::OnPaintControls(DibBitmap *pbm, UpdateMap *pupd)
{
	HostSoundServiceProc();
	int nServiceSfx = 0;
	for (int n = 0; n < m_cctl; n++) {
		if ((m_apctl[n]->m_wf & (kfCtlVisible | kfCtlRedraw)) != (kfCtlVisible | kfCtlRedraw))
			continue;
		nServiceSfx++;
		if ((nServiceSfx & 3) == 0)
			HostSoundServiceProc();
		m_apctl[n]->OnPaint(pbm);
		m_apctl[n]->SetFlags(m_apctl[n]->GetFlags() & ~kfCtlRedraw);
	}
}

void Form::ScrollInvalidate(UpdateMap *pupd)
{
	// Invalidate the children due to scrolling

	for (int n = 0; n < m_cctl; n++)
		m_apctl[n]->Invalidate();
}

// for override
void Form::OnScroll(int dx, int dy)
{
}

bool Form::EventProc(Event *pevt)
{
	switch (pevt->eType) {
	case penHoverEvent:
	case penDownEvent:
	case penMoveEvent:
	case penUpEvent:
	case penHoldEvent:
    case penDownEvent2:
    case penMoveEvent2:
    case penUpEvent2:            
		return OnPenEvent(pevt);
	}
    if (pevt->eType == keyDownEvent) {
		switch (pevt->chr) {
		case vchrBack:
            OnControlSelected(kidcCancel);
        }
    }
	return false;
}

Control *Form::GetControlPtr(word idc)
{
	for (int n = 0; n < m_cctl; n++) {
		if (m_apctl[n]->GetId() == idc)
			return m_apctl[n];
	}
	return NULL;
}

bool Form::OnPenEvent(Event *pevt)
{
	// Auto-handle control capturing. Auto-release, auto-capture.
	// Notify the control of important states:
	// - penDownEvent and captured
	// - penMoveEvent from inside to outside of captured control
	// - penMoveEvent from outside to inside of captured control	
	// - penUpEvent inside control releasing capture
	// - penUpEvent outside control releasing capture

	if (m_pctlCapture != NULL) {
		// Send on pen event to the captured control

		m_pctlCapture->OnPenEvent(pevt);
		word wf = m_pctlCapture->OnHitTest(pevt) >= 0 ? kfFrmPenInside : 0;

		// Handle the transition states

		Control *pctl = m_pctlCapture;
		switch (pevt->eType) {
		case penUpEvent:
			// Release capture. Are we inside or outside of the control?

			m_idcLast = pctl->GetId();
			m_wf &= ~kfFrmPenInside;
			m_pctlCapture = NULL;
			pctl->OnSelect(((wf & kfFrmPenInside) && pevt->dw == 0) ? knSelUpInside : knSelUpOutside);
			break;

		case penMoveEvent:
			if ((m_wf & kfFrmPenInside) != wf) {
				m_wf &= ~kfFrmPenInside;
				m_wf |= wf;
				pctl->OnSelect((wf & kfFrmPenInside) ? knSelMoveInside : knSelMoveOutside);
			}
			break;

        // This does happen. For example if a modal form is invoked in response
        // to a penHoldEvent it will take the penUpEvent. Also happens on
        // Windows a lot when debugging if something happens to invoke the
        // debugger (e.g., fault, breakpoint) while the mousebutton is down.

		case penDownEvent:
			//Assert("this shouldn't happen");
			break;

		case penHoldEvent:
			pctl->OnSelect(knSelHoldInside);
			break;

        // These events don't affect capture once it has already been set.
        case penDownEvent2:
        case penUpEvent2:
        case penMoveEvent2:
            break;
		}
		return true;
	}

	// If pen isn't captured and this is a pen down on a control,
	// capture future pen events to the control and let it know it
	// has been pressed.

	if (pevt->eType != penDownEvent)
		return false;

    // Auto-capture if we've hit a control

    Control *pctlHit = HitTestControls(pevt);
    if (pctlHit != NULL) {
        m_wf |= kfFrmPenInside;
        m_pctlCapture = pctlHit;
        m_pctlCapture->OnPenEvent(pevt);
        m_pctlCapture->OnSelect(knSelDownInside);
        return true;
    }

	return false;
}

Control *Form::HitTestControls(Event *pevt) {
    // Finger input may register hits on more than one control at a time.
    // In this case, use the "closest" control, measured by hit distance.
    // Pen input will simply register on the first control registering a hit.

    int nControlBest = -1;
    int nDistanceBest = 9999;
	for (int n = m_cctl - 1; n >= 0; n--) {
		// Is it on this control?

		Control *pctl = m_apctl[n];
        int nDistance = pctl->OnHitTest(pevt);
		if (nDistance < 0) {
			continue;
        }

        // Remember closest

        if (nDistance < nDistanceBest) {
            nDistanceBest = nDistance;
            nControlBest = n;
        }

        // No need to enumerate all controls if not finger input

        if (!(pevt->ff & kfEvtFinger)) {
            break;
        }
    }
    if (nControlBest >= 0) {
        return m_apctl[nControlBest];
    }
    return NULL;
}

void Form::BreakCapture()
{
    if (m_pctlCapture != NULL) {
        m_pctlCapture->OnBreakCapture();
        m_pctlCapture = NULL;
        m_wf &= ~kfFrmPenInside;
    }
}

bool Form::OnHitTest(Event *pevt)
{
	if (!(m_wf & kfFrmVisible))
		return false;


    // If this is a finger event, Check first if any control reports hittest.
    // This allows hit testing outside the normal rect of the form.

    bool fHit = false;
    if (pevt->ff & kfEvtFinger) {
        for (int n = 0; n < m_cctl; n++) {
            Control *pctl = m_apctl[n];
            if (pctl->OnHitTest(pevt) >= 0) {
                fHit = true;
                break;
            }
        }
    }

    if (!fHit) {
        fHit = m_rc.PtIn(pevt->x, pevt->y);
    }

	if (fHit || !(m_wf & kfFrmAutoTakedown))
		return fHit;

	// We're AutoTakedown and hit is outside form
	// We can receive pen ups and holds here if the pen was already
	// down and a trigger invokes than ecom.

	if (pevt->eType == penDownEvent)
		EndForm(kidcCancel);

	return false;
}

bool Form::OnKeyTest(Event *pevt)
{
	// If pevt == NULL we're just checking; no actual event

	if (pevt == NULL) {
		if (m_wf & kfFrmNoFocus)
			return false;
		return true;
	}

	if (m_wf & kfFrmAutoTakedown) {
		EndForm(kidcCancel);
		return false;
	}

	if (m_wf & kfFrmNoFocus)
		return false;
	return true;
}

bool Form::DoModal(int *pnResult, Sfx sfxShow, Sfx sfxHide)
{
#ifdef DEBUG
	bool fFirstTimeThrough = true;
#endif

	Show(true);
	gsndm.PlaySfx(sfxShow);
	m_wf |= kfFrmDoModal;
	while (m_wf & kfFrmDoModal) {
		// Only process if there is an event

		Event evt;
		if (!gevm.PeekEvent(&evt, -1))
			continue;

		// Leave appStopEvent and gameOverEvents in the queue so the whole 
		// call chain will see it and be able to respond appropriately.

		switch (evt.eType) {
		case appStopEvent:
		case gameOverEvent:
		case cancelModeEvent:
			m_idcLast = m_idcDefault;
			m_nResult = m_idcDefault;
			m_wf &= ~kfFrmDoModal;
			break;
		}

		// Autotakedown is handled inside PeekEvent/CookEvent/Form::OnHitTest
		// and can take us out of modal mode. If so, drop out here so the
		// event is left on the queue.

		if (!(m_wf & kfFrmDoModal))
			break;

#ifdef DEBUG
		fFirstTimeThrough = false;
#endif
		if (!gevm.GetEvent(&evt))
			continue;
		if (ggame.FilterEvent(&evt))
			continue;
        if (OnFilterEvent(&evt))
            continue;
		gevm.DispatchEvent(&evt);
	}

	if (pnResult != NULL)
		*pnResult = m_nResult;
	gsndm.PlaySfx(sfxHide);
	Show(false);
	return m_idcLast == kidcOk;
}

void Form::EndForm(int nResult)
{
	m_nResult = nResult;
	m_wf &= ~kfFrmDoModal;
}

void Form::GetRect(Rect *prc)
{
	*prc = m_rc;
}

void Form::SetRect(Rect *prc)
{
	InvalidateRect(NULL);
	m_rc = *prc;
	InvalidateRect(NULL);
}

bool Form::IsControlInside(Control *pctl)
{
	if (m_pctlCapture != NULL && (m_wf & kfFrmPenInside))
		return m_pctlCapture == pctl;
	return false;
}

void Form::OnControlSelected(word idc)
{
	EndForm(idc);
}

bool Form::OnControlHeld(word idc) 
{
	return false;
}

void Form::OnControlNotify(word idc, int nNotify)
{
}

void Form::InvalidateRect(Rect *prc)
{
	// If invisible, return

	if (!(m_wf & kfFrmVisible))
		return;
	if (prc == NULL)
		prc = &m_rc;

	// If not opaqued, then invalidate

	Rect rcOpaque;
	gpmfrmm->CalcOpaqueRect(this, NULL, &rcOpaque);
	Rect rcT;
	gpmfrmm->GetFormMgrRect(m_pfrmm, &rcT);
	rcOpaque.Intersect(&rcOpaque, &rcT);
	rcOpaque.Offset(-rcT.left, -rcT.top);
	if (!rcOpaque.RectIn(prc))
		m_pfrmm->InvalidateRect(prc);
}

void ShadowHelper(DibBitmap *pbm, UpdateMap *pupd, Rect *prc)
{
	Rect rc;
	if (prc == NULL) {
		Size siz;
		pbm->GetSize(&siz);
		rc.Set(0, 0, siz.cx, siz.cy);
		prc = &rc;
	}

	if (pupd == NULL) {
		pbm->Shadow(prc->left, prc->top, prc->Width(), prc->Height());
	} else {
		Rect rcInvalid;
		bool fFirst = true;
		while (pupd->EnumUpdateRects(fFirst, prc, &rcInvalid)) {
			fFirst = false;
			pbm->Shadow(rcInvalid.left, rcInvalid.top, rcInvalid.Width(), rcInvalid.Height());
		}
	}
}

void FillHelper(DibBitmap *pbm, UpdateMap *pupd, Rect *prc, Color clr)
{
	Rect rc;
	if (prc == NULL) {
		Size siz;
		pbm->GetSize(&siz);
		rc.Set(0, 0, siz.cx, siz.cy);
		prc = &rc;
	}

	if (pupd == NULL) {
		pbm->Fill(prc->left, prc->top, prc->Width(), prc->Height(), clr);
	} else {
		Rect rcInvalid;
		bool fFirst = true;
		while (pupd->EnumUpdateRects(fFirst, prc, &rcInvalid)) {
			fFirst = false;
			pbm->Fill(rcInvalid.left, rcInvalid.top, rcInvalid.Width(), rcInvalid.Height(), clr);
		}
	}
}

void BltHelper(DibBitmap *pbm, TBitmap *ptbm, UpdateMap *pupd, int xDst, int yDst)
{
	if (pupd == NULL) {
		ptbm->BltTo(pbm, xDst, yDst);
	} else {
		Size siz;
		ptbm->GetSize(&siz);
		Rect rc;
		rc.Set(xDst, yDst, xDst + siz.cx, yDst + siz.cy);
		Rect rcInvalid;
		bool fFirst = true;
		while (pupd->EnumUpdateRects(fFirst, &rc, &rcInvalid)) {
			fFirst = false;
			Rect rcSrc;
			rcSrc.left = rcInvalid.left - xDst;
			rcSrc.top = rcInvalid.top - yDst;
			rcSrc.right = rcSrc.left + rcInvalid.Width();
			rcSrc.bottom = rcSrc.top + rcInvalid.Height();
			ptbm->BltTo(pbm, rcInvalid.left, rcInvalid.top, &rcSrc);
		}
	}
}

//
// DialogForm
//

DialogForm::DialogForm()
{
	m_clrTitle = GetColor(kiclrGreen);
	m_iclrBorder = kiclrBlack;
	m_fClearDib = false;
	SetBackgroundColorIndex(kiclrShadow);
}

// The compiler generates this no matter what so by declaring it ourselves
// we can control which section it ends up in (not .text)

DialogForm::~DialogForm()
{
}

void DialogForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	// First clear dib if asked

	if (m_fClearDib)
		FillHelper(pbm, pupd, NULL, GetColor(m_iclrBackground));

	// Draw invalid parts of the background

	int cyTitle = PcFromFc(kcyTitle);
	Rect rc = m_rc;
	rc.left += gcxyBorder;
	rc.right -= gcxyBorder;
	rc.top += cyTitle;
	rc.bottom -= gcxyBorder;

	switch (m_iclrBackground) {
	case kiclrShadow2x:
		ShadowHelper(pbm, pupd, &rc);
		// ...FALL THROUGH...

	case kiclrShadow:
		ShadowHelper(pbm, pupd, &rc);
		break;

	default:
		FillHelper(pbm, pupd, &rc, GetColor(m_iclrBackground));
		break;
	}

	// Draw form border

	DrawBorder(pbm, &m_rc, gcxyBorder, GetColor(m_iclrBorder), pupd);

	// Draw title background

	rc = m_rc;
	rc.left += gcxyBorder;
	rc.right -= gcxyBorder;
	rc.top += gcxyBorder;
	rc.bottom = m_rc.top + cyTitle;

	FillHelper(pbm, pupd, &rc, m_clrTitle);
}

void DialogForm::SetBackgroundColorIndex(int iclr)
{
	m_iclrBackground = iclr;
	if (m_iclrBackground == kiclrShadow || m_iclrBackground == kiclrShadow2x) {
		m_wf |= kfFrmTranslucent;
	} else {
		m_wf &= ~kfFrmTranslucent;
	}
}

void DialogForm::OnPaint(DibBitmap *pbm)
{
	Form::OnPaint(pbm);
}

bool DialogForm::OnPenEvent(Event *pevt)
{
	if (Form::OnPenEvent(pevt))
		return true;

	return FormDragger(this, pevt);
}

bool DialogForm::DoModal(int *pnResult, Sfx sfxShow, Sfx sfxHide)
{
    bool fResult = Form::DoModal(pnResult, sfxShow, sfxHide);
    if (m_fClearDib) {
        m_pfrmm->InvalidateRect(NULL);
    }
    return fResult;
}

bool FormDragger(Form *pfrm, Event *pevt)
{
	static bool s_fDragging = false;
	static Rect s_rcInitial;
	static int s_xDown, s_yDown;

	if (pevt->eType == penDownEvent) {

		Rect rcForm;
		pfrm->GetRect(&rcForm);

		// Is event in title? If so, capture and save drag start info

		if (pevt->y - rcForm.top <= PcFromFc(kcyTitle) + gcxyBorder) {
			s_fDragging = true;
			s_rcInitial = rcForm;
			s_xDown = pevt->x;
			s_yDown = pevt->y;
			return true;
		}
		return false;
	} else if (pevt->eType == penMoveEvent) {

		// Are we dragging? If so, reposition form at new location

		if (s_fDragging) {
			Rect rc;
			rc.left = s_rcInitial.left + pevt->x - s_xDown;
			rc.top = s_rcInitial.top + pevt->y - s_yDown;
			rc.right = rc.left + s_rcInitial.Width();
			rc.bottom = rc.top + s_rcInitial.Height();
			pfrm->SetRect(&rc);
			return true;
		}
	} else if (pevt->eType == penUpEvent) {

		// Are we dragging? If so, release capture

		if (s_fDragging) {
			s_fDragging = false;
			return true;
		}
	}

	return false;
}

//
// Control
//

Control::Control()
{
	m_rc.SetEmpty();
	m_wf = 0;
	m_idc = 0;
	m_pfrm = NULL;
	m_pceh = NULL;
}

Control::~Control()
{
}

bool Control::Init(Form *pfrm, word idc, int x, int y, int cx, int cy)
{
	m_idc = idc;
	m_pfrm = pfrm;
	m_pceh = pfrm;
	m_wf |= kfCtlVisible;

	m_rc.Set(x, y, x + cx, y + cy);
	return true;
}

bool Control::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// idc (x y cx cy)
	
	int x, y, cx, cy;
	int idc;
	int cArgs = pini->GetPropertyValue(pfind, "%d (%d %d %d %d)", &idc,
			&x, &y, &cx, &cy);
	if (cArgs != 5)
		return false;

	// Scale form coordinates depending on the resolution of the device.

	if (pfrm->GetFlags() & kfFrmScaleCoords) {
		x = PcFromFc(x);
		y = PcFromFc(y);
		cx = PcFromFc(cx);
		cy = PcFromFc(cy);
	}
	m_rc.Set(x, y, x + cx, y + cy);
	return Init(pfrm, idc, x, y, cx, cy);
}

void Control::OnSelect(int nSelect)
{
	if (m_wf & kfCtlDisabled)
		return;

	if (nSelect == knSelUpInside)
		m_pceh->OnControlSelected(m_idc);
	else if (nSelect == knSelHoldInside) {
		if (m_pceh->OnControlHeld(m_idc)) {
//			m_pctlCapture = NULL;
//			m_wf &= ~kfFrmPenInside;
//			OnSelect(knSelUpOutside);
		}
	}
}

int Control::OnHitTest(Event *pevt)
{
	if (!(m_wf & kfCtlVisible))
		return -1;

    // Get the distance to a side of the control

    Rect rcForm;
    m_pfrm->GetRect(&rcForm);
    int nDist = m_rc.GetDistance(pevt->x - rcForm.left, pevt->y - rcForm.top);

    // Finger input hits on a larger rect and returns the distance to
    // the inner rect. Non-finger input just hits against the inner rect.

    if (pevt->ff & kfEvtFinger) {
        Rect rcT;
        GetFingerRect(&rcT);
        if (rcT.PtIn(pevt->x - rcForm.left, pevt->y - rcForm.top)) {
            return nDist;
        }
    } else {
        if (nDist == 0) {
            return 0;
        }
    }

    // No hit
        
    return -1;
}

word Control::GetId()
{
	return m_idc;
}

void Control::GetRect(Rect *prc)
{
	*prc = m_rc;
}

void Control::GetFingerRect(Rect *prc)
{
	*prc = m_rc;

    // Hack for now
    prc->Inflate(20, 20);
}

void Control::OnBreakCapture()
{
    Invalidate();
}

void Control::SetRect(Rect *prc, bool fCompareRect)
{
	if (fCompareRect) {
		if (prc->Equal(&m_rc))
			return;
	}
	Invalidate();
	m_rc = *prc;
	Invalidate();
}

void Control::SetPosition(int x, int y)
{
	if (m_rc.left == x && m_rc.top == y)
		return;

	Invalidate();
	m_rc.Offset(x - m_rc.left, y - m_rc.top);
	Invalidate();
}

void Control::Show(bool fShow)
{
	if (fShow) {
		if (m_wf & kfCtlVisible)
			return;
		m_wf |= kfCtlVisible;
		Invalidate();
	} else {
		if (!(m_wf & kfCtlVisible))
			return;
		Invalidate();
		m_wf &= ~kfCtlVisible;
	}
}

void Control::Enable(bool fEnable)
{
	if (fEnable) {
		if (!(m_wf & kfCtlDisabled))
			return;
		m_wf &= ~kfCtlDisabled;
		Invalidate();
	} else {
		if (m_wf & kfCtlDisabled)
			return;
		m_wf |= kfCtlDisabled;
		Invalidate();
	}
}

void Control::OnPaint(DibBitmap *pbm)
{
}

void Control::OnPenEvent(Event *pevt)
{
}

void Control::Invalidate()
{
	if (!(m_wf & kfCtlVisible))
		return;
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	Rect rcCtl;
	rcCtl = m_rc;
	rcCtl.Offset(rcForm.left, rcForm.top);
	m_pfrm->InvalidateRect(&rcCtl);
}

word Control::GetFlags()
{
	return m_wf;
}

void Control::SetFlags(word wf)
{
	word wfT = m_wf;
	m_wf = wf;
	if ((wfT ^ wf) & kfCtlSet)
		Invalidate();
}

} // namespace wi
