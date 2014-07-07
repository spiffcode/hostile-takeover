#include "ht.h"

namespace wi {

//
// ReactorGob implementation
//

static StructConsts gConsts;

//
// Gob methods
//

bool ReactorGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtReactor;
	gConsts.ut = kutReactor;

	// Sound effects

	gConsts.sfxAbortRepair = ksfxReactorAbortRepair;
	gConsts.sfxRepair = ksfxReactorRepair;
	gConsts.sfxDamaged = ksfxReactorDamaged;
	gConsts.sfxSelect = ksfxReactorSelect;
	gConsts.sfxDestroyed = ksfxReactorDestroyed;
	gConsts.sfxImpact = ksfxNothing;

	return StructGob::InitClass(&gConsts, pini);
}

void ReactorGob::ExitClass()
{
	StructGob::ExitClass(&gConsts);
}

ReactorGob::ReactorGob() : StructGob(&gConsts)
{
}

//
// StateMachine methods
//

#if defined(DEBUG_HELPERS)
char *ReactorGob::GetName()
{
	return "Reactor";
}
#endif

} // namespace wi