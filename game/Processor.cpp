#include "ht.h"
#include "wistrings.h"

namespace wi {

//
// ProcessorGob implementation
//

static StructConsts gConsts;

//
// Gob methods
//

bool ProcessorGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtProcessor;
	gConsts.ut = kutProcessor;
	gConsts.umPrerequisites = kumReactor;
	gConsts.wf |= kfUntcHasFullnessIndicator;

	// Sound effects

	gConsts.sfxAbortRepair = ksfxGalaxiteProcessorAbortRepair;
	gConsts.sfxRepair = ksfxGalaxiteProcessorRepair;
	gConsts.sfxDamaged = ksfxGalaxiteProcessorDamaged;
	gConsts.sfxSelect = ksfxGalaxiteProcessorSelect;
	gConsts.sfxDestroyed = ksfxGalaxiteProcessorDestroyed;
	gConsts.sfxImpact = ksfxNothing;

	return StructGob::InitClass(&gConsts, pini);
}

void ProcessorGob::ExitClass()
{
	StructGob::ExitClass(&gConsts);
}

ProcessorGob::ProcessorGob() : StructGob(&gConsts)
{
	m_aniOverlay.Init(m_pstruc->panid);
	StartAnimation(&m_aniOverlay, 3, 0, 0);
	m_gidMiner = kgidNull;
	m_wptFakeMiner.wx = kwxInvalid;
	m_fProcessingAnimationInProgress = false;
}

#define knVerProcessorGobState 2
bool ProcessorGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerProcessorGobState)
		return false;
	m_gidMiner = pstm->ReadWord();
	m_wptFakeMiner.wx = pstm->ReadWord();
	m_wptFakeMiner.wy = pstm->ReadWord();
	m_fProcessingAnimationInProgress = pstm->ReadByte() != 0 ? true : false;
	m_fDoorMoving = pstm->ReadByte() != 0 ? true : false;
	m_aniOverlay.LoadState(pstm);
	return StructGob::LoadState(pstm);
}

bool ProcessorGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerProcessorGobState);
	pstm->WriteWord(m_gidMiner);
	pstm->WriteWord(m_wptFakeMiner.wx);
	pstm->WriteWord(m_wptFakeMiner.wy);
	pstm->WriteByte(m_fProcessingAnimationInProgress);
	pstm->WriteByte(m_fDoorMoving);
	m_aniOverlay.SaveState(pstm);
	return StructGob::SaveState(pstm);
}

int CalcCreditsShare(Player *pplr)
{
	int cWarehouses = pplr->GetUnitCount(kutWarehouse);
	int cProcessors = pplr->GetUnitCount(kutProcessor);
	
	// Takeover the credits this Processor 'owns'

	return pplr->GetCredits() / (cWarehouses + cProcessors);
}

void ProcessorGob::GetClippingBounds(Rect *prc)
{
	UnitGob::GetClippingBounds(prc);

	if (m_wptFakeMiner.wx != kwxInvalid) {
		int ifrm = m_wptFakeMiner.wy < m_wy + kwcTile ? 1 : 0;
		int xMiner = PcFromUwc(m_wptFakeMiner.wx);
		int yMiner = PcFromWc(m_wptFakeMiner.wy);
		Rect rcFakeMiner;
		m_pstruc->panid->GetBounds(4, ifrm, &rcFakeMiner);
		rcFakeMiner.Offset(xMiner, yMiner);
		prc->Union(&rcFakeMiner);
	}
}

bool ProcessorGob::IsTakeoverable(Player *pplr)
{
	// Have to make sure there is limit space for the miner too

	if (m_gidMiner != kgidNull) {
		MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);	
		if (pgobMiner != NULL) {
			if (!ggobm.IsBelowLimit(knLimitMobileUnit, pplr)) {
				if (pplr == gpplrLocal)
					ShowAlert(kidsUnitLimitReached);
				return false;
			}
		}
	}
	return StructGob::IsTakeoverable(pplr);
}

// Override Takeover to funds to the new owner

void ProcessorGob::Takeover(Player *pplr)
{
	// Takeover the credits this Processor 'owns'

	int cCreditsTaken = CalcCreditsShare(m_pplr);
	m_pplr->SetCredits(m_pplr->GetCredits() - cCreditsTaken, true);
	pplr->SetCredits(pplr->GetCredits() + cCreditsTaken, true);

	// if there's a miner parked inside, winner!

	if (m_gidMiner != kgidNull) {

		// take it over

		MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);	
		if (pgobMiner != NULL) {
			pgobMiner->Deactivate();
			Assert(ggobm.IsBelowLimit(knLimitMobileUnit, pplr));
			ggobm.TrackGobCounts(pgobMiner, false);
			pgobMiner->SetOwner(pplr);
			ggobm.TrackGobCounts(pgobMiner, true);
			pgobMiner->Activate();
		} else {
			Assert(true);  // there should be a miner!
		}
	}

	// Takeover the Processor itself

	StructGob::Takeover(pplr);
}

void ProcessorGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	// Draw base

	StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

	// Draw overlay

	if (nLayer == knLayerDepthSorted) {

		// Don't draw the overlay on top of the destroyed base

		if (m_ani.GetStrip() != 2) {
			Side side = m_pplr->GetSide();

			// Draw fake miner if it is meant to be

			if (m_wptFakeMiner.wx != kwxInvalid) {
				int ifrm = m_wptFakeMiner.wy < m_wy + kwcTile ? 1 : 0;
				int xMiner = PcFromUwc(m_wptFakeMiner.wx) - xViewOrigin;
				int yMiner = PcFromWc(m_wptFakeMiner.wy) - yViewOrigin;
				m_pstruc->panid->DrawFrame(4, ifrm, pbm, xMiner, yMiner, side);
			}

			if (m_ff & kfGobDrawFlashed)
				side = (Side)-1;
			else if (m_ff & kfGobBeingBuilt)
				side = ksideNeutral;

			int x = PcFromUwc(m_wx) - xViewOrigin;
			int y = PcFromUwc(m_wy) - yViewOrigin;
			m_aniOverlay.Draw(pbm, x, y, side);
		}
	} else if (nLayer == knLayerSelection && (m_ff & kfGobSelected)) {
		WRect wrcT;
		GetUIBounds(&wrcT);
		Rect rcT;
		rcT.FromWorldRect(&wrcT);
		rcT.Offset(-xViewOrigin, -yViewOrigin);

		int nCapacity = m_pplr->GetCapacity();
		int nPips = 0;
		if (nCapacity != 0)
			nPips = ((m_pplr->GetCredits() * 10) + (nCapacity / 20)) / nCapacity;
		DrawFullnessIndicator(pbm, &rcT, nPips, 10);
	}
}

dword ProcessorGob::GetAnimationHash()
{
    dword dw = StructGob::GetAnimationHash();
    int nFrame = m_aniOverlay.GetFrame();
    int nStrip = m_aniOverlay.GetStrip();
    dw ^= (m_wptFakeMiner.wx << 16) | m_wptFakeMiner.wy;
    return dw ^ ((nFrame << 16) | nStrip);
}

void ProcessorGob::GetAnimationBounds(Rect *prc, bool fBase)
{
    if (fBase) {
        m_ani.GetAnimationData()->GetBounds(0, 0, prc);
        return;
    }

    StructGob::GetAnimationBounds(prc, fBase);
    if (m_wptFakeMiner.wx != kwxInvalid) {
        int ifrm = m_wptFakeMiner.wy < m_wy + kwcTile ? 1 : 0;
        Rect rcBounds;
        m_pstruc->panid->GetBounds(4, ifrm, &rcBounds);
        int xOffsetMiner = PcFromUwc(m_wptFakeMiner.wx - m_wx);
        int yOffsetMiner = PcFromWc(m_wptFakeMiner.wy - m_wy);
        rcBounds.Offset(xOffsetMiner, yOffsetMiner);
        prc->Union(&rcBounds);
    }
}

void ProcessorGob::DrawAnimation(DibBitmap *pbm, int x, int y)
{
    StructGob::DrawAnimation(pbm, x, y);

    // Don't go further if destroyed

    if (m_ani.GetStrip() == 2) {
        return;
    }
    Side side = m_pplr->GetSide();
    if (m_wptFakeMiner.wx != kwxInvalid) {
        int xOffsetMiner = PcFromUwc(m_wptFakeMiner.wx - m_wx);
        int yOffsetMiner = PcFromWc(m_wptFakeMiner.wy - m_wy);
        int ifrm = m_wptFakeMiner.wy < m_wy + kwcTile ? 1 : 0;
        m_pstruc->panid->DrawFrame(4, ifrm, pbm, x + xOffsetMiner,
                y + yOffsetMiner, side);
    }

    if (m_ff & kfGobDrawFlashed) {
        side = (Side)-1;
    } else if (m_ff & kfGobBeingBuilt) {
        side = ksideNeutral;
    }
    m_aniOverlay.Draw(pbm, x, y, side);
}

void ProcessorGob::NotifyMinersAttemptingDelivery(bool fDying)
{
	// Notify miners that are stuck attempting delivery to this processor that the
	// processor is available.

	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (pgobT->GetType() != kgtGalaxMiner)
			continue;
		if (pgobT->GetOwner() != m_pplr)
			continue;
		if (!(pgobT->GetFlags() & kfGobActive))
			continue;
		MinerGob *pmnr = (MinerGob *)pgobT;
		if (pmnr->IsAttemptingToDeliver(m_gid)) {
			if (fDying) {
				pmnr->SendDeliverCommand(kgidNull);
			} else {
				pmnr->SendDeliverCommand(m_gid);
			}
		}
	}
}

//
// StateMachine methods
//

#if defined(DEBUG_HELPERS)
char *ProcessorGob::GetName()
{
	return "Processor";
}
#endif

int ProcessorGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidGalaxiteDelivery)
		// Remember the MinerGob and hide it (deselect it first)
		// NOTE: It still occupies terrain and is counted in its owning
		// Player's unit counts.

		m_gidMiner = pmsg->smidSender;
		MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);
		pgobMiner->Hide(true);

		// Set / clear reservation / occupation bits

		TPoint tpt;
		pgobMiner->GetTilePosition(&tpt);
		gsim.GetLevel()->GetTerrainMap()->SetFlags(tpt.tx, tpt.ty, 1, 1, kbfStructure);
		gsim.GetLevel()->GetTerrainMap()->ClearFlags(tpt.tx, tpt.ty, 1, 1, kbfMobileUnit);

		SetState(kstProcessorGetMiner);

	OnMsg(kmidAnimationComplete)
		m_fProcessingAnimationInProgress = false;
		m_unvl.MinSkip();

	//-----------------------------------------------------------------------

	State(kstBeingBuilt)
		OnMsg(kmidBuildComplete)
			// When a Processor is built at run time it brings a fresh Miner along with
			// itself (included, free of charge!).

			if (ggobm.IsBelowLimit(knLimitMobileUnit, m_pplr)) {
				MinerGob *pmnr = new MinerGob();
				Assert(pmnr != NULL, "out of memory!");
				if (pmnr == NULL)
					goto lbError;
				if (!pmnr->Init(m_wx, m_wy + (kwcTile * 2), m_pplr, 0, 0, NULL)) {
					delete pmnr;
					goto lbError;
				}

				// Make sure the Miner prefers to return to this Processor

				pmnr->SetFavoriteProcessor(m_gid);

				// Hide the Miner so we can 'launch' it, same as it is launched
				// after it completes a Galaxite delivery.

				m_gidMiner = pmnr->GetId();
				pmnr->GetCenter(&m_wptFakeMiner);
				m_wptFakeMiner.wy = m_wy + WcFromTile16ths(15);
                m_wptFakeMiner.wx = m_wx + WcFromTile16ths(27);
				pmnr->Hide(true);

				// Set / clear reservation / occupation bits
				
				TPoint tpt;
				pmnr->GetTilePosition(&tpt);
				gsim.GetLevel()->GetTerrainMap()->SetFlags(tpt.tx, tpt.ty, 1, 1, kbfStructure);
				gsim.GetLevel()->GetTerrainMap()->ClearFlags(tpt.tx, tpt.ty, 1, 1, kbfMobileUnit);

				StartAnimation(&m_aniOverlay, 3, -1, 0);
				SetState(kstProcessorPutMiner);
			} else {
lbError:
                // The miner spot was reserved while the gob was being built. Since the new
                // miner wasn't created, clear the spot so other miners can use the processor.

                Assert(m_pstruc->ctx == 3 && m_pstruc->cty == 2);
                gsim.GetLevel()->GetTerrainMap()->ClearFlags(TcFromWc(m_wx) + 1, TcFromWc(m_wy) + m_pstruc->cty, 1, 1, kbfStructure);
				SetState(kstIdle);
			}
			Activate();

		OnEnter
			// init will mark the building location as occupied.
			// additionally here we'll mark the spot in front of our front door
			// as occupied so the miner can exit when we're done building.
			// We twiddle these flags on kmidGalaxiteDelivery and kstPutMiner

			Assert(m_pstruc->ctx == 3 && m_pstruc->cty == 2);
			gsim.GetLevel()->GetTerrainMap()->SetFlags(TcFromWc(m_wx), 
					TcFromWc(m_wy) + m_pstruc->cty, 1, 1, kbfStructure);

			return StructGob::ProcessStateMachineMessage(st, pmsg);

	//-----------------------------------------------------------------------

	State(kstProcessorGetMiner)
		OnEnter
			MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);
			if (pgobMiner != NULL) {
				pgobMiner->GetCenter(&m_wptFakeMiner);
				pgobMiner->Select(false);
                pgobMiner->Hilight(false);
				StartAnimation(&m_aniOverlay, 3, 0, 0);
				m_fDoorMoving = false;
			}

		OnExit
			// To keep StructGob from attempting to handle a state it doesn't understand

		OnUpdate
			m_unvl.MinSkip();	

			// Open the door when the miner gets close

			if (m_wptFakeMiner.wy < m_wy + WcFromTile16ths(35)) {
				AdvanceAnimation(&m_aniOverlay);
				if (!m_fDoorMoving) {
					m_fDoorMoving = true;
					if (m_pplr == gpplrLocal)
						gsndm.PlaySfx(ksfxGalaxiteProcessorDoorOpening);
				}
			}

			if (m_wptFakeMiner.wy > m_wy + WcFromTile16ths(19)) {
				m_wptFakeMiner.wy -= kwcTile16th;
                m_wptFakeMiner.wx += kwcTile / 21;
				MarkRedraw();
			} else {
				SetState(kstProcessorTakeGalaxite);
			}

			DefUpdate();

	State(kstProcessorTakeGalaxite)
		OnEnter
		OnExit
			// To keep StructGob from attempting to handle a state it doesn't understand

		OnUpdate
			m_unvl.MinSkip();

			// Add credits while taking galaxite from miner

			MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);
			Assert(pgobMiner != NULL);
			int nAmount = pgobMiner->GetGalaxiteAmount();
			if (nAmount > 0) {
				nAmount -= 2;
				int nGalaxiteValue = m_pplr->GetHandicap() & kfHcapIncreasedMinerLoadValue ? 
						((knGalaxiteValue * (100 + knIncreasedMinerLoadValuePercent)) + 50) / 100 : knGalaxiteValue;
				int nCreditsNew = m_pplr->GetCredits() + 2 * nGalaxiteValue;
				if (nCreditsNew <= m_pplr->GetCapacity()) {
					MarkRedraw();
					m_pplr->SetCredits(nCreditsNew, true);
					pgobMiner->SetGalaxiteAmount(nAmount);

					// Kick off the processing animation if it isn't already in progress

					if (!m_fProcessingAnimationInProgress) {
						if (ggobm.IsBelowLimit(knLimitSupport)) {
							PuffGob *pgob = new PuffGob();
							Assert(pgob != NULL, "out of memory!");
							if (pgob != NULL)
								pgob->Init(m_wx, m_wy, m_gid);
						}
						m_fProcessingAnimationInProgress = true;
					}
				} else {
					// Need more storage!

					gsim.GetLevel()->GetTriggerMgr()->SetConditionTrue(knGalaxiteCapacityReachedCondition,
							GetSideMask(GetSide()));

// TUNE:
#define kctIntervalStorageNotify (30 * 100)
					if (m_pplr == gpplrLocal) {
						static long s_tLastStorageNotify = 0;
						long tCurrent = HostGetTickCount();
						if (s_tLastStorageNotify == 0 || (tCurrent - s_tLastStorageNotify) >= kctIntervalStorageNotify) {
							s_tLastStorageNotify = tCurrent;
							gsndm.PlaySfx(ksfxGalaxiteWarehouseTooFull);
							ShowAlert(kidsWarehouseTooFull);
						}
					}
				}
			} else {
				SetState(kstProcessorPutMiner);
			}
			DefUpdate();
	
	State(kstProcessorPutMiner)
		OnEnter
			m_fDoorMoving = false;

		OnExit
			// Restore the MinerGob. 

			MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);
			Assert(pgobMiner != NULL);
			pgobMiner->Hide(false);

			// Set / clear reservation / occupation bits

			TPoint tpt;
			pgobMiner->GetTilePosition(&tpt);
			gsim.GetLevel()->GetTerrainMap()->ClearFlags(tpt.tx, tpt.ty, 1, 1, kbfStructure);
			gsim.GetLevel()->GetTerrainMap()->SetFlags(tpt.tx, tpt.ty, 1, 1, kbfMobileUnit);

			// Have the Miner resume mining
			// NOTE: We send a message rather than call MinerGob::Mine method directly 
			// so the state change will occur immediately.

			SendMineCommand(pgobMiner->GetId(), kwxInvalid, 0);

			m_gidMiner = kgidNull;
			m_wptFakeMiner.wx = kwxInvalid;

			// Notify any miners attempting to deliver

			NotifyMinersAttemptingDelivery(false);

		OnUpdate
			m_unvl.MinSkip();
	
			WPoint wptDst;
			((MinerGob *)ggobm.GetGob(m_gidMiner, false))->GetCenter(&wptDst);
			if (m_wptFakeMiner.wy > m_wy + WcFromTile16ths(27)) {
				int ifrm = m_aniOverlay.GetFrame();
				if (ifrm > 0) {
					SetAnimationFrame(&m_aniOverlay, --ifrm);
					if (!m_fDoorMoving) {
						m_fDoorMoving = true;
						if (m_pplr == gpplrLocal)
							gsndm.PlaySfx(ksfxGalaxiteProcessorDoorClosing);
					}
				}
			}

			if (m_wptFakeMiner.wy < wptDst.wy) {
                m_wptFakeMiner.wy += kwcTile16th;
                m_wptFakeMiner.wx -= kwcTile / 21;
                MarkRedraw();
			} else {
				SetState(kstIdle);
			}
			DefUpdate();

	State(kstDying)
		OnEnter
#if 0 // DWM: crazy last minute change. Players don't lose credits when they sell Processors
			// Sorry, player must lose some Credits along with this Processor

			if (m_ff & kfGobActive)
				m_pplr->SetCredits(m_pplr->GetCredits() - CalcCreditsShare(m_pplr), true);
#endif

			// Remove the Miner too if it's inside the Processor

			MinerGob *pgobMiner = (MinerGob *)ggobm.GetGob(m_gidMiner, false);	
			if (pgobMiner != NULL) {
				// Clear the structure bit so the "miner tile" is free again

				TPoint tpt;
				pgobMiner->GetTilePosition(&tpt);
				gsim.GetLevel()->GetTerrainMap()->ClearFlags(tpt.tx, tpt.ty, 1, 1, kbfStructure);

				// Deactivate the miner first which will subtract
				// it from its owning Player's unit counts.

				pgobMiner->Deactivate();
				ggobm.RemoveGob(pgobMiner);		
				delete pgobMiner;
				m_gidMiner = kgidNull;
			}

			// The miner spot is reserved while this gob is being built. Clear the bit just in case.
			// We don't need to check anything, just clearing it is safe.

			Assert(m_pstruc->ctx == 3 && m_pstruc->cty == 2);
			gsim.GetLevel()->GetTerrainMap()->ClearFlags(TcFromWc(m_wx),
					TcFromWc(m_wy) + m_pstruc->cty, 1, 1, kbfStructure);

			int nHandled = StructGob::ProcessStateMachineMessage(st, pmsg);

			// Now that this gob is deactivated, notify any miners attempting
			// deliver to find a different processor

			NotifyMinersAttemptingDelivery(true);
			return nHandled;

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

//
// PuffGob implementation
// UNDONE: Consider making AnimGob savable by passing in an index
// referring to a global array of AnimData's (then the index can be saved
// and we can get rid of PuffGob).
//

PuffGob::PuffGob()
{
}

bool PuffGob::Init(WCoord wx, WCoord wy, StateMachineId smidNotify)
{
	UnitConsts *puntc = gapuntc[kutProcessor];
	return AnimGob::Init(wx, wy, kfAnmDeleteWhenDone | kfAnmSmokeFireLayer, NULL, puntc->panid, puntc->panid->GetStripIndex("smoke"), smidNotify, NULL);
}

#define knVerPuffGobState 1
bool PuffGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerPuffGobState)
		return false;
	UnitConsts *puntc = gapuntc[kutProcessor];
	m_ani.Init(puntc->panid);
	m_ani.SetStrip(puntc->panid->GetStripIndex("smoke"));
	m_ani.LoadState(pstm);
	m_smidNotify = pstm->ReadWord();
	m_wfAnm = kfAnmDeleteWhenDone | kfAnmSmokeFireLayer;
	return AnimGob::LoadState(pstm);
}

bool PuffGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerPuffGobState);
	m_ani.SaveState(pstm);
	pstm->WriteWord(m_smidNotify);
	return AnimGob::SaveState(pstm);
}

bool PuffGob::IsSavable()
{
	// Because AnimGob is not savable, we need to override

	return true;
}

GobType PuffGob::GetType()
{
	return kgtPuff;
}

} // namespace wi
