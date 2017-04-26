#include "ht.h"

namespace wi {

static MobileUnitConsts gConsts;

#if defined(DEBUG_HELPERS)
char *MobileHqGob::GetName()
{
	return "MobileHQ";
}
#endif

static int s_anMovingStripIndices[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

bool MobileHqGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtMobileHeadquarters;
	gConsts.ut = kutMobileHeadquarters;
	gConsts.umPrerequisites = kumResearchCenter;
	gConsts.upgmPrerequisites = kupgmAdvancedVTS;

	// Initialize the frame indices arrays

	gConsts.anFiringStripIndices = s_anMovingStripIndices;
	gConsts.anMovingStripIndices = s_anMovingStripIndices;
	gConsts.anIdleStripIndices = s_anMovingStripIndices;

	// Sound effects

	gConsts.sfxFire = ksfxNothing;
	gConsts.sfxImpact = ksfxNothing;

	gConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gConsts.sfxcSelect = ksfxcMale01Select;
	gConsts.sfxcMove = ksfxcMale01Move;
	gConsts.sfxcAttack = ksfxcMale01Attack;

	return MobileUnitGob::InitClass(&gConsts, pini);
}

void MobileHqGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gConsts);
}

MobileHqGob::MobileHqGob() : MobileUnitGob(&gConsts)
{
	m_wfMunt &= ~kfMuntAggressivenessBits;
}

bool MobileHqGob::CanTransform(TPoint *ptp)
{
	// optionally return the point we decide is ok so we can use it when we 
	// decide to transform

	Assert(gbldcHq.ctx == 3 && gbldcHq.cty == 2);	// Because of the various '-1's sprinkled below

	// Can't transform on top of Galaxite. Check for it. watch the map edge too.

	TPoint tpt;
	GetTilePosition(&tpt);
	TCoord tx = tpt.tx - 1;
	TCoord ty = tpt.ty - 1;
	if (tx < 0)
		tx = 0;
	if (ty < 0)
		ty = 0;
	TCoord txR = tx + gbldcHq.ctx;
	TCoord tyB = ty + gbldcHq.cty;

	// this logic works because we won't actually occupy the spot in txR, just up to it.
	Size siz;
	gsim.GetLevel()->GetTileMap()->GetTCoordMapSize(&siz);
	if (txR > siz.cx) {
		txR = siz.cx;
		tx = txR - gbldcHq.ctx;
	}
	if (tyB > siz.cy) {
		tyB = siz.cy;
		ty = tyB - gbldcHq.cty;
	}
	if (ptp != NULL) {
		ptp->tx = tx;
		ptp->ty = ty;
	}
	bool fOccupied = false;
	TCoord txT = tx;
	FogMap *pfogm = gsim.GetLevel()->GetFogMap();
	for (; ty < tyB; ty++) {
		for (tx = txT; tx < txR; tx++) {
			Gob *pgob;
			if (!IsTileFree(tx, ty, kbfReserved | kbfStructure, &pgob)) {
				if (pgob != this) {
					fOccupied = true;
					break;
				}
			}
			if (pfogm->GetGalaxite(tx, ty) != 0) {
				fOccupied = true;
				break;
			}
		}
	}

	return !fOccupied;
}

void MobileHqGob::InitMenu(Form *pfrm)
{
	ButtonControl *pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcTransform);
	LabelControl *plbl = (LabelControl *)pfrm->GetControlPtr(kidcCantTransform);
	bool fCanTransform = CanTransform();
	pbtn->Show(fCanTransform);
	plbl->Show(!fCanTransform);
}

void MobileHqGob::OnMenuItemSelected(int idc)
{
	switch (idc) {
	case kidcTransform:
		{
			// The player shouldn't have been able to select this command unless
			// it can be carried out.

			Assert(CanTransform());

			gcmdq.Enqueue(kmidTransformCommand, m_gid);
		}
		break;

	case kidcSelfDestruct:
		gcmdq.Enqueue(kmidSelfDestructCommand, m_gid);
		break;
	}
}

int MobileHqGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidTransformCommand)

		// Due to client-server lag the space wanted for transformation
		// may no longer be free. Double-check and if it isn't simply fail.
		// UNDONE: bzzzt

		TPoint tptDest;
		if (!CanTransform(&tptDest))
			return knHandled;

		// Deactivate the MobileHQ

		Deactivate();

		// UNDONE: do some cool animation

		// Remove self from the Gob list

		ggobm.RemoveGob(this);

		// Start sfx

		if (m_pplr == gpplrLocal)
			gsndm.PlaySfx(ksfxMobileHeadquartersDeploy);

		// Create Headquarters

		HqGob *pgobHq = (HqGob *)CreateGob(kgtHeadquarters);
		if (pgobHq != NULL) {

			// Carry the MHQ's health forward to the HQ

			fix fxHealth = (fix)divfx(mulfx(m_fxHealth, gapuntc[kutHeadquarters]->GetArmorStrength()), m_puntc->GetArmorStrength());
			pgobHq->Init(WcFromTc(tptDest.tx), WcFromTc(tptDest.ty), m_pplr, fxHealth, 0, NULL);
		}

		// Delete self

		delete this;
		return knDeleted;

	OnMsg(kmidAttackCommand)

		// UNDONE: MobileHqs don't have an attack at the moment so they ignore
		// their auto-response impulses to chase their attacker

		// DO NOTHING (override Unit's Attack response)

#if 0
EndStateMachineInherit(MobileUnitGob)
#else
            return knHandled;
        }
    } else {
        return (int)MobileUnitGob::ProcessStateMachineMessage(st, pmsg); 
    }
    return (int)MobileUnitGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

} // namespace wi