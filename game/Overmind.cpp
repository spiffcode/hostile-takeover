#include "ht.h"

namespace wi {

//
// OvermindGob implementation
//

bool OvermindGob::InitClass(IniReader *pini)
{
	// No interesting global parameters have been defined for Overmind yet
	return true;
}

OvermindGob::OvermindGob()
{
	m_fEnabled = true;
}

OvermindGob::~OvermindGob()
{
}

//
// Gob methods
//

bool OvermindGob::Init(const char *pszName)
{
	m_wx = m_wy = 0;
	m_ff |= kfGobStateMachine;

#if 0	// The Overmind doesn't need to take sides. It will be given an owner Player (somewhere)
	// For now, Overmind takes the side the player isn't on

	if (gpplrLocal->GetSide() == kside1)
		m_side = kside2;
	else
		m_side = kside1;
#endif

	// Add the fresh Gob to the GobMgr. GobMgr::AddGob assigns this Gob a gid

	ggobm.AddGob(this);

	// Initialize the state machine by sending the first Enter msg
	// This message is posted so it will kick in at the first Update after
	// all the Gobs in the level have been instantiated.

	m_fInitializationComplete = false;
	return true;
}

bool OvermindGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
#if 0
	int wf = 0;
	char szBitmap[kcbFilename];
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%s,%d ,%d", szBitmap, &m_x, &m_y, &wf);
	if (cArgs < 3) {
		Assert("OvermindGob requires at least 3 valid initialization parameters");
		return false;
	}

	m_wf |= (word)wf;
#endif

	return Init(pszName);
}

bool OvermindGob::IsSavable()
{
	// False for the moment, but probably will be saved at some future point

	return false;
}

GobType OvermindGob::GetType()
{
	return kgtOvermind;
}

void OvermindGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	// CONSIDER: maybe draw some DEBUG status "insight into the Overmind"
}

//
// StateMachine methods
//

#if defined(DEBUG_HELPERS)
char *OvermindGob::GetName()
{
	return "Overmind";
}
#endif

void OvermindGob::Toggle()
{
	m_fEnabled = !m_fEnabled;
	m_unvl.MinSkip();
}

static UnitType s_autVtsBuildable[] = { kutLightTank, kutMediumTank, kutRocketVehicle, kutMachineGunVehicle, kutArtillery };
static UnitType s_autHrcBuildable[] = { kutShortRangeInfantry, kutLongRangeInfantry };

int OvermindGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnUpdate
		if (!m_fInitializationComplete) {
			m_fInitializationComplete = true;
			m_cAttackCountdown = 0;
			m_cBuildUnitCountdown = 0;

			// Search for any GalaxMiners owned by this Overmind and send them off to mine.

			for (Gob *pgobMiner = ggobm.GetFirstGob(); pgobMiner != NULL; pgobMiner = ggobm.GetNextGob(pgobMiner)) {
				if (pgobMiner->GetOwner() != m_pplr)
					continue;

				if (pgobMiner->GetType() != kgtGalaxMiner)
					continue;

				// Send the Miner off to mine
				// NOTE: We send a message rather than call MinerGob::Mine method directly 
				// so the state change will occur immediately.

				SendMineCommand(pgobMiner->GetId(), kwxInvalid, 0);
			}
		}

		if (!m_fEnabled)
			return knHandled;

		// Every 10 seconds, pick a unit with offensive capabilities and send them
		// after a strategic enemy structure

		m_cAttackCountdown -= m_unvl.GetUpdateCount();
		if (m_cAttackCountdown < 0) {
#ifdef STRESS
			m_cAttackCountdown = gfStress ? 13 : (int)(12.5 * 10);
#else
			m_cAttackCountdown = (int)(12.5 * 10);
#endif
			
			// Search the Gob list for a same-side Unit that doesn't have any commands pending and doesn't already
			// have a target.

			for (Gob *pgobPawn = ggobm.GetFirstGob(); pgobPawn != NULL; pgobPawn = ggobm.GetNextGob(pgobPawn)) {
				if (pgobPawn->GetOwner() != m_pplr)
					continue;

				dword ff = pgobPawn->GetFlags();
				if ((ff & (kfGobActive | kfGobMobileUnit)) != (kfGobActive | kfGobMobileUnit))
					continue;

				MobileUnitGob *pmuntPawn = (MobileUnitGob *)pgobPawn;
				if (!pmuntPawn->IsIdle())
					continue;

				if (pmuntPawn->HasAttackTarget())
					continue;

				// Don't bother attacking with a Miner!

				GobType gt = pmuntPawn->GetType();
				if (gt == kgtGalaxMiner || gt == kgtMobileHeadquarters)
					continue;

				// OK, now find a good target to attack

				int cTargets = 0;
				Gob *pgobTarget;
				for (pgobTarget = ggobm.GetFirstGob(); pgobTarget != NULL; pgobTarget = ggobm.GetNextGob(pgobTarget)) {
					if (pgobTarget->GetOwner() != m_pplr && 
#ifdef STRESS
							(((pgobTarget->GetFlags() & kfGobStructure) == kfGobStructure) || 
							(gfStress && (pgobTarget->GetFlags() & kfGobUnit)))
#else
							((pgobTarget->GetFlags() & kfGobStructure) == kfGobStructure)
#endif
							&& pmuntPawn->IsValidTarget(pgobTarget)) {
						cTargets++;
					}
				}

				if (cTargets != 0) {
					cTargets = GetRandom() % cTargets;
					for (pgobTarget = ggobm.GetFirstGob(); pgobTarget != NULL; pgobTarget = ggobm.GetNextGob(pgobTarget)) {
						if (pgobTarget->GetOwner() != m_pplr && 
								((pgobTarget->GetFlags() & kfGobStructure) == 
								kfGobStructure) && pmuntPawn->IsValidTarget(pgobTarget)) {
							if (cTargets-- == 0)
								break;
						}
					}
				}

				// If we can't find a good target, forget about it

				if (pgobTarget == NULL)
					break;
				Assert(pgobTarget->GetFlags() & kfGobUnit);
				UnitGob *puntTarget = (UnitGob *)pgobTarget;

				// Order our pawn to attack the target

				Message msgT;
				msgT.mid = kmidAttackAction;
				msgT.smidSender = pmuntPawn->GetId();
				msgT.smidReceiver = msgT.smidSender;
				puntTarget->GetAttackPoint(&msgT.AttackCommand.wptTarget);
				msgT.AttackCommand.gidTarget = puntTarget->GetId();
				msgT.AttackCommand.wptTargetCenter = msgT.AttackCommand.wptTarget;
				msgT.AttackCommand.tcTargetRadius = 0;
				msgT.AttackCommand.wcMoveDistPerUpdate = 0;
				gsmm.SendMsg(&msgT);
				break;
			}
		}
		m_unvl.MinSkip(m_cAttackCountdown);

		// Every 30 seconds, build a new offensive unit

		m_cBuildUnitCountdown -= m_unvl.GetUpdateCount();
		if (m_cBuildUnitCountdown < 0) {
#ifdef STRESS
			m_cBuildUnitCountdown = gfStress ? /*(int)(12.5 * 2)*/ 0 /* More! */ : (int)(12.5 * 30);
#else
			m_cBuildUnitCountdown = (int)(12.5 * 30);
#endif

			// Randomly choose between using an HRC or VTS to build a unit
			// and choose a random offensive unit for it to build.

			GobType gtBuilder;
			UnitType utBuildee;
			if ((GetRandom() & 1) == 1) {
				gtBuilder = kgtVehicleTransportStation;
				utBuildee = s_autVtsBuildable[GetRandom() % ARRAYSIZE(s_autVtsBuildable)];
			} else {
				gtBuilder = kgtHumanResourceCenter;
				utBuildee = s_autHrcBuildable[GetRandom() % ARRAYSIZE(s_autHrcBuildable)];
			}

			if (ggobm.IsBelowLimit(knLimitMobileUnit, m_pplr)) {
				// Search the Gob list for a same-side Builder that doesn't have any commands pending

				bool fBuilt = false;
				for (Gob *pgobBuilder = ggobm.GetFirstGob(); pgobBuilder != NULL; pgobBuilder = ggobm.GetNextGob(pgobBuilder)) {
					if (pgobBuilder->GetOwner() != m_pplr)
						continue;
					if (pgobBuilder->GetType() != gtBuilder)
						continue;

					BuilderGob *pbld = (BuilderGob *)pgobBuilder;
					if (pbld->IsBuildInProgress())
						continue;

					pbld->Build(utBuildee);
					fBuilt = true;
					break;
				}

#ifdef STRESS
				if (gfStress && !fBuilt) {

					// If it can't be built a 'legal' way, just spawn it at a random location

					MobileUnitGob *pmunt = (MobileUnitGob *)CreateGob(gapuntc[utBuildee]->gt);
					Assert(pmunt != NULL);
					if (pmunt != NULL) {
						// Find a free spot
						TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();

						while (true) {
							TCoord tx = GetRandom() % 64;
							TCoord ty = GetRandom() % 64;

							if (ptrmap->IsOccupied(tx, ty, 1, 1, (kbfStructure | kbfMobileUnit)))
								continue;

							bool fSuccess = pmunt->Init(WcFromTc(tx), WcFromTc(ty), m_pplr, 0, 0, NULL);
							Assert(fSuccess);
							if (!fSuccess)
								delete pmunt;
							break;
						}
					}
				}
#endif
			}
		}
		m_unvl.MinSkip(m_cBuildUnitCountdown);

EndStateMachine
}

} // namespace wi