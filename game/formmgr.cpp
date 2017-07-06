#include "ht.h"

namespace wi {

FormMgr *CreateFormMgr(DibBitmap *pbm)
{
	FormMgr *pfrmm = new FormMgr();
	Assert(pfrmm != NULL, "out of memory!");
	if (pfrmm == NULL)
		return NULL;
	if (!pfrmm->Init(pbm, true)) {
		delete pfrmm;
		return NULL;
	}
	return pfrmm;
}

FormMgr::FormMgr()
{
	m_wf = 0;
	m_fFreeDib = false;
	m_pupd = NULL;
	m_idfCapture = 0;
    m_cCaptureDowns = 0;
	m_cfrm = 0;
	m_dxScrollAccumulate = 0;
	m_dyScrollAccumulate = 0;
	memset(m_apfrm, 0, sizeof(m_apfrm));
}

FormMgr::~FormMgr()
{
	if (m_fFreeDib)
		delete m_pbm;
	while (m_cfrm != 0) {
		Form *pfrm = m_apfrm[0];
		RemoveForm(pfrm);
		delete pfrm;
	}
	delete m_pupd;
}

bool FormMgr::Init(DibBitmap *pbm, bool fFreeDib)
{
	m_pbm = pbm;
	m_fFreeDib = fFreeDib;
	m_pupd = new UpdateMap();
	Assert(m_pupd != NULL, "out of memory!");
	if (m_pupd == NULL)
		return false;
	Size siz;
	pbm->GetSize(&siz);
	if (!m_pupd->Init(&siz))
		return false;
	return true;
}

void FormMgr::AddForm(Form *pfrm)
{
	Assert(m_cfrm < kcFormsMax);
	if (m_cfrm >= kcFormsMax)
		return;

	// Newly added forms go on top, unless there aren't topmost
	// and topmost forms are already present, in which case the
	// added form is placed immediately underneath the bottomost
	// topmost form.

	int i;
	if (pfrm->GetFlags() & kfFrmTopMost) {
		i = m_cfrm;
	} else {
		for (i = 0; i < m_cfrm; i++) {
			if (m_apfrm[i]->GetFlags() & kfFrmTopMost)
				break;
		}

		if (i != m_cfrm)
			memmove(&m_apfrm[i + 1], &m_apfrm[i], sizeof(pfrm) * (m_cfrm - i));
	}

	m_apfrm[i] = pfrm;
	m_cfrm++;
	pfrm->InvalidateRect(NULL);
}

void FormMgr::RemoveForm(Form *pfrm)
{
	for (int n = 0; n < m_cfrm; n++) {
		if (m_apfrm[n] == pfrm) {
			if (n < kcFormsMax - 1)
				memmove(&m_apfrm[n], &m_apfrm[n + 1], sizeof(Form *) * (kcFormsMax - 1 - n));
			m_cfrm--;
			break;
		}
	}
	pfrm->InvalidateRect(NULL);
}

bool FormMgr::EcomSuppressed()
{
	for (int n = 0; n < m_cfrm; n++) {
		if (m_apfrm[n]!= NULL) {
			if (m_apfrm[n]->GetFlags() & kfFrmNoEcom)
				return true;
		}
	}
	return false;
}

bool FormMgr::CookKeyEvent(Event *pevt)
{
	// Hack for now: assume the topmost form wants key input
	// Maybe this'll be enough focus "knowledge".

	for (int n = m_cfrm - 1; n >= 0; n--) {
		if (m_apfrm[n]->OnKeyTest(pevt)) {
			pevt->idf = m_apfrm[n]->GetId();
			return true;
		}
	}
	return false;
}

Form *FormMgr::FindPen2Form()
{
    // Find a form that demands pen2

    for (int i = 0; i < m_cfrm; i++) {
        Form *pfrm = m_apfrm[i];
        if (pfrm->m_wf & kfFrmDemandPen2) {
            return pfrm;
        }
    }
    return NULL;
}

void FormMgr::BreakCapture()
{
    Form *pfrm = GetFormCapture();
    if (pfrm == NULL) {
        return;
    }
    pfrm->BreakCapture();
    m_idfCapture = 0;
    m_cCaptureDowns = 0;
}

bool FormMgr::CookPenEvent(Event *pevt)
{
	// Send the input to the capturing form. Auto-release if pen up,
	// auto-capture if pen down.

	if (m_idfCapture != 0) {
		pevt->idf = m_idfCapture;
        if (pevt->eType == penDownEvent || pevt->eType == penDownEvent2) {
            m_cCaptureDowns++;
        }
        if (pevt->eType == penUpEvent || pevt->eType == penUpEvent2) {
            m_cCaptureDowns--;
        }
        if (m_cCaptureDowns <= 0) {
            m_cCaptureDowns = 0;
			m_idfCapture = 0;
        }
		return true;
	} else {
		// Ask the topmost form first if it wants this pen input.
		// All coords are global.

		word idf = 0;
		for (int n = m_cfrm - 1; n >= 0; n--) {
			Form *pfrm = m_apfrm[n];
			if (pfrm->OnHitTest(pevt)) {
				idf = pfrm->GetId();
				break;
			}

			// If the top-most form is modal, don't pass the event on
			// to any lower forms.

			if (pfrm->GetFlags() & kfFrmDoModal) {
				pevt->idf = 0;
				return true;
			}
		}
		pevt->idf = idf;
		if (pevt->eType == penDownEvent || pevt->eType == penDownEvent2) {
            m_cCaptureDowns = 1;
			m_idfCapture = idf;
        }
		return pevt->idf != 0;
	}
}

Form *FormMgr::GetFocus()
{
	// Hack for now: assume the topmost form wants key input
	// Maybe this'll be enough focus "knowledge".

	for (int n = 0; n < m_cfrm; n++) {
		if (m_apfrm[n]->OnKeyTest(NULL)) {
			return m_apfrm[n];
		}
	}
	return NULL;
}

bool FormMgr::CookEvent(Event *pevt)
{
	pevt->idf = 0;
	switch (pevt->eType) {
	case penHoverEvent:
    case penUpEvent:
	case penDownEvent:
	case penMoveEvent:
    case penUpEvent2:
    case penDownEvent2:
    case penMoveEvent2:
	case penHoldEvent:
		{
			// If no capture, process as usual

			if (m_idfCapture == 0) {
				return CookPenEvent(pevt);
            }

			// Mouse is captured. If captureed but there is a new form on top,
			// gracefully release the capture (can happen if form shows while
			// mouse is captured)

			if (!CookPenEvent(pevt))
				return false;
			for (int n = m_cfrm - 1; n >= 0; n--) {
				Form *pfrm = m_apfrm[n];
				if ((pfrm->GetFlags() & (kfFrmDoModal | kfFrmVisible)) == (kfFrmDoModal | kfFrmVisible)) {
					if (pfrm->m_idf != pevt->idf) {
						pevt->dw = 1;
						pevt->eType = penUpEvent;
						m_idfCapture = 0;
                        m_cCaptureDowns = 0;
					}
				}
				break;
			}
			return true;
		}
		break;

	case keyDownEvent:
		return CookKeyEvent(pevt);
	}
	return false;
}

Form *FormMgr::GetFormPtr(word idf)
{
	for (int n = m_cfrm - 1; n >= 0; n--) {
		if (m_apfrm[n]->GetId() == idf)
			return m_apfrm[n];
	}
	return NULL;
}

Form *FormMgr::LoadForm(IniReader *pini, word idf, Form *pfrm)
{
	if (pfrm == NULL)
		return NULL;

	if (!pfrm->Init(this, pini, idf)) {
		delete pfrm;
		return NULL;
	}

	return pfrm;
}

void FormMgr::CalcOpaqueRect(Form *pfrmStop, Rect *prcOpaqueStart, Rect *prcResult)
{
	// Start from the top and expand the opaque rect when a portion lies safely inside
	// the new rectangle.

	if (prcOpaqueStart == NULL) {
		prcResult->SetEmpty();
	} else {
		*prcResult = *prcOpaqueStart;
	}
	for (int n = m_cfrm - 1; n >= 0; n--) {
		Form *pfrm = m_apfrm[n];
		if (pfrm == pfrmStop)
			break;
		word wf = pfrm->GetFlags();
		if (wf & kfFrmTranslucent)
			continue;
		if (!(wf & kfFrmVisible))
			continue;
		prcResult->Add(prcResult, &pfrm->m_rc);
	}
}

void FormMgr::Paint(bool fScrolled, Rect *prcOpaqueStart)
{
	// If scrolling, we want to scroll from a valid back buffer.
	// After invalidation has been calculated for this frame, merge in
	// damaged invalid; this'll make the back buffer valid after redraw.

	if (fScrolled)
		m_pupd->StartMergeDamagedInvalid();

	// Give some time to sound servicing

	HostSoundServiceProc();

	// Invalidate UpdateMap for this frame

	int j;
	Rect rcOpaque;
	for (j = 0; j < m_cfrm; j++) {
		if (!(m_apfrm[j]->m_wf & kfFrmVisible))
			continue;
		CalcOpaqueRect(m_apfrm[j], prcOpaqueStart, &rcOpaque);
		if (!rcOpaque.RectIn(&m_apfrm[j]->m_rc))
			m_apfrm[j]->OnUpdateMapInvalidate(m_pupd, &rcOpaque);
	}

	// Give some time to sound servicing

	HostSoundServiceProc();

	// The merge is a two part process

	if (fScrolled)
		m_pupd->EndMergeDamagedInvalid();

	// If nothing invalid, nothing to paint

	if (!m_pupd->IsInvalid())
		return;

	// Now paint against the update map

	for (j = 0; j < m_cfrm; j++) {
		if (!(m_apfrm[j]->m_wf & kfFrmVisible))
			continue;
		if (m_pupd->IsRectInvalid(&m_apfrm[j]->m_rc)) {
			CalcOpaqueRect(m_apfrm[j], prcOpaqueStart, &rcOpaque);
			if (!rcOpaque.RectIn(&m_apfrm[j]->m_rc)) {
				m_apfrm[j]->OnPaintBackground(m_pbm, m_pupd);
				HostSoundServiceProc();
				m_apfrm[j]->OnPaint(m_pbm);
				HostSoundServiceProc();
				m_apfrm[j]->OnPaintControls(m_pbm, m_pupd);
				HostSoundServiceProc();
			}
		}
	}
}

void FormMgr::Scroll(int dx, int dy)
{
	// If nothing to scroll, return

	if (dx == 0 && dy == 0)
		return;

	// Force the forms to invalidate the updatemap appropriately before the map
	// is scrolled

	int i;
	for (i = 0; i < m_cfrm; i++)
		m_apfrm[i]->ScrollInvalidate(m_pupd);

	// Now scroll the update map

	bool fScrolled = m_pupd->Scroll(dx, dy);

	// Give forms a chance to respond to the scroll

	for (i = 0; i < m_cfrm; i++)
		m_apfrm[i]->OnScroll(dx, dy);

	// Now invalidate again after the map has been scrolled

	for (i = 0; i < m_cfrm; i++)
		m_apfrm[i]->ScrollInvalidate(m_pupd);

	// Accumulate

	if (fScrolled) {
		m_dxScrollAccumulate += dx;
		m_dyScrollAccumulate += dy;
	}
}

bool FormMgr::ScrollBits()
{
	if (m_dxScrollAccumulate == 0 && m_dyScrollAccumulate == 0)
		return false;
	Size sizSrc;
	m_pbm->GetSize(&sizSrc);
	Rect rcSrc;

	rcSrc.left = 0;
	if (m_dxScrollAccumulate < 0) {
		rcSrc.left = -m_dxScrollAccumulate;
		m_dxScrollAccumulate = 0;
	}
	rcSrc.top = 0;
	if (m_dyScrollAccumulate < 0) {
		rcSrc.top = -m_dyScrollAccumulate;
		m_dyScrollAccumulate = 0;
	}
	rcSrc.right = sizSrc.cx;
	if (m_dxScrollAccumulate > 0)
		rcSrc.right -= m_dxScrollAccumulate;
	rcSrc.bottom = sizSrc.cy;
	if (m_dyScrollAccumulate > 0)
		rcSrc.bottom -= m_dyScrollAccumulate;

	// Hack: if the front dib is a rotated dib, it is a 3 dib architecture and
    // we can scroll the middle (rotated) dib rather than the back dib, giving
    // us a speedup.

#if !defined(WIN)
	DibBitmap *pbmFront = gpdisp->GetFrontDib();
	if (pbmFront->GetFlags() & kfDibWantScrolls) {
#if defined(PALM)
        // Change this over to DibBitmap::Scroll in the future
		RotatedFrontDib *pbmScroll = (RotatedFrontDib *)pbmFront;
		pbmScroll->ScrollBlt(&rcSrc, m_dxScrollAccumulate, m_dyScrollAccumulate);
#else
        pbmFront->Scroll(&rcSrc, m_dxScrollAccumulate, m_dyScrollAccumulate);
#endif
		m_dxScrollAccumulate = 0;
		m_dyScrollAccumulate = 0;
		return true;
	}
#endif

	// Perform blt of the back buffer

	m_pbm->Blt(m_pbm, &rcSrc, m_dxScrollAccumulate, m_dyScrollAccumulate);
	m_dxScrollAccumulate = 0;
	m_dyScrollAccumulate = 0;
	return true;
}

bool FormMgr::HasCapture()
{
	return m_idfCapture != 0;
}

Form *FormMgr::GetFormCapture()
{
    if (m_idfCapture == 0) {
        return NULL;
    }
	for (int n = m_cfrm - 1; n >= 0; n--) {
        if (m_apfrm[n]->m_idf == m_idfCapture) {
            return m_apfrm[n];
        }
    }
    return NULL;
}

Form *FormMgr::GetModalForm()
{
	for (int n = m_cfrm - 1; n >= 0; n--) {
		if ((m_apfrm[n]->m_wf & (kfFrmDoModal | kfFrmVisible)) == (kfFrmDoModal | kfFrmVisible))
			return m_apfrm[n];
	}
	return NULL;
}

void FormMgr::InvalidateRect(Rect *prc)
{
	Rect rcDib;
	if (prc == NULL) {
		Size siz;
		m_pbm->GetSize(&siz);
		rcDib.Set(0, 0, siz.cx, siz.cy);
		prc = &rcDib;
	}

	if (m_pupd != NULL)
		m_pupd->InvalidateRect(prc);
}

DibBitmap *FormMgr::GetDib()
{
	return m_pbm;
}

bool FormMgr::BltTo(DibBitmap *pbmDst, int yTop, bool fScrolled)
{
	// If nothing invalid, nothing to do

	if (!m_pupd->IsInvalid())
		return false;

    // If we've scrolled, we've already assured the back dib is valid, so just
    // copy it Unless the front dib is handling scrolls in which case do the
    // normal blt tiles thing.

	if (!(gpdisp->GetFrontDib()->GetFlags() & kfDibWantScrolls)) {
		if (fScrolled && !(m_wf & kfFrmmNoScroll)) {
			Size sizSrc;
			m_pbm->GetSize(&sizSrc);
			Rect rcSrc;
			rcSrc.Set(0, 0, sizSrc.cx, sizSrc.cy);
			pbmDst->Blt(m_pbm, &rcSrc, 0, yTop);
			return true;
		}
	}

	// Copy to pbmDst only tiles that are invalid

	pbmDst->BltTiles(m_pbm, m_pupd, yTop);
	return true;
}
    
void FormMgr::FrameStart()
{
    for (int n = 0; n < m_cfrm; n++) {
        m_apfrm[n]->FrameStart();
    }
}

void FormMgr::FrameComplete()
{
    for (int n = 0; n < m_cfrm; n++) {
        m_apfrm[n]->FrameComplete();
    }    
}    

//
// MultiFormMgr
//

MultiFormMgr::MultiFormMgr()
{
	m_cfrmm = 0;
	memset(m_apfrmm, 0, sizeof(m_apfrmm));
    memset(&m_evtPen1Down, 0, sizeof(m_evtPen1Down));
}

MultiFormMgr::~MultiFormMgr()
{
	while (m_cfrmm != 0) {
		FormMgr *pfrmm = m_apfrmm[0];
		RemoveFormMgr(pfrmm);
		delete pfrmm;
	}
}

void MultiFormMgr::AddFormMgr(FormMgr *pfrmm, Rect *prc)
{
	Assert(m_cfrmm < kcFormMgrMax);
	m_apfrmm[m_cfrmm] = pfrmm;
	m_arcFormMgr[m_cfrmm] = *prc;
	m_cfrmm++;
}

void MultiFormMgr::RemoveFormMgr(FormMgr *pfrmm)
{
	for (int n = 0; n < m_cfrmm; n++) {
		if (m_apfrmm[n] == pfrmm) {
			if (n < kcFormMgrMax - 1) {
				memmove(&m_apfrmm[n], &m_apfrmm[n + 1], sizeof(FormMgr *) * (kcFormMgrMax - 1 - n));
				memmove(&m_arcFormMgr[n], &m_arcFormMgr[n + 1], sizeof(Rect) * (kcFormMgrMax - 1 - n));
			}
			m_cfrmm--;
			break;
		}
	}
}

void MultiFormMgr::AddForm(Form *pfrm)
{
	FormMgr::AddForm(pfrm);
	InvalidateRect(NULL);
}

void MultiFormMgr::RemoveForm(Form *pfrm)
{
	FormMgr::RemoveForm(pfrm);
	InvalidateRect(NULL);
}

bool MultiFormMgr::EcomSuppressed()
{
	return FormMgr::EcomSuppressed();
}

Form *MultiFormMgr::GetFocus()
{
	Form *pfrm = FormMgr::GetFocus();
	if (pfrm == NULL) {
		if (m_cfrmm > 0)
			return m_apfrmm[m_cfrmm - 1]->GetFocus();
	}
	return NULL;
}

bool MultiFormMgr::StealCapturePen2(Event *pevt, FormMgr *pfrmmChildCapture)
{
    // If this is penDownEvent, remember the pre-cooked event for later
    // use.

    if (pevt->eType == penDownEvent) {
        m_evtPen1Down = *pevt;
        return false;
    }

    // The playfield allows multi-touch select. Sometimes
    // finger 1 goes down on a control surrounding the playfield accidentially,
    // and finger 2 goes down on the playfield. In this case, finger 1
    // needs to be "uncaptured", and both finger 1 and finger 2 down events
    // routed to the playfield. 

    if (pevt->eType != penDownEvent2) {
        return false;
    }

    // Find a form that demands pen 2

    Form *pfrmWantsPen2 = FindPen2Form();
    if (pfrmWantsPen2 == NULL) {
        for (int i = 0; i < m_cfrmm; i++) {
            pfrmWantsPen2 = m_apfrmm[i]->FindPen2Form();
            if (pfrmWantsPen2 != NULL) {
                break;
            }
        }
    }
    if (pfrmWantsPen2 == NULL) {
        return false;
    }

    // If this form already has capture, then no need to "steal" capture;
    // process normally.

    FormMgr *pfrmmWantsPen2 = pfrmWantsPen2->GetFormMgr();
    if (pfrmmWantsPen2->HasCapture()) {
        return false;
    }

    // Any modal forms up? Don't steal pen2

    if (GetModalForm() != NULL) {
        return false;
    }
    for (int i = 0; i < m_cfrmm; i++) {
        if (m_apfrmm[i]->GetModalForm() != NULL) {
            return false;
        }
    }

    // Pen1 is captured in a different form manager. Steal that, then
    // send pen1down and pen2down to the new form.

    if (pfrmmChildCapture != NULL) {
        pfrmmChildCapture->BreakCapture();
    } else {
        if (HasCapture()) {
            BreakCapture();
        }
    }

    // Force capture to this form specifically. This will by-pass
    // hit testing.

    pfrmmWantsPen2->m_idfCapture = pfrmWantsPen2->GetId();

    // Find the formmgr's rect
    Rect rc;
    rc.SetEmpty();
    for (int i = 0; i < m_cfrmm; i++) {
        if (m_apfrmm[i] == pfrmmWantsPen2) {
            rc = m_arcFormMgr[i];
            break;
        }
    }

    // These events need to be processed in the right order.
    // To do this, they will get posted after being cooked.

    Event evtT;
    evtT = m_evtPen1Down;
    evtT.x -= rc.left;
    evtT.y -= rc.top;
    pfrmmWantsPen2->CookEvent(&evtT);
    gevm.PostEvent(&evtT, false);

    evtT = *pevt;
    evtT.x -= rc.left;
    evtT.y -= rc.top;
    pfrmmWantsPen2->CookEvent(&evtT);
    gevm.PostEvent(&evtT, false);

    // This current event is going to be processed so change it
    // to something nop-ish.

    pevt->eType = nullEvent;
    pevt->idf = 0;
    return true;
}

bool MultiFormMgr::CookEvent(Event *pevt)
{
	// If no child has capture, give the multi form manager a crack at it

    FormMgr *pfrmmChildCapture = NULL; 
	int n;
	for (n = 0; n < m_cfrmm; n++) {
		if (m_apfrmm[n]->HasCapture()) {
            pfrmmChildCapture = m_apfrmm[n];
			break;
		}
	}

    // Treat Pen2 specially.

    if (StealCapturePen2(pevt, pfrmmChildCapture)) {
        return false;
    }

	// If no child has it captured and if the multiformmgr wants it, give it

	if (pfrmmChildCapture == NULL) {
		if (FormMgr::CookEvent(pevt))
			return true;
	} else {
		// Child has it captured. If the multiformmgr has a modal form, force
		// the child to release capture. Can happen if a form shows while
		// a child has the capture.

		if (GetModalForm() != NULL) {
			pevt->dw = 1;
			pevt->eType = penUpEvent;

            // Set this to zero so it doesn't get out of whack when two
            // fingers are down. It won't underflow since that is checked.

            pfrmmChildCapture->m_cCaptureDowns = 0;
		}
	}
	
	// Try to handle intelligently

	switch (pevt->eType) {
	case penHoverEvent:
	case penDownEvent:
	case penUpEvent:
	case penMoveEvent:
    case penDownEvent2:
    case penUpEvent2:
    case penMoveEvent2:
	case penHoldEvent:
		// Send to the form with the capture

		for (n = 0; n < m_cfrmm; n++) {
			if (m_apfrmm[n]->HasCapture() ||
                    m_apfrmm[n]->GetModalForm() != NULL) {
				pevt->x -= m_arcFormMgr[n].left;
				pevt->y -= m_arcFormMgr[n].top;
				if (m_apfrmm[n]->CookEvent(pevt))
					return true;
				
                // else allow it to choose not to handle an event even if it
                // had capture IE when it releases capture/ends modal and wants
                // to pass on that event.

				Assert(!m_apfrmm[n]->HasCapture() &&
                        m_apfrmm[n]->GetModalForm() == NULL);
				
				// restore the pevt
				pevt->x += m_arcFormMgr[n].left;
				pevt->y += m_arcFormMgr[n].top;
			}
		}

		// Send to the form the event is inside

		for (n = 0; n < m_cfrmm; n++) {
			if (m_arcFormMgr[n].PtIn(pevt->x, pevt->y)) {
				pevt->x -= m_arcFormMgr[n].left;
				pevt->y -= m_arcFormMgr[n].top;
				return m_apfrmm[n]->CookEvent(pevt);
			}
		}
		break;

	case keyDownEvent:
		if (FormMgr::CookKeyEvent(pevt))
			return true;

		// fall through

	default:
		// Send to the formmgr added last

		if (m_cfrmm > 0) {
			return m_apfrmm[m_cfrmm - 1]->CookEvent(pevt);
        }
	}

	return false;
}

Form *MultiFormMgr::GetFormPtr(word idf)
{
	Form *pfrm = FormMgr::GetFormPtr(idf);
	if (pfrm != NULL)
		return pfrm;

	for (int n = 0; n < m_cfrmm; n++) {
		pfrm = m_apfrmm[n]->GetFormPtr(idf);
		if (pfrm != NULL)
			return pfrm;
	}

	return NULL;
}

void MultiFormMgr::CalcOpaqueRect(Form *pfrmStop, Rect *prcOpaqueStart, Rect *prcResult)
{
	// If this form is part of the multi form mgr, then it's simple

	if (pfrmStop == NULL || pfrmStop->GetFormMgr() == this) {
		FormMgr::CalcOpaqueRect(pfrmStop, prcOpaqueStart, prcResult);
		return;
	}

	// Otherwise it's whatever is in the multi form mgr plus the child that
	// this form is within

	FormMgr::CalcOpaqueRect(NULL, prcOpaqueStart, prcResult);
	pfrmStop->GetFormMgr()->CalcOpaqueRect(pfrmStop, prcResult, prcResult);
}

bool MultiFormMgr::GetFormMgrRect(FormMgr *pfrmm, Rect *prc)
{
	if (pfrmm == this) {
		Size sizDib;
		m_pbm->GetSize(&sizDib);
		prc->left = 0;
		prc->top = 0;
		prc->right = sizDib.cx;
		prc->bottom = sizDib.cy;
		return true;
	}

	for (int n = 0; n < m_cfrmm; n++) {
		if (m_apfrmm[n] == pfrmm) {
			*prc = m_arcFormMgr[n];
			return true;
		}
	}

	Assert();
	return false;
}

bool MultiFormMgr::Paint(bool fForceBackBufferValid)
{
	// Calc opaque rect

	Rect rcOpaque;
	rcOpaque.SetEmpty();
	CalcOpaqueRect(NULL, NULL, &rcOpaque);

	bool fAnyScroll = false;
	int n;
	for (n = 0; n < m_cfrmm; n++) {
		// Paint children form mgr bits

		bool fScrolled = m_apfrmm[n]->ScrollBits();
		if (fForceBackBufferValid)
			fScrolled = true;
		if (fScrolled)
			fAnyScroll = true;

		// Paint / calc updatemaps for children

		Rect rcT;
		rcT.Intersect(&m_arcFormMgr[n], &rcOpaque);
		rcT.Offset(-m_arcFormMgr[n].left, -m_arcFormMgr[n].top);
		m_apfrmm[n]->Paint(fScrolled, &rcT);
	}

    // There is a form, need the children updatemap data to be reflected in the
    // parent updatemap

	for (n = 0; n < m_cfrmm; n++) {
		FormMgr *pfrmm = m_apfrmm[n];
		UpdateMap *pupd = pfrmm->GetUpdateMap();
		Rect rcT;
		bool fFirst = true;
		while (pupd->EnumUpdateRects(fFirst, NULL, &rcT)) {
			fFirst = false;
			rcT.Offset(m_arcFormMgr[n].left, m_arcFormMgr[n].top);
			m_pupd->InvalidateRect(&rcT);
		}
	}

	// Now that the multi-form mgr has all the invalid state reflected,
	// it is ok to paint

	rcOpaque.SetEmpty();
	FormMgr::Paint(false, &rcOpaque);
	return fAnyScroll;
}

bool MultiFormMgr::BltTo(DibBitmap *pbmDst, int yTop, bool fScrolled)
{
	bool fBlt = false;
	for (int n = 0; n < m_cfrmm; n++) {
		fBlt |= m_apfrmm[n]->BltTo(pbmDst, m_arcFormMgr[n].top, fScrolled);
	}

	return fBlt;
}

void MultiFormMgr::DrawFrame(bool fForceBackBufferValid, bool fPaint)
{
    // Tell forms about to draw frame
    
    for (int n = 0; n < m_cfrmm; n++)
		m_apfrmm[n]->FrameStart();
    
	// Notify display we're starting a frame

	gpdisp->FrameStart();

	// Draw forms

	bool fAnyScroll = fPaint ? Paint(fForceBackBufferValid) : false;

	// Notify the display we've completed a frame. Pass in update maps
    // so selective invalidation can occur

    UpdateMap *apupd[kcFormMgrMax];
    Rect arc[kcFormMgrMax];
    for (int n = 0; n < m_cfrmm; n++) {
        apupd[n] = m_apfrmm[n]->GetUpdateMap();
        arc[n] = m_arcFormMgr[n];
    }
	gpdisp->FrameComplete(m_cfrmm, apupd, arc, fAnyScroll);

	// Save avi frame

#if defined(WIN) && !defined(CE)
	if (gpavir != NULL)
		gpavir->AddFrame(gpdisp->GetFrontDib());
#endif

	// Draw update rects

#ifdef DRAW_UPDATERECTS
	if (gfDrawUpdateRects) {
		for (int n = 0; n < m_cfrmm; n++)
			DrawUpdateRects(m_apfrmm[n]->GetUpdateMap(), m_arcFormMgr[n].top);
	}
#endif

	// Validate updatemaps

	m_pupd->Validate();
	for (int n = 0; n < m_cfrmm; n++) {
		m_apfrmm[n]->GetUpdateMap()->Validate();
    }

    // Tell forms the frame is complete.
    
    for (int n = 0; n < m_cfrmm; n++) {
		m_apfrmm[n]->FrameComplete();
    }
}

#ifdef DRAW_UPDATERECTS
void MultiFormMgr::DrawUpdateRects(UpdateMap *pupd, int yTop)
{
	// Draw update rects

	if (gfDrawUpdateRects) {
		for (int n = 0; n < m_cfrmm; n++) {
			bool fFirst = true;
			Rect rc;
			while (pupd->EnumUpdateRects(fFirst, NULL, &rc)) {
				fFirst = false;
				rc.Offset(0, yTop);
				gpdisp->DrawFrameInclusive(&rc);
			}
		}
	}
}
#endif

void MultiFormMgr::InvalidateRect(Rect *prc)
{
	// Distribute to this form mgr

	Rect rcDib;
	if (prc == NULL) {
		Size siz;
		m_pbm->GetSize(&siz);
		rcDib.Set(0, 0, siz.cx, siz.cy);
		prc = &rcDib;
	}

	// Distribute to children form mgr's

	for (int n = 0; n < m_cfrmm; n++) {
		Rect rcT;
		if (rcT.Intersect(prc, &m_arcFormMgr[n])) {
			rcT.Offset(-m_arcFormMgr[n].left, -m_arcFormMgr[n].top);
			m_apfrmm[n]->InvalidateRect(&rcT);
		}
	}
}

void MultiFormMgr::CheckSetRedrawDirty()
{
	for (int n = 0; n < m_cfrmm; n++) {
		if (m_apfrmm[n]->GetUpdateMap()->IsInvalid()) {
			gevm.SetRedrawFlags(kfRedrawDirty);
			return;
		}
	}
}

bool MultiFormMgr::IsInvalid()
{
	for (int n = 0; n < m_cfrmm; n++) {
		if (m_apfrmm[n]->GetUpdateMap()->IsInvalid()) {
            return true;
		}
	}
    return false;
}

#ifdef STATS_DISPLAY
int MultiFormMgr::GetUpdateRectCount()
{
	int c = 0;
	for (int n = 0; n < m_cfrmm; n++) {
		UpdateMap *pupd = m_apfrmm[n]->GetUpdateMap();
		Size sizMap;
		pupd->GetMapSize(&sizMap);
		int cb = sizMap.cx * sizMap.cy;
		bool *pfInvalid = pupd->GetInvalidMap();
		while (cb-- != 0) {
			if (*pfInvalid++)
				c++;
		}
	}
	return c;
}
#endif

} // namespace wi
