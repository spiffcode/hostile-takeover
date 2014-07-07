#include "inc/basictypes.h"
#include "inc/rip.h"
#include "mpshared/netmessage.h"
#include "mpshared/mpht.h"
#include "base/bytebuffer.h"
#include "base/format.h"

namespace wi {

long gatGameSpeeds[15] = {
    64, 48, 32, 24, 16, 12, 10, 8, 7, 6, 5, 4, 3, 2, 1
};

#ifndef __CPU_68K

#define SwapWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))
#define SwapDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))

void MessageByteOrderSwap(word cmsgs, Message *pmsg, bool fNative)
{
	for (; cmsgs != 0; cmsgs--, pmsg++) {
		word mid = pmsg->mid;
        if (!fNative) {
            mid = base::ByteBuffer::HostToNetWord(pmsg->mid);
        }

		pmsg->mid = SwapWord(pmsg->mid);
		pmsg->smidSender = SwapWord(pmsg->smidSender);
		pmsg->smidReceiver = SwapWord(pmsg->smidReceiver);
		pmsg->tDelivery = SwapDword(pmsg->tDelivery);

		switch (mid) {
		case kmidHit:
		case kmidNearbyAllyHit:
		case kmidDelete:
		case kmidEnemyNearby:
		case kmidPowerLowHigh:
		case kmidMoveWaitingNearby:
		case kmidBeingUpgraded:
		case kmidUpgradeComplete:
		case kmidAnimationComplete:
		case kmidBuildComplete:
		case kmidFireComplete:
		case kmidSpawnSmoke:
		case kmidGalaxiteDelivery:
		case kmidMoveAction:
		case kmidAttackAction:
		case kmidGuardAction:
		case kmidGuardVicinityAction:
		case kmidGuardAreaAction:
		case kmidHuntEnemiesAction:
		case kmidFire:
		case kmidHeal:
		case kmidPlaySfx:
			// These are only sent. If they arrive here, it's a mistake

			Assert(false, "Message %d is classified as sent only", pmsg->mid);
			break;

		default:
			// Message unknown, not classified yet

			Assert(false, "Message %d unknown, not classified yet", pmsg->mid);
			break;

		// Posted

		case kmidMineCommand:
			pmsg->MineCommand.gidTarget = SwapWord(pmsg->MineCommand.gidTarget);
			pmsg->MineCommand.wptTarget.wx = SwapWord(pmsg->MineCommand.wptTarget.wx);
			pmsg->MineCommand.wptTarget.wy = SwapWord(pmsg->MineCommand.wptTarget.wy);
			break;

		case kmidAttackCommand:
			pmsg->AttackCommand.gidTarget = SwapWord(pmsg->AttackCommand.gidTarget);
			pmsg->AttackCommand.wptTarget.wx = SwapWord(pmsg->AttackCommand.wptTarget.wx);
			pmsg->AttackCommand.wptTarget.wy = SwapWord(pmsg->AttackCommand.wptTarget.wy);
			pmsg->AttackCommand.wptTargetCenter.wx = SwapWord(pmsg->AttackCommand.wptTargetCenter.wx);
			pmsg->AttackCommand.wptTargetCenter.wy = SwapWord(pmsg->AttackCommand.wptTargetCenter.wy);
			pmsg->AttackCommand.tcTargetRadius = SwapWord(pmsg->AttackCommand.tcTargetRadius);
			pmsg->AttackCommand.wcMoveDistPerUpdate = SwapWord(pmsg->AttackCommand.wcMoveDistPerUpdate);
			break;

		case kmidMoveCommand:
			pmsg->MoveCommand.gidTarget = SwapWord(pmsg->MoveCommand.gidTarget);
			pmsg->MoveCommand.wptTarget.wx = SwapWord(pmsg->MoveCommand.wptTarget.wx);
			pmsg->MoveCommand.wptTarget.wy = SwapWord(pmsg->MoveCommand.wptTarget.wy);
			pmsg->MoveCommand.wptTargetCenter.wx = SwapWord(pmsg->MoveCommand.wptTargetCenter.wx);
			pmsg->MoveCommand.wptTargetCenter.wy = SwapWord(pmsg->MoveCommand.wptTargetCenter.wy);
			pmsg->MoveCommand.tcTargetRadius = SwapWord(pmsg->MoveCommand.tcTargetRadius);
			pmsg->MoveCommand.wcMoveDistPerUpdate = SwapWord(pmsg->MoveCommand.wcMoveDistPerUpdate);
			break;

		case kmidBuildOtherCommand:
			pmsg->BuildOtherCommand.ut = SwapWord(pmsg->BuildOtherCommand.ut);
			pmsg->BuildOtherCommand.wpt.wx = SwapWord(pmsg->BuildOtherCommand.wpt.wx);
			pmsg->BuildOtherCommand.wpt.wy = SwapWord(pmsg->BuildOtherCommand.wpt.wy);
			break;

		case kmidAbortBuildOtherCommand:
			pmsg->AbortBuildOtherCommand.ut = SwapWord(pmsg->AbortBuildOtherCommand.ut);
			break;

		case kmidUpgradeCommand:
			pmsg->UpgradeCommand.wfUpgrade = SwapWord(pmsg->UpgradeCommand.wfUpgrade);
			break;

		case kmidDeliverCommand:
			pmsg->DeliverCommand.gidTarget = SwapWord(pmsg->DeliverCommand.gidTarget);
			break;

		case kmidTransformCommand:
		case kmidSelfDestructCommand:
		case kmidRepairCommand:
		case kmidAbortUpgradeCommand:
			// No parameters
			break;

        case kmidPlayerDisconnect:
            pmsg->PlayerDisconnect.pid = SwapWord(pmsg->PlayerDisconnect.pid);
            pmsg->PlayerDisconnect.nReason =
                    SwapWord(pmsg->PlayerDisconnect.nReason);
            break;
		}
	}
}

void SwapGameParams(GameParams *prams)
{
    prams->packid.id = SwapDword(prams->packid.id);
    prams->dwVersionSimulation = SwapDword(prams->dwVersionSimulation);
    prams->tGameSpeed = SwapDword(prams->tGameSpeed);
}

void NetMessageByteOrderSwap(NetMessage *pnm, bool fNative)
{
	// Swap endian order

    word nmid;
    if (fNative) {
        nmid = pnm->nmid;
    } else {
        nmid = SwapWord(pnm->nmid);
    }

	pnm->nmid = SwapWord(pnm->nmid);
	pnm->cb = SwapWord(pnm->cb);

	switch (nmid) {
	case knmidCsPlayerJoin:
	case knmidCsClientReady:
	case knmidScCantAcceptMoreConnections:
	case knmidScServerInfo:
	case knmidCsConnect:
    case knmidCsRequestBeginGame:
    case knmidScBeginGameFail:
    case knmidCsLagAcknowledge:
    case knmidCsChallengeWin:
		// Nothing to do for these

		break;

    case knmidCsWinStats:
        {
            WinStatsNetMessage *pwsnm = (WinStatsNetMessage *)pnm;
            pwsnm->ws.cCreditsAcquired = SwapDword(pwsnm->ws.cCreditsAcquired);
            pwsnm->ws.cCreditsConsumed = SwapDword(pwsnm->ws.cCreditsConsumed);
            pwsnm->ws.sidm = SwapWord(pwsnm->ws.sidm);
            pwsnm->ws.sidmAllies = SwapWord(pwsnm->ws.sidmAllies);
            pwsnm->ws.cEnemyMobileUnitsKilled =
                    SwapWord(pwsnm->ws.cEnemyMobileUnitsKilled);
            pwsnm->ws.cEnemyStructuresKilled =
                    SwapWord(pwsnm->ws.cEnemyStructuresKilled);
            pwsnm->ws.cMobileUnitsLost = SwapWord(pwsnm->ws.cMobileUnitsLost);
            pwsnm->ws.cStructuresLost = SwapWord(pwsnm->ws.cStructuresLost);
            pwsnm->ws.ff = SwapWord(pwsnm->ws.ff);
            for (int i = 0; i < ARRAYSIZE(pwsnm->ws.acut); i++) {
                pwsnm->ws.acut[i] = SwapWord(pwsnm->ws.acut[i]);
            }
            for (int i = 0; i < ARRAYSIZE(pwsnm->ws.acutBuilt); i++) {
                pwsnm->ws.acutBuilt[i] = SwapWord(pwsnm->ws.acutBuilt[i]);
            }
        }
        break;

    case knmidScCheckWin:
        {
            CheckWinNetMessage *pcwnm = (CheckWinNetMessage *)pnm;
            pcwnm->pid = SwapWord(pcwnm->pid);
        }
        break;

	case knmidScGameParams:
		{
			GameParamsNetMessage *pgpnm = (GameParamsNetMessage *)pnm;
            SwapGameParams(&pgpnm->rams);
		}
		break;

	case knmidScBeginGame:
		{
			BeginGameNetMessage *pbgnm = (BeginGameNetMessage *)pnm;
			pbgnm->ulRandomSeed = SwapDword(pbgnm->ulRandomSeed);
		}
		break;

	case knmidScPlayersUpdate:
		{
			PlayersUpdateNetMessage *punm = (PlayersUpdateNetMessage *)pnm;
			int cplrr = fNative ? punm->cplrr : SwapWord(punm->cplrr);
			punm->cplrr = SwapWord(punm->cplrr);
			for (int iplrr = 0; iplrr < cplrr; iplrr++) {
				PlayerRecord *pplrr = &punm->aplrr[iplrr];
				pplrr->pid = SwapWord(pplrr->pid);
				pplrr->side = SwapWord(pplrr->side);
                pplrr->wf = SwapWord(pplrr->wf);
			}
		}
		break;

	case knmidCsClientCommands:
		{
			ClientCommandsNetMessage *pccnm = (ClientCommandsNetMessage *)pnm;
			MessageByteOrderSwap(fNative ? pccnm->cmsgCommands : SwapWord(pccnm->cmsgCommands), pccnm->amsgCommands, fNative);
			pccnm->cmsgCommands = SwapWord(pccnm->cmsgCommands);
		}
		break;

	case knmidScUpdate:
		{
			UpdateNetMessage *punm = (UpdateNetMessage *)pnm;
			MessageByteOrderSwap(fNative ? punm->cmsgCommands : SwapWord(punm->cmsgCommands), punm->amsgCommands, fNative);
			punm->cmsgCommands = SwapWord(punm->cmsgCommands);
			punm->cUpdatesBlock = SwapDword(punm->cUpdatesBlock);
			punm->cUpdatesSync = SwapDword(punm->cUpdatesSync);
		}
		break;

	case knmidCsUpdateResult:
		{
			UpdateResultNetMessage *purnm = (UpdateResultNetMessage *)pnm;
			purnm->ur.cUpdatesBlock = SwapDword(purnm->ur.cUpdatesBlock);
			purnm->ur.hash = SwapDword(purnm->ur.hash);
			purnm->ur.cmsLatency = SwapDword(purnm->ur.cmsLatency);
		}
		break;

	case knmidScLagNotify:
		{
			LagNotifyNetMessage *plnnm = (LagNotifyNetMessage *)pnm;
			plnnm->pidLagging = SwapWord(plnnm->pidLagging);
			plnnm->cSeconds = SwapWord(plnnm->cSeconds);
		}
		break;

	case knmidScPlayerDisconnect:
		{
			PlayerDisconnectNetMessage *ppdnm = (PlayerDisconnectNetMessage *)pnm;
			ppdnm->pid = SwapWord(ppdnm->pid);
			ppdnm->nReason = SwapWord(ppdnm->nReason);
		}
		break;

    case knmidCsKillLaggingPlayer:
        {
            KillLaggingPlayerNetMessage *pklpnm =
                    (KillLaggingPlayerNetMessage *)pnm;
            pklpnm->pid = SwapWord(pklpnm->pid);
            pklpnm->fYes = SwapWord(pklpnm->fYes);
        }
        break;

	case knmidCsPlayerResign:
		{
			PlayerResignNetMessage *pprnm = (PlayerResignNetMessage *)pnm;
			pprnm->pid = SwapWord(pprnm->pid);
		}
		break;

    case knmidScSyncError:
        {
            SyncErrorNetMessage *psenm = (SyncErrorNetMessage *)pnm;
            for (int i = 0; i < kcPlayersMax; i++) {
                psenm->aur[i].cUpdatesBlock =
                        SwapDword(psenm->aur[i].cUpdatesBlock);
                psenm->aur[i].hash = SwapDword(psenm->aur[i].hash);
                psenm->aur[i].cmsLatency =
                        SwapDword(psenm->aur[i].cmsLatency);
            }
            psenm->urLastStraw.cUpdatesBlock =
                    SwapDword(psenm->urLastStraw.cUpdatesBlock);
            psenm->urLastStraw.hash = SwapDword(psenm->urLastStraw.hash);
            psenm->urLastStraw.cmsLatency =
                    SwapDword(psenm->urLastStraw.cmsLatency);
        }
        break;

	default:
		Assert("Unhandled knmid! (%d)", nmid);
		break;
	}
}
#endif // ifndef __CPU_68K

bool FindZero(const char *psz, int cb)
{
    for (const char *pszT = psz; pszT < &psz[cb]; pszT++) {
        if (*pszT == 0) {
            return true;
        }
    }
    return false;
}

bool ValidateGameParams(const GameParams& params)
{
    bool fFound = false;
    for (int i = 0; i < ARRAYSIZE(gatGameSpeeds); i++) {
        if (params.tGameSpeed == gatGameSpeeds[i]) {
            fFound = true;
            break;
        }
    }
    if (!fFound) {
        return false;
    }
    if (!FindZero(params.szLvlFilename, sizeof(params.szLvlFilename))) {
        return false;
    }
    return true;
}

#ifdef RELEASE_LOGGING
const char *GameParamsToString(const GameParams& params) {
    char szHash[33];
    strncpyz(szHash, base::Format::ToHex(params.packid.hash,
            sizeof(params.packid.hash)), sizeof(szHash));
    return base::Format::ToString(
"id=%d hash=%s dwVersionSimulation=%d tGameSpeed=%ld szLvlFilename=\"%s\'",
            params.packid.id, szHash, params.dwVersionSimulation,
            params.tGameSpeed, params.szLvlFilename);
}
#endif

#ifdef RELEASE_LOGGING
void LogGameParams(const GameParams& params) {
    RLOG() << GameParamsToString(params);
}
#endif

#ifdef DEBUG_LOGGING
const char *PszFromNetMessage(NetMessage *pnm)
{
    static char s_szT[256];

	switch (pnm->nmid) {
	case knmidScCantAcceptMoreConnections:
		return "ScCantAcceptMoreConnections";
	
	case knmidScServerInfo:
		((ServerInfoNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

	case knmidCsClientCommands:
		((ClientCommandsNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;
		
	case knmidScUpdate:
		((UpdateNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;
		
	case knmidScGameParams:
		((GameParamsNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

	case knmidScBeginGame:
		((BeginGameNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

	case knmidCsClientReady:
		return "CsClientReady";

	case knmidCsPlayerJoin:
		((PlayerJoinNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

	case knmidScPlayersUpdate:
		 ((PlayersUpdateNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

	case knmidCsUpdateResult:
		((UpdateResultNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidCsPlayerResign:
        ((PlayerResignNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidCsRequestBeginGame:
        return "CsRequestBeginGame";

    case knmidScBeginGameFail:
        return "ScBeginGameFail";

    case knmidScPlayerDisconnect:
        ((PlayerDisconnectNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidCsKillLaggingPlayer:
        ((KillLaggingPlayerNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidScLagNotify:
        ((LagNotifyNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidCsLagAcknowledge:
        ((LagAcknowledgeNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidScSyncError:
        return "ScSyncError";

    case knmidCsWinStats:
        ((WinStatsNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidScCheckWin:
        ((CheckWinNetMessage *)pnm)->ToString(s_szT, sizeof(s_szT));
        break;

    case knmidCsChallengeWin:
        return "CsChallengeWin";

	default:
		sprintf(s_szT, "NetMessage(nmid: %d)", pnm->nmid);
		break;
	}

	return s_szT;
}
#endif

} // namespace wi
