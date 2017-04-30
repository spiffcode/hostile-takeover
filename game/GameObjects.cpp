#include "game/ht.h"
#include "game/stateframe.h"

namespace wi {

byte grvlp[] = {
	// ctx & cty
	9, 9,

	// reveal mask
	0xf, 0xf, 0xa, 0x2, 0x2, 0x2, 0x3, 0xf, 0xf,
	0xf, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf,
	0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5,
	0xf, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xf,
	0xf, 0xf, 0xc, 0x4, 0x4, 0x4, 0x5, 0xf, 0xf
};

byte grvlpLarge[] = {
	// ctx & cty
	11, 11,

#if 0
	// reveal mask
	0xf, 0xf, 0xf, 0xa, 0x2, 0x2, 0x2, 0x3, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf, 0xf,
	0xf, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf,
	0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5,
	0xf, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xf,
	0xf, 0xf, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xc, 0x4, 0x4, 0x4, 0x5, 0xf, 0xf, 0xf,
#else
	// reveal mask
	0xf, 0xf, 0xf, 0xa, 0x2, 0x2, 0x2, 0x3, 0xf, 0xf, 0xf,
	0xf, 0xa, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x3, 0xf,
	0xf, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xf,
	0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5,
	0xf, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xf,
	0xf, 0xc, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x5, 0xf,
	0xf, 0xf, 0xf, 0xc, 0x4, 0x4, 0x4, 0x5, 0xf, 0xf, 0xf,
#endif
};

#if 0
byte grvlpMegaLarge[] = {
	// ctx & cty
	13, 13,

	// reveal mask
	0xf, 0xf, 0xf, 0xa, 0x2, 0x2, 0x2, 0x2, 0x2, 0x3, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf, 0xf,
	0xf, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf,
	0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
	0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5,
	0xf, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xf,
	0xf, 0xf, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xc, 0x4, 0x4, 0x4, 0x4, 0x4, 0x5, 0xf, 0xf, 0xf,
};
#endif


// Helper function. Creates but does not Init the Gob.

Gob *CreateGob(GobType gt)
{
	switch (gt) {
	case kgtScenery:
		return new SceneryGob();

	case kgtSurfaceDecal:
		return new SurfaceDecalGob();

	case kgtScorch:
		return new ScorchGob();

	case kgtShortRangeInfantry:
		return new SRInfantryGob();

	case kgtLongRangeInfantry:
		return new LRInfantryGob();

	case kgtHumanResourceCenter:
		return new HrcGob();

	case kgtReactor:
		return new ReactorGob();

	case kgtProcessor:
		return new ProcessorGob();

	case kgtGalaxMiner:
		return new MinerGob();

	case kgtVehicleTransportStation:
		return new VtsGob();

	case kgtHeadquarters:
		return new HqGob();

	case kgtRadar:
		return new RadarGob();

	case kgtResearchCenter:
		return new ResearchGob();

	case kgtLightTank:
		return new LTankGob();

	case kgtMediumTank:
		return new MTankGob();

	case kgtRocketVehicle:
		return new RTankGob();

	case kgtMachineGunVehicle:
		return new GTankGob();

	case kgtTakeoverSpecialist:
		return new SpInfantryGob();

	case kgtOvermind:
		return new OvermindGob();

	// NOTE: probably unnecessary. Don't plan to place TankShots in a level
	case kgtTankShot:
		return new TankShotGob();

	// NOTE: probably unnecessary. Don't plan to place Rockets in a level
	case kgtRocket:
		return new RocketGob();

	case kgtWarehouse:
		return new WarehouseGob();

	case kgtMachineGunTower:
		return new GunTowerGob();

	case kgtRocketTower:
		return new RocketTowerGob();

	case kgtMobileHeadquarters:
		return new MobileHqGob();

	case kgtSmoke:
		return new SmokeGob(0);

	case kgtPuff:
		return new PuffGob();

	case kgtArtillery:
		return new ArtilleryGob();

	case kgtAndy:
		return new AndyGob();

	case kgtReplicator:
		return new ReplicatorGob();

	case kgtActivator:
		return new ActivatorGob();

	case kgtFox:
		return new FoxGob();
	}

	Assert("Unrecognized GobType %d", gt);
	return NULL;
}

//
// Gob implementation
//

Gob::Gob()
{
	m_pgobNext = NULL;
	m_gidNext = kgidNull;
	m_gid = kgidNull;
	m_pplr = &gplrDummy;
	m_ff = kfGobRedraw;
	m_trcBoundingLast.txLeft = 127;
	m_trcBoundingLast.tyTop = 127;
	m_trcBoundingLast.txRight = -128;
	m_trcBoundingLast.tyBottom = -128;
}

Gob::~Gob()
{
}

#ifdef TRACKSTATE

dword gmpgtQuad[] = {
    'GNON', // kgtNone 0
    'GSRI', // kgtShortRangeInfantry 1
    'GLRI', // kgtLongRangeInfantry 2
    'GHRC', // kgtHumanResourceCenter 3
    'GDCL', // kgtSurfaceDecal 4
    'GSCN', // kgtScenery 5
    'GANI', // kgtAnimation 6
    'GREA', // kgtReactor 7
    'GPRC', // kgtProcessor 8
    'GSTU', // kgtStructure 9
    'GUNI', // kgtUnit 10
    'GMIN', // kgtGalaxMiner 11
    'GHQR', // kgtHeadquarters 12
    'GRES', // kgtResearchCenter 13
    'GVTS', // kgtVehicleTransportStation 14
    'GRDR', // kgtRadar 15
    'GLTK', // kgtLightTank 16
    'GMTK', // kgtMediumTank 17
    'GGNV', // kgtMachineGunVehicle 18
    'GRTV', // kgtRocketVehicle 19
    'GTAK', // kgtTakeoverSpecialist 20
    'GWAR', // kgtWarehouse 21
    'GMHQ', // kgtMobileHeadquarters 22
    'GOVR', // kgtOvermind 23
    'GTST', // kgtTankShot 24
    'GRKT', // kgtRocket 25
    'GMGT', // kgtMachineGunTower 26
    'GRTR', // kgtRocketTower 27
    'GSCO', // kgtScorch 28
    'GSMO', // kgtSmoke 29
    'GPUF', // kgtPuff 30
    'GBUL', // kgtBullet 31
    'GART', // kgtArtillery 32
    'GARS', // kgtArtilleryShot 33
    'GAND', // kgtAndy 34
    'GREP', // kgtReplicator 35
    'GACT', // kgtActivator 36
    'GFOX', // kgtFox 37
    'GANS', // kgtAndyShot 38
};

void Gob::TrackState(StateFrame *frame) {
    int i = frame->AddCountedValue(gmpgtQuad[GetType()]);
    frame->AddValue('ID  ', (dword)m_gid, i);
    frame->AddValue('WX  ', (dword)m_wx, i);
    frame->AddValue('WY  ', (dword)m_wy, i);
    frame->AddValue('GNXT', (dword)m_gidNext, i);
    frame->AddValue('ST  ', (dword)m_st, i);
    frame->AddValue('STNX', (dword)m_stNext, i);
    dword ff = m_ff;
    ff &= ~(kfGobRedraw | kfGobFlashing | kfGobDrawFlashed |
            kfGobDebug | kfGobSelected | kfGobVisibleLastFrame |
            kfGobIncludeFindVisible | kfGobTransitioningToVisible |
            kfGobAnimationChanged | kfGobLayerSelection | kfGobLayerMask);
    frame->AddValue('FF  ', ff, i);
}
#endif

bool Gob::Invalidate()
{
    // Get the clipping bounds, the rect that surrounds the visible pixels of
    // this gob

	Rect rc;
	GetClippingBounds(&rc);

	// Change into a tile rect

	TRectSmall trcNew;
	trcNew.txLeft = (char)TcFromPc(rc.left);
	trcNew.tyTop = (char)TcFromPc(rc.top);
	int xT = _min(rc.right + gcxTile - 1, (int)kpcMax);
	trcNew.txRight = (char)TcFromPc(xT);
	int yT = _min(rc.bottom + gcyTile - 1, (int)kpcMax);
	trcNew.tyBottom = (char)TcFromPc(yT);

    // If the gob was transitioning from off screen to on screen, use the
    // current invalidation rect. Otherwise Union with the old rect so that the
    // screen redraws the old location and the new location

	if (m_ff & kfGobTransitioningToVisible) {
		m_trcBoundingLast = trcNew;
	} else {
		m_trcBoundingLast.Union(&trcNew);
	}

	// Is this opaqued? If so nothing to do. Clip to edges of opaque rect; if all inside
	// return false

	if (gptrcMapOpaque != NULL) {
		if (gptrcMapOpaque->left <= m_trcBoundingLast.txLeft) {
			if (gptrcMapOpaque->right >= m_trcBoundingLast.txRight) {
				if (gptrcMapOpaque->top <= m_trcBoundingLast.tyTop) {
					if (gptrcMapOpaque->bottom >= m_trcBoundingLast.tyBottom) {
						return false;
					}
				}
			}
		}
	}

	// Perform invalidation of what's visible

	gpupdSim->InvalidateMapTileRect(&m_trcBoundingLast);

	// Remember new

	m_trcBoundingLast = trcNew;
	return true;
}

// Marks the gob for redraw; will cause update map invalidation later
// If the gob was visible last frame, include this gob in the next
// FindVisibleGobs call. This is so that gobs transitioning from
// visible to invisible have a chance to redraw the old on-screen
// location

void Gob::MarkRedraw() {
	m_ff |= kfGobRedraw;
	if (m_ff & kfGobVisibleLastFrame)
		m_ff |= kfGobIncludeFindVisible;
}

bool Gob::AdvanceAnimation(Animation *pani) {
	if (pani->Advance(m_ff & kfGobAnimationChanged ? 1 : m_unvl.GetUpdateCount())) {
		m_unvl.MinSkip(pani->GetRemainingFrameTime());
		MarkRedraw();
		m_ff &= ~kfGobAnimationChanged;
		return true;
	}
	m_ff &= ~kfGobAnimationChanged;
	return false;
}

void Gob::StartAnimation(Animation *pani, int nStrip, int nFrame, word wfAni) {
	if (nStrip != pani->GetStrip() || nFrame != pani->GetFrame())
		MarkRedraw();
	if (pani->Start(nStrip, nFrame, wfAni)) {
		m_unvl.MinSkip(pani->GetRemainingFrameTime());
		m_ff |= kfGobAnimationChanged;
	}
}

void Gob::SetAnimationStrip(Animation *pani, int nStrip) {
	if (nStrip != pani->GetStrip())
		MarkRedraw();
	pani->SetStrip(nStrip);
	m_ff |= kfGobAnimationChanged;
}

void Gob::SetAnimationFrame(Animation *pani, int nFrame) {
	if (nFrame != pani->GetFrame())
		MarkRedraw();
	pani->SetFrame(nFrame);
	m_ff |= kfGobAnimationChanged;
}

dword Gob::GetSortKey()
{
	return MakeSortKey(m_wy, m_gid);
}

#if defined(WIN) && defined(DEBUG)
void Gob::ToString(char *psz)
{
	sprintf(psz, "this=0x%08lx, gid=0x%08lx, gt=0x%lx, wx=0x%04lx, wy=0x%04lx", this, m_gid, GetType(), m_wx, m_wy);
}
#endif

// Clipping bounds is used to determine on-display visibility
// This bounds is a rectangle tightly enclosing all the pixels the Gob
// will be drawing to. Clipping bounds may change in size depending
// on the animation state of the Gob.
// NOTE: the values returned are in world coordinates

void Gob::GetClippingBounds(Rect *prc)
{
	prc->SetEmpty();
}

// UI bounds is reused for input hit testing and for drawing the selection
// indicator.  The UI bounds is an end-user 'logical' box surrounding Gobs
// they'll be tapping on.  Once determined, it does not change size as the Gob
// animates.  NOTE: the values returned are in world coordinates

void Gob::GetUIBounds(WRect *pwrc)
{
	pwrc->SetEmpty();
}

void Gob::GetPosition(WPoint *pwpt)
{
	pwpt->wx = m_wx;
	pwpt->wy = m_wy;
}

void Gob::SetPosition(WCoord wx, WCoord wy) 
{
	ggobm.MoveGob(this, m_wx, m_wy, wx, wy);
	m_wx = wx;
	m_wy = wy;
	MarkRedraw();
}

void Gob::GetCenter(WPoint *pwpt)
{
	pwpt->wx = m_wx;
	pwpt->wy = m_wy;
}

void Gob::GetTilePosition(TPoint *ptpt)
{
	ptpt->tx = TcFromWc(m_wx);
	ptpt->ty = TcFromWc(m_wy);
}

void Gob::GetTileRect(TRect *ptrc)
{
	ptrc->left = TcFromWc(m_wx);
	ptrc->top = TcFromWc(m_wy);
	ptrc->right = ptrc->left + 1;
	ptrc->bottom = ptrc->top + 1;
}

void Gob::GetTilePaddedWRect(WRect *pwrc)
{
	pwrc->left = WcTrunc(m_wx);
	pwrc->top = WcTrunc(m_wy);
	pwrc->right = pwrc->left + kwcTile;
	pwrc->bottom = pwrc->top + kwcTile;
}

/* Javascript (Jscript + Windows Scripting Host, actually) to generate the table below:
WScript.Echo("byte gmpDistFromDxy[10][10] = {");
for (y = 0; y < 10; y++) {
	str = "";
	for (x = 0; x < 10; x++) {
		d = Math.sqrt(x * x + y * y);
		strT = Math.round(d).toString();
		if (strT.length < 2)
			strT = " " + strT;
		str += strT + ",";
	}
	WScript.Echo("\t{ " + str + " },");
}
WScript.Echo("};");
*/

byte gmpDistFromDxy[10][10] = {
	{  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, },
	{  1, 1, 2, 3, 4, 5, 6, 7, 8, 9, },
	{  2, 2, 3, 4, 4, 5, 6, 7, 8, 9, },
	{  3, 3, 4, 4, 5, 6, 7, 8, 9, 9, },
	{  4, 4, 4, 5, 6, 6, 7, 8, 9,10, },
	{  5, 5, 5, 6, 6, 7, 8, 9, 9,10, },
	{  6, 6, 6, 7, 7, 8, 8, 9,10,11, },
	{  7, 7, 7, 8, 8, 9, 9,10,11,11, },
	{  8, 8, 8, 9, 9, 9,10,11,11,12, },
	{  9, 9, 9, 9,10,10,11,11,12,13, },
};

bool Gob::IsGobWithinRange(Gob *pgobTarget, TCoord tcRange)
{
	WPoint wpt;
	wpt.wx = m_wx;
	wpt.wy = m_wy;
	return IsTargetWithinRange(&wpt, pgobTarget, tcRange);
}

bool Gob::IsTargetWithinRange(WPoint *pwptTarget, Gob *pgobTarget, TCoord tcRange)
{
	Assert(tcRange < 10, "IsGobWithinRange tcRange must be less than 10");
	TCoord tx = TcFromWc(pwptTarget->wx);
	TCoord ty = TcFromWc(pwptTarget->wy);

	// Test every tile covered by structures

	if (pgobTarget->GetFlags() & kfGobStructure) {
		TRect trcTarget;
		pgobTarget->GetTileRect(&trcTarget);
		for (TCoord tyTarget = trcTarget.top; tyTarget < trcTarget.bottom; tyTarget++) {
			for (TCoord txTarget = trcTarget.left; txTarget < trcTarget.right; txTarget++) {
				TCoord dtx = abs(tx - txTarget);
				TCoord dty = abs(ty - tyTarget);
				if (dtx >= 10 || dty >= 10)
					continue;
				if (gmpDistFromDxy[dtx][dty] <= tcRange)
					return true;
			}
		}
		return false;

	} else {
		TCoord dtx = abs(tx - TcFromWc(pgobTarget->m_wx));
		TCoord dty = abs(ty - TcFromWc(pgobTarget->m_wy));
		if (dtx >= 10 || dty >= 10)
			return false;
		return gmpDistFromDxy[dtx][dty] <= tcRange;
	}
}

// Do-nothing default implementations

void Gob::PopupMenu()
{
}

void Gob::InitMenu(Form *pfrm)
{
}

void Gob::OnMenuItemSelected(int id)
{
}

#define knVerGobState 2
bool Gob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerGobState)
		return false;
	m_st = (State)pstm->ReadWord();
	m_ff |= pstm->ReadDword();
	m_wx = pstm->ReadWord();
	m_wy = pstm->ReadWord();
	Pid pid = pstm->ReadWord();
	m_pplr = gplrm.GetPlayerFromPid(pid);
	Gid gid = pstm->ReadWord();
	Assert(gid != kgidNull);
	ggobm.AddGob(this, gid);

	return pstm->IsSuccess();
}

bool Gob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerGobState);
	pstm->WriteWord(m_st);
	pstm->WriteDword(m_ff);
	pstm->WriteWord(m_wx);
	pstm->WriteWord(m_wy);
	if (m_pplr != NULL) {
		pstm->WriteWord(m_pplr->GetId());
	} else {
		pstm->WriteWord(kpidNeutral);
	}
	pstm->WriteWord(m_gid);

	return pstm->IsSuccess();
}

bool Gob::IsSavable()
{
	return true;
}

// StateMachine methods

#if defined(DEBUG_HELPERS)
char *Gob::GetName()
{
	return "Gob";
}
#endif

//
// SurfaceDecalGob implementation
//

bool SurfaceDecalGob::InitClass(IniReader *pini)
{
	// No interesting global parameters have been defined for SurfaceDecal yet
	return true;
}

SurfaceDecalGob::SurfaceDecalGob()
{
	m_ff |= kfGobLayerSurfaceDecal;
	m_ptbm = NULL;
}

SurfaceDecalGob::~SurfaceDecalGob()
{
}

bool SurfaceDecalGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	int tx, ty;
	char szBitmap[kcbFilename];
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%s,%d ,%d", szBitmap, &tx, &ty);
	if (cArgs < 3) {
		Assert("SurfaceDecalGob requires at least 3 valid initialization parameters");
		return false;
	}

	return Init(WcFromTc(tx), WcFromTc(ty), 0, szBitmap, NULL, pszName);
}

bool SurfaceDecalGob::Init(WCoord wx, WCoord wy, dword ff, const char *pszBitmap, TBitmap *ptbm, const char *pszName)
{
	m_wx = wx;
	m_wy = wy;

	if (pszBitmap != NULL) {
		m_ptbm = CreateTBitmap((char *)pszBitmap);
		if (m_ptbm == NULL)
			return false;
	} else {
		// NOTE: we assume if we were passed the bitmap that the caller is
		// going to take care of deleting it at an appropriate timne.

		m_ptbm = ptbm;
	}

	m_ff |= ff;

	// Add the fresh Gob to the GobMgr.

	ggobm.AddGob(this);

	return true;
}

bool SurfaceDecalGob::IsSavable()
{
	// Not currently savable

	return false;
}

GobType SurfaceDecalGob::GetType()
{
	return kgtSurfaceDecal;
}

void SurfaceDecalGob::GetClippingBounds(Rect *prc)
{
	Size sizBitmap;
	m_ptbm->GetSize(&sizBitmap);

	prc->left = PcFromUwc(m_wx) - ((sizBitmap.cx - 1) / 2);
	prc->top = PcFromUwc(m_wy) - (sizBitmap.cy - 1);
	prc->right = prc->left + sizBitmap.cx;
	prc->bottom = prc->top + sizBitmap.cy;
}

void SurfaceDecalGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	// UNDONE: bitmap origin

	if (nLayer == knLayerSurfaceDecal) {
		Size sizSrc;
		m_ptbm->GetSize(&sizSrc);

		// For now xOrigin is the center column (round up) of the bitmap,
		// yOrigin is the bottom row of the bitmap

		int x = PcFromUwc(m_wx) - xViewOrigin - ((sizSrc.cx - 1) / 2);
		int y = PcFromUwc(m_wy) - yViewOrigin - (sizSrc.cy - 1);
		m_ptbm->BltTo(pbm, x, y, m_pplr->GetSide());
	}
}

int gnSequenceScorch;

ScorchGob::ScorchGob()
{
	m_nScorch = 0;
	m_nSequence = gnSequenceScorch++;
}

int ScorchGob::GetSequence()
{
	return m_nSequence;
}

GobType ScorchGob::GetType()
{
	return kgtScorch;
}

bool ScorchGob::Init(WCoord wx, WCoord wy, int nScorch)
{
	m_nScorch = nScorch;
	return SurfaceDecalGob::Init(wx, wy, 0, NULL, gaptbmScorches[m_nScorch], NULL);
}

#define knVerScorchGobState 2
bool ScorchGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerScorchGobState)
		return false;
	m_nScorch = pstm->ReadWord();
	m_ptbm = gaptbmScorches[m_nScorch];
	m_nSequence = pstm->ReadWord();
	if (m_nSequence > gnSequenceScorch)
		gnSequenceScorch = m_nSequence;
	return SurfaceDecalGob::LoadState(pstm);
}

bool ScorchGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerScorchGobState);
	pstm->WriteWord(m_nScorch);
	pstm->WriteWord(m_nSequence);
	return SurfaceDecalGob::SaveState(pstm);
}

bool ScorchGob::IsSavable()
{
	return true;
}

//
// SceneryGob implementation
//

bool SceneryGob::InitClass(IniReader *pini)
{
	// No interesting global parameters have been defined for Scenery yet
	return true;
}

SceneryGob::SceneryGob()
{
	m_ptbm = NULL;
}

SceneryGob::~SceneryGob()
{
}

bool SceneryGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	int tx, ty;
	char szBitmap[kcbFilename];
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%s,%d ,%d", szBitmap, &tx, &ty);
	if (cArgs < 3) {
		Assert("SceneryGob requires at least 3 valid initialization parameters");
		return false;
	}

	return Init(WcFromTc(tx), WcFromTc(ty), 0, szBitmap, pszName);
}

bool SceneryGob::Init(WCoord wx, WCoord wy, dword ff, const char *pszBitmap, const char *pszName)
{
	m_wx = wx;
	m_wy = wy;

	m_ptbm = CreateTBitmap((char *)pszBitmap);
	if (m_ptbm == NULL)
		return false;

	m_ff |= ff | kfGobLayerDepthSorted;

	// Add the fresh Gob to the GobMgr. GobMgr::AddGob assigns this Gob a gid

	ggobm.AddGob(this);

	return true;
}

#define knVerSceneryGobState 2
bool SceneryGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerSceneryGobState)
		return false;
	char szBitmap[kcbFilename];
	pstm->ReadString(szBitmap, sizeof(szBitmap));
	m_ptbm = CreateTBitmap(szBitmap);
	return Gob::LoadState(pstm);
}

bool SceneryGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerSceneryGobState);
	char szBitmap[kcbFilename];
    strncpyz(szBitmap, m_ptbm->GetFileName(), kcbFilename);
	pstm->WriteString(szBitmap);
	return Gob::SaveState(pstm);
}

GobType SceneryGob::GetType()
{
	return kgtScenery;
}

dword SceneryGob::GetSortKey()
{
	return MakeSortKey(m_wy + WcFromPc(m_ptbm->GetBaseline()), m_gid);
}

void SceneryGob::GetClippingBounds(Rect *prc)
{
	Size sizBitmap;
	m_ptbm->GetSize(&sizBitmap);

	// Convert pixel coord size into world coord size

	prc->left = PcFromUwc(m_wx);
	prc->top = PcFromUwc(m_wy);
	prc->right = prc->left + sizBitmap.cx;
	prc->bottom = prc->top + sizBitmap.cy;
}

void SceneryGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	if (nLayer == knLayerDepthSorted) {
		Size sizSrc;
		m_ptbm->GetSize(&sizSrc);

		// For now xOrigin is the center column (round up) of the bitmap,
		// yOrigin is the baseline of the bitmap

		int x = PcFromUwc(m_wx) - xViewOrigin;
		int y = PcFromUwc(m_wy) - yViewOrigin;

		m_ptbm->BltTo(pbm, x, y, m_pplr->GetSide());
	}
}

//
// AnimGob implementation
//

AnimGob *CreateAnimGob(WCoord wx, WCoord wy, word wfAnm, const char *pszAniName, AnimationData *panid, int nStrip,
		StateMachineId smidNotify, const char *pszName)
{
	if (!ggobm.IsBelowLimit(knLimitSupport))
		return NULL;

	AnimGob *pgob = new AnimGob();
	Assert(pgob != NULL, "out of memory!");
	if (pgob == NULL)
		return NULL;

	if (!pgob->Init(wx, wy, wfAnm, pszAniName, panid, nStrip, smidNotify, pszName)) {
		delete pgob;
		return NULL;
	}

	return pgob;
}

AnimGob::AnimGob()
{
	m_ff |= kfGobStateMachine;
	m_smidNotify = ksmidNull;
}

AnimGob::~AnimGob()
{
}

bool AnimGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	int tx, ty;
	char szAniName[kcbFilename];
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%s,%d ,%d", szAniName, &tx, &ty);
	if (cArgs < 3) {
		Assert("AnimGob requires at least 3 valid initialization parameters");
		return false;
	}

	return Init(WcFromTc(tx), WcFromTc(ty), 0, szAniName, NULL, 0, ksmidNull, pszName);
}

bool AnimGob::Init(WCoord wx, WCoord wy, word wfAnm, const char *pszAniName, AnimationData *panid, int nStrip,
		StateMachineId smidNotify, const char *pszName)
{
	m_wx = wx;
	m_wy = wy;

	bool fFreeAnimationData = false;
	if (pszAniName != NULL) {
		panid = LoadAnimationData(pszAniName);
		fFreeAnimationData = true;
	}

	// Must be either passed a panid or be able to load one using pszAniName

	if (panid == NULL)
		return false;

	m_ani.Init(panid);
	StartAnimation(&m_ani, nStrip, 0, fFreeAnimationData ? kfAniFreeAnimationData : 0);

	m_wfAnm = wfAnm;
	m_smidNotify = smidNotify;

	if (m_wfAnm & kfAnmSmokeFireLayer)
		m_ff |= kfGobLayerSmokeFire;
	else if (m_wfAnm & kfAnmSurfaceDecalLayer)
		m_ff |= kfGobLayerSurfaceDecal;
	else
		m_ff |= kfGobLayerDepthSorted;

	// Add the fresh Gob to the GobMgr. GobMgr::AddGob assigns this Gob a gid

	ggobm.AddGob(this);

	return true;
}

bool AnimGob::IsSavable()
{
	return false;
}

bool AnimGob::OnStripDone()
{
	return false;
}

GobType AnimGob::GetType()
{
	return kgtAnimation;
}

void AnimGob::GetClippingBounds(Rect *prc)
{
	m_ani.GetBounds(prc);
	prc->Offset(PcFromUwc(m_wx), PcFromUwc(m_wy));
}

void AnimGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	int nLayerDesignated;
	if (m_wfAnm & kfAnmSmokeFireLayer)
		nLayerDesignated = knLayerSmokeFire;
	else if (m_wfAnm & kfAnmSurfaceDecalLayer)
		nLayerDesignated = knLayerSurfaceDecal;
	else
		nLayerDesignated = knLayerDepthSorted;

	if (nLayer == nLayerDesignated) {
		m_ani.Draw(pbm, PcFromUwc(m_wx) - xViewOrigin, PcFromUwc(m_wy) - yViewOrigin, m_pplr->GetSide());
	}
}

#if defined(DEBUG_HELPERS)
char *AnimGob::GetName()
{
	return "Animation";
}
#endif

int AnimGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnUpdate
		if (!AdvanceAnimation(&m_ani)) {
			if (m_wfAnm & kfAnmLoop)
				StartAnimation(&m_ani, m_ani.GetStrip(), 0, 0);

            // Notify the StateMachine that wants to know when the animation is
            // done

			if (m_smidNotify != ksmidNull)
				gsmm.SendMsg(kmidAnimationComplete, m_gid, m_smidNotify);

			// Allow inheriters to decide what to do when the animation is done

			bool fDelete = OnStripDone();
			if (!fDelete)
				m_unvl.MinSkip();

			// Remove the AnimGob if requested

			if (fDelete || (m_wfAnm & kfAnmDeleteWhenDone)) {
				ggobm.RemoveGob(this);
				delete this;
				return knDeleted;
			}
		}

EndStateMachine
}

//
// Helper functions
//

#ifdef DRAW_PATHS
static TBitmap *s_aptbmArrows[9];

void LoadArrows()
{
	s_aptbmArrows[0] = CreateTBitmap("arrow0.png");
	s_aptbmArrows[1] = CreateTBitmap("arrow1.png");
	s_aptbmArrows[2] = CreateTBitmap("arrow2.png");
	s_aptbmArrows[3] = CreateTBitmap("arrow3.png");
	s_aptbmArrows[4] = CreateTBitmap("arrow4.png");
	s_aptbmArrows[5] = CreateTBitmap("arrow5.png");
	s_aptbmArrows[6] = CreateTBitmap("arrow6.png");
	s_aptbmArrows[7] = CreateTBitmap("arrow7.png");
	s_aptbmArrows[8] = CreateTBitmap("x.png");
}

void FreeArrows()
{
	for (int i = 0; i < 9; i++) {
		delete s_aptbmArrows[i];
		s_aptbmArrows[i] = NULL;
	}
}

void DrawArrow(DibBitmap *pbm, int x, int y, Direction dir, Side side)
{
	s_aptbmArrows[dir]->BltTo(pbm, x, y, side);
}
#endif

void GetHealthColorAndLength(int nNumerator, int nDenominator, int cxWidth, Color *pclr, int *pnLength) secCode7;
void GetHealthColorAndLength(int nNumerator, int nDenominator, int cxWidth, Color *pclr, int *pnLength)
{
	// Break it down into three pieces.
	// 50% green (healthy), 25% yellow (heavily damaged), 25% red (close to death)

	if (nNumerator * 2 > nDenominator)
		*pclr = GetColor(kiclrGreen);
	else if (nNumerator * 4 > nDenominator)
		*pclr = GetColor(kiclrYellow);
	else
		*pclr = GetColor(kiclrRed);

	*pnLength = ((cxWidth - 2) * nNumerator) / nDenominator;

	// The health bar should show at least one pixel of life left until
	// the unit/structure is completely destroyed.

	if (*pnLength == 0 && nNumerator > 0)
		*pnLength = 1;
}

void DrawSelectionIndicator(DibBitmap *pbm, Rect *prc, int nNumerator, int nDenominator)
{
	int cxWidth = prc->Width();
	int cyHeight = prc->Height();
	int nCornerWidth = cxWidth / 4;
	int nCornerHeight = cyHeight / 4;
	int nThickness = 1;

	bool fSelectionBrackets = (gwfPerfOptions & kfPerfSelectionBrackets) != 0;

#ifdef __CPU_68K
	//debug
	//BitmapType *pbmpScreen = WinGetBitmap(WinGetDisplayWindow());
	//byte *pbBits = (byte *)BmpGetBits(pbmpScreen);
	byte *pbBits = pbm->GetBits();
	
	// Hack to make drawing a selection faster if not clipped
	// OPT: Could still be faster as TBitmaps

	Size sizDib;
	pbm->GetSize(&sizDib);

	if (prc->left >= 0 && prc->top - kcyHealthBar >= 0 && prc->right <= sizDib.cx && prc->bottom <= sizDib.cy) {
		Color clrHealth;
		int nLength;
		GetHealthColorAndLength(nNumerator, nDenominator, cxWidth, &clrHealth, &nLength);
		byte *pbDst = pbBits + (long)(prc->top - kcyHealthBar) * sizDib.cx + prc->left;
		DrawSelection68k(pbDst, sizDib.cx, cxWidth, cyHeight, nCornerWidth, nCornerHeight, GetColor(kiclrWhite), nLength, clrHealth, gmpiclriclrShadow, fSelectionBrackets);
		return;
	}
#endif

	// Slow clipped path

	if (fSelectionBrackets) {
		Color clr = GetColor(kiclrWhite);

		// Top-left corner

		pbm->Fill(prc->left, prc->top, nCornerWidth, nThickness, clr);
		pbm->Fill(prc->left, prc->top, nThickness, nCornerHeight, clr);

		// Top-right corner

		pbm->Fill(prc->right - nCornerWidth, prc->top, nCornerWidth, nThickness, clr);
		pbm->Fill(prc->right - nThickness, prc->top, nThickness, nCornerHeight, clr);

		// Bottom-left corner

		pbm->Fill(prc->left, prc->bottom - nThickness, nCornerWidth, nThickness, clr);
		pbm->Fill(prc->left, prc->bottom - nCornerHeight, nThickness, nCornerHeight, clr);

		// Bottom-right corner

		pbm->Fill(prc->right - nCornerWidth, prc->bottom - nThickness, nCornerWidth, nThickness, clr);
		pbm->Fill(prc->right - nThickness, prc->bottom - nCornerHeight, nThickness, nCornerHeight, clr);
	}

	DrawHealthIndicator(pbm, prc, nNumerator, nDenominator);
}

void DrawHealthIndicator(DibBitmap *pbm, Rect *prc, int nNumerator, int nDenominator)
{
	int cxWidth = prc->Width();
	Color clr;
	int nLength;
	GetHealthColorAndLength(nNumerator, nDenominator, cxWidth, &clr, &nLength);

#ifdef __CPU_68K
	Size sizDib;
	pbm->GetSize(&sizDib);
	if (prc->left >= 0 && prc->top - kcyHealthBar >= 0 && prc->right <= sizDib.cx && prc->bottom <= sizDib.cy) {
		byte *pbDst = pbm->GetBits() + (long)(prc->top - kcyHealthBar) * sizDib.cx + prc->left;
		DrawSelection68k(pbDst, sizDib.cx, cxWidth, 0, 0, 0, 0, nLength, clr, gmpiclriclrShadow, false);
		return;
	}
#endif

	pbm->Shadow(prc->left, prc->top - kcyHealthBar, cxWidth, kcyHealthBar);
	pbm->Fill(prc->left + 1, prc->top - (kcyHealthBar - 1), nLength, kcyHealthBar - 2, clr);
}

void DrawBuildProgressIndicator(DibBitmap *pbm, Rect *prc, int nNumerator, int nDenominator)
{
	Color clr = GetColor(kiclrYellow);
	int cxWidth = prc->right - prc->left;
	int nLength = ((cxWidth - 2) * nNumerator) / nDenominator;

	int y = prc->top + prc->Height() / 2;
//	gapfnt[0]->DrawText(pbm, "Building...", prc->left, y - gapfnt[0]->GetHeight());
	pbm->Shadow(prc->left, y, cxWidth, 6);
	pbm->Fill(prc->left + 1, y + 1, nLength, 4, clr);
}

void DrawFullnessIndicator(DibBitmap *pbm, Rect *prc, int nPips, int nPipsMax)
{
	// Don't draw anything if Miner/Processor/Warehouse is completely empty

	if (nPips == 0)
		return;

	// Allow for overflow as is needed (rarely) when capacity is exceeded
	// (e.g., self-destruct a structure while at capacity)

	if (nPips > nPipsMax)
		nPips = nPipsMax;

	Color clr = GetColor(kiclrFullnessIndicator);

	int cxyPip = PcFromWc(kwcFullnessPip);

	int cx = (nPipsMax * (cxyPip + 1)) + 1;
	int x = prc->left + ((prc->Width() - cx) / 2);
	int y = prc->bottom + 1;
	pbm->Shadow(x, y - 1, cx, kcyFullnessBar);

	x++;
	for (int i = 0; i < nPips; i++, x += cxyPip + 1)
		pbm->Fill(x, y, cxyPip, cxyPip, clr);
}

// Given an offset in x and y, CalcDir returns one of 8 direction values.
// Directions map to 8 points on a compass, incrementing clockwise from N(orth).
// 0 = N/up, 1 = NE, 2 = E/right, 3 = SE, 4 = S/down, 5 = SW, 6 = W/left, 7 = NW
//
// NOTE: this optimized calculation is somewhat inaccurate. The 360 degrees are
// unevenly divided. N/S/E/W get 53.13 degree arcs and the diagonal angles get
// 36.88 degree arcs.

Direction CalcDir(int dx, int dy)
{
	int dxAbs = abs(dx);
	int dyAbs = abs(dy);

	if (dxAbs > dyAbs) {
		if (dx >= 0) {
			if (dy != 0 && (dxAbs / 2) < dyAbs) { // > 22.5 degrees
				if (dy < 0)	// heading up
					return kdirNE;
				else
					return kdirSE;
			} else {
				return kdirE;
			}
		} else {
			if (dy != 0 && (dxAbs / 2) < dyAbs) { // > 22.5 degrees
				if (dy < 0) // heading up
					return kdirNW;
				else
					return kdirSW;
			} else {
				return kdirW;
			}
		}
	} else {
		if (dy < 0) {
			if (dx != 0 && (dyAbs / 2) < dxAbs) {
				if (dx >= 0)
					return kdirNE;
				else
					return kdirNW;
			} else {
				return kdirN;
			}
		} else {
			if (dx != 0 && (dyAbs / 2) < dxAbs) {
				if (dx >= 0)
					return kdirSE;
				else
					return kdirSW;
			} else {
				return kdirS;
			}
		}
	}
}

// OPT: this can be optimized to 1/8 is current size by packing
// the values 4 to a byte and by exploiting its diagonal symmetry

static byte s_mpnArcFromDxDy[16][16] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 3, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 2, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 2, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1 },
	{ 0, 0, 1, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1 },
	{ 0, 0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2 },
	{ 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2 },
	{ 0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2 },
	{ 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2 },
	{ 0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 },
	{ 0, 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3 },
	{ 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3 },
	{ 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3 },
	{ 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3 },
};

static int s_adirFromArc[32] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
	8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 0
};

Direction16 CalcDir16(int dx, int dy)
{
	int dxAbs = abs(dx);
	int dyAbs = abs(dy);

	while (dxAbs >= 16 || dyAbs >= 16) {
		dxAbs >>= 1;
		dyAbs >>= 1;
	}

	// UNDONE: maybe use 0 return value from s_mpnArc instead
	if (dxAbs == 0)
		return dy < 0 ? 0 : 8;
	if (dyAbs == 0)
		return dx >= 0 ? 4 : 12;

	int i = s_mpnArcFromDxDy[dyAbs][dxAbs];

	if (dxAbs >= dyAbs)
		i = 7 - i;

	if (dy > 0)
		i = 15 - i;

	if (dx < 0)
		i = 31 - i;

	return s_adirFromArc[i];
}

//
// GobMgr implementation
//

#define PpgobFromGid(gid)	(&m_apgobMaster[gid])
#define GidFromPpgob(ppgob) ((ppgob) - m_apgobMaster)
#define MakeUsedEntry(pw)	(Gob **)((pword)(pw) & ~1)
#define MakeFreeEntry(pw)	(Gob *)((pword)(pw) | 1)
#define IsFreeEntry(pw)		((pword)(pw) & 1)
#define IgidFromWXY(wx, wy)	(TcFromWc(wy) * m_ctx + TcFromWc(wx))

GobMgr::GobMgr()
{
	m_pgobHead = NULL;
	m_pgidMap = NULL;
#ifdef MP_DEBUG_SHAREDMEM
	m_apgobMaster = NULL;
#endif
	m_cpgobActive = 0;
	m_car = 0;
	m_cSceneryGobs = 0;
	m_cScorchGobs = 0;
	m_cSupportGobs = 0;
}

GobMgr::~GobMgr()
{
	Assert(m_cpgobActive == 0);
	Assert(m_pgidMap == NULL);
	Assert(m_apgobMaster == NULL);
	FreeAreas();
}

bool GobMgr::Init(TCoord ctx, TCoord cty, int cpgobMax)
{
	Assert(m_pgidMap == NULL);
	Assert(m_apgobMaster == NULL);

	m_ctx = ctx;
	m_cty = cty;
	m_cpgobMax = cpgobMax;

	// Allocate master Gob list (+ 1 entry for free list terminator)

	Gob **apgobMaster = new Gob *[m_cpgobMax + 1];
	Assert(apgobMaster != NULL, "out of memory!");
	if (apgobMaster == NULL)
		return false;

	// Alloc from storage ram

	int cbT = sizeof(Gob **) * (m_cpgobMax + 1);
	m_apgobMaster = (Gob **)gmmgr.AllocPtr(cbT);
	if (m_apgobMaster == NULL) {
		delete[] apgobMaster;
		return false;
	}

	// Initialize free list. Notice that the Gob* in the array are holding Gob** when in the free list.

	Gob **ppgobDstT = apgobMaster;
	Gob **ppgobSrcT = m_apgobMaster;
	Gob **ppgobFreeTail = apgobMaster + m_cpgobMax;
	for (; ppgobDstT < ppgobFreeTail; ppgobDstT++, ppgobSrcT++)
		*ppgobDstT = MakeFreeEntry((Gob *)(ppgobSrcT + 1));
	*ppgobFreeTail = NULL;

	// Move to storage ram
	// Free dyn ram version

	gmmgr.WritePtr(m_apgobMaster, 0, apgobMaster, cbT);
	m_ppgobFreeTail = m_apgobMaster + m_cpgobMax;
	m_ppgobFreeHead = m_apgobMaster;
	delete[] apgobMaster;

	// Allocate gidMap

	Gid *pgidMap = new Gid[m_ctx * m_cty];
	Assert(pgidMap != NULL, "out of memory!");
	if (pgidMap == NULL)
		return false;

	// Initialize gidMap

	int cgid = m_ctx * m_cty;
	Gid *pgidT = pgidMap;
	for (int i = 0; i < cgid; i++)
		*pgidT++ = kgidNull;

	// Alloc from storage ram
	// Free dyn ram version

	cbT = m_ctx * m_cty * sizeof(Gid);
	m_pgidMap = (Gid *)gmmgr.AllocPtr(cbT);
	if (m_pgidMap == NULL) {
		delete[] pgidMap;
		return false;
	}
	gmmgr.WritePtr(m_pgidMap, 0, pgidMap, cbT);
	delete[] pgidMap;

#ifdef INCL_VALIDATEHEAP
	gmmgr.Validate();
#endif

	return true;
}

#define knVerGobMgr 5

bool GobMgr::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerGobMgr);

	// Write areas

	pstm->WriteWord(m_car);
	for (int iar = 0; iar < m_car; iar++) {
		Area *par = &m_aar[iar];
		pstm->Write(par, sizeof(*par));
		if (par->aare != NULL)
			pstm->Write(par->aare, par->careAlloc * ELEMENTSIZE(par->aare));
	}

	return pstm->IsSuccess();
}

bool GobMgr::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerGobMgr)
		return false;

	// Read areas

	m_car = pstm->ReadWord();
	for (int iar = 0; iar < m_car; iar++) {
		Area *par = &m_aar[iar];
		pstm->Read(par, sizeof(*par));
		if (par->aare != NULL) {
			par->aare = new AreaEntry[par->careAlloc];
			if (par->aare == NULL) {
				pstm->Read(gpbScratch, par->careAlloc * ELEMENTSIZE(par->aare));
				memset(par, 0, sizeof(*par));
				par->iareFree = (word)-1;
				par->iareHead = (word)-1;
			} else {
				pstm->Read(par->aare, par->careAlloc * ELEMENTSIZE(par->aare));
			}
		}
	}

	return pstm->IsSuccess();
}

// Called between levels

void GobMgr::Reset()
{
	// Free master Gob list

	if (m_apgobMaster != NULL) {
		gmmgr.FreePtr(m_apgobMaster);
		m_apgobMaster = NULL;
	}

	m_cpgobActive = 0;

	// Free gidMap

	if (m_pgidMap != NULL) {
		gmmgr.FreePtr(m_pgidMap);
		m_pgidMap = NULL;
	}

	// Free all the Gobs

	Gob *pgobT = m_pgobHead;
	while (pgobT != NULL) {
		Gob *pgobNext = pgobT->m_pgobNext;
		delete pgobT;
		pgobT = pgobNext;
	}
	m_pgobHead = NULL;

	// Free all area lists

	FreeAreas();

	// Reset gob counts

	m_cSceneryGobs = 0;
	m_cScorchGobs = 0;
	m_cSupportGobs = 0;
}

void GobMgr::AddGob(Gob *pgob, Gid gid)
{
	// Add Gob to master Gob list
	// New Gob entries are taken from the HEAD of the free list

	// Make sure we haven't already run out of free entries

	Assert(*m_ppgobFreeHead != NULL);
	Assert(m_cpgobActive != m_cpgobMax);

	// Todo: return failure and deal with failure at some point

	if (m_cpgobActive >= m_cpgobMax)
		return;

	// When loading a saved game, we use the same gids for gobs.
	// When loading a game fresh, we take the next available gid.

	if (gid == kgidNull) {
		// Add to list; take the next available gid

		Gob **ppgob = m_ppgobFreeHead;
		Assert(*ppgob == MakeFreeEntry(*ppgob));
		m_ppgobFreeHead = MakeUsedEntry(*ppgob);
		gmmgr.WritePtr(m_apgobMaster, (byte *)ppgob - (byte *)m_apgobMaster, &pgob, sizeof(Gob *));
		//*ppgob = pgob;
		gid = GidFromPpgob(ppgob);
	} else {

		// We have a gid to use. Pull it out of the free list.
		// Note to self: try to make this more complicated if possible. More '*''s and macros would help.

		bool fFound = false;

		// handle head case seperately because m_ppgobFreeHead's contents are not marked free

		Gob **ppgobUse = PpgobFromGid(gid);
		if (m_ppgobFreeHead == ppgobUse) {
			fFound = true;
			m_ppgobFreeHead = MakeUsedEntry((Gob **)*ppgobUse);
		} else {

			// ppgobUse is now the address of the apgobMaster slot we're going to put a gob in. 
			// find the slot that points at this one in the free list & point it at the next entry

			for (Gob ***pppgobT = (Gob ***)m_ppgobFreeHead; *pppgobT != NULL; pppgobT = (Gob ***)MakeUsedEntry(*pppgobT)) {
				if (MakeUsedEntry(*pppgobT) == ppgobUse) {
					gmmgr.WritePtr(m_apgobMaster, (byte *)pppgobT - (byte *)m_apgobMaster, ppgobUse, sizeof(Gob *));
					//*pppgobT = (Gob **)*ppgobUse;

					// update the tail pointer if necessary

					if (m_ppgobFreeTail == ppgobUse)
						m_ppgobFreeTail = (Gob **) pppgobT;

					fFound = true;
					break;
				}
			}
		}
		Assert(fFound);
		if (!fFound)
			return;
		gmmgr.WritePtr(m_apgobMaster, (byte *)ppgobUse - (byte *)m_apgobMaster, &pgob, sizeof(Gob *));
		//*ppgobUse = pgob;
	}

	// Assign gid, link it in

	Assert(gid < (m_cpgobMax + 1));

	pgob->m_gid = gid;
	pgob->m_pgobNext = m_pgobHead;
	m_pgobHead = pgob;

	m_cpgobActive++;

	// Add Gob to gidMap

	MoveGob(pgob, -1, -1, pgob->m_wx, pgob->m_wy);

	// Increment gob counts

	TrackGobCounts(pgob, true);

#ifdef INCL_VALIDATEHEAP
	gmmgr.Validate();
#endif
}

void GobMgr::RemoveGob(Gob *pgob)
{
	// Ensure this gob isn't in any area lists still

#if defined(WIN) && !defined(CE) && defined(DEBUG)
	// Validate not already in these areas

	for (int iar = 0; iar < m_car; iar++) {
		Area *par = &m_aar[iar];
		for (word iareT = par->iareHead; iareT != (word)-1; iareT = par->aare[iareT].iareNext)
			Assert(par->aare[iareT].gid != pgob->m_gid);
	}
#endif

	// Mark this gob invalid in the update map so this area gets drawn correctly

	pgob->Invalidate();

	// Remove Gob from gidMap

	MoveGob(pgob, pgob->m_wx, pgob->m_wy, -1, -1);

	// Remove Gob from master Gob list
	// Freed Gob list entries go on the END of the free list

	Gob **ppgob = PpgobFromGid(pgob->m_gid);
	pgob->m_gid = kgidNull;
	Gob *pgobT = NULL;
	gmmgr.WritePtr(m_apgobMaster, (byte *)ppgob - (byte *)m_apgobMaster, &pgobT, sizeof(Gob *));
	//*ppgob = NULL;	// Mark new terminator
	pgobT = MakeFreeEntry(ppgob);
	gmmgr.WritePtr(m_apgobMaster, (byte *)m_ppgobFreeTail - (byte *)m_apgobMaster, &pgobT, sizeof(Gob *));
	//*m_ppgobFreeTail = MakeFreeEntry(ppgob);
	m_ppgobFreeTail = ppgob;

	// UNDONE: OPT: removing Gobs requires a traversal of the Gob list

	for (Gob** ppgobT = &m_pgobHead; *ppgobT != NULL; ppgobT = &(*ppgobT)->m_pgobNext) {
		if (*ppgobT == pgob) {
			*ppgobT = pgob->m_pgobNext;
			pgob->m_pgobNext = NULL;
			break;
		}
	}

	m_cpgobActive--;

	// Decrement gob counts

	TrackGobCounts(pgob, false);

#ifdef INCL_VALIDATEHEAP
	gmmgr.Validate();
#endif
}

void GobMgr::TrackGobCounts(Gob *pgob, bool fIncrement)
{
	if (!(pgob->GetFlags() & kfGobUnit)) {
		int nDir = fIncrement ? 1 : -1;
		switch (pgob->GetType()) {
		case kgtScenery:
			m_cSceneryGobs += nDir;
			break;

		case kgtScorch:
			m_cScorchGobs += nDir;
			break;

		default:
			m_cSupportGobs += nDir;
			break;
		}
	}

	// Validate counts so we know of errors

#ifdef DEBUG
	if (ggame.IsMultiplayer()) {
		for (Side side = ksideNeutral; side < kcSides; side++) {
			Player *pplr = gplrm.GetPlayer(side);
			if (pplr == NULL)
				continue;

			int cStructGobs = pplr->GetUnitInstanceCountFromMask(kumStructures);
			Assert(cStructGobs >= 0 && cStructGobs <= gcStructGobsLimitMP);

			int cMuntGobs = pplr->GetUnitInstanceCountFromMask(kumMobileUnits);
			Assert(cMuntGobs >= 0 && cMuntGobs <= gcMuntGobsLimitMP);
		}
	} else {
		int cMuntGobsComputer = gplrm.GetUnitInstanceCountFromMask(kumMobileUnits, kfPlrComputer);
		Assert(cMuntGobsComputer >= 0 && cMuntGobsComputer <= gcMuntGobsComputerLimitSP);

		int cStructGobsComputer = gplrm.GetUnitInstanceCountFromMask(kumStructures, kfPlrComputer);
		Assert(cStructGobsComputer >= 0 && cStructGobsComputer <= gcStructGobsComputerLimitSP + gcStructGobsComputerDeltaSP);

		int cMuntGobsHuman = gplrm.GetUnitInstanceCountFromMask(kumMobileUnits, 0);
		Assert(cMuntGobsHuman >= 0 && cMuntGobsHuman <= gcMuntGobsHumanLimitSP);

		int cStructGobsHuman = gplrm.GetUnitInstanceCountFromMask(kumStructures, 0);
		Assert(cStructGobsHuman >= 0 && cStructGobsHuman <= gcStructGobsHumanLimitSP + gcStructGobsHumanDeltaSP);

		Assert(m_cSceneryGobs >= 0 && m_cSceneryGobs <= gcSceneryGobsLimit);
		Assert(m_cScorchGobs >= 0 && m_cScorchGobs <= gcScorchGobsLimit);
		Assert(m_cSupportGobs >= 0 && m_cSupportGobs <= gcSupportGobsLimit);
	}
#endif
}

bool GobMgr::IsBelowLimit(int nLimit, Player *pplr)
{
	switch (nLimit) {
	case knLimitStruct:
		{
			Assert(pplr != NULL);
			if (!ggame.IsMultiplayer()) {
				int cQueued = BuilderGob::GetGlobalQueuedCount(pplr, kfUntcStructureBuilder);
				if (pplr->GetFlags() & kfPlrComputer) {
					int cStructGobsComputer = gplrm.GetUnitInstanceCountFromMask(kumStructures, kfPlrComputer);
					return (cStructGobsComputer + cQueued) < gcStructGobsComputerLimitSP + gcStructGobsComputerDeltaSP;
				} else {
					int cStructGobsHuman = gplrm.GetUnitInstanceCountFromMask(kumStructures, 0);
					return (cStructGobsHuman + cQueued) < gcStructGobsHumanLimitSP + gcStructGobsHumanDeltaSP;
				}
			} else {
				int cQueued = BuilderGob::GetQueuedCount(pplr, kfUntcStructureBuilder);
				return (pplr->GetUnitInstanceCountFromMask(kumStructures) + cQueued) < gcStructGobsLimitMP;
			}
		}
		break;

	case knLimitMobileUnit:
		{
			Assert(pplr != NULL);
			if (!ggame.IsMultiplayer()) {
				int cQueued = BuilderGob::GetGlobalQueuedCount(pplr, kfUntcMobileUnitBuilder);
				if (pplr->GetFlags() & kfPlrComputer) {
					int cMuntGobsComputer = gplrm.GetUnitInstanceCountFromMask(kumMobileUnits, kfPlrComputer);
					return (cMuntGobsComputer + cQueued) < gcMuntGobsComputerLimitSP;
				} else {
					int cMuntGobsHuman = gplrm.GetUnitInstanceCountFromMask(kumMobileUnits, 0);
					return (cMuntGobsHuman + cQueued) < gcMuntGobsHumanLimitSP;
				}
			} else {
				int cQueued = BuilderGob::GetQueuedCount(pplr, kfUntcMobileUnitBuilder);
                return (pplr->GetUnitInstanceCountFromMask(kumMobileUnits) + cQueued) < gcMuntGobsLimitMP;
			}
		}
		break;

	case knLimitScenery:
		return m_cSceneryGobs < gcSceneryGobsLimit;

	case knLimitScorch:
		return m_cScorchGobs < gcScorchGobsLimit;

	case knLimitSupport:
		return m_cSupportGobs < gcSupportGobsLimit;
	}

	Assert();
	return false;
}

bool GobMgr::MoveGob(Gob *pgob, WCoord wxOld, WCoord wyOld, WCoord wxNew, WCoord wyNew)
{
	Assert(wxOld >= -1 && TcFromWc(wxOld) < m_ctx && wyOld >= -1 && TcFromWc(wyOld) < m_cty);
	Assert(wxNew >= -1 && TcFromWc(wxNew) < m_ctx && wyNew >= -1 && TcFromWc(wyNew) < m_cty);
	Assert(pgob->m_gid < (m_cpgobMax + 1));

	int igidOld = -1;
	if (wxOld != -1 && wyOld != -1)
		igidOld = IgidFromWXY(wxOld, wyOld);

	int igidNew = -1;
	if (wxNew != -1 && wyNew != -1)
		igidNew = IgidFromWXY(wxNew, wyNew);

	if (igidOld == igidNew)
		return false;

	if (igidOld != -1) {
		// Remove Gob from old location within gidMap

		Gid *pgidT = m_pgidMap + igidOld;
		Assert((*pgidT & kwfGidEndMarker) == 0);
		if (*pgidT == pgob->m_gid) {
			Gid gidT = pgob->m_gidNext;
			gmmgr.WritePtr(m_pgidMap, (byte *)pgidT - (byte *)m_pgidMap, &gidT, sizeof(gidT));
		} else {
			while (((*pgidT) & kwfGidEndMarker) == 0) {
				if (*pgidT == pgob->m_gid) {
					*pgidT = pgob->m_gidNext;
					Assert(pgob->m_gidNext != pgob->m_gid);
					pgob->m_gidNext = kgidNull;
					break;
				}
				pgidT = &(*PpgobFromGid(*pgidT))->m_gidNext;
			}
		}
	}

	if (igidNew != -1) {
		// Add Gob to new location within gidMap

		Gid *pgidT = m_pgidMap + igidNew;
		pgob->m_gidNext = *pgidT;
		Assert(pgob->m_gidNext != pgob->m_gid);
		Gid gidT = pgob->m_gid;
		gmmgr.WritePtr(m_pgidMap, (byte *)pgidT - (byte *)m_pgidMap, &gidT, sizeof(gidT));
		//*pgidT = pgob->m_gid;
	}

#ifdef INCL_VALIDATEHEAP
	gmmgr.Validate();
#endif

	return true;
}

UnitGob *GobMgr::GetUnitGob(TCoord tx, TCoord ty)
{
	// Gets the unit gob at this spot. Checks shadowed gids too.

	Gid gid = m_pgidMap[ty * m_ctx + tx];
	while (true) {
		if (gid == kgidNull)
			return NULL;
		Gob *pgob = GetGob(gid & ~kwfGidEndMarker, false);
		if (pgob == NULL)
			return NULL;
		if (pgob->GetFlags() & kfGobUnit)
			return (UnitGob *)pgob;
		if (gid & kwfGidEndMarker)
			return NULL;
		gid = pgob->m_gidNext;
	}
}

Gob *GobMgr::GetShadowGob(TCoord tx, TCoord ty)
{
	// Get gid shadowed "inside" the end of list marker
	// Don't bother clipping

	Gid gid = m_pgidMap[ty * m_ctx + tx];
	while (true) {
		if (gid == kgidNull)
			return NULL;
		if ((gid & kwfGidEndMarker) != 0)
			return GetGob(gid & ~kwfGidEndMarker, false);
		Gob *pgob = GetGob(gid, false);
		if (pgob == NULL)
			return NULL;
		gid = pgob->m_gidNext;
	}
}

void GobMgr::ShadowGob(Gob *pgob, TCoord tx, TCoord ty, int ctx, int cty)
{
	// Shadow this gid "inside" the end of list marker.

	for (TCoord tyT = ty; tyT < ty + cty; tyT++) {
		for (TCoord txT = tx; txT < tx + ctx; txT++) {
			Gid *pgid = &m_pgidMap[tyT * m_ctx + txT];
			if ((*pgid & kwfGidEndMarker) != 0) {
				Gid gidT = pgob->m_gid | kwfGidEndMarker;
				gmmgr.WritePtr(m_pgidMap, (byte *)pgid - (byte *)m_pgidMap, &gidT, sizeof(gidT));
			} else {
				while ((*pgid & kwfGidEndMarker) == 0) {
					Gob *pgob = *PpgobFromGid(*pgid);
					pgid = &pgob->m_gidNext;
				}
				Assert((*pgid & kwfGidEndMarker) != 0);
				*pgid = pgob->m_gid | kwfGidEndMarker;
			}
		}
	}
}

void GobMgr::UnshadowGob(Gob *pgob, TCoord tx, TCoord ty, int ctx, int cty)
{
	// Unshadow this gob's gid from the end of list marker

	for (TCoord tyT = ty; tyT < ty + cty; tyT++) {
		for (TCoord txT = tx; txT < tx + ctx; txT++) {
			Gid *pgid = &m_pgidMap[tyT * m_ctx + txT];
			if ((*pgid & kwfGidEndMarker) != 0) {
				Gid gidT = kgidNull;
				gmmgr.WritePtr(m_pgidMap, (byte *)pgid - (byte *)m_pgidMap, &gidT, sizeof(gidT));
			} else {
				while ((*pgid & kwfGidEndMarker) == 0) {
					Gob *pgob = *PpgobFromGid(*pgid);
					pgid = &pgob->m_gidNext;
				}
				Assert((*pgid & kwfGidEndMarker) != 0);
				if ((*pgid & ~kwfGidEndMarker) == pgob->m_gid)
					*pgid = kgidNull;
			}
		}
	}
}

AreaMask GobMgr::CalcAreaMask(TCoord tx, TCoord ty, int ctx, int cty)
{
	AreaMask am = 0;
	for (TCoord tyT = ty; tyT < ty + cty; tyT++) {
		for (TCoord txT = tx; txT < tx + ctx; txT++) {
			am |= CalcAreaMask(txT, tyT);
		}
	}
	return am;
}

// Rects could be sorted to speed this up (as if it's a problem)
// Also AreaMask could be stored in UnitGob for a speed boost

AreaMask GobMgr::CalcAreaMask(TCoord tx, TCoord ty)
{
	if (gsim.GetLevel()->GetTerrainMap()->GetTerrainType(tx, ty) != kttArea)
		return 0;
	AreaMask am = 0;
	for (int nArea = 0; nArea < m_car; nArea++) {
		if (m_aar[nArea].trc.PtIn(tx, ty)) {
			am |= (1UL << nArea);
		}
	}
	Assert(am != 0);
	return am;
}

// Only called when there is a AreaMask change (rare).
// I have this in assembly (from pocketchess) if needed.

int GobMgr::GetAreasFromMask(AreaMask am, int *pnArea)
{
	Assert(kcAreasMax <= 32);
	int cArea = 0;
	for (int n = 0; n < kcAreasMax && am != 0; n++) {
		if (am & (1UL << n)) {
			*pnArea++ = n;
			cArea++;
			am &= (am - 1);
		}
	}
	return cArea;
}

void GobMgr::MoveGobBetweenAreas(Gid gid, AreaMask amOld, AreaMask amNew)
{
	if (amOld == amNew)
		return;
	AreaMask amRemove = amOld & ~amNew;
	if (amRemove != 0)
		RemoveGobFromAreas(gid, amRemove);
	AreaMask amAdd = amNew & ~amOld;
	if (amAdd != 0)
		AddGobToAreas(gid, amAdd);
}

#define kcareGrow 32

void GobMgr::AddGobToAreas(Gid gid, AreaMask am)
{
	// Loop through areas

	int aiar[kcAreasMax];
	int car = GetAreasFromMask(am, aiar);

#if defined(WIN) && !defined(CE) && defined(DEBUG)
	// Validate not already in these areas

	for (int iar = 0; iar < car; iar++) {
		Area *par = &m_aar[aiar[iar]];
		for (word iareT = par->iareHead; iareT != (word)-1; iareT = par->aare[iareT].iareNext)
			Assert(par->aare[iareT].gid != gid);
	}
#endif

	// Add to the following areas

	for (int iar = 0; iar < car; iar++) {
		// Add to list. Need more room?

		Area *par = &m_aar[aiar[iar]];
		if (par->iareFree == (word)-1) {
			Assert(par->careAlloc == par->careUsed);
			int careNew = par->careAlloc + kcareGrow;
			AreaEntry *aare = new AreaEntry[careNew];
			if (aare == NULL)
				continue;
			if (par->aare == NULL) {
				par->aare = aare;
			} else {
				memcpy(aare, par->aare, ELEMENTSIZE(aare) * par->careAlloc);
				delete[] par->aare;
				par->aare = aare;
			}

			// Initialize onto free list

			par->iareFree = par->careAlloc;
			for (int iareNew = par->iareFree; iareNew < careNew - 1; iareNew++)
				par->aare[iareNew].iareNext = iareNew + 1;
			par->aare[careNew - 1].iareNext = (word)-1;

			// New size

			par->careAlloc = careNew;
		}

		// Take one from the free list

		int iareNew = par->iareFree;
		Assert(iareNew != (word)-1);
		par->iareFree = par->aare[iareNew].iareNext;

		// Put on used list

		par->aare[iareNew].iareNext = par->iareHead;
		par->iareHead = iareNew;

		// This gob lives here

		par->careUsed++;
		par->aare[iareNew].gid = gid;

		// Update info about gobs in this area

		UnitGob *punt = (UnitGob *)*PpgobFromGid(gid);
		Assert(punt == (UnitGob *)GetGob(gid, false));
		if (punt != NULL && (punt->m_ff & kfGobUnit)) {
			par->sidm |= GetSideMask(punt->GetSide());
			par->um |= punt->GetConsts()->um;
		}
	}
}

void GobMgr::RemoveGobFromAreas(Gid gid, AreaMask am)
{
	// Loop through areas
	
	int aiar[kcAreasMax];
	int car = GetAreasFromMask(am, aiar);

#if defined(WIN) && !defined(CE) && defined(DEBUG)
	// Validate already in these areas

	for (int iar = 0; iar < car; iar++) {
		Area *par = &m_aar[aiar[iar]];
		bool fFound = false;
		for (word iareT = par->iareHead; iareT != (word)-1; iareT = par->aare[iareT].iareNext) {
			if (par->aare[iareT].gid == gid) {
				fFound = true;
				break;
			}
		}
		Assert(fFound);
	}
#endif

	// Remove from the following areas

	for (int iar = 0; iar < car; iar++) {
		// Remove from area list, put entry on free list
		// Recalc side and unit masks into along the way

		Area *par = &m_aar[aiar[iar]];
		par->sidm = 0;
		par->um = 0;

		word *piareNext = &par->iareHead;
		while (*piareNext != (word)-1) {
			AreaEntry *pare = &par->aare[*piareNext];
			if (pare->gid == gid) {
				// Remove this gid; should happen only once

				word iareT = *piareNext;
				*piareNext = pare->iareNext;
				pare->iareNext = par->iareFree;
				par->iareFree = iareT;
				par->careUsed--;
				Assert((short)par->careUsed >= 0);

				// piareNext is still point to last which is what we want
			} else {
				// Update info about gobs in this area

				UnitGob *punt = (UnitGob *)*PpgobFromGid(pare->gid);
				Assert(punt == (UnitGob *)GetGob(pare->gid, false));
				if (punt != NULL && (punt->m_ff & kfGobUnit)) {
					par->sidm |= GetSideMask(punt->GetSide());
					par->um |= punt->GetConsts()->um;
				}

				// Cycle to the next

				piareNext = &pare->iareNext;
			}
		}
	}
}

Gob *GobMgr::EnumGobsInArea(Enum *penm, int nArea, SideMask sidm, UnitMask um)
{
	// First time? Do overall filter match.
    // Note kEnmFirst is -1, the same value used here for the "end of list"
    // marker.

	Area *par = &m_aar[nArea];
	if (penm->m_dwUser == kEnmFirst) {
		if (!(par->sidm & sidm))
			return NULL;
		if (!(par->um & um))
			return NULL;
		penm->m_dwUser = 0;
		penm->m_wUser = par->iareHead;
	}

	while (penm->m_wUser != (word)-1) {
		AreaEntry *pare = &par->aare[penm->m_wUser];
		penm->m_wUser = pare->iareNext;

		// Check against filter

		Gob *pgobT = (Gob *)*PpgobFromGid(pare->gid);
		Assert(pgobT == GetGob(pare->gid, false));
		Assert(pgobT->GetFlags() & kfGobUnit);
		if (!(pgobT->GetFlags() & kfGobUnit))
			continue;
		UnitGob *puntT = (UnitGob *)pgobT;
		if (!(GetSideMask(puntT->GetSide()) & sidm))
			continue;
		if (!(puntT->GetConsts()->um & um))
			continue;

		// Matches filter, return it.

		// NOTE: this doesn't return the NEAREST Gob, just the first
		// found within the area

		return pgobT;
	}

	return NULL;
}

bool GobMgr::CheckUnitsInArea(int nArea, SideMask sidm, UnitMask um)
{
	// Only checks if unit type and given side, not if unit type is of given side

	Assert(nArea >= 0 && nArea < m_car);
	Area *par = &m_aar[nArea];
	if (!(par->sidm & sidm))
		return false;
	if (!(par->um & um))
		return false;
	return true;
}

bool GobMgr::IsGobWithinArea(Gob *pgobTarget, int nArea)
{
	TPoint tpt;
	pgobTarget->GetTilePosition(&tpt);
	Assert(nArea >= 0 && nArea < m_car);
	return m_aar[nArea].trc.PtIn(tpt.tx, tpt.ty);
}

void GobMgr::GetAreaRect(int nArea, TRect *ptrc, Side side)
{
	if (side == ksideNeutral || nArea >= 0) {
		Assert(nArea < m_car);
		*ptrc = m_aar[nArea].trc;
	} else {
		if (nArea == knAreaLastDiscovery) {
			Assert(side != ksideNeutral);
			Player *pplr = gplrm.GetPlayer(side);
			TPoint tpt = pplr->GetDiscoverPoint();
			ptrc->left = tpt.tx;
			ptrc->top = tpt.ty;
			ptrc->right = tpt.tx + 1;
			ptrc->bottom = tpt.ty + 1;
		}
	}
}

bool GobMgr::LoadAreas(IniReader *pini)
{
	// Initialize areas first

	for (int iar = 0; iar < ARRAYSIZE(m_aar); iar++) {
		Area *par = &m_aar[iar];
		memset(par, 0, sizeof(*par));
		par->iareFree = (word)-1;
		par->iareHead = (word)-1;
	}
	m_car = 0;

	// Might as well have the TriggerMgr own Areas as well for now

	char szProp[128];
	FindProp findArea;
	while (pini->FindNextProperty(&findArea, "Areas", szProp, sizeof(szProp))) {
#ifdef DEBUG_HELPERS
		strncpyz(m_aszAreaNames[m_car], szProp, 50);
#endif
		int nLeft, nTop, nWidth, nHeight;
		if (!pini->GetPropertyValue(&findArea, "%d,%d,%d,%d", &nLeft, &nTop, &nWidth, &nHeight))
			return false;
		m_aar[m_car].trc.Set(nLeft, nTop, nLeft + nWidth, nTop + nHeight);
		m_car++;
		Assert(m_car <= kcAreasMax);
	}
	return true;
}

void GobMgr::FreeAreas()
{
	for (int iar = 0; iar < m_car; iar++) {
		delete[] m_aar[iar].aare;
		m_aar[iar].aare = NULL;
	}
	m_car = 0;
}

Gob *GobMgr::GetGob(Gid gid, bool fActiveOnly)
{
    // Also catches kgidNull

	if (gid & kwfGidEndMarker) {
		return NULL;
    }
	Assert(gid < (m_cpgobMax + 1) * sizeof(Gob *));

	Gob *pgob = *PpgobFromGid(gid);
	if (pgob == NULL)
		return NULL;

	// Returns NULL if gid is invalid, i.e., now points to a free entry
	// Free entries have their low bit set, used entries always have it clear.

	if (IsFreeEntry(pgob))
		return NULL;

    // Most callers should consider a Gob as non-existant if it is no longer
    // Active

	if (fActiveOnly) {
		if ((pgob->GetFlags() & kfGobActive) == 0) {
			return NULL;
        }
    }

	return pgob;
}

// Return true if a same-side structure is within the required distance
// (kctMaxDistFromNeighbor) of the passed-in location+structure size. NOTE:
// this routine does not guarantee that there isn't something in the way.
// NOTE: this routine doesn't need to live in GobMgr

const int kctMaxDistFromNeighbor = 3;

bool GobMgr::IsStructurePlacementValid(StructConsts *pstruc, TCoord tx, TCoord ty, Player *pplr)
{
	// No part of the structure can be off an edge of the map

	if (tx < 0 || ty < 0 || tx + pstruc->ctxReserve > m_ctx || ty + pstruc->ctyReserve > m_cty) {
		return false;
    }

    // No part of the structure can be under the inaccessible part of the
    // minimap (when the map is scrolled to the bottom right corner)

    Form *pfrmMiniMap = gpmfrmm->GetFormPtr(kidfMiniMap);
    if (pfrmMiniMap != NULL) {
        Rect rcMiniMap;
        pfrmMiniMap->GetRect(&rcMiniMap);
        TCoord ctxMap, ctyMap;
        ggobm.GetMapSize(&ctxMap, &ctyMap);
        WCoord wxMiniMapLeft = WcFromTc(ctxMap) - WcFromUpc(rcMiniMap.Width());
        WCoord wyMiniMapTop = WcFromTc(ctyMap) - WcFromUpc(rcMiniMap.Height());
        WCoord wxPlacementLeft = WcFromTc(tx);
        WCoord wyPlacementTop = WcFromTc(ty);
        if (wxPlacementLeft > (wxMiniMapLeft - kwcTile) &&
                wyPlacementTop > (wyMiniMapTop - kwcTile)) {
            return false;
        }
    }

	TRect trcSearch;
	trcSearch.Set(tx - kctMaxDistFromNeighbor, ty - kctMaxDistFromNeighbor, 
			tx + pstruc->ctx + kctMaxDistFromNeighbor, ty + pstruc->cty + kctMaxDistFromNeighbor);

	// We let the player put towers a little farther away from neighboring
	// structures -- but not chain them! (see below)

	if (pstruc->um & kumTowers)
		trcSearch.Inflate(1, 1);

	// Don't search off any edge of the map

	if (trcSearch.left < 0)
		trcSearch.left = 0;
	if (trcSearch.right > m_ctx)
		trcSearch.right = m_ctx;
	if (trcSearch.top < 0)
		trcSearch.top = 0;
	if (trcSearch.bottom > m_cty)
		trcSearch.bottom = m_cty;

	for (int tyT = trcSearch.top; tyT < trcSearch.bottom; tyT++) {
		for (int txT = trcSearch.left; txT < trcSearch.right; txT++) {
			Gob *pgob = GetShadowGob(txT, tyT);
			if (pgob == NULL)
				continue;
			if ((pgob->GetFlags() & (kfGobStructure | kfGobActive)) == (kfGobStructure | kfGobActive)) {
				if (pgob->GetOwner() != pplr)
					continue;

				// Special hack to keep players from building long chains of towers

				GobType gt = pgob->GetType();
				if (gt != kgtMachineGunTower && gt != kgtRocketTower) {
					return true;
				}
			}
		}
	}

	return false;
}

struct GobSort { // gs
	Gob *pgob;
	dword key;
};

void SortGobs(Gob **apgob, int cpgob) secGob;

#if 0
// Called by Simulation::Draw to identify and sort the on-screen Gobs
// prior to drawing them.

int GobMgr::FindGobs(const Rect *prcBounds, Gob **apgob, int cpgobMax, byte *pbFogMap)
{
	TCoord txLeft, txRight, tyTop, tyBottom;
	txLeft = TcFromUpc(prcBounds->left);

	// This bounds check here is to avoid exceeding the range of the
	// pixel -> world coord lookup table.

	int xRight = prcBounds->right + gcxTile - 1;
	if (xRight >= kpcMax)
		xRight = kpcMax - 1;
	txRight = TcFromUpc(xRight);
	tyTop = TcFromUpc(prcBounds->top);
	int yBottom = prcBounds->bottom + gcyTile - 1;
	if (yBottom >= kpcMax)
		yBottom = kpcMax - 1;
	tyBottom = TcFromUpc(yBottom);

	// Expand the cells examined to include any that may contain Gobs
	// that intersect the specified bounding rectangle.
	// This carries some assumptions about the maximum size of Gob
	// clippinging rectangle and its relation to its origin.

	// Maximum Gob clipping rectangle is:
	// -3/+2 tiles wide from its origin
	// -3/+1 tiles high from its origin

	txLeft = _max(0, txLeft - 3);
	txRight = _min(m_ctx - 1, txRight + 2);
	tyTop = _max(0, tyTop - 3);
	tyBottom = _min(m_cty - 1, tyBottom + 1);

	int ctx = txRight - txLeft;

	int cpgob = 0;
	Gid *pgidLeft = m_pgidMap + (tyTop * m_ctx) + txLeft;
	for (int ty = tyTop; ty <= tyBottom; ty++, pgidLeft += m_ctx) {

		Gid *pgidRight = pgidLeft + ctx;

		for (Gid *pgidT = pgidLeft; pgidT <= pgidRight; pgidT++) {
			Gid gidT = *pgidT;
			Gid gidNext;

			for (; (gidT & kwfGidEndMarker) == 0; gidT = gidNext) {
				Gob *pgobT = *PpgobFromGid(gidT);
				gidNext = pgobT->m_gidNext;
				dword ffGob = pgobT->GetFlags();

				// UNDONE: OPT: this could be sped up a little. We only need
				// to test the clipping bounds of Gobs found when scanning the
				// expanded edges of the bounds.

				Rect rcClip;
				pgobT->GetClippingBounds(&rcClip);
				if (rcClip.left >= prcBounds->right)
					goto lbNextGob;
				if (rcClip.right <= prcBounds->left)
					goto lbNextGob;
				if (rcClip.top >= prcBounds->bottom)
					goto lbNextGob;
				if (rcClip.bottom <= prcBounds->top)
					goto lbNextGob;

				// Check if totally under fog
				// Fog disappears quickly; check top left right away

				if (pbFogMap != NULL) {
					int txLeft = TcFromPc(rcClip.left);
					if (txLeft < 0)
						txLeft = 0;
					int tyTop = TcFromPc(rcClip.top);
					if (tyTop < 0)
						tyTop = 0;
					byte *pbFogT = pbFogMap + m_ctx * tyTop + txLeft;
					if (IsFogOpaque(*pbFogT)) {
						int xT = rcClip.right + gcxTile - 1;
						if (xT >= kpcMax)
							xT = kpcMax - 1;
						int txRight = TcFromPc(xT);
						if (txRight > m_ctx)
							txRight = m_ctx;
						int yT = rcClip.bottom + gcyTile - 1;
						if (yT >= kpcMax)
							yT = kpcMax - 1;
						int tyBottom = TcFromPc(yT);
						if (tyBottom > m_cty)
							tyBottom = m_cty;
						bool fOpaque = true;
						for (int tyT = tyTop; tyT < tyBottom; tyT++) {
							for (int txT = txLeft; txT < txRight; txT++) {
								word wFog = *pbFogT++;
								if (!IsFogOpaque(wFog)) {
									fOpaque = false;
									break;
								}
							}
							if (!fOpaque)
								break;
							pbFogT += m_ctx - (txRight - txLeft);
						}
						if (fOpaque)
							goto lbNextGob;
					}
				}

				// Remember if this gob was not visible last frame; we'll need this info later

				if (!(ffGob & kfGobVisibleLastFrame)) {
					ffGob |= kfGobTransitioningToVisible;
				} else {
					ffGob &= ~kfGobTransitioningToVisible;
				}

				// Remember that this gob was visible in this frame

				ffGob = (ffGob & ~kfGobIncludeFindVisible) | kfGobVisibleLastFrame;
				pgobT->SetFlags(ffGob);

				// Add gob to list

				apgob[cpgob++] = pgobT;
				if (cpgob == cpgobMax)
					goto lbFull;
				continue;

lbNextGob:
				// Gob not visible, but add to list anyway if asked
				// Clear the "gob was visible this frame" bit since it isn't visible this frame.

				ffGob &= ~kfGobTransitioningToVisible;
				if (ffGob & kfGobIncludeFindVisible) {
					pgobT->SetFlags(ffGob & ~(kfGobIncludeFindVisible | kfGobVisibleLastFrame));
					apgob[cpgob++] = pgobT;
					if (cpgob == cpgobMax)
						goto lbFull;
				} else {
					pgobT->SetFlags(ffGob & ~kfGobVisibleLastFrame);
				}
			}
		}
	}

lbFull:
	SortGobs(apgob, cpgob);

	return cpgob;
}
#else
// Called by Simulation::Draw to identify and sort the on-screen Gobs
// prior to drawing them.

int GobMgr::FindGobs(const Rect *prcBounds, Gob **apgob, int cpgobMax, byte *pbFogMap)
{
	// This bounds check here is to avoid exceeding the range of the
	// pixel -> world coord lookup table.

	TCoord txLeft, txRight, tyTop, tyBottom;
	txLeft = TcFromUpc(prcBounds->left);
	int xRight = prcBounds->right + gcxTile - 1;
	if (xRight >= kpcMax)
		xRight = kpcMax - 1;
	txRight = TcFromUpc(xRight);
	tyTop = TcFromUpc(prcBounds->top);
	int yBottom = prcBounds->bottom + gcyTile - 1;
	if (yBottom >= kpcMax)
		yBottom = kpcMax - 1;
	tyBottom = TcFromUpc(yBottom);

	// Increment sequence visibility counter

	static word s_nSeqLastVisible;
	s_nSeqLastVisible++;

	// Expand by 1. Need to because we need to track gobs transitioning to invisible
	// Also clip to the map

	txLeft = _max(0, txLeft - 1);
	txRight = _min((int)m_ctx, txRight + 1);
	tyTop = _max(0, tyTop - 1);
	tyBottom = _min((int)m_cty, tyBottom + 1);

	int ctx = txRight - txLeft;
	int cty = tyBottom - tyTop;
	int cgidReturn = m_ctx - ctx;

	Gob **ppgobT = apgob;
	Gob **ppgobMax = &apgob[cpgobMax];
	Gid *pgidT = m_pgidMap + (tyTop * m_ctx) + txLeft;

	for (int ctyT = cty; ctyT != 0; ctyT--) {
		for (int ctxT = ctx; ctxT != 0; ctxT--) {
			Gid gidNext;
			for (Gid gidT = *pgidT; gidT != kgidNull; gidT = gidNext) {
				// Structure? If so, also end of list

				Gob *pgobT;
				if ((gidT & kwfGidEndMarker) != 0) {
					// Structure

					pgobT = *PpgobFromGid(gidT & ~kwfGidEndMarker);
					gidNext = kgidNull;
					Assert(pgobT->GetFlags() & kfGobStructure);
				} else {
					// Any gob

					pgobT = *PpgobFromGid(gidT);
					gidNext = pgobT->m_gidNext;
				}
				dword ffGob = pgobT->GetFlags();

				// If it's a structure make sure we're visiting it once

				if (ffGob & kfGobStructure) {
					StructGob *pstru = (StructGob *)pgobT;
					if (pstru->m_nSeqLastVisible == s_nSeqLastVisible)
						break;
					pstru->m_nSeqLastVisible = s_nSeqLastVisible;
				}

				// If it's an edge gob, check for visibility

				if (ctxT == ctx || ctxT == 1 || ctyT == cty || ctyT == 1) {
					Rect rcClip;
					pgobT->GetClippingBounds(&rcClip);
					if (rcClip.left >= prcBounds->right)
						goto lbNextGob;
					if (rcClip.right <= prcBounds->left)
						goto lbNextGob;
					if (rcClip.top >= prcBounds->bottom)
						goto lbNextGob;
					if (rcClip.bottom <= prcBounds->top)
						goto lbNextGob;
				}

				// Remember if this gob was not visible last frame; we'll need this info later

				if (!(ffGob & kfGobVisibleLastFrame)) {
					ffGob |= kfGobTransitioningToVisible;
				} else {
					ffGob &= ~kfGobTransitioningToVisible;
				}

				// Remember that this gob was visible in this frame

				ffGob = (ffGob & ~kfGobIncludeFindVisible) | kfGobVisibleLastFrame;
				pgobT->SetFlags(ffGob);

				// Add gob to list

				*ppgobT++ = pgobT;
				if (ppgobT == ppgobMax)
					goto lbFull;
				continue;

lbNextGob:
				// Gob not visible, but add to list anyway if asked
				// Clear the "gob was visible this frame" bit since it isn't visible this frame.

				ffGob &= ~kfGobTransitioningToVisible;
				if (ffGob & kfGobIncludeFindVisible) {
					pgobT->SetFlags(ffGob & ~(kfGobIncludeFindVisible | kfGobVisibleLastFrame));
					*ppgobT++ = pgobT;
					if (ppgobT == ppgobMax)
						goto lbFull;
				} else {
					pgobT->SetFlags(ffGob & ~kfGobVisibleLastFrame);
				}
			}

			// Next gid

			pgidT++;
		}

		// Next row

		pgidT += cgidReturn;
	}

lbFull:
	int cpgob = (int)(ppgobT - apgob);
	SortGobs(apgob, cpgob);
	return cpgob;
}
#endif

void SortGobs(Gob **apgob, int cpgob)
{
	if (cpgob <= 1)
		return;

	// Prep for sorting

#if 1
	GobSort *ags = (GobSort *)gpbScratch;
#else
	GobSort ags[kcpgobMax / 4];
#endif
	GobSort *pgsT = ags;
	Gob **ppgobT = apgob;
	int i;
	for (i = 0; i < cpgob; i++, pgsT++) {
		pgsT->pgob = *ppgobT++;
		pgsT->key = pgsT->pgob->GetSortKey();
	}

	// Sort (insertion sort)

	GobSort *pgs;
	GobSort *pgsEnd = ags + cpgob;
	for (pgs = ags + 1; pgs < pgsEnd; pgs++) {
		for (GobSort *pgsT = pgs; pgsT > ags && (pgsT - 1)->key > pgsT->key; pgsT--) {
			GobSort gsT = *pgsT;
			*pgsT = *(pgsT - 1);
			*(pgsT - 1) = gsT;
		}
	}

	// Copy sorted results to passed-in buffer

	pgsT = ags;
	ppgobT = apgob;
	for (i = 0; i < cpgob; i++, pgsT++)
		*ppgobT++ = pgsT->pgob;
}

#if 0
// For reference:

/* qsort -- qsort interface implemented by faster quicksort.
   J. L. Bentley and M. D. McIlroy, SPE 23 (1993) 1249-1265.
   Copyright 1993, John Wiley.
*/

    /*assume sizeof(long) is a power of 2 */
#define SWAPINIT(a, es) swaptype =         \
    (a-(char*)0 | es) % sizeof(long) ? 2 : es > sizeof(long);
#define swapcode(TYPE, parmi, parmj, n) {  \
    register TYPE *pi = (TYPE *) (parmi);  \
    register TYPE *pj = (TYPE *) (parmj);  \
    do {                                   \
        register TYPE t = *pi;             \
        *pi++ = *pj;                       \
        *pj++ = t;                         \
    } while ((n -= sizeof(TYPE)) > 0);     \
}
#include <stddef.h>
static void swapfunc(char *a, char *b, size_t n, int swaptype)
{   if (swaptype <= 1) swapcode(long, a, b, n)
    else swapcode(char, a, b, n)
}
#define swap(a, b)                         \
    if (swaptype == 0) {                   \
        t = *(long*)(a);                   \
        *(long*)(a) = *(long*)(b);         \
        *(long*)(b) = t;                   \
    } else                                 \
        swapfunc(a, b, es, swaptype)

#define PVINIT(pv, pm)                                \
    if (swaptype != 0) { pv = a; swap(pv, pm); }      \
    else { pv = (char*)&v; *(long*)pv = *(long*)pm; }

#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)

static char *med3(char *a, char *b, char *c, int (*cmp)(const void *pElement1, const void *pElement2))
{	return cmp(a, b) < 0 ?
		  (cmp(b, c) < 0 ? b : cmp(a, c) < 0 ? c : a)
		: (cmp(b, c) > 0 ? b : cmp(a, c) > 0 ? c : a);
}

void qsort(char *a, size_t n, size_t es, int (*cmp)(const void *pElement1, const void *pElement2))
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
	int r, swaptype;
	long t, v;
	size_t s;

	SWAPINIT(a, es);
	if (n < 7) {	 /* Insertion sort on smallest arrays */
		for (pm = a + es; pm < a + n*es; pm += es)
			for (pl = pm; pl > a && cmp(pl-es, pl) > 0; pl -= es)
				swap(pl, pl-es);
		return;
	}
	pm = a + (n/2)*es;    /* Small arrays, middle element */
	if (n > 7) {
		pl = a;
		pn = a + (n-1)*es;
		if (n > 40) {    /* Big arrays, pseudomedian of 9 */
			s = (n/8)*es;
			pl = med3(pl, pl+s, pl+2*s, cmp);
			pm = med3(pm-s, pm, pm+s, cmp);
			pn = med3(pn-2*s, pn-s, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp); /* Mid-size, med of 3 */
	}
	PVINIT(pv, pm);       /* pv points to partition value */
	pa = pb = a;
	pc = pd = a + (n-1)*es;
	for (;;) {
		while (pb <= pc && (r = cmp(pb, pv)) <= 0) {
			if (r == 0) { swap(pa, pb); pa += es; }
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, pv)) >= 0) {
			if (r == 0) { swap(pc, pd); pd -= es; }
			pc -= es;
		}
		if (pb > pc) break;
		swap(pb, pc);
		pb += es;
		pc -= es;
	}
	pn = a + n*es;
	s = _min(pa-a,  pb-pa   ); vecswap(a,  pb-s, s);
	s = _min(pd-pc, pn-pd-es); vecswap(pb, pn-s, s);
	if ((s = pb-pa) > es) qsort(a,    s/es, es, cmp);
	if ((s = pd-pc) > es) qsort(pn-s, s/es, es, cmp);
}
#endif

const int kcpgobInRangeMax = 100;

// Called by GobMgr::FindEnemyWithinRange to locate Gobs within a certain tile radius
// of a Gob looking for a fight.

int GobMgr::FindGobs(const TRect *ptrcBounds, Gob **apgob, int cpgobMax)
{
	TCoord txLeft, txRight, tyTop, tyBottom;

	txLeft = ptrcBounds->left;
	txRight = ptrcBounds->right;
	tyTop = ptrcBounds->top;
	tyBottom = ptrcBounds->bottom;

	// Expand the cells examined to include any that may contain Gobs
	// that intersect the specified bounding rectangle.
	// This carries some assumptions about the maximum size of Gob
	// clippinging rectangle and its relation to its origin.

	// Maximum Gob clipping rectangle is:
	// -2/+2 tiles wide from its origin
	// -3/+1 tiles high from its origin

	txLeft = _max(0, txLeft - 2);
	txRight = _min(m_ctx - 1, txRight + 2);
	tyTop = _max(0, tyTop - 3);
	tyBottom = _min(m_cty - 1, tyBottom + 1);

	int ctx = txRight - txLeft;

	int cpgob = 0;
	Gid *pgidLeft = m_pgidMap + (tyTop * m_ctx) + txLeft;
	for (int ty = tyTop; ty <= tyBottom; ty++, pgidLeft += m_ctx) {

		Gid *pgidRight = pgidLeft + ctx;

		for (Gid *pgidT = pgidLeft; pgidT <= pgidRight; pgidT++) {
			Gob *pgobT;
			for (Gid gidT = *pgidT; (gidT & kwfGidEndMarker) == 0; gidT = pgobT->m_gidNext) {
				pgobT = *PpgobFromGid(gidT);

				// Callers only care about active Gobs

				if ((pgobT->GetFlags() & kfGobActive) == 0)
					continue;

				TRect trcClip;
				pgobT->GetTileRect(&trcClip);
				if (trcClip.left >= ptrcBounds->right)
					continue;
				if (trcClip.right <= ptrcBounds->left)
					continue;
				if (trcClip.top >= ptrcBounds->bottom)
					continue;
				if (trcClip.bottom <= ptrcBounds->top)
					continue;

				apgob[cpgob++] = pgobT;
				if (cpgob == cpgobMax)
					goto lbFull;
			}
		}
	}

lbFull:
	return cpgob;
}

#if defined(DEBUG) && defined(WIN)
Gob *GobMgr::FindEnemyWithinRange(UnitGob *punt, TCoord tcRange, bool fStructures)
{
	TRect trc;
	punt->GetTileRect(&trc);
	trc.left -= tcRange;
	trc.right += tcRange;
	trc.top -= tcRange;
	trc.bottom += tcRange;

	Gob *apgob[kcpgobInRangeMax];
	int cpgob = FindGobs(&trc, apgob, sizeof(apgob));
	Assert(cpgob <= kcpgobInRangeMax);

	if (cpgob == 0)
		return NULL;

	Gob **ppgobT = apgob;
	for (int i = 0; i < cpgob; i++, ppgobT++) {
		Gob *pgobT = *ppgobT;

		// Don't fire on non-military targets or allies

		if (!punt->IsValidTarget(pgobT))
			continue;

		// or enemy structures

		if (!fStructures && (pgobT->GetFlags() & kfGobStructure))
			continue;

		// NOTE: this doesn't return the NEAREST Gob, just the first
		// found within range

		return pgobT;
	}

	return NULL;
}
#endif

} // namespace wi
