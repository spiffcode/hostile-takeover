#include "ht.h"

namespace wi {

//
// WarehouseGob implementation
//

static StructConsts gConsts;

//
// Gob methods
//

bool WarehouseGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtWarehouse;
	gConsts.ut = kutWarehouse;
	gConsts.umPrerequisites = kumReactor | kumProcessor;
	gConsts.wf |= kfUntcHasFullnessIndicator;

	// Sound effects

	gConsts.sfxAbortRepair = ksfxGalaxiteWarehouseAbortRepair;
	gConsts.sfxRepair = ksfxGalaxiteWarehouseRepair;
	gConsts.sfxDamaged = ksfxGalaxiteWarehouseDamaged;
	gConsts.sfxSelect = ksfxGalaxiteWarehouseSelect;
	gConsts.sfxDestroyed = ksfxGalaxiteWarehouseDestroyed;
	gConsts.sfxImpact = ksfxNothing;

	return StructGob::InitClass(&gConsts, pini);
}

void WarehouseGob::ExitClass()
{
	StructGob::ExitClass(&gConsts);
}

WarehouseGob::WarehouseGob() : StructGob(&gConsts)
{
}

// Override Takeover to transfer funds to the new owner

void WarehouseGob::Takeover(Player *pplr)
{
	// Takeover the credits this Warehouse 'owns'

	long cCreditsTaken = CalcCreditsShare(m_pplr);
	m_pplr->SetCredits(m_pplr->GetCredits() - cCreditsTaken, true);
	pplr->SetCredits(pplr->GetCredits() + cCreditsTaken, true);

	// Takeover the Warehouse itself

	StructGob::Takeover(pplr);
}

void WarehouseGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	// Don't waste time drawing a dead Warehouse's fullness indicator, etc

	if (!(m_ff & kfGobActive)) {
		StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);
	} else {

		long nCapacity = m_pplr->GetCapacity();

		// OPT: add a Player::GetFullnessPips to cache this calculation and have 
		// SetCapacity/SetCredits set a recalc bit.

		int nPips = 0;
		if (nCapacity != 0) {
			nPips = ((m_pplr->GetCredits() * 10) + (nCapacity / 20)) / nCapacity;
			if (nPips > 10)	// over capacity!
				nPips = 10;
		}

		// Select the appropriate frame depending on how close the player is to capacity

		SetAnimationFrame(&m_ani, nPips / 3); // 0,1,2=empty, 3,4,5=half full, 6,7,8=mostly full, 9,10=full

		StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

		if (nLayer == knLayerSelection && (m_ff & kfGobSelected)) {
			WRect wrcT;
			GetUIBounds(&wrcT);
			Rect rcT;
			rcT.FromWorldRect(&wrcT);
			rcT.Offset(-xViewOrigin, -yViewOrigin);
			DrawFullnessIndicator(pbm, &rcT, nPips, 10);
		}
	}
}

//
// StateMachine methods
//

#if defined(DEBUG_HELPERS)
char *WarehouseGob::GetName()
{
	return "Warehouse";
}
#endif

// UNDONE: when a Warehouse is destroyed some amount of the Galaxite
// is carrying should be lost

int WarehouseGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	//-----------------------------------------------------------------------

	State(kstIdle)
		// Override to avoid animating through the Galaxite level frames
		OnUpdate
			DefUpdate();

	//-----------------------------------------------------------------------
	

	State(kstDying)
		OnEnter
			// Sorry, player must lose some Credits along with this Warehouse

			if (m_ff & kfGobActive)
				m_pplr->SetCredits(m_pplr->GetCredits() - CalcCreditsShare(m_pplr), true);

			return StructGob::ProcessStateMachineMessage(st, pmsg);

		// Override to avoid animating through the Galaxite level frames
		OnUpdate

	//-----------------------------------------------------------------------

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