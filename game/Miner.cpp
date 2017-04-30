#include "ht.h"
#include "wistrings.h"

namespace wi {

static MinerConsts gConsts;
AnimationData *MinerGob::s_panidVacuum = NULL;

#if defined(DEBUG_HELPERS)
char *MinerGob::GetName()
{
	return "Miner";
}
#endif

static int s_anMovingStripIndices[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

bool MinerGob::InitClass(IniReader *pini)
{
	s_panidVacuum = LoadAnimationData("vacuum.anir");
	Assert(s_panidVacuum != NULL);
	if (s_panidVacuum == NULL)
		return false;

	gConsts.gt = kgtGalaxMiner;
	gConsts.ut = kutGalaxMiner;
	gConsts.umPrerequisites = kumProcessor;
	gConsts.wf |= kfUntcHasFullnessIndicator;

	// Initialize the frame indices arrays

	gConsts.anFiringStripIndices = s_anMovingStripIndices;
	gConsts.anMovingStripIndices = s_anMovingStripIndices;
	gConsts.anIdleStripIndices = s_anMovingStripIndices;

	// Sound effects

	gConsts.sfxImpact = ksfxNothing;
	gConsts.sfxMine = ksfxGalaxMinerMine;
	gConsts.sfxUnderAttack = ksfxGalaxMinerUnderAttack;
	gConsts.sfxFire = ksfxNothing;

	gConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gConsts.sfxcSelect = ksfxcMajor01Select;
	gConsts.sfxcMove = ksfxcMajor01Move;
	gConsts.sfxcAttack = ksfxcNothing;

	return MobileUnitGob::InitClass(&gConsts, pini);
}

void MinerGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gConsts);
	delete s_panidVacuum;
	s_panidVacuum = NULL;
}

MinerGob::MinerGob() : MobileUnitGob(&gConsts)
{
	m_aniVacuum.Init(s_panidVacuum);
	StartAnimation(&m_aniVacuum, 0, 0, kfAniLoop);
	m_nGalaxiteAmount = 0;
	m_gidFavoriteProcessor = kgidNull;
	m_tptGalaxite.tx = kxInvalid;
	m_fMinerUnderAttack = false;
	m_wfMunt &= ~kfMuntAggressivenessBits;
	m_fHidden = false;
	m_fAttemptingToDeliver = false;
}

#define knVerMinerGobState 2
bool MinerGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerMinerGobState)
		return false;
	m_nGalaxiteAmount = pstm->ReadWord();
	m_cDelay = (char)pstm->ReadByte();
	m_gidFavoriteProcessor = pstm->ReadWord();
	m_tptGalaxite.tx = pstm->ReadWord();
	m_tptGalaxite.ty = pstm->ReadWord();
	m_fMinerUnderAttack = pstm->ReadByte() != 0 ? true : false;
	m_fHidden = pstm->ReadByte() != 0 ? true : false;
	m_fAttemptingToDeliver = pstm->ReadByte() != 0 ? true : false;
	return MobileUnitGob::LoadState(pstm);
}

bool MinerGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerMinerGobState);
	pstm->WriteWord(m_nGalaxiteAmount);
	pstm->WriteByte(m_cDelay);
	pstm->WriteWord(m_gidFavoriteProcessor);
	pstm->WriteWord(m_tptGalaxite.tx);
	pstm->WriteWord(m_tptGalaxite.ty);
	pstm->WriteByte(m_fMinerUnderAttack);
	pstm->WriteByte(m_fHidden);
	pstm->WriteByte(m_fAttemptingToDeliver);
	return MobileUnitGob::SaveState(pstm);
}

// when the miner goes into the processor, we hide the miner gob and the
// processor gob draws it pulling in and out.  Leave our spot marked occupied
// so it saves it for us. This is kindof half a deactivate.

void MinerGob::Hide(bool fHide)
{
	if (m_fHidden == fHide)
		return;

	m_fHidden = fHide;
	if (fHide) {
		m_ff &= ~(kfGobActive | kfGobDrawFlashed);
		// make sure this unit's menu is not left up

		UnitConsts *puntc = GetUnitConsts(GetType());
		Assert(puntc->pfrmMenu != NULL);
		if (puntc->pfrmMenu->GetOwner() == (UnitGob *)this)
			puntc->pfrmMenu->EndForm(kidcCancel);

		Invalidate();
	} else {
		m_ff |= kfGobActive;
		Invalidate();
	}
}

// so far it's just an assert, but let's not violate the general rule that we
// don't deactivate inactive gobs.

void MinerGob::Deactivate() 
{
	if (m_fHidden)
		m_ff |= kfGobActive;
	MobileUnitGob::Deactivate();
}

void MinerGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
#ifdef DRAW_OCCUPIED_TILE_INDICATOR
		{
			WRect wrcT;
			GetTilePaddedWRect(&wrcT);
			Rect rcT;
			rcT.FromWorldRect(&wrcT);
			rcT.Offset(-xViewOrigin, -yViewOrigin);
			DrawBorder(pbm, &rcT, 1, GetColor(kiclrWhite));
		}
#endif

	if (m_fHidden)
		return;

	MobileUnitGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

	if (m_st == kstMinerSuck && nLayer == knLayerDepthSorted) {
        // HACK: m_ani is dir16 but m_aniVacuum only has 8 strips, divide by 2
		SetAnimationStrip(&m_aniVacuum, m_ani.GetStrip() / 2);
		m_aniVacuum.Draw(pbm, PcFromUwc(m_wx) - xViewOrigin, PcFromUwc(m_wy) - yViewOrigin);
	} else if (nLayer == knLayerSelection && (m_ff & kfGobSelected)) {
		Rect rcT;
		rcT.FromWorldRect(&m_pmuntc->wrcUIBounds);
		rcT.Offset(PcFromUwc(m_wx) - xViewOrigin, PcFromUwc(m_wy) - yViewOrigin);
		DrawFullnessIndicator(pbm, &rcT, m_nGalaxiteAmount / knGalaxiteValue / 2, knMinerGalaxiteMax / knGalaxiteValue / 2);
	}
}

void MinerGob::InitMenu(Form *pfrm)
{
	ButtonControl *pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcDeliver);
	pbtn->Show(GetGalaxiteAmount() != 0);
}

void MinerGob::OnMenuItemSelected(int idc)
{
	switch (idc) {
	case kidcDeliver:
		{
			// The player shouldn't have been able to select this command unless
			// it can be carried out.

			Assert(GetGalaxiteAmount() != 0);

			Message msg;
			msg.mid = kmidDeliverCommand;
			msg.smidSender = m_gid;
			msg.smidReceiver = m_gid;
			msg.DeliverCommand.gidTarget = kgidNull;
			msg.tDelivery = 0;
			gcmdq.Enqueue(&msg);
			gsndm.PlaySfx(ksfxGalaxMinerDeliver);
		}
		break;
	}
}

bool MinerGob::IsValidTarget(Gob *pgobTarget)
{
	// no soldier squishing this version

	return (pgobTarget->GetType() == kgtProcessor && m_pplr == pgobTarget->GetOwner())
			&& (pgobTarget->GetFlags() & kfGobActive);
}

void MinerGob::GetClippingBounds(Rect *prc)
{
	UnitGob::GetClippingBounds(prc);

	if (m_st == kstMinerSuck) {
        // HACK: m_ani is dir16 but m_aniVacuum only has 8 strips, divide by 2
		SetAnimationStrip(&m_aniVacuum, m_ani.GetStrip() / 2);
		Rect rc;
		m_aniVacuum.GetBounds(&rc);
		rc.Offset(PcFromUwc(m_wx), PcFromUwc(m_wy));
		prc->Union(&rc);
	}
}

void MinerGob::SetTarget(Gid gid, WCoord wx, WCoord wy, WCoord wxCenter, WCoord wyCenter, TCoord tcRadius, WCoord wcMoveDistPerUpdate)
{
	if (gid == kgidNull) {

		// If the target is Galaxite then translate this command into a MineCommand

		FogMap *pfogm = gsim.GetLevel()->GetFogMap();
		if (pfogm->GetGalaxite(TcFromWc(wx), TcFromWc(wy)) != 0) {

			// Play mine sfx - this is called in a move so the general unit
			// move sound will also be called, and the mine sound is currently
			// no different. Needs to somehow be an attack but SimUI.cpp
			// only checks for mobile units as targets.
			//if (m_pplr == gpplrLocal)
			//	gsndm.PlaySfx(m_pmnrc->sfxMine);

			Message msg;
			memset(&msg, 0, sizeof(msg));
			msg.mid = kmidMineCommand;
			msg.MineCommand.gidTarget = gid;
			msg.MineCommand.wptTarget.wx = wx;
			msg.MineCommand.wptTarget.wy = wy;
			msg.smidReceiver = m_gid;
			gcmdq.Enqueue(&msg);
			return;
		}

		// If target is not Galaxite let the MobileUnitGob handle command

		MobileUnitGob::SetTarget(gid, wx, wy, wxCenter, wyCenter, tcRadius, wcMoveDistPerUpdate);
		return;
	}

	// If the target no longer exists, discard the command

	Gob *pgobTarget = ggobm.GetGob(gid);
	if (pgobTarget == NULL)
		return;

	// If target is not a friendly processor let the MobileUnitGob handle command differentiation

	if (pgobTarget->GetType() != kgtProcessor || !IsAlly(pgobTarget->GetSide())) {
		MobileUnitGob::SetTarget(gid, wx, wy, wxCenter, wyCenter, tcRadius, wcMoveDistPerUpdate);
		return;
	}

	// Play deliver sfx

	gsndm.PlaySfx(ksfxGalaxMinerDeliver);

	// Flash the target Gob

	if (m_pplr == gpplrLocal)
		pgobTarget->Flash();

	Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.mid = kmidDeliverCommand;
	msg.MineCommand.gidTarget = gid;
	msg.MineCommand.wptTarget.wx = wx;
	msg.MineCommand.wptTarget.wy = wy;
	msg.smidReceiver = m_gid;
	gcmdq.Enqueue(&msg);
}

void MinerGob::Mine(WCoord wx, WCoord wy)
{
	Assert(!InTransition());

//temp
if (!(m_ff & kfGobActive)) {
    Assert();
}

	// If we aren't being told to where to mine and we don't already
	// have a previous mining location then find a new place to mine.

	if (wx == kwxInvalid) {
		if (m_tptGalaxite.tx == kxInvalid) {
			SetState(kstMinerFindGalaxite);
		} else {

			// Go to Galaxite closest to the previous location

			FogMap *pfogm = gsim.GetLevel()->GetFogMap();
			if (pfogm->FindNearestGalaxite(m_tptGalaxite.tx, m_tptGalaxite.ty, 
					&m_tptGalaxite, (m_pplr->GetFlags() & kfPlrComputer) != 0 || ggame.IsMultiplayer())) {
				m_wptTarget.wx = WcFromTc(m_tptGalaxite.tx);
				m_wptTarget.wy = WcFromTc(m_tptGalaxite.ty);
				SetState(kstMinerApproachGalaxite);
			} else {
				// If no Galaxite found, just stay where we are (NOTE:
				// where we are is in the way of any other Miners trying
				// to use the same Processor!)

				gsim.GetLevel()->GetTriggerMgr()->SetConditionTrue(knMinerCantFindGalaxiteCondition, GetSideMask(GetSide()));
				if (m_pplr == gpplrLocal)
					ShowAlert(kidsMinerNeedsGalaxite);
				SetState(kstGuard);
			}
		}
	} else {
		// Stash mine parameters to be picked up by the Mine state

		m_wptTarget.wx = wx;
		m_wptTarget.wy = wy;
		m_tptGalaxite.tx = TcFromWc(m_wptTarget.wx);
		m_tptGalaxite.ty = TcFromWc(m_wptTarget.wy);
		SetState(kstMinerApproachGalaxite);
	}
}

void MinerGob::PerformAction(char *szAction)
{
	int nUnitAction;
	if (IniScanf(szAction, "%d", &nUnitAction) == 0) {
		Assert(false);
		return;
	}

	if (nUnitAction == knMineUnitAction) {
		SendMineCommand(m_gid, kwxInvalid, kwxInvalid);
		return;
	}

	// Don't override this action, let base handle it

	MobileUnitGob::PerformAction(szAction);
}

/*
MinerGob pseudo-code
-----------------
Inherit all MobileUnitGob message and state handlers

OnDeliverCommand {
>	Deliver() // Galaxite to Processor
=>	Mine(last mining position)
}

OnMineCommand(target) {
=>	Mine(target)
}

Mine(target) {
	if no target
		target = nearest (reachable?) Galaxite tile

1>	move within range of target

	while true {
		while not full {
			do {
				target = nearest (reachable?) Galaxite tile
				if on top of target
2>					move off target, forcefully
				else
3>					move within range of target, forcefully
			} while target tile has no Galaxite

4>			rotate to face the tile to mine from

			do {
5>				take Galaxite from target
			} while not full AND target has Galaxite
		}

6>		Deliver() // Galaxite to Processor

7>		move to last mining position
	}
}

Deliver() {
	if preferred Processor no longer exists {
		find the closest friendly Processor
		if no Processors exist
			abort Delivery (set state to Guard)
	}

>	move to position in front of Processor
>	rotate for entry
	notify Processor of Galaxite delivery
>	wait for Galaxite to be processed
}
*/

int MinerGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidHit)
		// It is possible to be hit by a shot AFTER the process of
		// being pulled into a Processor has begun. We don't want the
		// Processor to have to deal with its 'taken' Miner being
		// destroyed so when this happens we just ignore the hit.
		// UNDONE: this should be considered along with the general
		// fix of removing the MinerGob when the Processor takes it
		// and adding it back when the delivery is done.

		if (!(m_ff & kfGobActive))
			return knHandled;

		// If we've been hit, announce it to the user

		if (gpplrLocal == m_pplr) {
			// After what interval?

			if (!m_fMinerUnderAttack) {
				m_fMinerUnderAttack = true;
				gsndm.PlaySfx(ksfxGalaxMinerUnderAttack);
				
				// update status
				ShowAlert(kidsMinerUnderAttack);
			}
		}
		return MobileUnitGob::ProcessStateMachineMessage(st, pmsg);

	OnMsg(kmidDeliverCommand)
		// Return to specified Processor. If Processor is gone pick the 
		// closest one and deliver there.

		// If a path is being followed we have to wait until a tile center is
		// reached before acting on a deliver command.

		if (InTransition()) {
			m_msgPending = *pmsg;
			m_wfMunt |= kfMuntCommandPending;
		} else {
			// Use the deliver target. If that no longer exists, find the closest processor

			Gob *pgobTarget = NULL;
			m_gidTarget = pmsg->DeliverCommand.gidTarget;
			if (m_gidTarget != kgidNull)
				pgobTarget = ggobm.GetGob(m_gidTarget);
			if (pgobTarget == NULL) {
				m_gidTarget = FindClosestProcessor();
				if (m_gidTarget != kgidNull)
					pgobTarget = ggobm.GetGob(m_gidTarget);
			}

			// Stash deliver parameters to be picked up by the MinerMoveToProcessor state

			if (pgobTarget != NULL && !(pgobTarget->GetFlags() & kfGobBeingBuilt)) {
				m_gidFavoriteProcessor = pgobTarget->GetId();

				// Adjust target x/y to point to the center of the tile 
				// closest to the Processor's entrance.

				pgobTarget->GetPosition(&m_wptTarget);
				m_wptTarget.wx += kwcTileHalf;
				m_wptTarget.wy += (kwcTile * 2) + kwcTileHalf;
				SetState(kstMinerMoveToProcessor);
			} else {
				SetState(kstGuard);
			}
		}

	OnMsg(kmidMineCommand)

		// If a path is being followed we have to wait until a tile center is
		// reached before acting on a deliver command.

		if (InTransition()) {
			m_msgPending = *pmsg;
			m_wfMunt |= kfMuntCommandPending;
		} else {
			Mine(pmsg->MineCommand.wptTarget.wx, pmsg->MineCommand.wptTarget.wy);
		}

	OnMsg(kmidAttackCommand)

		// UNDONE: Miners don't have an attack at the moment so they ignore
		// their auto-response impulses to chase their attacker

		// DO NOTHING (override Unit's Attack response)
		
	//-----------------------------------------------------------------------

	State(kstMinerMoveToProcessor)
		OnEnter
			MoveEnter(true);

		OnExit
			MoveExit();

		OnUpdate
			switch (MoveUpdate()) {
			case knMoveTargetReached: 
				{
					// let's take a second look at our target to be sure it was not 
					// destroyed or taken over whilst we were enroute

					Gob *pgobProcessor = ggobm.GetGob(m_gidTarget);
					if (pgobProcessor != NULL && m_pplr == pgobProcessor->GetOwner()) {
						SetState(kstMinerRotateForEntry);
					} else {
						// Our processor is gone. See if we can find another

						Gid gid = FindClosestProcessor();
						if (gid != kgidNull) {
							SendDeliverCommand(gid);
						} else {
							SetState(kstGuard);
						}
					}
				}
				break;

			case knMoveStuck:
				// The processor will look for candidate miners when possible. 
				// It'll know this miner wants to deliver because of the m_fAttemptingToDeliver
				// flag.

				m_fAttemptingToDeliver = true;
				SetState(kstGuard);
				break;
			}

//			DefUpdate(); // called inside of MoveUpdate()

	State(kstMinerRotateForEntry)
		OnUpdate
			m_unvl.MinSkip();

			// Rotate to face south west

			if (m_dir != kdir16SW) {
				m_dir = TurnToward16(kdir16SW, m_dir);
				StartAnimation(&m_ani, m_pmuntc->anMovingStripIndices[m_dir], 0, kfAniLoop | kfAniIgnoreFirstAdvance);
			} else {

                // When facing south west send message to Processor and switch to
                // guard state

				Gob *pgobProcessor = ggobm.GetGob(m_gidTarget);
				if (pgobProcessor != NULL)
					gsmm.SendMsg(kmidGalaxiteDelivery, m_gid, m_gidTarget);

                // We'll sit in this state while the Processor extracts the
                // Galaxite.  When it's done it will send a kmidMineCommand
                // with a negative target.  OnMsg(kmidMineCommand) will
                // recognize this as a signal to continue mining where we last
                // were.

				m_fAttemptingToDeliver = false;
				SetState(kstGuard);

                // Miner can't be under attack now. This state is used to
                // determine if it's ok to announce "miner under attack!"

				m_fMinerUnderAttack = false;
			}
			DefUpdate();

	//-----------------------------------------------------------------------

// If not already full of Galaxite, find some to mine

	State(kstMinerFindGalaxite)
		OnUpdate
			// Are we full?

			if (m_nGalaxiteAmount == knMinerGalaxiteMax) {

				// Yes, deliver our load to our favorite Processor

				SendDeliverCommand(m_gidFavoriteProcessor);

			} else {

				// No, let's find some Galaxite

				// Is there some right in front of us?

				TCoord tx = TcFromWc(m_wx);
				TCoord ty = TcFromWc(m_wy);
				TCoord txTarget = tx + g_mpDirToDx[m_dir];
				TCoord tyTarget = ty + g_mpDirToDy[m_dir];

				m_wptTarget.wx = WcFromTc(txTarget);
				m_wptTarget.wy = WcFromTc(tyTarget);
				FogMap *pfogm = gsim.GetLevel()->GetFogMap();
				if (pfogm->GetGalaxite(txTarget, tyTarget) != 0) {

					// Yes, get busy with it

					m_tptGalaxite.tx = txTarget;
					m_tptGalaxite.ty = tyTarget;

//temp
if (!(m_ff & kfGobActive)) {
    Assert();
}


					SetState(kstMinerSuck);
				} else {

					// No, look around for some
					// Miners controlled by computer players or in a network game can (must) ignore fog.

					bool fFound = pfogm->FindNearestGalaxite(tx, ty, &m_tptGalaxite, 
							(m_pplr->GetFlags() & kfPlrComputer) != 0 || ggame.IsMultiplayer());
					if (!fFound) {

						// Can't find any! Deliver what we have & then we'll park it by our
						// processor. If we've never found any then record that this is the closest 
						// we've come to finding any

						if (m_tptGalaxite.tx == kwxInvalid){
							gsim.GetLevel()->GetTriggerMgr()->SetConditionTrue(knMinerCantFindGalaxiteCondition, GetSideMask(GetSide()));
							if (m_pplr == gpplrLocal)
								ShowAlert(kidsMinerNeedsGalaxite);
							SetState(kstGuard);
						} else {
							SendDeliverCommand(m_gidFavoriteProcessor);
						}

					} else {
						// Are we on top of the Galaxite we want?

						if (tx == m_tptGalaxite.tx && ty == m_tptGalaxite.ty) {

							// Yes, pick an unoccupied adjacent tile to move to

							for (int dir = 0; dir < 8; dir++) {
								TCoord txT = tx + g_mpDirToDx[dir];
								TCoord tyT = ty + g_mpDirToDy[dir];
								if (!IsTileFree(txT, tyT))
									continue;

								m_wptTarget.wx = WcFromTc(txT);
								m_wptTarget.wy = WcFromTc(tyT);
								SetState(kstMinerStepAside);
								break;
							}

							// NOTE: if can't move to a new spot then we'll try
							// again next Update.
							// NOTE: ideally we should just issue the move command and
							// let the move waiting code handle this case.

							m_unvl.MinSkip();

						} else {

							// No, move within range of the target tile, forcefully

							m_wptTarget.wx = WcFromTc(m_tptGalaxite.tx);
							m_wptTarget.wy = WcFromTc(m_tptGalaxite.ty);
							SetState(kstMinerApproachGalaxite);
						}
					}
				}
			}
			DefUpdate();

// Rotate to face the desired tile and advance to state kstMinerSuck

	State(kstMinerFaceGalaxite)
		OnEnter
			m_cDelay = 0;

		OnUpdate
			// Introduce a little rotation delay

			m_cDelay -= m_unvl.GetUpdateCount();
			if (m_cDelay < 0) {
				m_cDelay = 1;

				// Make sure we're facing towards the Galaxite tile

				Direction16 dirTo = CalcDir16(m_tptGalaxite.tx - TcFromWc(m_wx), m_tptGalaxite.ty - TcFromWc(m_wy));
				if (m_dir != dirTo) {
					m_dir = TurnToward16(dirTo, m_dir);
					StartAnimation(&m_ani, m_pmuntc->anMovingStripIndices[m_dir], 0, kfAniLoop | kfAniIgnoreFirstAdvance);
				} else {
//temp
if (!(m_ff & kfGobActive)) {
}

					SetState(kstMinerSuck);
				}
			}
			m_unvl.MinSkip(m_cDelay);
			DefUpdate();

// Move off a Galaxite tile so we can mine from it.
// An appropriate target tile has already been selected.

	State(kstMinerStepAside)
		OnEnter
			MoveEnter(true);

		OnExit
			MoveExit();

		OnUpdate
			switch (MoveUpdate()) {
			case knMoveTargetReached:
				// Turn around to face the tile we want to mine from
//temp
if (!(m_ff & kfGobActive)) {
    Assert();
}

				SetState(kstMinerFaceGalaxite);
				break;

			case knMoveStuck:
				// UNDONE: what to do? Doing nothing means we repath every
				// Update until a valid path can be determined.

				SetState(kstGuard);
				break;
			}

//			DefUpdate(); // called inside of MoveUpdate()

// Move within range of target and advance to state kstMinerFaceGalaxite

	State(kstMinerApproachGalaxite)
		OnEnter
			MoveEnter(true);

		OnExit
			MoveExit();

		OnUpdate
			// Are we within 1 tile of the target?

			if (!InTransition()) {
				if (abs(WcTrunc(m_wx) - WcTrunc(m_wptTarget.wx)) <= kwcTile &&
						abs(WcTrunc(m_wy) - WcTrunc(m_wptTarget.wy)) <= kwcTile) {

					// Yes, this is where we want to be!
//temp
if (!(m_ff & kfGobActive)) {
    Assert();
}

					SetState(kstMinerFaceGalaxite);
					return knHandled;
				}
			}

			switch (MoveUpdate()) {
			case knMoveTargetReached:
//temp
if (!(m_ff & kfGobActive)) {
    Assert();
}


				SetState(kstMinerFaceGalaxite);
				break;

			case knMoveStuck:
				// UNDONE: what to do? Doing nothing means we repath every
				// Update until a valid path can be determined.

				SetState(kstGuard);
				break;
			}

//			DefUpdate(); // called inside of MoveUpdate()

// Dig in, if there's any Galaxite left

	State(kstMinerSuck)
		OnEnter
//temp
if (!(m_ff & kfGobActive)) {
    Assert();
}
			// Begin countdown to Galaxite decrement

			m_cDelay = kcMinerSuckDelay;

			// Are we full?

			if (m_nGalaxiteAmount == knMinerGalaxiteMax)
				SendDeliverCommand(m_gidFavoriteProcessor);

		OnUpdate
			// Are we full?

			if (m_nGalaxiteAmount == knMinerGalaxiteMax) {

				// Yes, deliver our load to our favorite Processor

				MarkRedraw();
				SendDeliverCommand(m_gidFavoriteProcessor);

			} else {
				// If target tile is out of Galaxite try another

				FogMap *pfogm = gsim.GetLevel()->GetFogMap();
				if (pfogm->GetGalaxite(m_tptGalaxite.tx, m_tptGalaxite.ty) == 0) {

					// Look for another

					MarkRedraw();
					SetState(kstMinerFindGalaxite);
				} else {
					AdvanceAnimation(&m_aniVacuum);

					// Time to consume some Galaxite?

					m_cDelay -= m_unvl.GetUpdateCount();
					if (m_cDelay < 0) {
						m_cDelay = kcMinerSuckDelay;
						m_unvl.MinSkip(m_cDelay);
						pfogm->DecGalaxite(m_tptGalaxite.tx, m_tptGalaxite.ty);
						m_nGalaxiteAmount += 10;
						MarkRedraw();
					}
				}
			}

			DefUpdate();

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

#ifdef MP_DEBUG_SHAREDMEM
void MinerGob::MPValidate()
{
	// TODO: animation state
	MPValidateGobMember(MinerGob, m_nGalaxiteAmount);
	MPValidateGobMember(MinerGob, m_cDelay);
	MPValidateGobMember(MinerGob, m_gidFavoriteProcessor);
	MPValidateGobMember(MinerGob, m_tptGalaxite);

// gpplrLocal specific
//	MPValidateGobMember(MinerGob, m_fMinerUnderAttack);

	MPValidateGobMember(MinerGob, m_fAttemptingToDeliver);
	MPValidateGobMember(MinerGob, m_fHidden);

// Visibility specific
//	m_aniVacuum.MPValidate((Animation *)(((byte *)MPGetGobPtr(m_gid)) + OFFSETOF(MinerGob, m_aniVacuum)));

	MobileUnitGob::MPValidate();
}
#endif

Gid MinerGob::FindClosestProcessor()
{
	// Find the closest Processor owned by the Miner's owner
	TCoord tcxyClosest = ktcMax;
	TPoint tpt;
	TCoord tc;

	Gid gidClosest = kgidNull;
	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (pgobT->GetType() != kgtProcessor)
			continue;
		if (pgobT->GetOwner() != m_pplr)
			continue;
		if (pgobT->GetFlags() & kfGobBeingBuilt)
			continue;
		if (!(pgobT->GetFlags() & kfGobActive))
			continue;

		// candidate processor. Is it closer than our last candidate?

		pgobT->GetTilePosition(&tpt);
		tc = CalcRange(tpt.tx, tpt.ty, this);
		if (tc < tcxyClosest) {
			gidClosest = pgobT->GetId();
			tcxyClosest = tc;
		}
	}

	return gidClosest;
}

void MinerGob::SendDeliverCommand(Gid gidProcessor)
{
	// Do it this way instead of calling SetTarget... SetTarget does
	// some extra "user feedback" things we don't want here

	Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.mid = kmidDeliverCommand;
	msg.MineCommand.gidTarget = gidProcessor;
	msg.MineCommand.wptTarget.wx = 0;
	msg.MineCommand.wptTarget.wy = 0;
	msg.smidReceiver = m_gid;
	gsmm.SendMsg(&msg);
}

} // namespace wi
