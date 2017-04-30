#include "game/ht.h"
#include "game/wistrings.h"
#include "game/stateframe.h"

namespace wi {

CommandQueue gcmdq;

#ifdef STATS_DISPLAY
extern int gcBitmapsDrawn;
#endif
extern int gcMessagesPerUpdate;

extern bool gfDragSelecting;

//---------------------------------------------------------------------------
// Simulation implementation

Simulation::Simulation()
{
    m_plvl = NULL;
    m_fPaused = false;
    m_nMiniMapScale = 1;
    m_cgobVisible = 0;
    m_wxViewSave = 0;
    m_wyViewSave = 0;
    m_cUpdatesSave = -1;
}

Simulation::~Simulation()
{
}

bool Simulation::OneTimeInit()
{
    // Initialize all the Gob classes

    Status("Parse GobTemplates.ini...");
    IniReader *piniGobTemplates = LoadIniFile(gpakr, "GobTemplates.ini");
    if (piniGobTemplates == NULL)
        return false;

    Status("Init SurfaceDecalGob...");
    bool fSuccess = SurfaceDecalGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init SceneryGob...");
    fSuccess = fSuccess && SceneryGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init SRInfantryGob...");
    fSuccess = fSuccess && SRInfantryGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init HrcGob...");
    fSuccess = fSuccess && HrcGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init ProcessorGob...");
    fSuccess = fSuccess && ProcessorGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init ReactorGob...");
    fSuccess = fSuccess && ReactorGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init MinerGob...");
    fSuccess = fSuccess && MinerGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init VtsGob...");
    fSuccess = fSuccess && VtsGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init RadarGob...");
    fSuccess = fSuccess && RadarGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init HqGob...");
    fSuccess = fSuccess && HqGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init ResearchGob...");
    fSuccess = fSuccess && ResearchGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init LRInfantryGob...");
    fSuccess = fSuccess && LRInfantryGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init LTankGob...");
    fSuccess = fSuccess && LTankGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init MTankGob...");
    fSuccess = fSuccess && MTankGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init RTankGob...");
    fSuccess = fSuccess && RTankGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init GTankGob...");
    fSuccess = fSuccess && GTankGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init SpInfantryGob...");
    fSuccess = fSuccess && SpInfantryGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init OvermindGob...");
    fSuccess = fSuccess && OvermindGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init TankShotGob...");
    fSuccess = fSuccess && TankShotGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init RocketGob...");
    fSuccess = fSuccess && RocketGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init BulletGob...");
    fSuccess = fSuccess && BulletGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init AndyShotGob...");
    fSuccess = fSuccess && AndyShotGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init WarehouseGob...");
    fSuccess = fSuccess && WarehouseGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init GunTowerGob...");
    fSuccess = fSuccess && GunTowerGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init RocketTowerGob...");
    fSuccess = fSuccess && RocketTowerGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init MobileHqGob...");
    fSuccess = fSuccess && MobileHqGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init ArtilleryGob...");
    fSuccess = fSuccess && ArtilleryGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init ArtilleryShotGob...");
    fSuccess = fSuccess && ArtilleryShotGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init ReplicatorGob...");
    fSuccess = fSuccess && ReplicatorGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init AndyGob...");
    fSuccess = fSuccess && AndyGob::InitClass(piniGobTemplates);
    Assert(fSuccess);
    Status("Init FoxGob...");
    fSuccess = fSuccess && FoxGob::InitClass(piniGobTemplates);
    Assert(fSuccess);

    delete piniGobTemplates;

    // Do some other one-time initialization that we didn't want to
    // do at game startup because we want to startup snappily.

#ifdef DRAW_PATHS
    // Load the direction arrow bitmaps

    Status("Load path arrow bitmaps...");
    LoadArrows();
#endif
    
    return fSuccess;
}

void Simulation::OneTimeExit()
{
    SRInfantryGob::ExitClass();
    HrcGob::ExitClass();
    ProcessorGob::ExitClass();
    ReactorGob::ExitClass();
    MinerGob::ExitClass();
    VtsGob::ExitClass();
    RadarGob::ExitClass();
    HqGob::ExitClass();
    ResearchGob::ExitClass();
    LRInfantryGob::ExitClass();
    LTankGob::ExitClass();
    MTankGob::ExitClass();
    RTankGob::ExitClass();
    GTankGob::ExitClass();
    SpInfantryGob::ExitClass();
    TankShotGob::ExitClass();
    RocketGob::ExitClass();
    BulletGob::ExitClass();
    AndyShotGob::ExitClass();
    WarehouseGob::ExitClass();
    GunTowerGob::ExitClass();
    RocketTowerGob::ExitClass();
    MobileHqGob::ExitClass();
    ArtilleryGob::ExitClass();
    ArtilleryShotGob::ExitClass();
    ReplicatorGob::ExitClass();
    AndyGob::ExitClass();
    FoxGob::ExitClass();

#ifdef DRAW_PATHS
    FreeArrows();
#endif

    delete m_plvl;
    m_plvl = NULL;
}

bool Simulation::PerLevelInit()
{
#ifdef DEBUG_HELPERS
    void ClearLog();
    ClearLog();
#endif

    if (!gcmdq.Init())
        return false;

    m_wxView = m_wyView = 0;
    m_cUpdates = -1;
    m_tCurrent = 0;
    m_fGameOver = false;
    m_pbldm = new BuildMgr();
    if (m_pbldm == NULL)
        return false;

    m_cupdTriggerMgrUpdateLast = 0;

    // So visible gobs get recalced

    m_wxViewSave = (WCoord)-1;
    m_wyViewSave = (WCoord)-1;
    m_cUpdatesSave = -2;

    return true;
}

void Simulation::PerLevelExit()
{
    gcmdq.Exit();

    delete m_plvl;
    m_plvl = NULL;
    delete m_pbldm;
    m_pbldm = NULL;
}

bool Simulation::LoadLevel(const char *pszLevelName)
{
    Assert(m_plvl == NULL);

    m_plvl = new Level();
    if (!m_plvl->Init(pszLevelName)) {
        delete m_plvl;
        m_plvl = NULL;
        return false;
    }

    // Initialize level-specific player info for each player

    Player *pplr = gplrm.GetNextPlayer(NULL);
    for (; pplr != NULL; pplr = gplrm.GetNextPlayer(pplr)) {
        SideInfo *psidi = m_plvl->GetSideInfo(pplr->GetSide());
        pplr->SetCredits(psidi->nInitialCredits, false);

        // UNDONE: Level could (should?) provide this state
        pplr->SetUpgrades(0);

        if (pplr == gpplrLocal) {

            // Center the view around the level-specified initial view position

            Size siz;
            gpupdSim->GetMapSize(&siz);
            SetViewPos(psidi->wptInitialView.wx - (WcFromTc(siz.cx) / 2), 
                    psidi->wptInitialView.wy - (WcFromTc(siz.cy) / 2), true);
        }

        // In a multiplayer game supporting a range of players (e.g. 2-4) dummy
        // players are allocated for any 'human' sides that aren't in use by
        // human players. The dummy players must be present so the level, which
        // contains units for all sides, can load properly. Here as we load the
        // level we remove all units owned by the unfulfilled players.

        word wf = pplr->GetFlags();
        if (wf & kfPlrUnfulfilled) {
            Gob *pgobT;
            for (pgobT = ggobm.GetFirstGob(); pgobT != NULL; ) {

                // Get the next Gob BEFORE possibly deleting the one it points
                // to

                Gob *pgobNext = ggobm.GetNextGob(pgobT);
                if (pgobT->GetOwner() == pplr) {
                    if (pgobT->GetFlags() & kfGobUnit) {
                        UnitGob *punt = (UnitGob *)pgobT;
                        punt->Deactivate();
                        punt->Delete();

                        // Don't count these units as ever having been built
                        pplr->DecUnitBuiltCount(punt->GetUnitType());
                    } else {
                        // Not a unit gob, but still owned by this player
                        // (for example, a tank shot). Remove / delete it.
                        ggobm.RemoveGob(pgobT);
                        delete pgobT;
                    }
                }
                pgobT = pgobNext;
            }
        }
    }

    return true;
}

#define knVerSimState 3
bool Simulation::LoadState(Stream *pstm)
{
    // Do version handling

    byte nVer = pstm->ReadByte();
    if (nVer != knVerSimState)
        return false;

    // Load update count!

    m_cUpdates = pstm->ReadDword();

    // Load view

    WCoord wxView = pstm->ReadWord();
    WCoord wyView = pstm->ReadWord();

    // Load Simulation tick count

    m_tCurrent = pstm->ReadDword();
    m_cupdTriggerMgrUpdateLast = pstm->ReadDword();

    // Load level

    Assert(m_plvl == NULL);
    m_plvl = new Level();
    if (m_plvl == NULL)
        return false;
    if (!m_plvl->LoadState(pstm)) {
        delete m_plvl;
        m_plvl = NULL;
        return false;
    }

    // Load state machine mgr (delayed messages)

    gsmm.LoadState(pstm);

    // Save command queue (usually empty)

    gcmdq.LoadState(pstm);

    // Set view pos

    SetViewPos(wxView, wyView, true);

    // Done

    return pstm->IsSuccess();
}

bool Simulation::SaveState(Stream *pstm)
{
    // Save version

    pstm->WriteByte(knVerSimState);

    // Save update count

    pstm->WriteDword(m_cUpdates);

    // Save view x,y

    pstm->WriteWord(m_wxView);
    pstm->WriteWord(m_wyView);

    // Save Simulation tick count

    pstm->WriteDword((dword)m_tCurrent);
    pstm->WriteDword(m_cupdTriggerMgrUpdateLast);

    // Save level

    m_plvl->SaveState(pstm);

    // Save state machine mgr (delayed messages...)

    gsmm.SaveState(pstm);

    // Save command queue (usually empty, but just in case...)

    gcmdq.SaveState(pstm);

    return pstm->IsSuccess();
}

long Simulation::GetTickCount()
{
    return m_tCurrent;
}

void Simulation::AddTimer(Timer *ptmr, long ct)
{
    // fyi you are the first user of a timer based on simulation time.
    // you'll need to add code to LoadState & SaveState to preserve
    // your timer times because simulation time is preserved across load/save
    Assert(false);
    AddTimer(ptmr, ct);
}

// The tCurrent increment can't really ever be anything other than 8 (80 ms)
// because Gobs assume it as their update rate and increment animations, etc
// every time they're Updated.

void Simulation::Update(CommandQueue *pcmdq)
{
    if (m_fPaused)
        return;

    // Give some time to sound servicing

    HostSoundServiceProc();

    // Advance Simulation time

    m_tCurrent += kcmsUpdate / 10;
    m_cUpdates++;

    // Update triggers. This is done here so the triggers act on what the
    // player is currently seeing (e.g., Credits will show >= NNNN when
    // >= NNNN Credits condition is satisfied).
    // PostEvent so that modal actions can occur

    // Note - CountdownTimer trigger depends on this being better than once a second.
    // TUNE:
    #define kcupdTriggerMgrUpdate 6

    if (m_cupdTriggerMgrUpdateLast == 0 || abs(m_cUpdates - m_cupdTriggerMgrUpdateLast) >= kcupdTriggerMgrUpdate) {
        m_cupdTriggerMgrUpdateLast = m_cUpdates;
        ggame.ScheduleUpdateTriggers();
    }

    // Update Players

    gplrm.Update(m_cUpdates);

    // Update unit groups.

    m_plvl->GetUnitGroupMgr()->Update();

    // Update Build Manager
    // OPT: this doesn't have to be done every update

    m_pbldm->Update();

    // Process all queued commands
#ifdef DEBUG_HELPERS
    extern void UpdateCommandQueueViewer();
    UpdateCommandQueueViewer();
#endif

    int cmsg = pcmdq->GetCount();
#ifdef STATS_DISPLAY
    extern int gcbCommandsQueued;
    gcbCommandsQueued += cmsg * sizeof(Message);
#endif
    int i = 0;
    Message *pmsg = pcmdq->GetFirst();
    for (; i < cmsg; i++, pmsg++) {
        // Disconnect is processed in order, by all clients, at the same time.
        // To do this, it is turned into a message, since messages have a
        // synchronization mechanism.
        if (pmsg->mid == kmidPlayerDisconnect) {
            HandlePlayerDisconnect(pmsg);
            continue;
        }
        gsmm.RouteMessage(pmsg);
    }
    pcmdq->Clear();

    gcMessagesPerUpdate = 0;

    ScanDispatch(m_tCurrent);

    // Send Update messages to all Gobs with state machines

    Message msgUpdate;
    memset(&msgUpdate, 0, sizeof(msgUpdate));
    msgUpdate.mid = kmidReservedUpdate;
    msgUpdate.smidSender = ksmidNull;

    // Snapshot the gids of the Gobs we care about so we aren't vulnerable
    // to the insertion or deletion of elements on the Gob list during the
    // kmidReserveUpdate callbacks.

    Gid agid[kcpgobMax];
    Gid *pgidT = agid;
    int cgid = 0;
    Gob *pgobT;
    for (pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
        if (pgobT->GetFlags() & kfGobStateMachine) {
            *pgidT++ = pgobT->GetId();
            cgid++;
        }
    }

    pgidT = agid;
    for (i = 0; i < cgid; i++, pgidT++) {

        // Gob might have been destroyed as a consequence of an earlier 
        // Gob's Update. If so, skip it.

        pgobT = ggobm.GetGob(*pgidT, false);
        if (pgobT == NULL)
            continue;

#ifdef MP_DEBUG_SHAREDMEM
        gsideCurrent = pgobT->GetOwner()->GetSide();
        ggidCurrent = pgobT->GetId();
        gcupdCurrent = m_cUpdates;
        ggtCurrent = pgobT->GetType();
#endif

        UpdateInterval *punvl = pgobT->GetUpdateInterval();
        if (punvl->Decrement()) {
            msgUpdate.smidReceiver = *pgidT;
            gsmm.SendMsg(&msgUpdate);

#ifdef PIL
            if (gfOS5Pa1Device) {
                if ((i & 15) == 0)
                    HostSoundServiceProc();
            }
#endif
        }
    }

    // Delayed messages are dispatched AFTER Update messages are sent so if
    // respondants create new Gobs they won't receive an immediate Update
    // before their first paint. This is important for cases like delayed shots.

#ifdef DEBUG_HELPERS
    extern void UpdateDelayedMessageViewer();
    UpdateDelayedMessageViewer();
#endif
    gsmm.DispatchDelayedMessages();

    long tCurrent = HostGetTickCount();

    // Sound effects

    // Credits increasing or decreasing?
#if 0
    switch (gpplrLocal->GetCreditsDirection()) {
    case 0:
        break;

    case 1:
        gsndm.PlaySfx(ksfxGameCreditsIncreasing);
        break;

    case -1:
        // If the credits user is for repair, only play decrease sound over a longer interval
        // TODO: figure out ultimate approach for managing interval for credit spending
// TUNE:
#define kctIntervalRepairNotify (75)
        if (true) { // gpplrLocal->GetCreditsConsumer() == knConsumerRepair) {
            static long s_tLastRepairNotify = 0;
            if (s_tLastRepairNotify == 0 || abs(tCurrent - s_tLastRepairNotify) >= kctIntervalRepairNotify) {
                s_tLastRepairNotify = tCurrent;
                gsndm.PlaySfx(ksfxGameCreditsDecreasing);
            }
        } else {
            gsndm.PlaySfx(ksfxGameCreditsDecreasing);
        }
        break;
    }
#endif

    // Base under attack?

// TUNE:
#define kctIntervalAttackNotify (30 * 100)
    if (gpplrLocal->GetFlags() & kfPlrStructureAttacked) {
        gpplrLocal->SetFlags(gpplrLocal->GetFlags() & ~kfPlrStructureAttacked);
        static long s_tLastAttackNotify = 0;
        if (s_tLastAttackNotify == 0 || abs((int)(tCurrent - s_tLastAttackNotify)) >= kctIntervalAttackNotify) {
            s_tLastAttackNotify = tCurrent;
            gsndm.PlaySfx(ksfxGameBaseUnderAttack);
            ShowAlert(kidsBaseUnderAttack);
        }
    }

    // Power too low?

// TUNE:
#define kctIntervalPowerLowNotify (30 * 100)
    if (gpplrLocal->GetPowerDemand() != 0 && gpplrLocal->GetPowerDemand() > gpplrLocal->GetPowerSupply()) {
        static long s_tLastPowerLowNotify = 0;
        if (s_tLastPowerLowNotify == 0 || (tCurrent - s_tLastPowerLowNotify) >= kctIntervalPowerLowNotify) {
            s_tLastPowerLowNotify = tCurrent;
            gsndm.PlaySfx(ksfxReactorPowerTooLow);
            ShowAlert(kidsLowPower);
        }
    }


#ifdef MP_STRESS
    // Has to be at the end of Update() because gcmdq serves the needs of both
    // collecting client commands and collecting multiplayer commands. This
    // role switching occurs in in knmidScUpdate handling in
    // SimUIForm::OnReceive.

    extern void MPStressUpdate();
    MPStressUpdate();
#endif

}

void Simulation::HandlePlayerDisconnect(Message *pmsg) {
    // Re-route this as a NetMessage to get proper handling
    if (gptra == NULL) {
        return;
    }
    IGameCallback *pgcb = gptra->GetGameCallback();
    if (pgcb == NULL) {
        return;
    }

    // This must not be modal
    PlayerDisconnectNetMessage pdnm;
    pdnm.pid = pmsg->PlayerDisconnect.pid;
    pdnm.nReason = pmsg->PlayerDisconnect.nReason;
    NetMessage *pnm = &pdnm;
    pgcb->OnNetMessage(&pnm);
}

#ifdef TRACKSTATE
void Simulation::TrackState(StateFrame *frame) {
    // Sim stuff

    int i = frame->AddCountedValue('SIMU');
    frame->AddValue('SEED', (dword)GetRandomSeed(), i);
    frame->AddValue('CMPU', (dword)gcMessagesPerUpdate, i);
    frame->AddValue('CUPD', (dword)m_cUpdates, i);

    // gobs

    Gob *pgobT;
    for (pgobT = ggobm.GetFirstGob(); pgobT != NULL;
            pgobT = ggobm.GetNextGob(pgobT)) {
        if (!(pgobT->GetFlags() & kfGobStateMachine)) {
            continue;
        }
        pgobT->TrackState(frame);
    }

    // players

    gplrm.TrackState(frame);
}
#endif

void Simulation::SetSelection(Rect *prc)
{
    if (gfLassoSelection || !gfDragSelecting) {
        return;
    }

    for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL;
            pgobT = ggobm.GetNextGob(pgobT)) {
        dword ff = pgobT->GetFlags();

        // If were in select mode then only Gobs inside the selection rectangle
        // get to be selected.

        if ((ff & kfGobUnit) == 0) {
            continue;
        }
        bool fSelect = false;
        UnitGob *punt = (UnitGob *)pgobT;
        if ((gfGodMode || punt->GetOwner() == gpplrLocal) &&
                ((ff & (kfGobActive | kfGobUnit)) == (kfGobActive | kfGobUnit))) {
            if ((ff & kfGobStructure) == 0 ||
                    (punt->GetConsts()->um & kumTowers) != 0) {
                WPoint wptGobCenter;
                punt->GetCenter(&wptGobCenter);
                if (prc->PtIn(wptGobCenter.wx, wptGobCenter.wy)) {
                    fSelect = true;
                }
            }
        }
        punt->Select(fSelect);
    }
}

void Simulation::ClearGobSelection() 
{
    // Deselect any selected Gobs

    for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
        dword ff = pgobT->GetFlags();
        if ((ff & (kfGobSelected | kfGobUnit)) == (kfGobSelected | kfGobUnit))
            ((UnitGob *)pgobT)->Select(false);
    }
}

void Simulation::SetGobSelected(Gob *pgob)
{
    ClearGobSelection();
    if ((pgob->GetFlags() & (kfGobSelected | kfGobUnit)) == kfGobUnit)
        ((UnitGob *)pgob)->Select(true);
}

void Simulation::SelectSameUnitTypes(UnitGob *punt, TRect *ptrc)
{
    Player *pplr = punt->GetOwner();
    if (pplr != gpplrLocal)
        return;

    UnitType ut = punt->GetUnitType();

    // Store the gobs on the stack.
    Gob *apgob[kcpgobMax / 2];

    int cpgob = ggobm.FindGobs(ptrc, apgob, ARRAYSIZE(apgob));
    for (int i = 0; i < cpgob; i++) {
        MobileUnitGob *pmuntT = (MobileUnitGob *)apgob[i];
        dword ff = pmuntT->GetFlags();
        if (!(ff & kfGobMobileUnit))
            continue;
        if (pmuntT->GetOwner() != pplr)
            continue;
        if (pmuntT->GetUnitType() != ut)
            continue;

        pmuntT->Select(true);
    }
}

void Simulation::FindVisibleGobs(Gob ***pppgobVisible, int *pcgobVisible)
{
    // Reuse the gob list if it's the same update count and view pos

    if (m_wxView != m_wxViewSave || m_wyView != m_wyViewSave || m_cUpdates != m_cUpdatesSave) {
        // Get visible gobs / refresh cache info

        Size siz;
        ggame.GetPlayfieldSize(&siz);
        short xView = PcFromUwc(m_wxView) & 0xfffe;
        short yView = PcFromUwc(m_wyView) & 0xfffe;
        Rect rcVisible;
        rcVisible.left = xView;
        rcVisible.top = yView;
        rcVisible.right = xView + siz.cx;
        rcVisible.bottom = yView + siz.cy;
        m_cgobVisible = ggobm.FindGobs(&rcVisible, m_apgobVisible, ARRAYSIZE(m_apgobVisible), m_plvl->GetFogMap()->GetMapPtr());
        m_wxViewSave = m_wxView;
        m_wyViewSave = m_wyView;
        m_cUpdatesSave = m_cUpdates;
    }

    // Return if asked

    if (pppgobVisible != NULL)
        *pppgobVisible = m_apgobVisible;
    if (pcgobVisible != NULL)
        *pcgobVisible = m_cgobVisible;
}

void Simulation::DrawBackground(UpdateMap *pupd, DibBitmap *pbm)
{
    // Draw tiles where the updatemap is invalid

    short xView = PcFromUwc(m_wxView) & 0xfffe;
    short yView = PcFromUwc(m_wyView) & 0xfffe;
    FogMap *pfogm = m_plvl->GetFogMap();
    TileMap *ptmap = m_plvl->GetTileMap();
    Size sizDib;
    pbm->GetSize(&sizDib);
    ptmap->Draw(pbm, 0, 0, sizDib.cx, sizDib.cy, xView, yView, pfogm->GetMapPtr(), pupd);

    HostSoundServiceProc();

    // Draw galaxite where the updatemap is invalid

    pfogm->DrawGalaxite(pbm, xView, yView, pupd, m_plvl->GetTerrainMap()->GetMapPtr());
    HostSoundServiceProc();
}

void Simulation::Draw(UpdateMap *pupd, DibBitmap *pbm)
{
    // Maps only draw on even coords
    // OPT: can this be streamlined?

    short xView = PcFromUwc(m_wxView) & 0xfffe;
    short yView = PcFromUwc(m_wyView) & 0xfffe;
    WCoord wxView = WcFromUpc(xView);
    WCoord wyView = WcFromUpc(yView);

    //
    // Normal full-size map (not mini-map)
    //

    // Set the view origin so that blt-style map scrolling is possible

    pupd->SetViewOrigin(xView, yView);

    // Give some time to sound servicing

    HostSoundServiceProc();

    // Find visible Gobs

    FindVisibleGobs();

#ifdef STATS_DISPLAY
    gcBitmapsDrawn = 0;
#endif

#ifdef DRAW_PATHS
    if (gfDrawPaths) {

        // Draw paths

        for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
            if ((pgob->GetFlags() & kfGobMobileUnit) && ((MobileUnitGob *)pgob)->m_ppathUnit != NULL)
                ((MobileUnitGob *)pgob)->DrawPath(pbm, wxView, wyView);
        }
    }
#endif

    HostSoundServiceProc();

    // Figure out what layers are in use. Much of the time only 1 or 2 layers are in use

    dword ffLayers = 0;
    Gob **ppgobStop = &m_apgobVisible[m_cgobVisible];
    Gob **ppgobT;
    for (ppgobT = m_apgobVisible; ppgobT < ppgobStop; ppgobT++)
        ffLayers |= (*ppgobT)->GetFlags();
    ffLayers &= kfGobLayerMask;

    // Turn off symbols layers if off

    if (!(gwfPerfOptions & kfPerfSymbolFlashing))
        ffLayers &= ~kfGobLayerSymbols;

    // Draw gobs. Skip layers that aren't in use; only call a gob if it wants to draw on
    // this layer

    dword ffLayerT = kfGobLayerSurfaceDecal;
    for (int nLayer = knLayerSurfaceDecal; nLayer <= knLayerEnd; nLayer++, ffLayerT <<= 1) {
        if (!(ffLayerT & ffLayers))
            continue;
        int cT = 0;
        for (ppgobT = m_apgobVisible; ppgobT < ppgobStop; ppgobT++) {
            Gob *pgobT = *ppgobT;
            dword ff = pgobT->GetFlags();

            // If gob is marked for redraw...

            if (ff & kfGobRedraw) {
                // If gob is painting on this layer then paint

                if (ff & ffLayerT) {
                    pgobT->Draw(pbm, xView, yView, nLayer);

#ifdef PIL
                    if (gfOS5Pa1Device) {
                        cT++;
                        if ((cT & 15) == 0)
                            HostSoundServiceProc();
                    }
#endif
                }
            }
        }
    }

    // Give some time to sound servicing

    HostSoundServiceProc();

    // Clear kfGobRedraw since the gobs are now "valid".
    // Do this after painting because some painting code ends up setting this bit

    for (ppgobT = m_apgobVisible; ppgobT < ppgobStop; ppgobT++) {
        Gob *pgobT = *ppgobT;
        pgobT->SetFlags(pgobT->GetFlags() & ~kfGobRedraw);
    }

#ifdef DRAW_LINES
    if (gfDrawLines) {
        // Draw target lines

        for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
            // Total hack-o-rama 
            if ((pgob->GetFlags() & kfGobMobileUnit) && ((MobileUnitGob *)pgob)->m_wptTarget.wx != kwxInvalid)
                ((MobileUnitGob *)pgob)->DrawTargetLine(pbm, xView, yView);
            if ((pgob->GetType() == kgtRocketTower || pgob->GetType() == kgtMachineGunTower) && 
                    ((TowerGob *)pgob)->m_wptTarget.wx != kwxInvalid)
                ((TowerGob *)pgob)->DrawTargetLine(pbm, xView, yView);
        }
    }
#endif


#ifdef MARK_TILE_BOUNDARIES
    {
        Color clr = GetColor(kiclrWhite);
        Size sizT;
        ggame.GetPlayfieldSize(&sizT);
        for (int y = -PcFromUwc(WcFrac(wyView)); y < sizT.cy; y += gcyTile) {
            for (int x = -PcFromUwc(WcFrac(wxView)); x < sizT.cx; x += gcxTile) {
                pbm->Fill(x, y, 2, 2, clr);
            }
        }
    }
#endif

#ifdef MARK_OCCUPIED_TILES
    {
        Color clr = GetColor(kiclrWhite);
        Size sizT;
        ggame.GetPlayfieldSize(&sizT);
        int ctxView = sizT.cx / gcxTile + 1;
        int ctyView = sizT.cy / gcyTile + 1;
        TerrainMap *ptrmap = m_plvl->GetTerrainMap();

        int xView = PcFromWc(wxView);
        int yView = PcFromWc(wyView);
        TCoord txView = TcFromWc(wxView);
        TCoord tyView = TcFromWc(wyView);
        for (TCoord ty = tyView; ty < tyView + ctyView; ty++) {
            for (TCoord tx = txView; tx < txView + ctxView; tx++) {
                if (ptrmap->IsOccupied(tx, ty, 1, 1)) {
                    int x = PcFromTc(tx) - xView;
                    int y = PcFromTc(ty) - yView;
                    pbm->Fill(x + gcxTile / 2 - 1, y + gcyTile / 2 - 1, 2, 2, clr);
                }
            }
        }
    }
#endif

    pupd->SetViewOrigin(0, 0);
    HostSoundServiceProc();

	// If the screen is larger than the map size we clear those areas to black color

	Size sizDib;
    pbm->GetSize(&sizDib);
    Size sizMap;
    m_plvl->GetTileMap()->GetMapSize(&sizMap);

    if (sizMap.cx < sizDib.cx) {
		Rect rc;
		rc.Set(sizMap.cx, 0, sizDib.cx, sizMap.cy);
		FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
	}
    if (sizMap.cy < sizDib.cy) {
		Rect rc;
		rc.Set(0, sizMap.cy, sizDib.cx, sizDib.cy);
		FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
	}
}

void Simulation::DrawFog(UpdateMap *pupd, DibBitmap *pbm)
{
    // Draw fog map

    short xView = PcFromUwc(m_wxView) & 0xfffe;
    short yView = PcFromUwc(m_wyView) & 0xfffe;
    m_plvl->GetFogMap()->Draw(pbm, xView, yView, pupd);
    HostSoundServiceProc();
}

bool Simulation::SetViewPos(WCoord wx, WCoord wy, bool fInit)
{
    // Pin the passed position to the bounds of the map

    Size sizMap;
    m_plvl->GetTileMap()->GetMapSize(&sizMap);

    Size sizPlayfield;
    ggame.GetPlayfieldSize(&sizPlayfield);

    WCoord wcxMax, wcyMax;
    int cxMax = sizMap.cx - sizPlayfield.cx;
    int cyyMax = sizMap.cy - sizPlayfield.cy;

    if (cxMax <= 0 || cxMax > kpcMax)
        wcxMax = 0;
    else
        wcxMax = WcFromUpc(cxMax);

    if (cyyMax <= 0 || cyyMax > kpcMax)
        wcyMax = 0;
    else
        wcyMax = WcFromUpc(cyyMax);

    if (wx < 0)
        wx = 0;
    else if (wx > wcxMax)
        wx = wcxMax;

    if (wy < 0)
        wy = 0;
    else if (wy > wcyMax)
        wy = wcyMax;

    // If we're initing, we're loading a new level. Invalidate and
    // set the map offset

    if (fInit) {
        gpupdSim->Reset();
        gpdisp->ResetScrollOffset();
        m_wxView = WcFromPc(0);
        m_wyView = WcFromPc(0);
    }
    
    if (m_wxView == wx && m_wyView == wy)
        return false;

    // Otherwise scroll the map

    int dx = (PcFromWc(m_wxView) & 0xfffe) - (PcFromWc(wx) & 0xfffe);
    int dy = (PcFromWc(m_wyView) & 0xfffe) - (PcFromWc(wy) & 0xfffe);
    gpfrmmSim->Scroll(dx, dy);

    // Set the new view pos

    m_wxView = wx;
    m_wyView = wy;

    // Force redraw before next update

    gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);

    // Debug

//    Trace("View: %d,%d", PcFromWc(m_wxView) & 0xfffe, PcFromWc(m_wyView) & 0xfffe);
    
    return true;
}

void Simulation::GetViewPos(WCoord *pwx, WCoord *pwy)
{
    Assert(pwx != NULL);
    Assert(pwy != NULL);
    
    *pwx = m_wxView;
    *pwy = m_wyView;
}

// Passed coordinates are in world coordinates

bool Simulation::HitTest(Enum *penm, WCoord wx, WCoord wy, word wf, Gob **ppgob)
{
    if (penm->m_pvNext == (void *)kEnmFirst) {
        FindVisibleGobs();
        penm->m_dwUser = 0;
    }

    for (; (int)penm->m_dwUser < m_cgobVisible; penm->m_dwUser++) {
        Gob *pgob = m_apgobVisible[penm->m_dwUser];
        if ((pgob->GetFlags() & wf) != wf)
            continue;
 
        WRect wrcT;
        pgob->GetUIBounds(&wrcT);
        if (!wrcT.PtIn(wx, wy))
            continue;

        *ppgob = pgob;
        penm->m_dwUser++;
        return true;
    }

    return false;
}

// Passed coordinates are in world coordinates

Gob *Simulation::FingerHitTest(WCoord wx, WCoord wy, word wf,
        bool *pfHitSurrounding)
{
    // Finger hit testing is tricky, because currently at least,
    // a given tile is smaller than a finger. It's difficult to pick
    // a cell on a grid, when the cells are smaller than a finger. One
    // solution is to make the tiles larger. Instead, the gobs will be
    // hit-tested specially, so that close hits register, while taking
    // special precaution for open terrain between closely packed
    // gobs.

    // Calc the world rect of the tile being hit

    WRect wrcTileHit;
    wrcTileHit.left = WcTrunc(wx);
    wrcTileHit.top = WcTrunc(wy);
    wrcTileHit.right = wrcTileHit.left + kwcTile;
    wrcTileHit.bottom = wrcTileHit.top + kwcTile;
    WRect wrcTileHitAccum;
    wrcTileHitAccum.SetEmpty();
    
    // Find the "closest" gob being hit, and keep track of hit area overlap
    // with the tile being hit.

    Gob *pgobHitBest = NULL;
    WCoord wcDistanceBest = kwcMax;

    FindVisibleGobs();
    for (int i = 0; i < m_cgobVisible; i++) {
        Gob *pgobHit = m_apgobVisible[i];
        if ((pgobHit->GetFlags() & wf) != wf) {
            continue;
        }
 
        // Calc the inside rect - at least a tile's width and height
        // The inside rect is a direct hit on the gob. Return the
        // first direct hit, if there is one.

        WRect wrcInside;
        pgobHit->GetUIBounds(&wrcInside);
        WCoord cwxInflate = 0;
        if (wrcInside.Width() < kwcTile) {
            cwxInflate += (kwcTile - wrcInside.Width()) / 2;
        }
        WCoord cwyInflate = 0;
        if (wrcInside.Height() < kwcTile) {
            cwyInflate += (kwcTile - wrcInside.Height()) / 2;
        }
        wrcInside.Inflate(cwxInflate, cwyInflate);

        // Calc distance to this. If 0, we're inside and we're done
        // Remember "closest" gob.
        
        WCoord wcDistance = wrcInside.GetDistance(wx, wy);
        if (wcDistance == 0) {
            *pfHitSurrounding = false;
            return pgobHit;
        }
        if (wcDistance < wcDistanceBest) {
            wcDistanceBest = wcDistance;
            pgobHitBest = pgobHit;
        }

        // Calc the outside rect - accumulate overlap with the tile rect.
        
        WRect wrcOutside = wrcInside;
        cwxInflate = 0;
        if (wrcOutside.Width() < kwcTile * 2) {
            cwxInflate += (kwcTile * 2 - wrcOutside.Width()) / 2;
        }
        cwyInflate = 0;
        if (wrcOutside.Height() < kwcTile * 2) {
            cwyInflate += (kwcTile * 2 - wrcOutside.Height()) / 2;
        }
        wrcOutside.Inflate(cwxInflate, cwyInflate);

        // Intersect and accumlate with tile rect.

        WRect wrcT;
        if (wrcT.Intersect(&wrcTileHit, &wrcOutside)) {
            if (wrcTileHitAccum.IsEmpty()) {
                wrcTileHitAccum = wrcT;
            } else {
                wrcTileHitAccum.Union(&wrcT);
            }
        }
    }

    // If no gob or hit too far away, no hit.

    if (pgobHitBest == NULL) {
        return NULL;
    }

    // Structures get no slop since they are big already
    if (pgobHitBest->GetFlags() & kfGobStructure) {
        if (wcDistanceBest != 0) {
            return NULL;
        }
    }

    // Check if this is a mobile headquarters that is moving. In this case,
    // selection is prioritized over local targetting, so that transforming
    // is easier. Local targetting is higher priority if the mobile HQ
    // is not moving.

    if (pgobHitBest->GetType() == kgtMobileHeadquarters) {
        MobileUnitGob *pmunt = (MobileUnitGob *)pgobHitBest;
        if (pmunt->IsMobile()) {
            if (wcDistanceBest <= kwcTile) {
                *pfHitSurrounding = true;
                return pgobHitBest;
            }
        }
    }

    // See if the unit is close enough to qualify for being hit on.

    if (wcDistanceBest > kwcTileHalf) {
        return NULL;
    }

    // Check to see if the tile hit is completely obscured by overlapping
    // hit regions. If yes, then leave as open terrain (so that terrain can
    // be hit between closely packed gobs).

    if (wrcTileHitAccum.Equal(&wrcTileHit)) {
        return NULL;
    }

    // Independent of hit thumbprint, if a gob is already selected, allow
    // the terrain next to it to be selected.

    if (pgobHitBest->GetFlags() & kfGobSelected) {
        return NULL;
    }

    // Structures are big enough to hit the regular bounds.
    if (pgobHitBest->GetFlags() & kfGobStructure) {
        return NULL;
    }

    // This gob has been hit, in the "surrounding" area.

    *pfHitSurrounding = true;
    return pgobHitBest;
}
            
void Simulation::Pause(bool fPause)
{
    m_fPaused = fPause;
}

bool Simulation::IsPaused()
{
    return m_fPaused;
}

} // namespace wi
