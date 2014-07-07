#include "ht.h"
#include "strings.h"

namespace wi {

//
// ReplicatorGob implementation
//

static AnimationData *s_panidActivator = NULL;
static BuilderConsts gConsts;

#if defined(DEBUG_HELPERS)
char *ReplicatorGob::GetName()
{
	return "Replicator";
}
#endif

bool ReplicatorGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtReplicator;
	gConsts.ut = kutReplicator;

	// Sound effects
	gConsts.sfxUnitReady = ksfxReplicatorBuild;

#if 0
	gConsts.sfxPowerOn = ksfxReplicatorOn;
	gConsts.sfxPowerOff = ksfxReplicatorOff;
	gConsts.sfxUnitBuildAbort = ksfxReplicatorAbortRecruiting;
	gConsts.sfxUnitReady = ksfxReplicatorUnitReady;
	gConsts.sfxAbortRepair = ksfxReplicatorAbortRepair;
	gConsts.sfxDamaged = ksfxReplicatorDamaged;
	gConsts.sfxDestroyed = ksfxReplicatorDestroyed;
	gConsts.sfxRepair = ksfxReplicatorRepair;
	gConsts.sfxSelect = ksfxReplicatorSelect;
#endif

	// Preload the Activator animation data

	if (s_panidActivator == NULL) {
		s_panidActivator = LoadAnimationData("activator.anir");
		if (s_panidActivator == NULL)
			return false;
	}
    
	return StructGob::InitClass(&gConsts, pini);
}

void ReplicatorGob::ExitClass()
{
	delete s_panidActivator;
	s_panidActivator = NULL;

	StructGob::ExitClass(&gConsts);
}

ReplicatorGob::ReplicatorGob() : StructGob(&gConsts)
{
	m_fEnabled = false;
	m_fReplicating = false;
	m_pplrNeedCredits = NULL;
}

bool ReplicatorGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	if (!StructGob::Init(wx, wy, pplr, fxHealth, ff, pszName))
		return false;

	// Clear out the Replicator's entrance and exits

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	ptrmap->ClearFlags(TcFromWc(m_wx) + kdtxReplicatorInput, TcFromWc(m_wy) + kdtyReplicatorInput, 1, 1, kbfStructure);
	ptrmap->ClearFlags(TcFromWc(m_wx) + kdtxReplicatorOutput1 - 1, TcFromWc(m_wy) + kdtyReplicatorOutput1 + 1, 1, 2, kbfStructure);
	ptrmap->ClearFlags(TcFromWc(m_wx) + kdtxReplicatorOutput2 + 1, TcFromWc(m_wy) + kdtyReplicatorOutput2 + 1, 1, 2, kbfStructure);

	return true;
}

#define knVerReplicatorGobState 3
bool ReplicatorGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerReplicatorGobState)
		return false;
	m_fEnabled = pstm->ReadByte() != 0;
	m_fReplicating = pstm->ReadByte() != 0;
	m_ifrmLights = pstm->ReadByte();
	s_cReplicators = pstm->ReadWord();
	pstm->Read(s_atptInput, sizeof(s_atptInput));
	Pid pid = pstm->ReadWord();
	if (pid != kpidNeutral)
		m_pplrNeedCredits = gplrm.GetPlayerFromPid(pid);

	return StructGob::LoadState(pstm);
}

bool ReplicatorGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerReplicatorGobState);
	pstm->WriteByte(m_fEnabled);
	pstm->WriteByte(m_fReplicating);
	pstm->WriteByte(m_ifrmLights);
	pstm->WriteWord(s_cReplicators);
	pstm->Write(s_atptInput, sizeof(s_atptInput));
	Pid pid = m_pplrNeedCredits == NULL ? kpidNeutral : m_pplrNeedCredits->GetId();
	pstm->WriteWord(pid);

	return StructGob::SaveState(pstm);
}

void ReplicatorGob::GetClippingBounds(Rect *prc)
{
	// Union of the two strips that combine to form the Replicator image

	m_pstruc->panid->GetBounds(0, 0, prc);
	Rect rcT;
	m_pstruc->panid->GetBounds(1, 0, &rcT);
	prc->Union(&rcT);

	// If selected, union with that rect

	if (gwfPerfOptions & kfPerfSelectionBrackets) {
		if (m_ff & kfGobSelected) {
			Rect rcSel;
			rcSel.FromWorldRect(&m_pstruc->wrcUIBounds);
//			rcSel.top -= kcyHealthBar;
			prc->Union(&rcSel);
		}
	}

	prc->Offset(PcFromUwc(m_wx), PcFromUwc(m_wy));
}

void ReplicatorGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	if (nLayer == knLayerDepthSorted) {
		int x = PcFromUwc(m_wx) - xViewOrigin;
		int y = PcFromUwc(m_wy) - yViewOrigin;

		Side side = m_pplr->GetSide();
		if (m_ff & kfGobDrawFlashed)
			side = (Side)-1;

		m_ani.SetFrame(0);
		m_ani.SetStrip(m_fEnabled ? 2 : 0);
		m_ani.Draw(pbm, x, y, side);
		m_ani.SetStrip(m_fEnabled ? 3 : 1);
#ifdef IPHONE
        // When this is scaled up by 1 1/3 (scaling 24 art to 32),
        // the upper right quadrant piece is 55 high, which translates after
        // rounding to 73, yet the lower right quad piece wants to go at
        // location 56 (effectively) which scales to 75, so there is a gap.

		m_ani.Draw(pbm, x, y - 1, side); // ugly hack
#else
		m_ani.Draw(pbm, x, y, side);
#endif

		if (m_fReplicating) {
			m_ani.SetStrip(m_pmuntc->panid->GetStripIndex("l 0"));
			m_ani.SetFrame(m_ifrmLights / 2);
			m_ani.Draw(pbm, x, y, side);
		}

	} else if (nLayer == knLayerSelection) {
		if (m_ff & kfGobSelected) {
			Rect rcT;
			rcT.FromWorldRect(&m_puntc->wrcUIBounds);
			rcT.Offset(-xViewOrigin + PcFromWc(m_wx), -yViewOrigin + PcFromWc(m_wy));
			DrawSelectionIndicator(pbm, &rcT, m_fxHealth, m_puntc->GetArmorStrength());
		}

	} else {
		StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);
	}
}

void ReplicatorGob::GetInputTilePosition(TPoint *ptpt)
{
	GetTilePosition(ptpt);
	ptpt->tx += kdtxReplicatorInput;
	ptpt->ty += kdtyReplicatorInput;
}

void ReplicatorGob::Enable(bool fEnable)
{
	if (m_fEnabled != fEnable) {
		gsndm.PlaySfx(fEnable ? ksfxReplicatorOn : ksfxReplicatorOff);
		m_fEnabled = fEnable;
		MarkRedraw();
	}
}

void ReplicatorGob::Activate()
{
	TPoint tpt;
	GetTilePosition(&tpt);
	AddReplicatorInputPoint(tpt.tx + kdtxReplicatorInput, tpt.ty + kdtyReplicatorInput);
	StructGob::Activate();
}

void ReplicatorGob::Deactivate()
{
	TPoint tpt;
	GetTilePosition(&tpt);
	RemoveReplicatorInputPoint(tpt.tx + kdtxReplicatorInput, tpt.ty + kdtyReplicatorInput);
	StructGob::Deactivate();
}

bool ReplicatorGob::ClearOutputBay(TCoord txBay, TCoord tyBay, TCoord txOut, TCoord tyOut)
{
	for (Gid gid = ggobm.GetFirstGid(txBay, tyBay); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
		MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(gid);
		if (pmunt == NULL)
			continue;
		if (!(pmunt->GetFlags() & kfGobMobileUnit))
			continue;

		// Don't ask it to move if it is already trying to

		if (pmunt->IsReadyForCommand() && !pmunt->IsMobile()) {
			Message msgT;
			msgT.mid = kmidMoveCommand;
			msgT.smidSender = m_gid;
			msgT.smidReceiver = pmunt->GetId();
			msgT.MoveCommand.wptTarget.wx = WcFromTc(txOut) + kwcTileHalf;
			msgT.MoveCommand.wptTarget.wy = WcFromTc(tyOut) + kwcTileHalf;
			msgT.MoveCommand.gidTarget = kgidNull;
			msgT.MoveCommand.wptTargetCenter.wx = msgT.MoveCommand.wptTarget.wx;
			msgT.MoveCommand.wptTargetCenter.wy = msgT.MoveCommand.wptTarget.wy;
			msgT.MoveCommand.tcTargetRadius = 0;
			msgT.MoveCommand.wcMoveDistPerUpdate = ((MobileUnitConsts *)pmunt->GetConsts())->GetMoveDistPerUpdate();
			gsmm.SendMsg(&msgT);
		}

		// Jammed

		return true;
	}

	// Maybe a unit is moving into the output bay (i.e., it is jammed)

	if (!IsTileFree(txBay, tyBay, kbfMobileUnit))
		return true; // jammed

	// Open
	
	return false;
}

int ReplicatorGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidPlaySfx)
		gsndm.PlaySfx((Sfx)pmsg->PlaySfx.sfx);

	State(kstIdle)
		OnUpdate
			// If a mobile unit is at either output, ask it to move on

			TPoint tpt;
			GetTilePosition(&tpt);
			Gid gid;

			// Clear left and right output bays
			Player *pplrNeedCredits = NULL;
			bool fJammed = ClearOutputBay(tpt.tx + kdtxReplicatorOutput1, tpt.ty + kdtyReplicatorOutput1, tpt.tx + kdtxReplicatorOutput1 - 1, tpt.ty + kdtyReplicatorOutput1 + 1);
			fJammed = ClearOutputBay(tpt.tx + kdtxReplicatorOutput2, tpt.ty + kdtyReplicatorOutput2, tpt.tx + kdtxReplicatorOutput2 + 1, tpt.ty + kdtyReplicatorOutput2 + 1) || fJammed;

			// If there's nothing clogging the outputs we can consider the input
			if (m_fEnabled && !fJammed) {
				// loop through all the gobs on this tile to find the mobile unit and ignore ourself, or
				// shots flying by
				for (gid = ggobm.GetFirstGid(tpt.tx + kdtxReplicatorInput, tpt.ty + kdtyReplicatorInput); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
					MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(gid);
					if (pmunt == NULL)
						continue;
					if (!(pmunt->GetFlags() & kfGobMobileUnit))
						continue;

					// Something there! If it's ready, clone it.

					if (!(pmunt->GetMobileUnitFlags() & kfMuntAtReplicatorInput))
						continue;

					// At the limit?
// TUNE:
#define kctIntervalLimitNotify 600

					if (!ggobm.IsBelowLimit(knLimitMobileUnit, pmunt->GetOwner())) {
						if (pmunt->GetOwner() == gpplrLocal) {
							static long s_tLastNotify;
							long tCurrent = gtimm.GetTickCount();
							if (abs(s_tLastNotify - tCurrent) >= kctIntervalLimitNotify) {
								s_tLastNotify = tCurrent;
								ShowAlert(kidsUnitLimitReached);
							}
						}
						continue;
					}

					// Does the player have enough credits to perform the replication?

					MobileUnitConsts *pmuntc = (MobileUnitConsts *)pmunt->GetConsts();
					Player *pplr = pmunt->GetOwner();
					long cCredits = pplr->GetCredits();

					int nCost = pmuntc->GetCost();
					int cReplicationCost = GetReplicationCost(nCost);
					if (cCredits < cReplicationCost) {
						pplrNeedCredits = pplr;
						break; 
					}

					// Take the money!

					pplr->SetCredits(cCredits - cReplicationCost, true);

					// Start the replicating animation

					m_fReplicating = true;
					m_ifrmLights = -1;

					MobileUnitGob *pmuntClone = (MobileUnitGob *)CreateGob(pmunt->GetType());
					if (pmuntClone != NULL) {
						pmuntClone->Init(WcFromTc(tpt.tx + kdtxReplicatorOutput2), WcFromTc(tpt.ty + kdtyReplicatorOutput2), pmunt->GetOwner(), pmunt->GetHealth(), 0, NULL);

						// Clone acquires the selection state of the original

						if (pmunt->GetFlags() & kfGobSelected)
							pmuntClone->Select(true);

						// Replicated GalaxMiners lose their load of Galaxite
						// UNDONE: if there is more of this special casing go with virtual UnitGob::Replicate()

						if (pmunt->GetType() == kgtGalaxMiner)
							((MinerGob *)pmunt)->SetGalaxiteAmount(0);

						// If this unit is a member of a group add its clone to the group too

						UnitGroup *pug = gsim.GetLevel()->GetUnitGroupMgr()->GetUnitGroup(pmunt->GetId());
						if (pug != NULL)
							pug->AddUnit(pmuntClone, true);
					}

					// Warp the original Unit to the left output port

					TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
					ptrmap->ClearFlags(tpt.tx + kdtxReplicatorInput, tpt.ty + kdtyReplicatorInput, 1, 1, kbfMobileUnit);
					ptrmap->SetFlags(tpt.tx + kdtxReplicatorOutput1, tpt.ty + kdtyReplicatorOutput1, 1, 1, kbfMobileUnit);
					pmunt->SetPosition(WcFromTc(tpt.tx + kdtxReplicatorOutput1) + kwcTileHalf, WcFromTc(tpt.ty + kdtyReplicatorOutput1) + kwcTileHalf);

					// Clear the bit

					pmunt->SetMobileUnitFlags(pmunt->GetMobileUnitFlags() & ~kfMuntAtReplicatorInput);

					// Let player know the replication process has happened
					// play cool replication sound effect

					gsndm.PlaySfx(ksfxReplicatorBuild);

					// wait a quarter second and let the new unit announce itself

					Sfx sfx = SfxFromCategory(pmuntc->sfxcSelect);
					Message msgT;
					memset(&msgT, 0, sizeof(msgT));
					msgT.mid = kmidPlaySfx;
					msgT.smidSender = m_gid;
					msgT.smidReceiver = m_gid;
					msgT.PlaySfx.sfx = sfx;
					gsmm.SendDelayedMsg(&msgT, 24);

					// Play it again in another quarter of a second to sound cool

					memset(&msgT, 0, sizeof(msgT));
					msgT.mid = kmidPlaySfx;
					msgT.smidSender = m_gid;
					msgT.smidReceiver = m_gid;
					msgT.PlaySfx.sfx = sfx;
					gsmm.SendDelayedMsg(&msgT, 48);
					break;
				}
			}

			if (m_fReplicating) {
				m_ifrmLights++;
				int nStrip = m_pmuntc->panid->GetStripIndex("l 0");
				if (m_ifrmLights / 2 >= m_pmuntc->panid->GetFrameCount(nStrip)) {
					m_ifrmLights = -1;
					m_fReplicating = false;
				}
				MarkRedraw();
			}
			WaitingForCredits(pplrNeedCredits != NULL, false, pplrNeedCredits);
			m_unvl.MinSkip();
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

void ReplicatorGob::WaitingForCredits(bool fNeed, bool fOverride, Player *pplr)
{
	// we need to be using the unit's player, not our own. Remember so when the unit
	// disappears we can decrement the right player's count

	if (fNeed) {
		Assert((m_pplrNeedCredits == NULL) || (m_pplrNeedCredits == pplr) || (fOverride));
		if (fOverride) 
			pplr = m_pplrNeedCredits;
		else
			m_pplrNeedCredits = pplr;
	} else {
		Assert(pplr == NULL);
		pplr = m_pplrNeedCredits;
		m_pplrNeedCredits = NULL;
	}

	StructGob::WaitingForCredits(fNeed, fOverride, pplr);
}


// Replicator input point management

int ReplicatorGob::s_cReplicators;
TPoint ReplicatorGob::s_atptInput[10];

void ReplicatorGob::AddReplicatorInputPoint(TCoord tx, TCoord ty)
{
	Assert(s_cReplicators < ARRAYSIZE(s_atptInput) - 1);
	if (s_cReplicators >= ARRAYSIZE(s_atptInput) - 1)
		return;
	s_atptInput[s_cReplicators].tx = tx;
	s_atptInput[s_cReplicators].ty = ty;
	s_cReplicators++;
}

void ReplicatorGob::RemoveReplicatorInputPoint(TCoord tx, TCoord ty)
{
	for (int n = 0; n < ARRAYSIZE(s_atptInput); n++) {
		if (s_atptInput[n].tx == tx && s_atptInput[n].ty) {
			memmove(&s_atptInput[n], &s_atptInput[n + 1], (s_cReplicators - n - 1) * ELEMENTSIZE(s_atptInput));
			s_cReplicators--;
			return;
		}
	}
	Assert();
}

//
// ActivatorGob implementation
//

#if defined(DEBUG_HELPERS)
char *ActivatorGob::GetName()
{
	return "Activator";
}
#endif

bool ActivatorGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	int side, tx, ty;
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%d ,%d ,%d", &side, &tx, &ty);
	if (cArgs < 3) {
		Assert("ActivatorGob requires at least 3 valid initialization parameters");
		return false;
	}

	// Keep players from building on top of Activators

	gsim.GetLevel()->GetTerrainMap()->SetFlags(tx, ty, 1, 1, kbfReserved);

	return AnimGob::Init(WcFromTc(tx), WcFromTc(ty), kfAnmLoop | kfAnmSurfaceDecalLayer, NULL, s_panidActivator, 0, ksmidNull, pszName);
}

#define knVerActivatorGobState 1
bool ActivatorGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerActivatorGobState)
		return false;
	m_fActivated = pstm->ReadByte() != 0;
	m_ani.Init(s_panidActivator);
	m_ani.LoadState(pstm);
	m_wfAnm = kfAnmLoop | kfAnmSurfaceDecalLayer;
	return AnimGob::LoadState(pstm);
}

bool ActivatorGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerActivatorGobState);
	pstm->WriteByte(m_fActivated);
	m_ani.SaveState(pstm);
	return AnimGob::SaveState(pstm);
}

bool ActivatorGob::IsSavable()
{
	// Because AnimGob is not savable, we need to override

	return true;
}

GobType ActivatorGob::GetType()
{
	return kgtActivator;
}

int ActivatorGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnUpdate
		bool fActivated = false;
		for (Gid gid = ggobm.GetFirstGid(TcFromWc(m_wx), TcFromWc(m_wy)); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
			MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(gid);
			if (pmunt == NULL)
				continue;
			if (!(pmunt->GetFlags() & kfGobMobileUnit))
				continue;
			fActivated = true;
			break;
		}

		if (fActivated != m_fActivated) {
			m_fActivated = fActivated;
			StartAnimation(&m_ani, fActivated ? 1 : 0, 0, kfAniLoop | kfAnmSurfaceDecalLayer);
			gsndm.PlaySfx(fActivated ? ksfxActivatorOn : ksfxActivatorOff);
			
		}

		// Check every 3 updates for the presence of a MobileUnit on top of the Activator

		m_unvl.MinSkip(3);

		return AnimGob::ProcessStateMachineMessage(st, pmsg);

#if 0
EndStateMachineInherit(AnimGob)
#else
            return knHandled;
        }
    } else {
        return (int)AnimGob::ProcessStateMachineMessage(st, pmsg); 
    }
    return (int)AnimGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

} // namespace wi
