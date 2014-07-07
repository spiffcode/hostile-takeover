#include "ht.h"

namespace wi {

//
// MobileUnitBuilderGob implementation
// base gob for VTS and HRC
//

#if defined(DEBUG_HELPERS)
char *MobileUnitBuilderGob::GetName()
{
	return "MobileUnitBuilder";
}
#endif

MobileUnitBuilderGob::MobileUnitBuilderGob(MobileUnitBuilderConsts *pmubc) : BuilderGob(pmubc)
{
	m_puntc->wf |= kfUntcNotifyPowerLowHigh;
}

void MobileUnitBuilderGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	// UNDONE: rethink this
	bool fUpgraded = (m_pplr->GetUpgrades() & ((MobileUnitBuilderConsts *)m_puntc)->fUpgrade) != 0;
	if (fUpgraded)
		SetAnimationStrip(&m_ani, m_ani.GetStrip() + 3);

	BuilderGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

	if ((nLayer == knLayerDepthSorted) && ((m_pplr->GetUpgrades() & ((MobileUnitBuilderConsts *)m_puntc)->fUpgradeInProgress) != 0))
		BuilderGob::DrawUpgradeEffect(pbm, xViewOrigin, yViewOrigin);

	if (fUpgraded)
		SetAnimationStrip(&m_ani, m_ani.GetStrip() - 3);
}

void MobileUnitBuilderGob::InitMenu(Form *pfrm)
{
	Control *pctl = pfrm->GetControlPtr(kidcBuild);
	pctl->Enable((m_pplr->GetUpgrades() & ((MobileUnitBuilderConsts *)m_pmuntc)->fUpgradeInProgress) != ((MobileUnitBuilderConsts *)m_puntc)->fUpgradeInProgress);

	BuilderGob::InitMenu(pfrm);
}

void MobileUnitBuilderGob::OnMenuItemSelected(int idc)
{
	switch (idc) {
	case kidcBuild:
		gpmfrmm->AddForm(((MobileUnitBuilderConsts *)m_puntc)->pfrmBuild);
		((MobileUnitBuilderConsts *)m_puntc)->pfrmBuild->SetOwner(this);
		((MobileUnitBuilderConsts *)m_puntc)->pfrmBuild->DoModal();
		gpmfrmm->RemoveForm(((MobileUnitBuilderConsts *)m_puntc)->pfrmBuild);
		break;

	default:
		BuilderGob::OnMenuItemSelected(idc);
		break;
	}
}

void MobileUnitBuilderGob::Takeover(Player *pplr)
{
	// new owner gets to keep upgrades

	pplr->SetUpgrades(pplr->GetUpgrades() | (m_pplr->GetUpgrades() & ((MobileUnitBuilderConsts *)m_puntc)->fUpgrade));
	BuilderGob::Takeover(pplr);
}

void MobileUnitBuilderGob::Deactivate()
{
	BuilderGob::Deactivate();
	if (m_pmubc->pfrmBuild != NULL && m_pmubc->pfrmBuild->GetOwner() == this)
		m_pmubc->pfrmBuild->EndForm();
}

int MobileUnitBuilderGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnEnter
		SetState(kstIdle);

	OnMsg(kmidBuildOtherCommand)
        // Counts will already have been checked in the order UI. However since
        // the order is queued, the UI only guesses if it is possible to build
        // based on current state when the button is pressed. Here we make a
        // bedrock decision. This ensures the limits are enforced and that the
        // same decision gets made on all clients in a multiplayer game.

		if (ggobm.IsBelowLimit(knLimitMobileUnit, m_pplr))
			Build(pmsg->BuildOtherCommand.ut);

	OnMsg(kmidAbortBuildOtherCommand)
		// UNDONE: deal with queuing?

		AbortBuild(true, pmsg->AbortBuildOtherCommand.ut);

	OnMsg(kmidSelfDestructCommand)
        // override and call abort build here so we can tell it to refund the
        // value.  AbortBuild is also in BuilderGob::Deactivate but will do
        // nothing if called a 2nd time, and there it wouldnt refund if we left
        // it

		AbortBuild(true);
		SelfDestruct();

	//-----------------------------------------------------------------------

	State(kstIdle)
		OnMsg(kmidUpgradeComplete)
			// Redraw to get rid of the upgrade graphics

			m_ff &= ~kfGobBeingUpgraded;
			MarkRedraw();

		OnMsg(kmidBeingUpgraded)
			// Wake up so the animation can occur

			m_ff |= kfGobBeingUpgraded;
			m_unvl.MinSkip();

		OnUpdate
			// Invalidate if upgrading is occuring so that this gob can draw the upgrade
			// effect.

			if (m_pplr->GetUpgrades() & ((MobileUnitBuilderConsts *)m_puntc)->fUpgradeInProgress) {
				// Smallest interval so we can keep invalidating ourselves while being upgraded

				MarkRedraw();
				m_unvl.MinSkip();
			}

			return BuilderGob::ProcessStateMachineMessage(st, pmsg);

	//-----------------------------------------------------------------------

	State(kstBuildOtherCompleting)
		OnEnter
			// init built Gob
			{
				UnitGob *puntBuild = GetBuiltGob();
				if (puntBuild != NULL) {
					WPoint wpt;
					FindInitPosition(&wpt);
					puntBuild->Init(wpt.wx, wpt.wy, m_pplr, 0, 0, NULL);

					// If this is a Bullpup built by a human player send it off to mine.

					if (!(m_pplr->GetFlags() & kfPlrComputer) && puntBuild->GetType() == kgtGalaxMiner) {
						SendMineCommand(puntBuild->GetId(), kwxInvalid, 0);

					} else {
#ifdef RALLY_POINTS
/*
UNDONE: issues with rally points
- rally point needs to be visible when the Builder is selected (e.g., flag)
- units are vulnerable along the way. Need to attack-move (respond to being hit, then continue to rally point)
- can't set rally point on the Replicator
x need to know when to send a unit to the rally point (e.g., it's not at the default value)
x units force others out of the way to get to the rally point
*/
					// If the rally point is not at the default, send the new unit there

					if (m_tptRally.tx != ktxInvalid) {
						Message msgT;
						msgT.mid = kmidMoveCommand;
						msgT.smidSender = m_gid;
						msgT.smidReceiver = puntBuild->GetId();
						FindNearestFreeTile(m_tptRally.tx, m_tptRally.ty, &msgT.MoveCommand.wptTarget);
						msgT.MoveCommand.gidTarget = kgidNull;
						msgT.MoveCommand.wptTargetCenter.wx = msgT.MoveCommand.wptTarget.wx;
						msgT.MoveCommand.wptTargetCenter.wy = msgT.MoveCommand.wptTarget.wy;
						msgT.MoveCommand.tcTargetRadius = 0;
						msgT.MoveCommand.wcMoveDistPerUpdate = ((MobileUnitConsts *)puntBuild->GetConsts())->GetMoveDistPerUpdate();
						gsmm.SendMsg(&msgT);
					}
#endif
					}
				}

				// Notify the BuildMgr that this Unit is complete

				gsim.GetBuildMgr()->OnBuilt(puntBuild, this);
				((MobileUnitBuilderConsts *)m_puntc)->pfrmBuild->OnUnitCompleted(this, puntBuild->GetUnitType());
				ClearBuiltGob();
			}
			SetState(kstIdle);

		// These are here to keep the message from routing up to BuilderGob's message handler
		OnUpdate
		OnExit

#if 0
EndStateMachineInherit(BuilderGob)
#else
            return knHandled;
        }
    } else {
        return (int)BuilderGob::ProcessStateMachineMessage(st, pmsg);
    }
    return (int)BuilderGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

void MobileUnitBuilderGob::DefUpdate()
{
	// give the build form a chance to show progress

	((MobileUnitBuilderConsts *)m_puntc)->pfrmBuild->DefUpdate(this, IsBuildInProgress());

	// and continue with the normal idle processing

	BuilderGob::DefUpdate(); 
}

} // namespace wi
