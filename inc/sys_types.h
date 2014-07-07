/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: sys_types.h
 *
 * Release: eclipse 5 SDK (68K) R3.
 *
 * Description:
 *	  This module contains the Pilot equivalent of the "standard" unix
 * header file <sys/types.h>
 *
 *****************************************************************************/

#ifndef __UNIX_SYS_TYPES_H__
#define __UNIX_SYS_TYPES_H__

#include	<NetMgr.h>						// Include Pilot Net Manager equates

// Common Unix types
typedef	UInt8		u_char;
typedef	UInt16	u_short;
typedef	UInt16	u_int;
typedef	UInt32	u_long;

typedef	UInt16	ushort;				/* System V compatibility */
typedef	UInt16	uint;					/* System V compatibility */

// Define this so we don't get duplicate define warnings. 
#define	__size_t__
#ifndef PNO // DWM: added to eliminate conflict when building x86 PNO
typedef	UInt32 	size_t;				// must be signed for some unix apps 
#endif

// File Descriptor Utilities, Pilot native forms in <NetMgr.h>
typedef	NetFDSetType	fd_set;
#define	FD_SETSIZE		netFDSetSize

#define	FD_SET(n,p)		netFDSet(n,p)
#define	FD_CLR(n,p)		netFDClr(n,p)
#define	FD_ISSET(n,p)	netFDIsSet(n,p)
#define	FD_ZERO(p)		netFDZero(p)


// Basic functions
#define	bcopy(b1,b2,len)		\
				MemMove(b2,b1,len)

#define	bzero(b,len)		\
				MemSet(b,len,0)
				
#define	bcmp(b1,b2,len) 	\
				MemCmp(b1,b2,len)
				
				

#endif  /* __UNIX_SYS_TYPES_H__ */
