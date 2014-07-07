#include "ht.h"
#include "base/messagequeue.h"

namespace wi {

//#define TIME_STEPPER

#ifdef TIME_STEPPER
// This makes time "stop" when in the debugger
class TimeStepper : public base::MessageHandler {
public:
    TimeStepper() : m_ct(0), m_fInitialized(false) {}

    long GetTickCount() {
        if (!m_fInitialized) {
            OneShot();
            m_fInitialized = true;
        }
        return m_ct;
    }

private:
    void OneShot() {
        base::Message msg;
        msg.handler = this;
        thread_.PostDelayed(&msg, 1);
    }

    virtual void OnMessage(base::Message *pmsg) {
        m_ct++;
        OneShot();
    }

    long m_ct;
    bool m_fInitialized;
};
#endif

TimerMgr::TimerMgr()
{
	m_fEnabled = true;
	m_ptmrFirst = NULL;
	m_ptmrNotifying = NULL;
	m_ptmrTriggerNext = NULL;
	m_fForceScan = true;
}

TimerMgr::~TimerMgr()
{
	Assert(m_ptmrFirst == NULL);
}

long TimerMgr::GetTickCount()
{
#ifdef TIME_STEPPER
    // Makes time stop while in the debugger, for easier debugging

    static TimeStepper stepper;
    return stepper.GetTickCount();
#else
	return HostGetTickCount();
#endif
}

long TimerMgr::ScanDispatch(long tCurrent)
{
	if (!m_fEnabled)
		return -1;

	// If currently scanning and being re-entered, disallow it.

	Assert(m_ptmrNotifying == NULL);
	if (m_ptmrNotifying != NULL)
		return -1;

	// If the timer state hasn't changed since the last scan, we can exit if not
	// enough time has elapsed for a timer to go off.

	if (!m_fForceScan) {
		if (m_ptmrTriggerNext != NULL) {
			long ctT = m_ptmrTriggerNext->m_tTrigger - tCurrent;
			if (ctT > 0)
				return ctT;
		}
	}
	m_fForceScan = false;

	// Scan timers to look for next timer to go off.
	// Unfortunately these can't be sorted since the firing order is unique for any
	// point in time.

	Timer *ptmrT;
	for (ptmrT = m_ptmrFirst; ptmrT != NULL; ) {

        // If the timer has triggered, set its next trigger time to its current
        // trigger time plus its rate. This is an attempt at keeping timing
        // consistent and predictable. However, if this new time is less than
        // or equal to the current real time we set it to the real time so we
        // don't get stuck in a loop only processing timers.

		bool fRemoved = false;
		Timer *ptmrNext = ptmrT->m_ptmrNext;

		if (ptmrT->m_tTrigger <= tCurrent) {

			// Remember that this timer is notifying.

			m_ptmrNotifying = ptmrT;
			ptmrT->OnTimer(tCurrent);
			m_ptmrNotifying = NULL;

			fRemoved = ptmrT->m_tTrigger == -1;

			if (!fRemoved) {
                // Reset the rate after notification in case the rate was
                // changed during notification.

				ptmrT->m_tTrigger += ptmrT->m_ctRate;

				// If the timer is falling behind real time catch it up

				tCurrent = GetTickCount();
				if (ptmrT->m_tTrigger < tCurrent)
					ptmrT->m_tTrigger = tCurrent;
			}
		}

		ptmrT = ptmrNext;
	}

	// Determine the next timer to trigger. This is done outside the
	// above loop because timers, including the m_ptmrTriggerNext, may
	// be removed during the OnTimer callback.

	m_ptmrTriggerNext = m_ptmrFirst;
	for (ptmrT = m_ptmrFirst; ptmrT != NULL; ptmrT = ptmrT->m_ptmrNext) {
		if (ptmrT->m_tTrigger < m_ptmrTriggerNext->m_tTrigger)
			m_ptmrTriggerNext = ptmrT;
	}

	// Return time till next timer

	if (m_ptmrTriggerNext == NULL)
		return -1;
	long ct = tCurrent - m_ptmrTriggerNext->m_tTrigger;
	if (ct < 0)
		return 0;
	return ct;
}

void TimerMgr::AddTimer(Timer *ptmr, long ct)
{
	for (Timer *ptmrT = m_ptmrFirst; ptmrT != NULL; ptmrT = ptmrT->m_ptmrNext) {
		Assert(ptmrT != ptmr, "Timer already added!");
		if (ptmrT == ptmr)
			return;
	}

	// Link it into the list

	if (m_ptmrFirst != NULL)
		m_ptmrFirst->m_ptmrPrev = ptmr;
	ptmr->m_ptmrPrev = NULL;

	ptmr->m_ptmrNext = m_ptmrFirst;
    ptmr->m_ptimm = this;
	m_ptmrFirst = ptmr;

	SetTimerRate(ptmr, ct);
}

void TimerMgr::RemoveTimer(Timer *ptmr)
{
	bool fFound = false;
	for (Timer *ptmrT = m_ptmrFirst; ptmrT != NULL; ptmrT = ptmrT->m_ptmrNext) {
		if (ptmrT == ptmr) {
			fFound = true;
			break;
		}
	}
	Assert(fFound, "Timer not on list! (already removed?)");
	if (!fFound)
		return;

	ptmr->m_tTrigger = -1;
    ptmr->m_ptimm = NULL;

	if (ptmr->m_ptmrPrev != NULL) {
		ptmr->m_ptmrPrev->m_ptmrNext = ptmr->m_ptmrNext;
	} else {
		m_ptmrFirst = ptmr->m_ptmrNext;
	}

	if (ptmr->m_ptmrNext != NULL)
		ptmr->m_ptmrNext->m_ptmrPrev = ptmr->m_ptmrPrev;

	if (m_ptmrNotifying != ptmr && m_ptmrTriggerNext == ptmr) {
		m_ptmrTriggerNext = NULL;
		m_fForceScan = true;
	}
}

void TimerMgr::SetTimerRate(Timer *ptmr, long ct)
{
	// If the timer rate is decreasing, this may change the time till the next
	// timer goes off, so force a scan next time.

	ptmr->m_ctRate = ct;
	ptmr->m_tTrigger = GetTickCount() + ct;
	m_fForceScan = true;
}

void TimerMgr::BoostTimer(Timer *ptmr, long ct) {
    ptmr->m_tTrigger += ct;
	m_fForceScan = true;
}

// Force this timer to be dispatched at the next call to TimerMgr::ScanDispatch 
// (presumably by EventMgr::GetEvent)

void TimerMgr::TriggerTimer(Timer *ptmr)
{
	ptmr->m_tTrigger = GetTickCount();
	m_fForceScan = true;
}

bool TimerMgr::IsAdded(Timer *ptmr)
{
    Timer *ptmrT = m_ptmrFirst;
    while (ptmrT != NULL) {
        if (ptmrT == ptmr) {
            break;
        }
        ptmrT = ptmrT->m_ptmrNext;
    }
    return ptmrT == ptmr;
}

Timer::~Timer() {
    if (m_ptimm != NULL && m_ptimm->IsAdded(this)) {
        Assert("Timer being destroyed, but still in list!");
    }
}

} // namespace wi
