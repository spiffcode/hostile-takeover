//btwcelib.h

#pragma once
#ifdef WC13
#pragma comment(lib, "sdkce.lib")

#include "BtIfClasses13.h"

#else

#pragma comment(lib, "btsdkce30.lib")

#include "BtIfDefinitions.h"
#include "BtIfClasses.h"
#include "BtIfObexHeaders.h"
#endif

