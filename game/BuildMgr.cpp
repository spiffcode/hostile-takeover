#include "ht.h"

namespace wi {

// UNDONE: load/save

BuildMgr::BuildMgr()
{
	m_pbldeFirst = NULL;
}

BuildMgr::~BuildMgr()
{
	BuildEntry *pblde = m_pbldeFirst;
	while (pblde != NULL) {
		BuildEntry *pbldeT = pblde;
		pblde = pblde->pbldeNext;
		delete pbldeT;
	}
}

// Add the requested UnitType to the build list. The specified UnitGroup
// will be notified when the Unit is built.

bool BuildMgr::BuildUnit(UnitType ut, UnitGroup *pug, int nArea)
{
	BuildEntry *pblde = new BuildEntry;
	Assert(pblde != NULL, "out of memory!");
	if (pblde == NULL)
		return false;

	pblde->ut = ut;
	pblde->pug = pug;
	pblde->nArea = nArea;
	pblde->gidBuilder = kgidNull;

	// Miners have special priority and jump to the head of list.
	// This is to reduce the likelihood that level AI will run out
	// of money because the player destroyed its Miners

	if (pblde->ut == kutGalaxMiner) {
		pblde->pbldeNext = m_pbldeFirst;
		m_pbldeFirst = pblde;
		return true;
	}

	// Link the new BuildEntry to the end of the build list so it will be built
	// chronologically after previous requests. (Not strictly though, because it
	// may be possible to start building this Unit sooner than a Unit earlier on
	// the list because that Unit relies on a Builder type that isn't available).

	pblde->pbldeNext = NULL;
	BuildEntry **ppblde = &m_pbldeFirst;
	while (*ppblde != NULL)
		ppblde = &((*ppblde)->pbldeNext);
	*ppblde = pblde;

	return true;
}

// BuilderGobs call this method when they have completed building a Unit

void BuildMgr::OnBuilt(UnitGob *punt, BuilderGob *pbldr) 
{
	UnitType utBuilt = punt->GetConsts()->ut;
	Gid gidBuilder = pbldr->GetId();
	Player *pplrBuilt = punt->GetOwner();

	BuildEntry **ppblde = &m_pbldeFirst;
	while (*ppblde != NULL) {
		BuildEntry *pblde = *ppblde;
		
		// Match the built Unit with the right BuildEntry (must be of correct 
		// UnitType, produced by the expected BuilderGob which must be owned 
		// by the UnitGroup's owner)

		if (utBuilt == pblde->ut && gidBuilder == pblde->gidBuilder && pplrBuilt == pblde->pug->GetOwner()) {

			// Unlink BuildEntry

			*ppblde = pblde->pbldeNext;

			// Yay, it's been built!

			pblde->pug->OnBuilt(punt);

			delete pblde;
			return;
		}
			
		ppblde = &((*ppblde)->pbldeNext);
	}
}

// OPT: Doesn't need to be called every Simulation::Update

void BuildMgr::Update()
{
	if (m_pbldeFirst == NULL)
		return;

	for (BuildEntry *pblde = m_pbldeFirst; pblde != NULL; pblde = pblde->pbldeNext) {

		// if gidBuilder is invalid/inactive or is no longer owned by the same side
		//     clear gidBuilder

		BuilderGob *pbldr = (BuilderGob *)ggobm.GetGob(pblde->gidBuilder);
		if (pbldr == NULL || pbldr->GetOwner() != pblde->pug->GetOwner())
			pblde->gidBuilder = kgidNull;
	}

	// Check each BuilderGob to see if it can build one of the BuildEntries
	// (it must be idle, owned by the same side, and be the right type of builder)

	// Scan the Gob list looking for BuilderGobs

	Gob *pgob = ggobm.GetFirstGob();
	for (; pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {

		// Is this a Structure?

		if (!(pgob->GetFlags() & kfGobStructure))
			continue;	// no

		StructGob *pstru = (StructGob *)pgob;
		StructConsts *pstruc = (StructConsts *)pstru->GetConsts();

		// Is it a Builder?

		if (!(pstruc->um & kumBuilder))
			continue;	// no

		// Is this builder busy?

		BuilderGob *pbldr = (BuilderGob *)pstru;
		if (pbldr->IsBuildInProgress())
			continue;	// yes

		// Let's see if it can build any of the UnitTypes we have pending

		Player *pplrBuilderOwner = pbldr->GetOwner();
		BuilderConsts *pbldrc = (BuilderConsts *)pbldr->GetConsts();

		for (BuildEntry *pblde = m_pbldeFirst; pblde != NULL; pblde = pblde->pbldeNext) {

			// Is this entry already being built?

			if (pblde->gidBuilder != kgidNull)
				continue;	// yes

			// Does the Player wanting to build own the builder?

			if (pblde->pug->GetOwner() != pplrBuilderOwner)
				continue;	// no

			// Can this builder build the type we want?

			UnitMask um = (1UL << pblde->ut);
			if (pbldrc->umCanBuild & um) {
				// Yes!

				if (um & kumStructures) {
					// Check to see if limits have been reached

					if (!ggobm.IsBelowLimit(knLimitStruct, pplrBuilderOwner))
						continue;	// limit reached

					// Ok

					Message msg;
					memset(&msg, 0, sizeof(msg));
					msg.mid = kmidBuildOtherCommand;
					msg.smidSender = ksmidNull;
					msg.smidReceiver = pbldr->GetId();
					msg.BuildOtherCommand.ut = pblde->ut;
					TRect trc;
					ggobm.GetAreaRect(pblde->nArea, &trc);

					// Is space required by the structure free?

					StructConsts *pstruc = (StructConsts *)gapuntc[pblde->ut];
					if (gsim.GetLevel()->GetTerrainMap()->IsOccupied(trc.left, trc.top, pstruc->ctxReserve, pstruc->ctyReserve, kbfStructure | kbfMobileUnit))
						continue; // no

					msg.BuildOtherCommand.wpt.wx = WcFromTc(trc.left);
					msg.BuildOtherCommand.wpt.wy = WcFromTc(trc.top);
					gsmm.SendMsg(&msg);

				} else {
					// Check to see if limits have been reached

					if (!ggobm.IsBelowLimit(knLimitMobileUnit, pplrBuilderOwner))
						continue;	// limit reached

					pbldr->Build(pblde->ut);
				}
				pblde->gidBuilder = pbldr->GetId();
				break;		// Move on to the next BuilderGob for any remaining BuildEntries
			}
		}
	}
}

} // namespace wi