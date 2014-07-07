#include "game/ht.h"

namespace wi {

SimUIForm::StylusHandler::StylusHandler(SimUIForm *psui)
{
    m_psui = psui;
    m_fDragging = false;
}

bool SimUIForm::StylusHandler::OnPenEvent(Event *pevt, bool fScrollOnly)
{
    // Can't scroll the playfield from this PenHandler

    if (fScrollOnly) {
        return true;
    }

	// Dragging mode captures all pen events except pen-hold for the duration of
	// the drag to perform drag-scrolling, etc (shift-drag or graffiti-scroll)

	if (m_fDragging && pevt->eType != penHoldEvent) {
		OnPenDrag(pevt);

		// Force draw to keep dragging as interactive as possible
		// Raises redraw priority above next update

		gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);
		return true;
	}

	switch (pevt->eType) {
	case penHoldEvent:
		// Popup Gob menu if pen held on one of them

		OnPenHold(pevt);
		break;

	case penDownEvent:
        // Handle the beginning of a drag selection operation or a tap on a
        // unit/structure

		OnPenDown(pevt);
		break;

	case penUpEvent:
		// Handle the canceling of any in progress modes

		OnPenUp(pevt);
		break;
	}

	return true;
}

void SimUIForm::StylusHandler::OnPenDown(Event *pevt)
{
    // Enter dragging mode; this'll cause OnPenDrag to be called until
    // that mode exits.
    
	m_fDragging = true;

    // Calc a hit target in world coords
    
	WCoord wxPen = WcFromPc(pevt->x);
	WCoord wyPen = WcFromPc(pevt->y);
	WCoord wxTarget, wyTarget;
	gsim.GetViewPos(&wxTarget, &wyTarget);
	wxTarget += wxPen;
	wyTarget += wyPen;

	// s_wptSelect1 & 2 are used to determine the transition from a
	// tentative selection to a real one and must contain the beginning
	// and current pen positions, respectively.

	s_wptSelect1.wx = wxTarget;
	s_wptSelect1.wy = wyTarget;
	s_wptSelect2 = s_wptSelect1;

	if (gfLassoSelection) {
		s_awptSelection[0] = s_wptSelect1;
		s_cwptSelection = 1;
	} else {
		gwrcSelection.Set(s_wptSelect1, s_wptSelect2);
		gsim.SetSelection(&gwrcSelection);
	}

	// Only Select. Leave moving and attacking to the pen up handler.

	m_psui->MoveOrAttackOrSelect(pevt->x, pevt->y, kfMasSelect);
}

void SimUIForm::StylusHandler::OnPenUp(Event *pevt)
{
    CancelModes();
}

// TUNE:
#define kwcMinSelection WcFromTile16ths(8)

// The WI playfield finger UI:
//
// 1. One finger for selecting and rubber banding around troops
// 2. Two fingers for scrolling the map.
// 3. Two finger flick for momentum scrolls.
//
// Notes:
// - During a select operation, the second finger may come down. This cancels
//   the selection and enters scroll mode. Units that were selected as part of
//   entering the mode are unselected when entering scroll mode. Units that
//   were selected before select mode was entered are left selected.
//
// - Once entered, scroll mode remains even if one of the fingers goes up
//
// - At the beginning of scroll mode, the first down is the reference
//   point.
//
// - If in scroll mode, and one of the fingers goes up while the other remains
//   down, the aggregate is still down, and the coordinate is based on the
//   reference point so there is no map jumping.

void SimUIForm::StylusHandler::OnPenDrag(Event *pevt)
{
    WCoord wxPen = WcFromPc(pevt->x);
    WCoord wyPen = WcFromPc(pevt->y);
    WCoord wxView, wyView;
    gsim.GetViewPos(&wxView, &wyView);
    
	switch (pevt->eType) {
	case penUpEvent:
		// If this is a forced up event, cancel our modes

		if (pevt->dw == 1) {
			CancelModes();
			break;
		}

		// Real pen event. Leaving drag mode.

		m_fDragging = false;

		// If we're drag selecting complete the process. Otherwise handle this
		// event as a tap on an enemy (attack it) or location (move to it)

		if (gfDragSelecting) {
			// Invalidate this area first so we get it to redraw

			m_psui->InvalidateDragSelection();

			// Turn off selection

			gfDragSelecting = false;

			if (gfLassoSelection && s_cwptSelection > 1) {
				bool fGobsSelected = false;

				Gob *pgobT;
				for (pgobT = ggobm.GetFirstGob(); pgobT != NULL;
                        pgobT = ggobm.GetNextGob(pgobT)) {
					dword ff = pgobT->GetFlags();

                    // If were in select mode then only Gobs inside the
                    // selection rectangle get to be selected.

					bool fSelect = false;
					if ((gfGodMode || pgobT->GetOwner() == gpplrLocal) &&
                            ((ff & (kfGobActive | kfGobUnit)) ==
                            (kfGobActive | kfGobUnit))) {
						WPoint wptGobCenter;
						pgobT->GetCenter(&wptGobCenter);
						if (PtInPolygon(s_awptSelection, s_cwptSelection,
                                wptGobCenter.wx, wptGobCenter.wy)) {
							fSelect = true;
							fGobsSelected = true;
						}
					}
					if (ff & kfGobUnit) {
						((UnitGob *)pgobT)->Select(fSelect);
                    }
				}

				s_cwptSelection = 0;
			}
		} else {
			m_psui->MoveOrAttackOrSelect(pevt->x, pevt->y, kfMasMove | kfMasAttack | kfMasShowMenu);
		}
		break;
            
	case penMoveEvent:
		{            
			if (gfDragSelecting) {
				// Inval old pos of selection rect

				m_psui->InvalidateDragSelection();
			}

            // Update() code uses s_wptSelect2 to determine if edge scrolling
            // is necessary

			s_wptSelect2.wx = wxView + WcFromPc(pevt->x);
			s_wptSelect2.wy = wyView + WcFromPc(pevt->y);
 			gwrcSelection.Set(s_wptSelect1, s_wptSelect2);

			if (!gfDragSelecting) {
				if (gwrcSelection.Width() >= kwcMinSelection ||
                        gwrcSelection.Height() >= kwcMinSelection) {
					gfDragSelecting = true;
				}
			}

			if (gfDragSelecting) {
				// Inval new size of selection rect

				m_psui->InvalidateDragSelection();

				if (gfLassoSelection) {
					AddPointToLassoSelection(s_wptSelect2);
                }

				gsim.SetSelection(&gwrcSelection);
			}
		}
		break;
	}
}

void SimUIForm::StylusHandler::OnPenHold(Event *pevt)
{
	WCoord wxTarget, wyTarget;
	gsim.GetViewPos(&wxTarget, &wyTarget);
	wxTarget += WcFromPc(pevt->x);
	wyTarget += WcFromPc(pevt->y);

	Gob *pgobHit = NULL;
	Enum enm;
	if (!gsim.HitTest(&enm, wxTarget, wyTarget, kfGobActive | kfGobUnit,
            &pgobHit)) {
		return;
    }

	// Don't popup menu if the player doesn't own the Gob (unless in God mode)
	// God mode is limited for multiplayer games.

	if ((!gfGodMode || ggame.IsMultiplayer()) && 
			pgobHit->GetSide() != gpplrLocal->GetSide()) {
		return;
    }

	m_fDragging = false;
	pgobHit->PopupMenu();
}

void SimUIForm::StylusHandler::CancelModes()
{
	m_fDragging = false;

	if (gfDragSelecting) {
		// Invalidate this area first so we get it to redraw

		m_psui->InvalidateDragSelection();
		gfDragSelecting = false;
	}
}

void SimUIForm::StylusHandler::CheckScroll()
{
	// If the player is rectangle selecting and at the screen edge scroll the
	// screen.

    if (!gfDragSelecting) {
        return;
    }

    Size sizPlayfield;
    ggame.GetPlayfieldSize(&sizPlayfield);
    WCoord wcxPlayfield = WcFromUpc(sizPlayfield.cx);
    WCoord wcyPlayfield = WcFromUpc(sizPlayfield.cy);

    WCoord wxView, wyView;
    gsim.GetViewPos(&wxView, &wyView);
    
    WCoord wxViewNew = wxView;
    WCoord wyViewNew = wyView;

    if (s_wptSelect2.wx < wxView + kwcScrollBorderSize) {
        wxViewNew -= kwcScrollStepSize;
    } else if (s_wptSelect2.wx > wxView + wcxPlayfield - kwcScrollBorderSize) {
        wxViewNew += kwcScrollStepSize;
    }

    if (s_wptSelect2.wy < wyView + kwcScrollBorderSize) {
        wyViewNew -= kwcScrollStepSize;
    } else if (s_wptSelect2.wy > wyView + wcyPlayfield - kwcScrollBorderSize) {
        wyViewNew += kwcScrollStepSize;
    }

    if (wxViewNew != wxView || wyViewNew != wyView) {
        gsim.SetViewPos(wxViewNew, wyViewNew);
        WCoord wxViewActual, wyViewActual;
        gsim.GetViewPos(&wxViewActual, &wyViewActual);
        s_wptSelect2.wx += wxViewActual - wxView;
        s_wptSelect2.wy += wyViewActual - wyView;
        gwrcSelection.Set(s_wptSelect1, s_wptSelect2);

        if (gfLassoSelection)
            AddPointToLassoSelection(s_wptSelect2);

        gsim.SetSelection(&gwrcSelection);
    }
}

} // namespace wi
