#include "game/ht.h"
#include "base/tick.h"

namespace wi {

const int SMALL_DRAG = kwcTile / 4;
const int MEDIUM_DRAG = kwcTile / 3;
const int LARGE_DRAG = kwcTile / 2;

const long QUICK_UP_TICKS = 20;

SimUIForm::FingerHandler::FingerHandler(SimUIForm *psui) {
    m_psui = psui;
    m_gidHitLast = kgidNull;
    m_gidHilight = kgidNull;
    m_xDownLast = 0;
    m_yDownLast = 0;
    m_state = FHS_NONE;
    m_fShowUnitMenu = false;
    m_pfrmUnitTitle = NULL;
    m_ff = 0;

    // Tell the placement form to pass input through, which will
    // allow the this handler to scroll the playfield.
    if (gpfrmPlace != NULL) {
        gpfrmPlace->SetPassOnInput(true);
    }

    m_pselspr = gpsprm->CreateSelectionSprite();
    if (m_pselspr != NULL) {
        m_pselspr->Show(false);
    }
}

SimUIForm::FingerHandler::~FingerHandler() {
    delete m_pfrmUnitTitle;
    delete m_pselspr;
}

bool SimUIForm::FingerHandler::OnPenEvent(Event *pevt, bool fScrollOnly) {
#if 0
    switch (pevt->eType) {
    case penDownEvent:
        IPhone::Log("penDownEvent x=%d y=%d", pevt->x, pevt->y);
        break;

    case penDownEvent2:
        IPhone::Log("penDownEvent2 x=%d y=%d", pevt->x, pevt->y);
        break;

    case penUpEvent:
        IPhone::Log("penUpEvent x=%d y=%d", pevt->x, pevt->y);
        break;

    case penUpEvent2:
        IPhone::Log("penUpEvent2 x=%d y=%d", pevt->x, pevt->y);
        break;

    case penMoveEvent:
        IPhone::Log("penMoveEvent x=%d y=%d", pevt->x, pevt->y);
        break;

    case penMoveEvent2:
        IPhone::Log("penMoveEvent2 x=%d y=%d", pevt->x, pevt->y);
        break;

    case penHoldEvent:
        IPhone::Log("penHoldEvent x=%d y=%d", pevt->x, pevt->y);
        break;
    }
#endif

    switch (pevt->eType) {
    case penDownEvent:
        OnPenDown(pevt, fScrollOnly);
        break;

    case penDownEvent2:
        OnPenDown2(pevt, fScrollOnly);
        break;

    case penUpEvent:
        OnPenUp(pevt, fScrollOnly);
        break;

    case penUpEvent2:
        OnPenUp2(pevt, fScrollOnly);
        break;

    case penMoveEvent:
        OnPenMove(pevt);
        break;

    case penMoveEvent2:
        OnPenMove2(pevt);
        break;

    case penHoldEvent:
        OnPenHold(pevt);
        break;

    default:
        return false;
    }
    return true;
}

void SimUIForm::FingerHandler::OnPenDown(Event *pevt, bool fScrollOnly) {
    m_tDown = base::GetTickCount();
    m_xDownLast = pevt->x;
    m_yDownLast = pevt->y;
    m_x1 = pevt->x;
    m_y1 = pevt->y;
    m_ff |= kfPhFinger1Down;

    // The first finger can come down while in FHS_SELECT mode
    if (m_state == FHS_SELECT) {
        UpdateSelect(pevt);
        return;
    }

    // Not in select mode, then there should be no modes operating
    Gob *pgob = m_psui->HitTestGob(pevt->x, pevt->y, true, &m_wxTarget,
            &m_wyTarget, &m_fHitSurrounding);

    // Pretend no gob hittesting if it is in fog
    if (pgob != NULL) {
        TRect trc;
        pgob->GetTileRect(&trc);
        if (gsim.GetLevel()->GetFogMap()->IsCovered(&trc)) {
            pgob = NULL;
        }
    }
    m_gidHitLast = (pgob == NULL) ? kgidNull : pgob->GetId();

    EnterNone();

    if (fScrollOnly) {
        EnterDrag(pevt);
        return;
    }

    if (CheckSelect(pevt)) {
        EnterSelect(pevt);
        return;
    }
 
    if (pgob != NULL) {
        EnterHilight(pgob);
        return;
    }

    // Enter drag right away to make the UI responsive
    EnterDrag(pevt);
}

void SimUIForm::FingerHandler::OnPenDown2(Event *pevt, bool fScrollOnly) {
    m_x2 = pevt->x;
    m_y2 = pevt->y;
    m_ff |= kfPhFinger2Down;

    // fScrollOnly is set if in observe mode, or in structure placement mode
    if (fScrollOnly) {
        return;
    }

    // May already be in FHS_SELECT when finger 2 comes down
    if (m_state == FHS_SELECT) {
        UpdateSelect(pevt);
        return;
    }

    // Otherwise, enter it
    if (CheckSelect(pevt)) {
        EnterSelect(pevt);
        return;
    }
}

void SimUIForm::FingerHandler::OnPenMove(Event *pevt) {
    m_x1 = pevt->x;
    m_y1 = pevt->y;

    if (m_state == FHS_DRAG) {
        UpdateDrag(pevt);
        return;
    }

    if (m_state == FHS_SELECT) {
        UpdateSelect(pevt);
        return;
    }

    if (m_state == FHS_HILIGHT) {
        if (CheckDragged(pevt, LARGE_DRAG)) {
            EnterDrag(pevt);
        }
    }
}

void SimUIForm::FingerHandler::OnPenMove2(Event *pevt) {
    m_x2 = pevt->x;
    m_y2 = pevt->y;

    if (m_state == FHS_SELECT) {
        UpdateSelect(pevt);
        return;
    }
}

void SimUIForm::FingerHandler::OnPenUp(Event *pevt, bool fScrollOnly) {
    m_x1 = pevt->x;
    m_y1 = pevt->y;
    m_ff &= ~kfPhFinger1Down;

    // The form manager sometimes posts these when modes need to be
    // gracefully broken out of when a new form shows during capture.

    if (pevt->dw == 1) {
        m_ff &= ~(kfPhFinger1Down | kfPhFinger2Down);
        EnterNone();
        return;
    }

    bool fTarget = false;
    switch (m_state) {
    case FHS_DRAG:
        // Try to detect a quick targetting jab and don't scroll the map
        if (IsQuickUp(pevt)) {
            fTarget = !CheckDragged(pevt, MEDIUM_DRAG);
        } else {
            fTarget = !CheckDragged(pevt, SMALL_DRAG);
        }
        if (fTarget) {
            break;
        }
        // fall through
    default:
        // Update the position, since there may be a new position
        OnPenMove(pevt);
    }

    if (m_state == FHS_NONE) {
        return;
    }

    if (m_state == FHS_SELECT) {
        if (m_ff & kfPhFinger2Down) {
            UpdateSelect(pevt);
        } else {
            EnterNone();
        }
        return;
    }

    if (m_state == FHS_DRAG) {
        if (gpfrmPlace != NULL && gpfrmPlace->IsBusy()) {
            if (!CheckDragged(pevt, LARGE_DRAG)) {
                gpfrmPlace->UpdatePosition(pevt);
            }
            EnterNone();
            return;
        }
        if (fTarget && !fScrollOnly) {
            m_psui->MoveOrAttackOrSelect(NULL, m_wxTarget, m_wyTarget,
                    kfMasMove);
        }
        EnterNone();
        return;
    }

    // Select or attack hilighted gob.
    if (m_state == FHS_HILIGHT) {
        if (m_gidHilight == kgidNull) {
            EnterNone();
            return;
        }

        // EnterNone before showing the menu since that is modal
        Gob *pgobT = ggobm.GetGob(m_gidHilight);
        bool fShowUnitMenu = m_fShowUnitMenu;
        EnterNone();

        if (pgobT == NULL || (pgobT->GetFlags() & kfGobUnit) == 0) {
            return;
        }

        // Structures always get their menu shown when selected this way.
        if (pgobT->GetFlags() & kfGobStructure) {
            fShowUnitMenu = true;
        }

        // If showing the unit menu, make sure the command is selection.
        // When a miner targets a friendly processor, it isn't selection for
        // example.
        if (fShowUnitMenu && !m_psui->IsSelectionCommand(pgobT)) {
            fShowUnitMenu = false;
        }

        // Select or attack
        m_psui->MoveOrAttackOrSelect(pgobT, m_wxTarget, m_wyTarget,
                kfMasSelect | kfMasAttack);

        // Show the menu last since it is modal
        if (fShowUnitMenu) {
            m_psui->ShowUnitMenu(pgobT);
        }
        return;
    }

    // What is this state?
    Assert();
    EnterNone();
}

void SimUIForm::FingerHandler::OnPenUp2(Event *pevt, bool fScrollOnly) {
    m_ff &= ~kfPhFinger2Down;
    m_x2 = pevt->x;
    m_y2 = pevt->y;

    if (m_state == FHS_SELECT) {
        if (m_ff & kfPhFinger1Down) {
            UpdateSelect(pevt);
        } else {
            EnterNone();
        }
        return;
    }
}

void SimUIForm::FingerHandler::OnPenHold(Event *pevt) {
    if (m_state == FHS_HILIGHT) {
        if (m_gidHilight == kgidNull) {
            return;
        }

        // If actually hit on the surrounding gob hit test area,
        // and not the gob itself, switch to drag. This allows for
        // precise move targeting around gobs. This is only useful
        // if there are friendly units selected.
        if (m_fHitSurrounding && m_psui->HasSelectedUnits()) {
            // Don't enter drag for the mobile HQ, so that transforming
            // it is easier. This trades of nearby targetting with easy
            // of transforming.
            Gob *pgob = ggobm.GetGob(m_gidHilight);
            if (pgob == NULL || pgob->GetType() != kgtMobileHeadquarters) {
                EnterDrag(pevt);
                return;
            }
        }

        // Hit directly on gob. Don't change modes. This is simple
        // to understand for the user. When then pen goes up,
        // show the unit menu.
        m_fShowUnitMenu = true;

        // In the meantime, show the unit title. This gives confirmation
        // to the user that they've held the pen long enough.
        ShowUnitTitle(ggobm.GetGob(m_gidHilight));
        return;
    }
}

void SimUIForm::FingerHandler::UnhilightGob() {
    if (m_gidHilight != kgidNull) {
		Gob *pgobT = ggobm.GetGob(m_gidHilight);
        if (pgobT != NULL && (pgobT->GetFlags() & kfGobUnit)) {
            ((UnitGob *)pgobT)->Hilight(false);
        }
        m_gidHilight = kgidNull;
    }
}

void SimUIForm::FingerHandler::EnterNone() {
    m_pselspr->Show(false);
    UnhilightGob();
    m_fShowUnitMenu = false;
    delete m_pfrmUnitTitle;
    m_pfrmUnitTitle = NULL;
    m_state = FHS_NONE;
}

void SimUIForm::FingerHandler::EnterHilight(Gob *pgob) {
    Assert((pgob->GetFlags() & kfGobUnit) != 0);
    ((UnitGob *)pgob)->Hilight(true);
    m_gidHilight = pgob->GetId();
    m_state = FHS_HILIGHT;
}

bool SimUIForm::FingerHandler::CheckSelect(Event *pevt) {
    return (m_ff & (kfPhFinger1Down | kfPhFinger2Down)) ==
            (kfPhFinger1Down | kfPhFinger2Down);
}

void SimUIForm::FingerHandler::EnterSelect(Event *pevt) {
    // Leave current modes
    EnterNone();
    
    // Form a box around the two down points
    Rect rc;
    if (m_x1 < m_x2) {
        rc.left = m_x1;
        rc.right = m_x2;
    } else {
        rc.left = m_x2;
        rc.right = m_x1;
    }
    if (m_y1 < m_y2) {
        rc.top = m_y1;
        rc.bottom = m_y2;
    } else {
        rc.top = m_y2;
        rc.bottom = m_y1;
    }

    // Initialize the drag rect. It expects a bottom left coordinate
    // system. If the rect is rotated 90 cw, then it fits SimUI's
    // coordinate system if x/y are swapped.
        
    DPoint pt0, pt1, pt2;
    pt0.x = rc.top;
    pt0.y = rc.left;
    pt1.x = rc.bottom;
    pt1.y = rc.left;
    pt2.x = rc.bottom;
    pt2.y = rc.right;
    DragRect drc;
    drc.Init(pt0, pt1, pt2);

    // Figure out the tracking masks

#define kcpCloseEnough 32.0

    DPoint ptA;
    ptA.x = m_y1;
    ptA.y = m_x1;
    m_maskA = drc.HitTest(ptA, &m_vOffsetA);
    if (m_vOffsetA.mag() > kcpCloseEnough) {
        m_vOffsetA = Vec2d(0, 0);
    }

    DPoint ptB;
    ptB.x = m_y2;
    ptB.y = m_x2;
    m_maskB = drc.HitTest(ptB, &m_vOffsetB);
    if (m_vOffsetB.mag() > kcpCloseEnough) {
        m_vOffsetB = Vec2d(0, 0);
    }

    // Show the drag rect

    m_pselspr->SetDragRect(drc);
    m_pselspr->Show(true);

    // Redraw the screen so the selection updates
    gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);

    // Set selection based on this rect
    SetSelection();

    // Enter FHS_SELECT mode
    m_state = FHS_SELECT;
}

void SimUIForm::FingerHandler::UpdateSelect(Event *pevt) {
    DPoint ptA, ptB;
    ptA.x = m_y1;
    ptA.y = m_x1;
    ptB.x = m_y2;
    ptB.y = m_x2;

    // Update tracking masks
    DragRect drc = m_pselspr->GetDragRect();
    switch (pevt->eType) {
    case penDownEvent:
        m_maskA = drc.HitTest(ptA, &m_vOffsetA);
        if (m_vOffsetA.mag() > kcpCloseEnough) {
            m_vOffsetA = Vec2d(0, 0);
        }
        break;

    case penDownEvent2:
        m_maskB = drc.HitTest(ptB, &m_vOffsetB);
        if (m_vOffsetB.mag() > kcpCloseEnough) {
            m_vOffsetB = Vec2d(0, 0);
        }
        break;

    case penUpEvent:
        m_maskA = 0;
        break;

    case penUpEvent2:
        m_maskB = 0;
        break;
    }

    drc.TrackPoints(m_maskA, m_vOffsetA.add(ptA), m_maskB, m_vOffsetB.add(ptB));
    m_pselspr->SetDragRect(drc);
    SetSelection();

    // Redraw the screen so the selection updates
    gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);
}

void SimUIForm::FingerHandler::CheckScroll() {
    // Don't auto-scroll if not drag selecting
    if (!m_pselspr->IsVisible()) {
        return;
    }

    // Beta feedback: users don't like auto scrolling.
    // Don't auto scroll if both fingers are down. Do auto scroll if
    // one finger is extending a rect - this keeps the old behavior.

    if (m_maskA != 0 && m_maskB != 0) {
        return;
    }

    // First see if either finger is in the "scroll border" area

    Size sizPlayfield;
    ggame.GetPlayfieldSize(&sizPlayfield);
    int cpBorder = PcFromUwc(kwcScrollBorderSize);
    word wfAdjust = 0;

    // The portrait mode status bar on the iPhone isn't visible when
    // running the game, but it still eats input! Scroll before hitting
    // that.

    int cpLeftExtra = PcFromUwc(kwcScrollLeftExtra);

    // For some reason, the iphone passes a finger up before the finger
    // gets close enough to the right edge of the screen to scroll within
    // cpBorder. So, subtract extra from the right side.

    int cpRightExtra = cpLeftExtra;

    if (m_maskA != 0) {
        if (m_x1 < cpBorder + cpLeftExtra) {
            wfAdjust |= 1;
        } else if (m_x1 > sizPlayfield.cx - cpBorder - cpRightExtra) {
            wfAdjust |= 4;
        }
        if (m_y1 < cpBorder) {
            wfAdjust |= 2;
        } else if (m_y1 > sizPlayfield.cy - cpBorder) {
            wfAdjust |= 8;
        }
    }
            
    if (m_maskB != 0) {
        if (m_x2 < cpBorder + cpLeftExtra) {
            wfAdjust |= 1;
        } else if (m_x2 > sizPlayfield.cx - cpBorder - cpRightExtra) {
            wfAdjust |= 4;
        }
        if (m_y2 < cpBorder) {
            wfAdjust |= 2;
        } else if (m_y2 > sizPlayfield.cy - cpBorder) {
            wfAdjust |= 8;
        }
    }
    if (wfAdjust == 0) {
        return;
    }

    // Set the new view pos

    WCoord wxView, wyView;
    gsim.GetViewPos(&wxView, &wyView);
    WCoord wxViewNew, wyViewNew;
    wxViewNew = wxView;
    wyViewNew = wyView;
    if (wfAdjust & 1) {
        wxViewNew -= kwcScrollStepSize;
    }
    if (wfAdjust & 4) {
        wxViewNew += kwcScrollStepSize;
    }
    if (wfAdjust & 2) {
        wyViewNew -= kwcScrollStepSize;
    }
    if (wfAdjust & 8) {
        wyViewNew += kwcScrollStepSize;
    }
    gsim.SetViewPos(wxViewNew, wyViewNew);
        
    // If tracking two fingers, the rect stays where it is. It
    // only needs to update the selection.

    if (m_maskA != 0 && m_maskB != 0) {
        SetSelection();
        gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);
        return;
    }

    // Otherwise tracking one finger; expand the size of the rect.
    // Note the coordinates passed to scroll are rotated since that
    // is how the drag rect is being maintained.

    WCoord wxViewActual, wyViewActual;
    gsim.GetViewPos(&wxViewActual, &wyViewActual);
    int dx = PcFromWc(wxView - wxViewActual);
    int dy = PcFromWc(wyView - wyViewActual);
    DragRect drc = m_pselspr->GetDragRect();
    drc.ScrollExpand(m_maskA, m_maskB, dy, dx);
    m_pselspr->SetDragRect(drc);
    SetSelection();
    gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);
}

bool SimUIForm::FingerHandler::IsQuickUp(Event *pevt) {
    if (pevt->eType != penUpEvent) {
        return false;
    }
    return (base::GetTickCount() - m_tDown) <= QUICK_UP_TICKS;
}

bool SimUIForm::FingerHandler::CheckDragged(Event *pevt, int wcDrag) {  
    int pcDrag = PcFromWc(wcDrag);
    if (abs(pevt->x - m_xDownLast) > pcDrag ||
            abs(pevt->y - m_yDownLast) > pcDrag) {
        return true;
    }
    return false;
}

void SimUIForm::FingerHandler::EnterDrag(Event *pevt) {
    EnterNone();
    m_xDrag = pevt->x;
    m_yDrag = pevt->y;
    gsim.GetViewPos(&m_wxViewDrag, &m_wyViewDrag);
    m_state = FHS_DRAG;
}

void SimUIForm::FingerHandler::UpdateDrag(Event *pevt) {
    Vec2d v(m_xDrag - pevt->x, m_yDrag - pevt->y);
    v = v.scale(gnScrollSpeed);
    gsim.SetViewPos(m_wxViewDrag + WcFromPc(v.dx),
            m_wyViewDrag + WcFromPc(v.dy));
}

void SimUIForm::FingerHandler::OnPaint(DibBitmap *pbm) {
}

void SimUIForm::FingerHandler::ShowUnitTitle(Gob *pgob) {
    // Must be a unit gob
    if (pgob == NULL || (pgob->GetFlags() & kfGobUnit) == 0) {
        return;
    }

    // Don't show this twice
    if (m_pfrmUnitTitle != NULL) {
        return;
    }

    // Create the menu
    m_pfrmUnitTitle = new UnitMenu();
    if (m_pfrmUnitTitle == NULL) {
        return;
    }
    if (!m_pfrmUnitTitle->Init(gpmfrmm, gpiniForms, kidfUnitMenu)) {
        return;
    }

    // Initialize and show, non-modal
    m_pfrmUnitTitle->SetOwner((UnitGob *)pgob, false);
    m_pfrmUnitTitle->Show(true);
    gsndm.PlaySfx(ksfxGuiFormShow);
}

void SimUIForm::FingerHandler::SetSelection() {
    if (m_pselspr == NULL) {
        return;
    }

    WCoord wxView, wyView;
    gsim.GetViewPos(&wxView, &wyView);

    const DragRect& drc = m_pselspr->GetDragRect();
    Rect rcBounding;
    drc.GetBoundingRect(&rcBounding);

    // Rotate the rect since drc is rotated

    int t = rcBounding.left;
    rcBounding.left = rcBounding.top;
    rcBounding.top = t;
    t = rcBounding.right;
    rcBounding.right = rcBounding.bottom;
    rcBounding.bottom = t;

	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL;
            pgobT = ggobm.GetNextGob(pgobT)) {

        // Only unit gobs that are active
		dword ff = pgobT->GetFlags();
        if ((ff & (kfGobUnit | kfGobActive)) != (kfGobUnit | kfGobActive)) {
            continue;
        }
        UnitGob *punt = (UnitGob *)pgobT;

        // Player must own this unit, or the game must be in god mode
        bool fSelect = true;
        if (punt->GetOwner() != gpplrLocal && !gfGodMode) {
            fSelect = false;
        }

        // No structures unless it's a tower
        if ((ff & kfGobStructure) && (punt->GetConsts()->um & kumTowers) == 0) {
            fSelect = false;
        }

#if 0
        // No miners, by popular request
        if (punt->GetType() == kgtGalaxMiner) {
            fSelect = false;
        }
#endif

        // If center not in bounding rect, discard
        WPoint wptCenter;
        punt->GetCenter(&wptCenter);
        Point ptCenter;
        ptCenter.x = PcFromWc(wptCenter.wx - wxView);
        ptCenter.y = PcFromWc(wptCenter.wy - wyView);
        if (!rcBounding.PtIn(ptCenter.x, ptCenter.y)) {
            fSelect = false;
        }

        // Center must be in drag rect
        if (fSelect) {
            DPoint pt;
            pt.y = ptCenter.x;
            pt.x = ptCenter.y;
            if (!drc.PtIn(pt)) {
                fSelect = false;
            }
        }
        punt->Select(fSelect);
	}
}

} // namespace wi
