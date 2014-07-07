#include "ht.h"

namespace wi {

//
// Alerts
//

void ShowAlert(int id)
{
	char sz[kcchAlertText];
	if (gpstrtbl == NULL)
		return;
	gpstrtbl->GetString(id, sz, sizeof(sz));
	ShowAlert(sz);
}

void ShowAlert(const char *psz)
{
	SimUIForm *pfrm = ggame.GetSimUIForm();
	if (pfrm == NULL)
		return;
	((AlertControl *)pfrm->GetControlPtr(kidcAlert))->AddText(psz);
}

//
// AlertControl
//

AlertControl::AlertControl()
{
	m_wf = 0;
}

#define kfTimerSet 1
AlertControl::~AlertControl()
{
	if (m_wf & kfTimerSet)
		gtimm.RemoveTimer(this);
}

bool AlertControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	char szT[1];
	szT[0] = 0;
	return LabelControl::Init(kifntShadow, szT, szT, szT);
}

#define kctInterval 400 // 400 ticks, or 4 seconds
void AlertControl::AddText(const char *psz)
{
	// Add new Alert

	SetText(psz);
	if (m_wf & kfTimerSet) {
		gtimm.SetTimerRate(this, kctInterval);
	} else {
		gtimm.AddTimer(this, kctInterval);
		m_wf |= kfTimerSet;
	}
	Show(true);
}

void AlertControl::OnTimer(long tCurrent)
{
	// on timer firing the status should no longer be displayed

	Assert((m_wf & kfTimerSet) != 0);
	Show(false);
	gtimm.RemoveTimer(this);
	m_wf &= ~kfTimerSet;
}

} // namespace wi
