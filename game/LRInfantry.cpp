#include "ht.h"

namespace wi {

static MobileUnitConsts gConsts;

#if defined(DEBUG_HELPERS)
char *LRInfantryGob::GetName()
{
	return "LRInfantry";
}
#endif

static int s_anFiringStripIndices[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
static int s_anMovingStripIndices[16] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 };
static int s_anIdleStripIndices[16] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool LRInfantryGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtLongRangeInfantry;
	gConsts.ut = kutLongRangeInfantry;
	gConsts.upgmPrerequisites = kupgmAdvancedHRC;
	gConsts.wf |= kfUntcNotifyEnemyNearby;

	// Initialize the frame indices arrays

	gConsts.anFiringStripIndices = s_anFiringStripIndices;
	gConsts.anMovingStripIndices = s_anMovingStripIndices;
	gConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gConsts.sfxFire = ksfxRocketInfantryFire;
	gConsts.sfxImpact = ksfxRocketInfantryImpact;

	gConsts.sfxcDestroyed = ksfxcInfantryDestroyed;
	gConsts.sfxcSelect = ksfxcMajor02Select;
	gConsts.sfxcMove = ksfxcMajor02Move;
	gConsts.sfxcAttack = ksfxcMajor02Attack;

	return MobileUnitGob::InitClass(&gConsts, pini);
}

void LRInfantryGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gConsts);
}

LRInfantryGob::LRInfantryGob() : MobileUnitGob(&gConsts)
{
}

bool LRInfantryGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// MobileUnitGob handles rotating towards the target, firing delay, 
	// and starting the fire animation

	if (!MobileUnitGob::Fire(puntTarget, wx, wy, wdx, wdy))
		return false;

	// Fire off the shot!

	wdx += ((GetRandom() & 7) - 3) * kwcTile16th;
	wdy += ((GetRandom() & 7) - 3) * kwcTile16th;
	Point ptSpecial;
	m_ani.GetSpecialPoint(&ptSpecial); // from the tip of the launcher

	CreateRocketGob(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), m_wx + wdx, m_wy + wdy, 
			GetDamageTo(puntTarget), m_gid, puntTarget->GetId());

	// Play sfx

	gsndm.PlaySfx(m_pmuntc->sfxFire);

	return true;
}

void LRInfantryGob::Idle()
{
	// 1/4 of the time we pivot left, 1/4 we pivot right, and 1/2 we play the idle

	switch (GetRandom() & 3) {
	case 0:
		m_dir--;
		if (m_dir < 0)
			m_dir = 15;
		StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniResetWhenDone);
		break;

	case 1:
		m_dir++;
		if (m_dir > 15)
			m_dir = 0;
		StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniResetWhenDone);
		break;

	default:
		StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniResetWhenDone);
		break;
	}
}

int LRInfantryGob::GetIdleCountdown()
{
    return (GetRandom() % 100) + 50; // somewhere between 8 & 12 seconds
}

int LRInfantryGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	State(kstDying)
		OnEnter
			TRect trc;
			GetTileRect(&trc);

			Deactivate();

			// Redraw this part of minimap. It will skip inactive munts

			gpmm->RedrawTRect(&trc);

			m_ff ^= kfGobLayerDepthSorted | kfGobLayerSurfaceDecal;

			gsndm.PlaySfx(SfxFromCategory(m_pmuntc->sfxcDestroyed));
			m_ani.Start("die 3", kfAniIgnoreFirstAdvance);
			MarkRedraw();

			// Fade corpse away in 10 seconds

			gsmm.SendDelayedMsg(kmidDelete, 1000, m_gid, m_gid);

		OnUpdate
			AdvanceAnimation(&m_ani);

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


//
// RocketGob implementation
//

AnimationData *RocketGob::s_panidRocket = NULL;
int RocketGob::s_nTrailStrip;

RocketGob *CreateRocketGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget)
{
	if (!ggobm.IsBelowLimit(knLimitSupport))
		return NULL;

	RocketGob *pgob = new RocketGob();
	Assert(pgob != NULL, "out of memory!");
	if (pgob == NULL)
		return NULL;

	if (!pgob->Init(wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget)) {
		delete pgob;
		return NULL;
	}

	return pgob;
}

bool RocketGob::InitClass(IniReader *pini)
{
	s_panidRocket = LoadAnimationData("rocket.anir");
	if (s_panidRocket == NULL)
		return false;
	s_nTrailStrip = s_panidRocket->GetStripIndex("trail");
	return true;
}

void RocketGob::ExitClass()
{
	delete s_panidRocket;
	s_panidRocket = NULL;
}

RocketGob::RocketGob()
{
	m_ff |= kfGobStateMachine | kfGobLayerSmokeFire;
}

bool RocketGob::Init(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget) 
{
	// Units fire from their special point which may be off the edge of the map.
	// We cannot allow Gobs to be off the map so here we bring it on.

	BringInBounds(&wx, &wy);

#ifdef DEBUG
	TileMap *ptmap = gsim.GetLevel()->GetTileMap();
	Size sizT;
	ptmap->GetTCoordMapSize(&sizT);
	Assert(wx < WcFromTc(sizT.cx) && wy < WcFromTc(sizT.cy));
	Assert(wxTarget < WcFromTc(sizT.cx) && wyTarget < WcFromTc(sizT.cy));
#endif

	m_gidOwner = gidOwner;
	m_gidTarget = gidTarget;
	m_nDamage = nDamage;

	// TUNE: rocket movement rate

	m_li.Init(wx, wy, wxTarget, wyTarget, kwcTile / 3);	// was 6.0

	// LineIterator initializes x,y to the first step-integral point on the line,
	// presuming that the final step should be at the target x,y

	m_wx = m_li.GetWX();
	m_wy = m_li.GetWY();

	m_ani.Init(s_panidRocket);
	StartAnimation(&m_ani, 0, CalcDir(wxTarget - wx, wyTarget - wy), kfAniDone);

	// Add the fresh Gob to the GobMgr. GobMgr::AddGob assigns this Gob a gid

	ggobm.AddGob(this);

	// Let the target know when it will be hit. Doing it this way
	// means the hit will arrive in the same number of updates for all
	// players (the number of updates calc'd by the shooter). Depending
	// on screen resolution, the animated travel time may be off by an update

	Message msgT; 
	msgT.mid = kmidHit; 
	msgT.smidSender = m_gidOwner; 
	msgT.smidReceiver = m_gidTarget; 
	msgT.Hit.gidAssailant = m_gidOwner; 
	msgT.Hit.sideAssailant = ggobm.GetGob(m_gidOwner)->GetSide();
	msgT.Hit.nDamage = m_nDamage; 

	// BUGBUG: m_li.GetStepsRemaining is influenced by the firing point which is resolution dependent.
	// Instead, this message's delay should be derived using the distance from the world coordinate
	// centers of the source and target

	gsmm.SendDelayedMsg(&msgT, (m_li.GetStepsRemaining() + 1) * (kcmsUpdate / 10)); 

	return true;
}

// RocketGobs don't get loaded

bool RocketGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	return true;
}

bool RocketGob::IsSavable()
{
	return false;
}

GobType RocketGob::GetType()
{
	return kgtRocket;
}

void RocketGob::GetClippingBounds(Rect *prc)
{
	// hardcoded that the travel is strip 0 and the impact is strip 1

	if (m_ani.GetStrip() == 0) {
		if (!(gwfPerfOptions & kfPerfRocketShots)) {
			prc->SetEmpty();
			return;
		}
	} else {
		if (!(gwfPerfOptions & kfPerfRocketImpacts)) {
			prc->SetEmpty();
			return;
		}
	}

	m_ani.GetBounds(prc);
	prc->Offset(PcFromUwc(m_wx), PcFromUwc(m_wy));
}

void RocketGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	if (nLayer == knLayerSmokeFire) {
		// hardcoded that the travel is strip 0 and the impact is strip 1

		if (m_ani.GetStrip() == 0) {
			if (!(gwfPerfOptions & kfPerfRocketShots))
				return;
		} else {
			if (!(gwfPerfOptions & kfPerfRocketImpacts))
				return;
		}

		m_ani.Draw(pbm, PcFromUwc(m_wx) - xViewOrigin, PcFromUwc(m_wy) - yViewOrigin, m_pplr->GetSide());
	}
}

#if defined(DEBUG_HELPERS)
char *RocketGob::GetName()
{
	return "Rocket";
}
#endif

int RocketGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnUpdate
		// Advance the Rocket animation

		if (m_ani.GetStrip() == 0) {
			AdvanceAnimation(&m_ani);

			// Advance the Rocket position

			if (m_li.Step()) {
				WCoord wxOld = m_wx;
				WCoord wyOld = m_wy;
				m_wx = m_li.GetWX();
				m_wy = m_li.GetWY();

				// Keep GobMgr in the loop so it can maintain proper depth sorting

				if (m_wx != wxOld || m_wy != wyOld) {
					ggobm.MoveGob(this, wxOld, wyOld, m_wx, m_wy);
					MarkRedraw();
				}

				// Spawn a puff at the old position

				if (gwfPerfOptions & kfPerfRocketTrails) {
					if (m_li.GetStepsRemaining() & 1)
						CreateAnimGob(wxOld, wyOld, kfAnmDeleteWhenDone | kfAnmSmokeFireLayer, NULL, s_panidRocket, s_nTrailStrip, ksmidNull, NULL);
				}

				// Rocket gobs require every update

				m_unvl.MinSkip();

			} else {

				// Play impact sound

				UnitGob *punt = (UnitGob *)ggobm.GetGob(m_gidOwner);
				if (punt != NULL) {
					UnitConsts *puntc = (UnitConsts *)punt->GetConsts();
					gsndm.PlaySfx(puntc->sfxImpact);
				}

				// Start the impact animation

				StartAnimation(&m_ani, 1, 0, 0);

				// Expect to get called next update

				m_unvl.MinSkip();
			}

			// Assume valid if we're not drawing rockets

			if (!(gwfPerfOptions & kfPerfRocketShots))
				m_ff &= ~kfGobRedraw;			
		} else {

			// Advance the impact animation

			if (!AdvanceAnimation(&m_ani)) {

				// Kill this Rocket

				ggobm.RemoveGob(this);
				delete this;
				return knDeleted;
			}

			// Assume valid if we're not drawing rocket impacts

			if (!(gwfPerfOptions & kfPerfRocketImpacts))
				m_ff &= ~kfGobRedraw;
		}

EndStateMachine
}

} // namespace wi