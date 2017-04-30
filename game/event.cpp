#include "ht.h"

namespace wi {

EventMgr gevm;
long gcmsNextPaint = 0;

EventMgr::EventMgr()
{
	Init();
}

EventMgr::~EventMgr()
{
}

bool EventMgr::PeekEvent(Event *pevt, long ctWait)
{
	if (m_fPeekKeep) {
		*pevt = m_evtPeek;
		return true;
	}
	if (!GetEvent(pevt, ctWait))
		return false;

	m_fPeekKeep = true;
	m_evtPeek = *pevt;
	return true;
}

// TUNE:
const long kctPenHoldDelay = 50;

bool EventMgr::GetEvent(Event *pevt, long ctWait, bool fCheckPaints)
{
#if defined(WIN) && !defined(CE) && defined(DEBUG)
	Assert(_CrtCheckMemory());
#endif

	// Only needed for Tapwave bluetooth

#ifdef PIL
	if (gfTapwave && gptra != NULL) {
		extern void HostTrackServiceCallsPerGetEvent();
		HostTrackServiceCallsPerGetEvent();
	}
#endif

	// Force stopping once we've started. Retain modifiers to the appStopEvent if there are any

	if (m_fAppStopping) {
		if ((m_fPeekKeep) && (m_evtPeek.eType == appStopEvent)) {
			*pevt = m_evtPeek;
		} else {
			pevt->eType = appStopEvent;
		}
		return true;
	}

	// Service sound right away

	HostSoundServiceProc();
	long tStart = gtimm.GetTickCount();
	long ctTimerNext = 0;

	// Check the peek keep - if one exists, return it

	if (m_fPeekKeep) {
		*pevt = m_evtPeek;
		m_fPeekKeep = false;
		return true;
	}

	// GetEvent w/wait + timer scan and dispatch loop

	long ctTotal = ctWait;
	long ctElapsed = 0;
	long tCurrent = tStart;
	while (true) {
		// Check for posted events

		if (m_cevt != 0) {
			*pevt = m_aevtQ[0];
			m_cevt--;
			memmove(m_aevtQ, m_aevtQ + 1, m_cevt * ELEMENTSIZE(m_aevtQ));
			if (pevt->eType == appStopEvent)
				m_fAppStopping = true;
			return true;
		}

		// Which is shorter, the timer wait or the asked wait?

		long ctNext;
		if (ctWait == -1) {
			ctNext = ctTimerNext;
		} else {
			ctNext = ctTotal - ctElapsed;
			if (ctNext < 0)
				ctNext = 0;
			if (ctTimerNext != -1 && ctTimerNext < ctNext) {
				ctNext = ctTimerNext;
			}
		}

		// Don't wait any longer than it takes for a penHoldEvent to be produced

		if (m_fTimingPenHold) {
			int ctPenHoldRemaining = (int)(kctPenHoldDelay - (tCurrent - m_tPenDown));
			if (ctPenHoldRemaining < 0)
				ctPenHoldRemaining = 0;

			if (ctNext == -1 || ctPenHoldRemaining < ctNext)
				ctNext = ctPenHoldRemaining;
		}

		// This sleep might be too long for sound needs. Ask the sound manager

		if (ctNext != 0)
			ctNext = gsndm.FilterSleepTicks(ctNext);

		// Get an event / wait this long (or don't wait at all if a redraw is
        // pending)

		bool fHaveEvent = false;

//#define PAINT_BEFORE_INPUT
#ifdef  PAINT_BEFORE_INPUT
		if ((m_wfRedraw & (kfRedrawDirty | kfRedrawBeforeInput)) != (kfRedrawDirty | kfRedrawBeforeInput)) {
#endif

#ifdef __CPU_68K
			// In general we like to call HostGetEvent so it can sleep,
            // do I/O processing, whatever. However, on 68K Palms SysGetEvent
            // is very slow and feeds us a lot of pen events the processing of
            // which also slows the game so on 68K devices we throttle our
			// calls to GetEvent some.

			if (abs(tCurrent - m_tLastMoveEvent) >
                    (long)m_ctMoveEventInterval) {
#else
            {
#endif
                // Ensure that the skipped paint will be painted before sleeping kctForever

                if ((m_wfRedraw & kfRedrawPaintSkipped) && ctWait == -1) {

                    // Convert next paint to ticks

                    long ctNextPaint = (gcmsNextPaint / 10);

                    // Adjust wait time to next possible paint

                    tCurrent = gtimm.GetTickCount();
                    ctElapsed = tCurrent - tStart;
                    ctWait = ctElapsed + ctNextPaint;

                    m_wfRedraw &= ~kfRedrawPaintSkipped;
                    m_wfRedraw |= kfRedrawDirty;
                }

                // Get the event from the host. The host doesn't coalesce
                // messages, but does mark messages has being coalesce
                // candidates, based on what was in the queue when the post
                // occured. On the game side, all pen events go into a history
                // buffer for calculating flick vectors. Then, if the message
                // was marked as a coalesced message, it is ignored.
                
                while (true) {
                    fHaveEvent = HostGetEvent(pevt, (m_wfRedraw & kfRedrawDirty) ? 0 : ctNext);
                    if (!fHaveEvent)
                        break;
                    UpdatePenHistory(pevt);
                    if (!(pevt->ff & kfEvtCoalesce))
                        break;
                    
                    // There should always be a non-coalesced pen event
                    // following a coalesced one, but just in case...
                    ctNext = 0;
                }
            }   
#ifdef PAINT_BEFORE_INPUT
		}
#endif

		// Manage the production of penHoldEvents

		if (fHaveEvent) {
			switch (pevt->eType) {
			case penDownEvent:
				// Hold events are generated when the pen is pressed down and
                // held for a specified amount of time before being released.

				m_fTimingPenHold = true;
				m_tPenDown = gtimm.GetTickCount();
				m_xHold = pevt->x;
				m_yHold = pevt->y;
				break;

			case penMoveEvent:
                // If the pen moves by more than N pixels in any dimension then
                // we'll assume the player isn't trying to tap-hold and cancel
                // it.

				if (m_fTimingPenHold) {
					// TUNE: penhold distance threshold
					if (abs(pevt->x - m_xHold) > 7 ||
                                abs(pevt->y - m_yHold) > 7) {
						m_fTimingPenHold = false;
                    }
				}
				break;

			case penUpEvent:
				// When the pen is released the timer is halted.

				m_fTimingPenHold = false;
				break;
			}
		} else {
			// penHoldEvents are only produced when no other events are queued.
			// This seems OK.

			if (m_fTimingPenHold && tCurrent - m_tPenDown >= kctPenHoldDelay) {
				m_fTimingPenHold = false;
				fHaveEvent = true;
				pevt->eType = penHoldEvent;
				pevt->idf = 0; // UNDONE: could have post-CookEvent/penDown below stash idf for use here
				pevt->x = m_xHold;
				pevt->y = m_yHold;
			}
		}

		// Check for timers to dispatch

		HostSoundServiceProc();
		tCurrent = gtimm.GetTickCount();
		if ((m_wfRedraw & (kfRedrawDirty | kfRedrawBeforeTimer)) !=
                (kfRedrawDirty | kfRedrawBeforeTimer)) {
			ctTimerNext = gtimm.ScanDispatch(tCurrent);
        }

		// If we have an event, cook it and return it
		
		if (fHaveEvent) {            
            // Cook and return
            
			gpmfrmm->CookEvent(pevt);
			switch (pevt->eType) {
			case appStopEvent:
				m_fAppStopping = true;
				break;

			case penMoveEvent:
				m_tLastMoveEvent = tCurrent;

				// fall through
			case penDownEvent:
				// Raise redraw priority while the pen is down

				m_wfRedraw |= kfRedrawBeforeInput;
				break;
			}
			return true;
		}

		// If asked to check for paints, do it here
		// Sometimes this is not done from here, like during game simulation

		if (fCheckPaints)
			gpmfrmm->CheckSetRedrawDirty();

		// If redraw is needed, now is the time

		if (m_wfRedraw & (kfRedrawDirty | kfRedrawMax)) {
			if (m_wfRedraw & kfRedrawMax) {
				m_wfRedraw &= ~(kfRedrawBeforeTimer | kfRedrawBeforeInput);
			} else {
				m_wfRedraw &= ~(kfRedrawDirty | kfRedrawBeforeTimer | kfRedrawBeforeInput);
			}

            // Is it within the max fps rate?

            if (CheckPaintFPS()) {
                m_wfRedraw &= ~kfRedrawPaintSkipped;
                pevt->eType = gamePaintEvent;
                return true;
            } else {
                m_wfRedraw |= kfRedrawPaintSkipped;
            }
		}

		// If the user timeout expired, return timeout

		tCurrent = gtimm.GetTickCount();
		ctElapsed = tCurrent - tStart;
		if (ctWait != -1) {
			if (ctElapsed >= ctWait)
				return false;
		}
	}
}

void EventMgr::UpdatePenHistory(Event *pevt)
{    
    bool fPenEvent = false;
    
    switch (pevt->eType) {
    case penDownEvent:
    case penMoveEvent:
    case penUpEvent:
        fPenEvent = true;
        if (m_fPenHistoryInitialized) {
            m_aevtPen1History[m_ievtPen1Next] = *pevt;            
            m_ievtPen1Next = (m_ievtPen1Next + 1) & (kcevtPenHistory - 1);
        }
        break;
        
    case penDownEvent2:
    case penMoveEvent2:
    case penUpEvent2:
        fPenEvent = true;
        if (m_fPenHistoryInitialized) {            
            m_aevtPen2History[m_ievtPen2Next] = *pevt;            
            m_ievtPen2Next = (m_ievtPen2Next + 1) & (kcevtPenHistory - 1);
        }
        break;
    }
    
    if (fPenEvent && !m_fPenHistoryInitialized) {
        for (int n = 0; n < ARRAYSIZE(m_aevtPen1History); n++) {
            m_aevtPen1History[n] = *pevt;
            m_aevtPen2History[n] = *pevt;
        }
        m_ievtPen1Next = 0;
        m_ievtPen2Next = 0;
        m_fPenHistoryInitialized = true;
    }
}

bool EventMgr::GetFlickVector(int nPen, FlickVector *pfliv)
{
    memset(pfliv, 0, sizeof(*pfliv));
    long msNow = HostGetMillisecondCount();    
    Point ptStart;    
    if (!QueryPenHistory(nPen, msNow - kcmsFlickQuantum, &ptStart))
        return false;
    Point ptEnd;
    if (!QueryPenHistory(nPen, msNow, &ptEnd))
        return false;
    pfliv->dx = ptEnd.x - ptStart.x;
    pfliv->dy = ptEnd.y - ptStart.y;
    pfliv->cms = kcmsFlickQuantum;
    return true;
}
    
bool EventMgr::QueryPenHistory(int nPen, long ms, Point *ppt)
{
    if (!m_fPenHistoryInitialized) {
        return false;
    }
    
    int ievtFirst, ievtLast;
    Event *aevt;
    if (nPen == 1) {
        aevt = m_aevtPen1History;
        ievtFirst = m_ievtPen1Next;                
        ievtLast = (m_ievtPen1Next - 1) & (kcevtPenHistory - 1);
    } else if (nPen == 2) {
        aevt = m_aevtPen2History;
        ievtFirst = m_ievtPen2Next;                
        ievtLast = (m_ievtPen2Next - 1) & (kcevtPenHistory - 1);
    } else {
        return false;
    }
    
    // Find event at time t

    Event *pevtOnOrBefore = NULL;
    Event *pevtOnOrAfter = NULL;
    for (int ievt = ievtFirst; true; ievt = (ievt + 1) & (kcevtPenHistory - 1)) {
        Event *pevt = &aevt[ievt];
        if (pevt->ms == ms) {
            pevtOnOrBefore = pevt;
            pevtOnOrAfter = NULL;
            break;
        }
        if (pevt->ms < ms) {
            pevtOnOrBefore = pevt;
        }
        if (pevt->ms > ms) {
            pevtOnOrAfter = pevt;
            break;
        }
        if (ievt == ievtLast)
             break;
    }
    
    if (pevtOnOrBefore == NULL && pevtOnOrAfter != NULL) {
        ppt->x = pevtOnOrAfter->x;
        ppt->y = pevtOnOrAfter->y;
        return true;
    }
    
    if (pevtOnOrBefore != NULL && pevtOnOrAfter == NULL) {
        ppt->x = pevtOnOrBefore->x;
        ppt->y = pevtOnOrBefore->y;
        return true;
    }
    
    // Calculate a point between these two events
    
    float flPercent = (float)(ms - pevtOnOrBefore->ms) / (float)(pevtOnOrAfter->ms - pevtOnOrBefore->ms);
    ppt->x = pevtOnOrBefore->x + (int)(flPercent * (pevtOnOrAfter->x - pevtOnOrBefore->x) + 0.5f);
    ppt->y = pevtOnOrBefore->y + (int)(flPercent * (pevtOnOrAfter->y - pevtOnOrBefore->y) + 0.5f);    
    
    return true;
}

bool EventMgr::CheckPaintFPS()
{
    long cmsCurrent = HostGetMillisecondCount();

    // First time through?

    if (gcmsNextPaint == 0) {
        gcmsNextPaint = cmsCurrent + gcmsDisplayUpdate;
        return true;
    } else if (cmsCurrent >= gcmsNextPaint) {

        // Is cmsCurrent is way ahead? This can occur after sleeping

        if (cmsCurrent >= gcmsNextPaint + gcmsDisplayUpdate * 5) {
            gcmsNextPaint = cmsCurrent + gcmsDisplayUpdate;
        } else {
            gcmsNextPaint += gcmsDisplayUpdate;
        }
        return true;
    }

    // There hasn't been enough time since the last draw

    return false;
}
    
void EventMgr::Init()
{
	m_fPeekKeep = false;
	m_cevt = 0;
	m_fAppStopping = false;
	m_wfRedraw = 0;
	m_fTimingPenHold = false;
	m_ctMoveEventInterval = 0;
	m_nctMoveEventFraction = 0;
    m_fPenHistoryInitialized = false;
}

bool EventMgr::DispatchEvent(Event *pevt)
{
	Form *pfrm = gpmfrmm->GetFormPtr(pevt->idf);
//	Trace("EventMgr::DispatchEvent: evt 0x%0lx, eType %d, idf %d", pevt, pevt->eType, pevt->idf); 
	if (pfrm != NULL)
		return pfrm->EventProc(pevt);
	return false;
}

void EventMgr::PostEvent(Event *pevt, bool fCoalesce)
{
	if (fCoalesce) {
		for (int nT = m_cevt - 1; nT >= 0; nT--) {
			if (m_aevtQ[nT].eType == pevt->eType) {
				m_aevtQ[nT] = *pevt;
				return;
			}
		}
		if (m_fPeekKeep && m_evtPeek.eType == pevt->eType) {
			m_evtPeek = *pevt;
			return;
		}
	}

	Assert(m_cevt < kcevtPostMax);
	m_aevtQ[m_cevt] = *pevt;
	m_cevt++;
}

void EventMgr::PostEvent(int eType, bool fCoalesce)
{
	Event evt;
	memset(&evt, 0, sizeof(evt));
	evt.eType = eType;
	PostEvent(&evt);
}

void EventMgr::SetPenEventInterval(word ctInterval)
{
	// Set the interval at which we'll accept pen events per second. This prevents the OS
	// from flooding unneeded events through the system.

	if (ctInterval >= 50)
		return;

	// Allow the interval to increase instantly but decrease more slowly. This way we get
	// instant game response and the performance doesn't ping-pong back and forth as the
	// interval gets adjusted.

	word nctFractionNew = ctInterval << 4;
	if (nctFractionNew > m_nctMoveEventFraction) {
		m_nctMoveEventFraction = nctFractionNew;
	} else {
		m_nctMoveEventFraction -= (m_nctMoveEventFraction - nctFractionNew) / 4;
	}

	// Divide the final by 2 so that the mouse event rate is twice the frame rate. This way
	// there will always be a new pen position available, at the cost of some event overhead.

	m_ctMoveEventInterval = (m_nctMoveEventFraction + 16) >> 5;
}

void EventMgr::ClearAppStopping()
{
	// Swallow the appStopEvent if it exists. Eating events may set fAppStopping again
	// so clear it constantly

	Event evt;
	while (GetEvent(&evt, 0, false))
		m_fAppStopping = false;

	// Shouldn't be needed but just in case it is set and GetEvent returns false

	m_fAppStopping = false;
}

} // namespace wi
