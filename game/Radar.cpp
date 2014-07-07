#include "ht.h"

namespace wi {

//
// RadarGob implementation
//

static StructConsts gConsts;

//
// Gob methods
//

bool RadarGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtRadar;
	gConsts.ut = kutRadar;
	gConsts.umPrerequisites = kumReactor;

	// Sound effects

	gConsts.sfxAbortRepair = ksfxRadarAbortRepair;
	gConsts.sfxRepair = ksfxRadarRepair;
	gConsts.sfxDamaged = ksfxRadarDamaged;
	gConsts.sfxSelect = ksfxRadarSelect;
	gConsts.sfxDestroyed = ksfxRadarDestroyed;
	gConsts.sfxImpact = ksfxNothing;

	// Wants power notification

	gConsts.wf |= kfUntcNotifyPowerLowHigh;

	return StructGob::InitClass(&gConsts, pini);
}

void RadarGob::ExitClass()
{
	StructGob::ExitClass(&gConsts);
}

RadarGob::RadarGob() : StructGob(&gConsts)
{
}

// Struct gob methods

void RadarGob::Activate()
{
	StructGob::Activate();
	if (gpplrLocal == m_pplr && m_pplr->GetUnitCount(kutRadar) == 1)
		gpmm->CalcPoweredRadar();
}

void RadarGob::Deactivate()
{
	StructGob::Deactivate();
	if (gpplrLocal == m_pplr && m_pplr->GetUnitCount(kutRadar) == 0)
		gpmm->CalcPoweredRadar();
}

//
// StateMachine methods
//

#if defined(DEBUG_HELPERS)
char *RadarGob::GetName()
{
	return "Radar";
}
#endif

int RadarGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidPlaySfx)
        if (gpplrLocal == m_pplr) {
            gsndm.PlaySfx((Sfx)pmsg->PlaySfx.sfx);
        }

	State(kstBeingBuilt)
		OnMsg(kmidBuildComplete)

			// wait a bit, then announce new structure options if 
			// this is our first radar

			if (m_pplr->GetUnitCount(kutRadar) == 0) {
				Message msgT;
				memset(&msgT, 0, sizeof(msgT));
				msgT.mid = kmidPlaySfx;
				msgT.smidSender = m_gid;
				msgT.smidReceiver = m_gid;
				msgT.PlaySfx.sfx = ksfxGameNewStructureOptions;
				gsmm.SendDelayedMsg(&msgT, 96);
			}

			return StructGob::ProcessStateMachineMessage(st, pmsg);

	State(kstIdle)
		OnUpdate
			// Don't animate if power is too low
			// This relies on kmidPowerLowHigh to wake up if power situation changes

			if (!m_pplr->IsPowerLow())
				AdvanceAnimation(&m_ani);

			DefUpdate();

#if 0
EndStateMachineInherit(StructGob)
#else
            return knHandled;
        }
    } else {
        return (int)StructGob::ProcessStateMachineMessage(st, pmsg); 
    }
    return (int)StructGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

} // namespace wi
