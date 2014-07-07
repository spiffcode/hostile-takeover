#include "../ht.h"
#include "xtransport.h"

namespace wi {
    
TransportMgr gtram;

//---------------------------------------------------------------------------
// TransportMgr implementation

int TransportMgr::GetTransportDescriptions(TransportDescription *atrad, int ctradMax)
{
	return XTransport::GetTransportDescriptions(atrad, ctradMax);
}

} // namespace wi